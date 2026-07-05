#include <gtest/gtest.h>

#include <fmt/format.h>

#include <cstdint>
#include <string>

#include "qret/base/log.h"
#include "qret/base/string.h"
#include "qret/codegen/machine_function.h"
#include "qret/target/sc_ls_fixed_v0/calc_compile_info.h"
#include "qret/target/sc_ls_fixed_v0/compile_info.h"
#include "qret/target/sc_ls_fixed_v0/lowering.h"
#include "qret/target/sc_ls_fixed_v0/mapping.h"
#include "qret/target/sc_ls_fixed_v0/routing.h"
#include "qret/target/sc_ls_fixed_v0/sc_ls_fixed_v0_target_machine.h"
#include "qret/target/sc_ls_fixed_v0/topology.h"
#include "qret/transforms/ipo/inliner.h"
#include "qret/transforms/scalar/decomposition.h"
#include "qret/transforms/scalar/ignore_global_phase.h"
#include "qret/transforms/scalar/static_condition_pruning.h"

#include "test_utils.h"

using namespace qret;
using namespace qret::sc_ls_fixed_v0;

namespace {
void PrepareCircuit(ir::Function& circuit) {
    ir::RecursiveInlinerPass().RunOnFunction(circuit);
    ir::StaticConditionPruningPass().RunOnFunction(circuit);
    ir::DecomposeInst().RunOnFunction(circuit);
    ir::IgnoreGlobalPhase().RunOnFunction(circuit);
}

ScLsFixedV0TargetMachine CreateTargetMachine(
        const std::string& topology_path = "quration-core/examples/data/topology/tutorial.yaml"
) {
    auto topology = Topology::FromYAML(LoadFile(topology_path));
    return ScLsFixedV0TargetMachine(
            topology,
            ScLsFixedV0MachineOption{
                    .topology_type = GetTopologyType(*topology),
                    .magic_generation_period = 15,
                    .magic_generation_maximum_stock = 10000,
                    .entanglement_generation_period = 100,
                    .entanglement_generation_maximum_stock = 10,
                    .reaction_time = 1,
            }
    );
}

void PrepareRoutedMachineFunction(
        MachineFunction& mf,
        ir::IRContext& context,
        const std::string& circuit_path,
        const std::string& function_name
) {
    auto* circuit = tests::LoadCircuitFromJsonFile(circuit_path, function_name, context);
    if (circuit == nullptr) {
        ADD_FAILURE() << fmt::format("Circuit not found: {}", function_name);
        return;
    }
    PrepareCircuit(*circuit);

    mf.SetIR(circuit);
    Lowering().RunOnMachineFunction(mf);
    Mapping().RunOnMachineFunction(mf);
    Routing().RunOnMachineFunction(mf);
}

ScLsFixedV0CompileInfo CompileAndCalculateInfo(
        const std::string& circuit_path,
        const std::string& function_name,
        const std::string& topology_path = "quration-core/examples/data/topology/tutorial.yaml"
) {
    const auto target = CreateTargetMachine(topology_path);
    auto mf = MachineFunction(&target);
    auto context = ir::IRContext();
    PrepareRoutedMachineFunction(mf, context, circuit_path, function_name);

    CompileInfoWithoutTopology().RunOnMachineFunction(mf);
    CompileInfoWithTopology().RunOnMachineFunction(mf);

    return *static_cast<const ScLsFixedV0CompileInfo*>(mf.GetCompileInfo());
}
}  // namespace

TEST(CompileInfo, RuntimeWithoutTopologyDoesNotExceedRoutedRuntimeForAddCraig) {
    const auto info = CompileAndCalculateInfo(
            "quration-core/tests/data/circuit/add_craig_5.json",
            "AddCraig(5)"
    );

    EXPECT_LE(info.execution_time_without_topology, info.execution_time);
}

TEST(CompileInfo, WithTopologyDoesNotClampRuntimeWithoutTopology) {
    const auto target = CreateTargetMachine();
    auto mf = MachineFunction(&target);
    auto context = ir::IRContext();
    PrepareRoutedMachineFunction(
            mf,
            context,
            "quration-core/tests/data/circuit/add_craig_5.json",
            "AddCraig(5)"
    );

    InitCompileInfo().RunOnMachineFunction(mf);
    auto& info = *static_cast<ScLsFixedV0CompileInfo*>(mf.GetMutCompileInfo());
    constexpr auto SentinelRuntime = std::uint64_t{1000000};
    info.execution_time_without_topology = SentinelRuntime;

    const auto output_to_console = Logger::OutputToConsole();
    Logger::DisableConsoleOutput();
    CompileInfoWithTopology().RunOnMachineFunction(mf);
    if (output_to_console) {
        Logger::EnableConsoleOutput();
    }

    EXPECT_EQ(info.execution_time_without_topology, SentinelRuntime);
    EXPECT_LT(info.execution_time, info.execution_time_without_topology);
}

TEST(CompileInfo, RuntimeWithoutTopologyDoesNotExceedRoutedRuntimeForAddCuccaro) {
    const auto info = CompileAndCalculateInfo(
            "quration-core/tests/data/circuit/add_cuccaro_5.json",
            "AddCuccaro(5)"
    );

    EXPECT_LE(info.execution_time_without_topology, info.execution_time);
}

TEST(CompileInfo, RuntimeWithoutTopologyDoesNotTimeoutForDistributedAddCuccaro) {
    const auto info = CompileAndCalculateInfo(
            "quration-core/tests/data/circuit/add_cuccaro_5.json",
            "AddCuccaro(5)",
            "quration-core/tests/data/topology/distribute.yaml"
    );

    EXPECT_GT(info.entanglement_factory_count, 0);
    EXPECT_LE(info.execution_time_without_topology, info.execution_time);
}
