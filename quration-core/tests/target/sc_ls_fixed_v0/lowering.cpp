#include "qret/target/sc_ls_fixed_v0/lowering.h"

#include <gtest/gtest.h>

#include <fmt/format.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include "qret/base/cast.h"
#include "qret/base/string.h"
#include "qret/codegen/machine_function.h"
#include "qret/ir/basic_block.h"
#include "qret/ir/context.h"
#include "qret/ir/function.h"
#include "qret/ir/instructions.h"
#include "qret/ir/module.h"
#include "qret/math/pauli.h"
#include "qret/target/sc_ls_fixed_v0/constants.h"
#include "qret/target/sc_ls_fixed_v0/instruction.h"
#include "qret/target/sc_ls_fixed_v0/sc_ls_fixed_v0_target_machine.h"
#include "qret/target/sc_ls_fixed_v0/topology.h"
#include "qret/transforms/ipo/inliner.h"
#include "qret/transforms/scalar/decomposition.h"

#include "test_utils.h"

using namespace qret;
using namespace qret::sc_ls_fixed_v0;

namespace {
ir::Function* LoadAddCuccaroCircuit(std::size_t size, ir::IRContext& context) {
    const auto path = fmt::format("quration-core/tests/data/circuit/add_cuccaro_{}.json", size);
    const auto name = fmt::format("AddCuccaro({})", size);
    return tests::LoadCircuitFromJsonFile(path, name, context);
}
}  // namespace

