#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "qret/algorithm/arithmetic/modular.h"
#include "qret/base/type.h"
#include "qret/frontend/builder.h"
#include "qret/ir/context.h"
#include "qret/ir/json.h"
#include "qret/transforms/ipo/inliner.h"

struct MultiControlledModBiMulImmParams {
    std::string modulus;
    std::string multiplier;
    std::size_t num_control_qubits;
    std::size_t num_system_qubits;
};

void from_json(const nlohmann::json& j, MultiControlledModBiMulImmParams& p) {
    p.modulus = j.at("modulus").get<std::string>();
    p.multiplier = j.at("multiplier").get<std::string>();
    p.num_control_qubits = j.at("num_control_qubits").get<std::size_t>();
    p.num_system_qubits = j.at("num_system_qubits").get<std::size_t>();
}

MultiControlledModBiMulImmParams LoadMultiControlledModBiMulImmJson(const std::string& path) {
    auto ifs = std::ifstream(path.data());
    if (!ifs.good()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    nlohmann::json j;
    ifs >> j;
    return j.get<MultiControlledModBiMulImmParams>();
};

int main(std::int32_t argc, const char* const* const argv) {
    namespace po = boost::program_options;
    po::options_description desc(
            "Create MultiControlledModBiMulImm circuit from JSON file\n\n"
            "Input JSON fields:\n"
            "  modulus: Modulus used for modular multiplication, as a decimal string.\n"
            "  multiplier: Immediate multiplier embedded in the circuit, as a decimal string.\n"
            "  num_control_qubits: Number of control qubits.\n"
            "  num_system_qubits: Number of qubits in each modular value register."
    );
    desc.add_options()
        ("help", "Print usage instructions")
        ("input", po::value<std::string>()->required(), "Input JSON file")
        ("output", po::value<std::string>()->required(), "Path to the output file")
        ("inline", "Option to enable inline expansion.");

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
    const auto params = LoadMultiControlledModBiMulImmJson(input_file);
    const auto modulus = qret::BigInt(params.modulus);
    const auto multiplier = qret::BigInt(params.multiplier);

    qret::ir::IRContext context;
    auto* module = qret::ir::Module::Create("MultiControlledModBiMulImmModule", context);
    auto builder = qret::frontend::CircuitBuilder(module);
    auto gen = qret::frontend::gate::MultiControlledModBiMulImmGen(
            &builder,
            modulus,
            multiplier,
            params.num_control_qubits,
            params.num_system_qubits
    );
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
