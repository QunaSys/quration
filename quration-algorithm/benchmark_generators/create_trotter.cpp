#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "qret/algorithm/phase_estimation/trotter.h"
#include "qret/frontend/builder.h"
#include "qret/ir/context.h"
#include "qret/ir/json.h"
#include "qret/transforms/ipo/inliner.h"

struct TrotterParams {
    qret::frontend::gate::Hamiltonian hamiltonian;
    double time;
    std::size_t num_trotter_steps;
};

void from_json(const nlohmann::json& j, TrotterParams& p) {
    p.hamiltonian = j.get<qret::frontend::gate::Hamiltonian>();
    p.time = j.at("time").get<double>();
    p.num_trotter_steps = j.at("num_trotter_steps").get<std::size_t>();
}

TrotterParams LoadTrotterJson(const std::string& path) {
    auto ifs = std::ifstream(path.data());
    if (!ifs.good()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    nlohmann::json j;
    ifs >> j;
    return j.get<TrotterParams>();
};

int main(std::int32_t argc, const char* const* const argv) {
    namespace po = boost::program_options;
    po::options_description desc(
            "Create a time-evolution circuit from a given Hamiltonian using Trotter expansion\n\n"
            "Input JSON fields:\n"
            "  time: Total time for Hamiltonian time evolution.\n"
            "  num_trotter_steps: Number of Trotter steps.\n"
            "  num_qubits: Number of qubits in the Hamiltonian.\n"
            "  pauli_terms: Terms of the Hamiltonian, each of which consists of coeff and dict from index to Pauli operator.\n"
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
    const auto params = LoadTrotterJson(input_file);

    qret::ir::IRContext context;
    auto* module = qret::ir::Module::Create("TrotterModule", context);
    auto builder = qret::frontend::CircuitBuilder(module);
    auto gen = qret::frontend::gate::TrotterGen(
            &builder,
            params.hamiltonian,
            params.num_trotter_steps,
            params.time
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
