#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "qret/base/log.h"
#include "qret/codegen/machine_function.h"
#include "qret/target/sc_ls_fixed_v0/calc_compile_info.h"
#include "qret/target/sc_ls_fixed_v0/compile_info.h"
#include "qret/target/sc_ls_fixed_v0/sc_ls_fixed_v0_target_machine.h"

using namespace qret;
using namespace qret::sc_ls_fixed_v0;

TEST(MagicFactoryCellCount, DefaultValueIsOne) {
    const ScLsFixedV0MachineOption option{};
    EXPECT_EQ(option.magic_factory_cell_count, 1UL);
}

// effective_cell_count = chip_cell_count + magic_factory_count *
// (magic_factory_cell_count - 1) num_physical_qubits = 2 * (d + 1)^2 *
// effective_cell_count

TEST(MagicFactoryCellCount, CellCountOneGivesChipCellCountUnchanged) {
    // magic_factory_cell_count=1: effective = chip_cell_count + factories * 0 =
    // chip_cell_count
    const std::uint64_t d = 5;
    const std::uint64_t chip_cell_count = 100;
    const std::uint64_t magic_factory_count = 4;
    constexpr std::uint64_t magic_factory_cell_count = 1;

    const auto effective = chip_cell_count + (magic_factory_count * (magic_factory_cell_count - 1));
    EXPECT_EQ(effective, chip_cell_count);

    const auto qubits =
            CompileInfoWithQecResourceEstimation::EstimatePhysicalQubitCount(d, effective);
    EXPECT_EQ(qubits, 2 * (d + 1) * (d + 1) * chip_cell_count);
}

TEST(MagicFactoryCellCount, PhysicalQubitCountFormula) {
    const std::uint64_t d = 5;
    const std::uint64_t chip_cell_count = 100;
    const std::uint64_t magic_factory_count = 4;

    for (const std::uint64_t cell_count : {1UL, 4UL, 8UL, 15UL}) {
        const auto effective = chip_cell_count + (magic_factory_count * (cell_count - 1));
        const auto qubits =
                CompileInfoWithQecResourceEstimation::EstimatePhysicalQubitCount(d, effective);
        EXPECT_EQ(qubits, 2 * (d + 1) * (d + 1) * effective) << "cell_count=" << cell_count;
    }
}

TEST(MagicFactoryCellCount, LargerCellCountIncreasesPhysicalQubits) {
    const std::uint64_t d = 5;
    const std::uint64_t chip_cell_count = 100;
    const std::uint64_t magic_factory_count = 4;

    const auto effective_base = chip_cell_count + (magic_factory_count * (1UL - 1));
    const auto effective_large = chip_cell_count + (magic_factory_count * (8UL - 1));

    const auto qubits_base =
            CompileInfoWithQecResourceEstimation::EstimatePhysicalQubitCount(d, effective_base);
    const auto qubits_large =
            CompileInfoWithQecResourceEstimation::EstimatePhysicalQubitCount(d, effective_large);

    EXPECT_LT(qubits_base, qubits_large);
}

TEST(MagicFactoryCellCount, ZeroCellCountIsRejected) {
    qret::sc_ls_fixed_v0::ScLsFixedV0TargetMachine target;
    target.machine_option.magic_factory_cell_count = 0;
    target.machine_option.logical_error_rate_base = 1e-3;
    target.machine_option.logical_error_rate_drop_rate = 0.1;
    target.machine_option.allowed_failure_probability = 1e-6;
    target.machine_option.code_cycle_time_sec = 1e-6;

    qret::MachineFunction mf(&target);
    mf.InitializeCompileInfo(std::make_unique<qret::sc_ls_fixed_v0::ScLsFixedV0CompileInfo>());

    auto& info =
            *static_cast<qret::sc_ls_fixed_v0::ScLsFixedV0CompileInfo*>(mf.GetMutCompileInfo());
    info.chip_cell_count = 100;
    info.magic_factory_count = 4;
    info.qubit_volume = 1000;
    info.execution_time = 100;

    qret::Logger::EnableConsoleOutput();
    auto captured = std::ostringstream();
    auto* original = std::cout.rdbuf(captured.rdbuf());

    qret::sc_ls_fixed_v0::CompileInfoWithQecResourceEstimation pass;
    pass.RunOnMachineFunction(mf);

    std::cout.rdbuf(original);
    qret::Logger::DisableConsoleOutput();

    EXPECT_EQ(info.physical_qubit_count, 0UL);
    EXPECT_NE(std::string::npos, captured.str().find("magic_factory_cell_count must be at least 1"));
}
