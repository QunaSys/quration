#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "qret/algorithm/data/qrom.h"
#include "qret/base/type.h"
#include "qret/frontend/builder.h"
#include "qret/frontend/circuit_generator.h"
#include "qret/ir/context.h"
#include "qret/ir/json.h"
#include "qret/transforms/ipo/inliner.h"

using qret::BigInt;

struct QROMParams {
    std::uint64_t address_size;
    std::uint64_t value_size;
    std::string multiplier;
    std::string offset;
};

void from_json(const nlohmann::json& j, QROMParams& p) {
    p.address_size = j.at("address_size").get<std::uint64_t>();
    p.value_size = j.at("value_size").get<std::uint64_t>();
    p.multiplier = j.at("multiplier").get<std::string>();
    p.offset = j.at("offset").get<std::string>();
}

QROMParams LoadQROMJson(const std::string& path) {
    auto ifs = std::ifstream(path.data());
    if (!ifs.good()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    nlohmann::json j;
    ifs >> j;
    return j.get<QROMParams>();
};

std::vector<BigInt> MakeQROMDict(const QROMParams& params) {
    const auto max_address = std::size_t{1} << params.address_size;
    const auto max_value = BigInt{1} << params.value_size;
    const auto multiplier = BigInt(params.multiplier);
    const auto offset = BigInt(params.offset);
    const auto get_value = [max_value, multiplier, offset](const auto x) -> BigInt {
        return (x * multiplier + offset) % max_value;
    };
    auto dict = std::vector<BigInt>();
    dict.reserve(max_address);
    for (auto address_value = std::size_t{0}; address_value < max_address; ++address_value) {
        dict.emplace_back(get_value(address_value));
    }
    return dict;
}

struct UncomputeQROMGen : public qret::frontend::CircuitGenerator {
    const std::vector<BigInt>& dict;
    std::uint64_t address_size;
    std::uint64_t value_size;

    static inline const char* Name = "UncomputeQROM";
    explicit UncomputeQROMGen(
            qret::frontend::CircuitBuilder* builder,
            std::uint64_t address_size,
            std::uint64_t value_size,
            const std::vector<BigInt>& dict
    )
        : CircuitGenerator(builder)
        , address_size{address_size}
        , value_size{value_size}
        , dict{dict} {}
    std::string GetName() const override {
        return Name;
    }
    // std::string GetCacheKey() const override;
    void SetArgument(Argument& arg) const override {
        arg.Add("address", Type::Qubit, address_size, Attribute::Operate);
        arg.Add("value", Type::Qubit, value_size, Attribute::Operate);
        arg.Add("aux", Type::Qubit, address_size - 1, Attribute::CleanAncilla);
    }
    qret::frontend::Circuit* Generate() const override {
        BeginCircuitDefinition();
        auto address = GetQubits(0);
        auto value = GetQubits(1);
        auto aux = GetQubits(2);

        qret::frontend::gate::QROMImm(address, value, aux, dict);
        qret::frontend::gate::UncomputeQROMImm(address, value, aux, dict);

        return EndCircuitDefinition();
    }
};

int main(std::int32_t argc, const char* const* const argv) {
    namespace po = boost::program_options;
    po::options_description desc(
            "Create UncomputeQROM circuit from JSON file\n\n"
            "The i-th element is computed as (i * multiplier + offset) mod 2^value_size.\n\n"
            "Input JSON fields:\n"
            "  address_size: Number of address qubits.\n"
            "  value_size: Number of value qubits.\n"
            "  multiplier: Linear generator multiplier for dictionary values, as a decimal string.\n"
            "  offset: Linear generator offset for dictionary values, as a decimal string."
    );
    desc.add_options()
        ("help", "Print usage instructions")
        ("input", po::value<std::string>()->required(), "Input JSON file")
        ("output", po::value<std::string>()->required(), "Path to the output file")
        ("inline", "Option to enable inline expansion");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help") > 0) {
            std::cout << desc << std::endl;
            return 0;
        }
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    std::string input_file;
    if (vm.count("input") > 0) {
        input_file = vm["input"].as<std::string>();
    }
    const auto params = LoadQROMJson(input_file);
    const auto dict = MakeQROMDict(params);

    qret::ir::IRContext context;
    auto* module = qret::ir::Module::Create("UncomputeQROMModule", context);
    auto builder = qret::frontend::CircuitBuilder(module);
    auto gen = UncomputeQROMGen(&builder, params.address_size, params.value_size, dict);
    auto* circuit = gen.Generate();
    auto* ir_circuit = circuit->GetIR();

    // Inline expansion
    if (vm.count("inline") > 0) {
        qret::ir::RecursiveInlinerPass().RunOnFunction(*ir_circuit);
    }

    std::string output_file;
    if (vm.count("output") > 0) {
        output_file = vm["output"].as<std::string>();
    }
    std::ofstream ofs(output_file);
    if (!ofs) {
        std::cerr << "Failed to open output file: " << output_file << std::endl;
        return 1;
    }
    auto module_json = qret::Json(*module);
    ofs << module_json;
    ofs.close();

    return 0;
}
