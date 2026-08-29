#include "qret/runtime/simulator.h"

#include <gtest/gtest.h>

#include "qret/base/type.h"
#include "qret/frontend/builder.h"
#include "qret/frontend/circuit.h"
#include "qret/frontend/circuit_generator.h"
#include "qret/frontend/intrinsic.h"
#include "qret/math/pauli.h"

using namespace qret;

struct CRandGen : public frontend::CircuitGenerator {
    static inline const char* Name = "CRand";
    explicit CRandGen(frontend::CircuitBuilder* builder)
        : CircuitGenerator(builder) {}
    std::string GetName() const override {
        return Name;
    }
    // std::string GetCacheKey() const override;
    void SetArgument(Argument& arg) const override {
        arg.Add("c", Type::Register, 10, Attribute::Output);
    }
    frontend::Circuit* Generate() const override {
        BeginCircuitDefinition();
        const auto c = GetRegisters(0);
        frontend::gate::DiscreteDistribution({0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0}, c.Range(2, 3));
        return EndCircuitDefinition();
    }
};

struct PauliProductMeasureGen : public frontend::CircuitGenerator {
    static inline const char* Name = "PauliProductMeasure";
    explicit PauliProductMeasureGen(frontend::CircuitBuilder* builder)
        : CircuitGenerator(builder) {}
    std::string GetName() const override {
        return Name;
    }
    void SetArgument(Argument& arg) const override {
        arg.Add("q", Type::Qubit, 2, Attribute::Operate);
        arg.Add("r", Type::Register, 1, Attribute::Output);
    }
    frontend::Circuit* Generate() const override {
        BeginCircuitDefinition();
        const auto q = GetQubits("q");
        const auto r = GetRegisters("r");
        frontend::gate::H(q[0]);
        frontend::gate::CX(q[1], q[0]);
        frontend::gate::PauliProductMeasure(q, {math::Pauli::Z, math::Pauli::Z}, r[0]);
        return EndCircuitDefinition();
    }
};

struct PauliProductMeasureEdgeCasesGen : public frontend::CircuitGenerator {
    static inline const char* Name = "PauliProductMeasureEdgeCases";
    explicit PauliProductMeasureEdgeCasesGen(frontend::CircuitBuilder* builder)
        : CircuitGenerator(builder) {}
    std::string GetName() const override {
        return Name;
    }
    void SetArgument(Argument& arg) const override {
        arg.Add("q", Type::Qubit, 4, Attribute::Operate);
        arg.Add("r", Type::Register, 4, Attribute::Output);
    }
    frontend::Circuit* Generate() const override {
        BeginCircuitDefinition();
        const auto q = GetQubits("q");
        const auto r = GetRegisters("r");

        frontend::gate::H(q[0]);
        frontend::gate::CX(q[1], q[0]);
        frontend::gate::PauliProductMeasure(
                q.Range(0, 2),
                {math::Pauli::Z, math::Pauli::Z},
                r[0]
        );
        frontend::gate::PauliProductMeasure(
                q.Range(0, 2),
                {math::Pauli::X, math::Pauli::X},
                r[1]
        );
        frontend::gate::PauliProductMeasure(
                q.Range(0, 2),
                {math::Pauli::Y, math::Pauli::Y},
                r[2]
        );

        frontend::gate::H(q[2]);
        frontend::gate::S(q[2]);
        frontend::gate::X(q[3]);
        frontend::gate::PauliProductMeasure(
                q.Range(2, 2),
                {math::Pauli::Y, math::Pauli::Z},
                r[3]
        );
        return EndCircuitDefinition();
    }
};

TEST(Simulator, DiscreteDistInst) {
    static constexpr auto NumSimulations = 10000;

    ir::IRContext context;
    auto* module = ir::Module::Create("crand", context);
    auto builder = frontend::CircuitBuilder(module);
    auto gen = CRandGen(&builder);
    auto* circuit = gen.Generate();

    auto hist = std::unordered_map<std::size_t, int>{};
    for (auto i = 0; i < NumSimulations; ++i) {
        auto config = runtime::SimulatorConfig{
                .state_type = runtime::SimulatorConfig::StateType::Toffoli,
                .seed = static_cast<std::uint64_t>(i)
        };
        auto simulator = runtime::Simulator(config, circuit->GetIR());
        simulator.RunAll();

        const auto val = BoolArrayAsInt(simulator.GetState().ReadRegisters(2, 3));
        EXPECT_TRUE(val == 1 || val == 3 || val == 4 || val == 6);

        if (hist.contains(val)) {
            hist[val] += 1;
        } else {
            hist[val] = 1;
        }
    }

    EXPECT_EQ(4, hist.size());
    for (const auto& [_, num] : hist) {
        EXPECT_LT(NumSimulations / 5, num);
        EXPECT_LT(num, NumSimulations / 3);
    }
}

TEST(Simulator, PauliProductMeasurementInst) {
    ir::IRContext context;
    auto* module = ir::Module::Create("product_measurement", context);
    auto builder = frontend::CircuitBuilder(module);
    auto gen = PauliProductMeasureGen(&builder);
    auto* circuit = gen.Generate();

    auto config = runtime::SimulatorConfig{
            .state_type = runtime::SimulatorConfig::StateType::FullQuantum,
            .seed = 12345,
    };
    auto simulator = runtime::Simulator(config, circuit->GetIR());
    simulator.RunAll();

    EXPECT_FALSE(simulator.GetState().ReadRegister(0));
}

TEST(Simulator, PauliProductMeasurementInstAcrossStateTypes) {
    ir::IRContext context;
    auto* module = ir::Module::Create("product_measurement_edge_cases", context);
    auto builder = frontend::CircuitBuilder(module);
    auto gen = PauliProductMeasureEdgeCasesGen(&builder);
    auto* circuit = gen.Generate();

    for (const auto state_type :
         {
                 runtime::SimulatorConfig::StateType::Toffoli,
                 runtime::SimulatorConfig::StateType::FullQuantum,
                 runtime::SimulatorConfig::StateType::Clifford,
         }) {
        auto config = runtime::SimulatorConfig{
                .state_type = state_type,
                .seed = 12345,
                .max_superpositions = 16,
        };
        auto simulator = runtime::Simulator(config, circuit->GetIR());
        simulator.RunAll();

        EXPECT_FALSE(simulator.GetState().ReadRegister(0));
        EXPECT_FALSE(simulator.GetState().ReadRegister(1));
        EXPECT_TRUE(simulator.GetState().ReadRegister(2));
        EXPECT_TRUE(simulator.GetState().ReadRegister(3));
    }
}