TEST(Lowering, Plane) {
    const auto size = std::size_t{3};

    ir::IRContext context;
    auto* circuit = LoadAddCuccaroCircuit(size, context);
    ASSERT_NE(nullptr, circuit);
    ir::DecomposeInst().RunOnFunction(*circuit);
    ir::InlinerPass().RunOnFunction(*circuit);

    auto topology = Topology::FromYAML(LoadFile("quration-core/tests/data/topology/plane.yaml"));
    const auto target = ScLsFixedV0TargetMachine(topology, ScLsFixedV0MachineOption());
    auto mf = MachineFunction(&target);
    mf.SetIR(circuit);
    Lowering().RunOnMachineFunction(mf);

    for (const auto& mbb : mf) {
        for (const auto& minst : mbb) {
            std::cout << minst->ToString() << std::endl;
        }
    }
}
TEST(Lowering, CXLowersToCnot) {
    const auto size = std::size_t{3};

    ir::IRContext context;
    auto* circuit = LoadAddCuccaroCircuit(size, context);
    ASSERT_NE(nullptr, circuit);
    ir::DecomposeInst().RunOnFunction(*circuit);
    ir::InlinerPass().RunOnFunction(*circuit);

    auto topology = Topology::FromYAML(LoadFile("quration-core/tests/data/topology/plane.yaml"));
    const auto target = ScLsFixedV0TargetMachine(topology, ScLsFixedV0MachineOption());
    auto mf = MachineFunction(&target);
    mf.SetIR(circuit);
    Lowering().RunOnMachineFunction(mf);

    auto has_cnot = false;
    for (const auto& mbb : mf) {
        for (const auto& minst : mbb) {
            has_cnot = has_cnot
                    || (DynCast<Cnot>(static_cast<const ScLsInstructionBase*>(minst.get()))
                        != nullptr);
        }
    }
    EXPECT_TRUE(has_cnot);
}
TEST(Lowering, PauliProductMeasurementLowersToLatticeSurgery) {
    ir::IRContext context;
    auto* module = ir::Module::Create("module", context);
    auto* circuit = ir::Function::Create("product_measurement", module);
    circuit->AddQubit("q", 2);
    circuit->AddRegister("r", 1);
    auto* bb = ir::BasicBlock::Create("entry", circuit);
    circuit->SetEntryBB(bb);

    ir::PauliProductMeasurementInst::Create(
            {ir::Qubit{0}, ir::Qubit{1}},
            {math::Pauli::X, math::Pauli::Z},
            ir::Register{0},
            bb
    );
    ir::ReturnInst::Create(bb);

    auto topology = Topology::FromYAML(LoadFile("quration-core/tests/data/topology/plane.yaml"));
    const auto target = ScLsFixedV0TargetMachine(topology, ScLsFixedV0MachineOption());
    auto mf = MachineFunction(&target);
    mf.SetIR(circuit);
    Lowering().RunOnMachineFunction(mf);

    const LatticeSurgery* found = nullptr;
    for (const auto& mbb : mf) {
        for (const auto& minst : mbb) {
            if (const auto* ls =
                        DynCast<LatticeSurgery>(static_cast<const ScLsInstructionBase*>(minst.get()))) {
                found = ls;
                break;
            }
        }
    }

    ASSERT_NE(nullptr, found);
    const auto qs = std::vector<QSymbol>{found->QubitList().begin(), found->QubitList().end()};
    const auto basis = std::vector<Pauli>{found->BasisList().begin(), found->BasisList().end()};
    EXPECT_EQ(qs, (std::vector<QSymbol>{QSymbol{0}, QSymbol{1}}));
    EXPECT_EQ(basis, (std::vector<Pauli>{Pauli::X(), Pauli::Z()}));
}
TEST(Lowering, SingleQubitPauliProductMeasurementLowersToMeasZX) {
    struct Case {
        math::Pauli pauli;
        std::uint32_t zx;
        std::size_t num_twists;
    };

    for (const auto test_case :
         std::vector<Case>{
                 {math::Pauli::X, MeasZX::X, 0},
                 {math::Pauli::Y, MeasZX::X, 2},
                 {math::Pauli::Z, MeasZX::Z, 0},
         }) {
        ir::IRContext context;
        auto* module = ir::Module::Create("module", context);
        auto* circuit = ir::Function::Create("single_product_measurement", module);
        circuit->AddQubit("q", 1);
        circuit->AddRegister("r", 1);
        auto* bb = ir::BasicBlock::Create("entry", circuit);
        circuit->SetEntryBB(bb);

        ir::PauliProductMeasurementInst::Create(
                {ir::Qubit{0}},
                {test_case.pauli},
                ir::Register{0},
                bb
        );
        ir::ReturnInst::Create(bb);

        auto topology = Topology::FromYAML(LoadFile("quration-core/tests/data/topology/plane.yaml"));
        const auto target = ScLsFixedV0TargetMachine(topology, ScLsFixedV0MachineOption());
        auto mf = MachineFunction(&target);
        mf.SetIR(circuit);
        Lowering().RunOnMachineFunction(mf);

        const MeasZX* found = nullptr;
        auto num_lattice_surgery = std::size_t{0};
        auto num_twists = std::size_t{0};
        for (const auto& mbb : mf) {
            for (const auto& minst : mbb) {
                const auto* base = static_cast<const ScLsInstructionBase*>(minst.get());
                if (const auto* meas = DynCast<MeasZX>(base)) {
                    found = meas;
                }
                if (DynCast<LatticeSurgery>(base) != nullptr) {
                    ++num_lattice_surgery;
                }
                if (DynCast<Twist>(base) != nullptr) {
                    ++num_twists;
                }
            }
        }

        ASSERT_NE(nullptr, found);
        EXPECT_EQ(found->Qubit(), QSymbol{0});
        EXPECT_EQ(found->CDest(), CSymbol{NumReservedCSymbols});
        EXPECT_EQ(found->ZX(), test_case.zx);
        EXPECT_EQ(num_lattice_surgery, std::size_t{0});
        EXPECT_EQ(num_twists, test_case.num_twists);
    }
}
TEST(Lowering, Grid) {
    const auto size = std::size_t{3};

    ir::IRContext context;
    auto* circuit = LoadAddCuccaroCircuit(size, context);
    ASSERT_NE(nullptr, circuit);
    ir::DecomposeInst().RunOnFunction(*circuit);
    ir::InlinerPass().RunOnFunction(*circuit);

    auto topology = Topology::FromYAML(LoadFile("quration-core/tests/data/topology/grid.yaml"));
    const auto target = ScLsFixedV0TargetMachine(topology, ScLsFixedV0MachineOption());
    auto mf = MachineFunction(&target);
    mf.SetIR(circuit);
    Lowering().RunOnMachineFunction(mf);

    for (const auto& mbb : mf) {
        for (const auto& minst : mbb) {
            std::cout << minst->ToString() << std::endl;
        }
    }
}
TEST(Lowering, Distribute) {
    const auto size = std::size_t{3};

    ir::IRContext context;
    auto* circuit = LoadAddCuccaroCircuit(size, context);
    ASSERT_NE(nullptr, circuit);
    ir::DecomposeInst().RunOnFunction(*circuit);
    ir::InlinerPass().RunOnFunction(*circuit);

    auto topology =
            Topology::FromYAML(LoadFile("quration-core/tests/data/topology/distribute.yaml"));
    const auto target = ScLsFixedV0TargetMachine(topology, ScLsFixedV0MachineOption());
    auto mf = MachineFunction(&target);
    mf.SetIR(circuit);
    Lowering().RunOnMachineFunction(mf);

    for (const auto& mbb : mf) {
        for (const auto& minst : mbb) {
            std::cout << minst->ToString() << std::endl;
        }
    }
}
