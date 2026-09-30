#include "qret/runtime/toffoli_state.h"

#include <gtest/gtest.h>

#include <ranges>
#include <vector>

#include "qret/math/pauli.h"

namespace {
using qret::math::Pauli;
using qret::runtime::ToffoliState;

void ExpectRepeatedProductMeasurementMatchesFinalZParity(ToffoliState& state) {
    state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 0);
    const auto parity_is_minus = state.ReadRegister(0);

    state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 1);
    EXPECT_EQ(parity_is_minus, state.ReadRegister(1));

    state.Measure(0, 2);
    state.Measure(1, 3);
    EXPECT_EQ(parity_is_minus, state.ReadRegister(2) != state.ReadRegister(3));
}
}  // namespace

struct ToffoliStateTest : public ::testing::TestWithParam<std::uint64_t> {
    void SetUp() override {
        // Get the seed value from the fixture's parameters
        seed = GetParam();
        num_qubits = 10;
        eps = 1e-10;
        state = std::make_unique<qret::runtime::ToffoliState>(seed, num_qubits, eps);

        // Apply H and MCX gates after ToffoliState initialization
        std::vector<std::uint64_t> control_qubits;
        for (const auto q : std::views::iota(std::uint64_t{0}, num_qubits / 2)) {
            control_qubits.emplace_back(q);
        }
        for (const auto q : control_qubits) {
            state->H(q);
        }
        state->MCX(num_qubits - 1, control_qubits);
    }

    void TearDown() override {}

    std::uint64_t seed;
    std::uint64_t num_qubits;
    double eps;
    std::unique_ptr<qret::runtime::ToffoliState> state;  // Manage with a smart pointer
};

TEST_P(ToffoliStateTest, Measurement) {
    ASSERT_GE(num_qubits, 4);  // Ensure NumQubits >= 4

    EXPECT_FALSE(state->IsComputationalBasis());
    EXPECT_EQ(std::uint64_t{1} << (num_qubits / 2), state->GetNumSuperpositions());

    EXPECT_TRUE(state->IsSuperposed(0));
    EXPECT_FALSE(state->IsClean(0));
    EXPECT_NEAR(0.5, state->Calc0Prob(0), eps);
    EXPECT_NEAR(0.5, state->Calc1Prob(0), eps);

    EXPECT_FALSE(state->IsSuperposed(num_qubits - 2));
    EXPECT_TRUE(state->IsClean(num_qubits - 2));
    EXPECT_NEAR(1, state->Calc0Prob(num_qubits - 2), eps);
    EXPECT_NEAR(0, state->Calc1Prob(num_qubits - 2), eps);

    EXPECT_TRUE(state->IsSuperposed(num_qubits - 1));
    EXPECT_FALSE(state->IsClean(num_qubits - 1));
    EXPECT_NEAR(1 - std::pow(0.5, num_qubits / 2), state->Calc0Prob(num_qubits - 1), eps);
    EXPECT_NEAR(std::pow(0.5, num_qubits / 2), state->Calc1Prob(num_qubits - 1), eps);

    state->Measure(num_qubits - 1, 10);

    if (state->ReadRegister(10)) {
        ASSERT_EQ(1, state->GetNumSuperpositions());
        EXPECT_NEAR(1.0, state->GetPhase().real(), eps);
        EXPECT_NEAR(0.0, state->GetPhase().imag(), eps);
    } else {
        EXPECT_EQ((std::uint64_t{1} << (num_qubits / 2)) - 1, state->GetNumSuperpositions());
        for (const auto& [coeff, _] : state->GetRawVector()) {
            EXPECT_NEAR(1 / (std::pow(2.0, num_qubits / 2) - 1.0), std::norm(coeff), eps);
        }
    }
}

TEST(ToffoliState, PauliProductMeasurement) {
    auto state = qret::runtime::ToffoliState(12345, 2);

    state.H(0);
    state.CX(1, 0);

    state.MeasurePauliProduct({0, 1}, {qret::math::Pauli::Z, qret::math::Pauli::Z}, 0);
    EXPECT_FALSE(state.ReadRegister(0));

    state.MeasurePauliProduct({0, 1}, {qret::math::Pauli::X, qret::math::Pauli::X}, 1);
    EXPECT_FALSE(state.ReadRegister(1));
}

TEST(ToffoliState, PauliProductMeasurementDeterministicMinusEigenstate) {
    auto state = ToffoliState(12345, 3);
    state.X(1);

    state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 0);
    EXPECT_TRUE(state.ReadRegister(0));

    state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 1);
    EXPECT_TRUE(state.ReadRegister(1));
    EXPECT_NEAR(0.0, state.Calc1Prob(0), 1e-10);
    EXPECT_NEAR(1.0, state.Calc1Prob(1), 1e-10);
}

TEST(ToffoliState, PauliProductMeasurementSupportsYAndIdentity) {
    auto y_z_state = ToffoliState(12345, 2);
    y_z_state.H(0);
    y_z_state.S(0);
    y_z_state.X(1);
    y_z_state.MeasurePauliProduct({0, 1}, {Pauli::Y, Pauli::Z}, 0);
    EXPECT_TRUE(y_z_state.ReadRegister(0));

    auto identity_state = ToffoliState(12345, 3);
    identity_state.X(0);
    identity_state.X(1);
    identity_state.X(2);
    identity_state.MeasurePauliProduct({0, 1, 2}, {Pauli::Z, Pauli::I, Pauli::Z}, 0);
    EXPECT_FALSE(identity_state.ReadRegister(0));
}

TEST(ToffoliState, PauliProductMeasurementCollapseIsRepeatable) {
    for (const auto seed : std::views::iota(std::uint64_t{0}, std::uint64_t{16})) {
        auto state = ToffoliState(seed, 2);
        state.H(0);
        state.H(1);
        ExpectRepeatedProductMeasurementMatchesFinalZParity(state);
    }
}

INSTANTIATE_TEST_SUITE_P(
        ToffoliStateMeasurementWithMultipleSeeds,
        ToffoliStateTest,
        ::testing::Range(std::uint64_t{0}, std::uint64_t{100})
);
