#include "qret/runtime/full_quantum_state.h"

#include <gtest/gtest.h>

#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include "qret/math/pauli.h"
#include "qret/runtime/quantum_state.h"

using namespace qret;

using qret::runtime::FullQuantumState;
using qret::runtime::QuantumState;
static constexpr auto Pi = std::numbers::pi;

using namespace qret;

class FullQuantumStateTest : public ::testing::Test {
protected:
    std::uint64_t seed_ = 12345;

    // Helper to verify complex numbers
    bool IsClose(std::complex<double> actual, std::complex<double> expected, double tol = 1e-9) {
        return std::abs(actual - expected) < tol;
    }

    // Helper to get element from column-major raw pointer
    // dim: dimension of the matrix (2^num_qubits)
    std::complex<double> GetMatrixElement(
            const std::complex<double>* ptr,
            std::size_t dim,
            std::size_t row,
            std::size_t col
    ) {
        // The operation matrix pointer is stored in column-major order.
        return ptr[(col * dim) + row];
    }
};

namespace {
using qret::math::Pauli;

void ExpectPauliProductMeasurementEdgeCases(std::uint64_t seed, bool use_qulacs) {
    auto minus_state = FullQuantumState(seed, 3, use_qulacs, false);
    minus_state.X(1);
    minus_state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 0);
    EXPECT_TRUE(minus_state.ReadRegister(0));
    minus_state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 1);
    EXPECT_TRUE(minus_state.ReadRegister(1));
    EXPECT_NEAR(0.0, minus_state.Calc1Prob(0), QuantumState::Eps);
    EXPECT_NEAR(1.0, minus_state.Calc1Prob(1), QuantumState::Eps);

    auto y_z_state = FullQuantumState(seed, 2, use_qulacs, false);
    y_z_state.H(0);
    y_z_state.S(0);
    y_z_state.X(1);
    y_z_state.MeasurePauliProduct({0, 1}, {Pauli::Y, Pauli::Z}, 0);
    EXPECT_TRUE(y_z_state.ReadRegister(0));

    auto identity_state = FullQuantumState(seed, 3, use_qulacs, false);
    identity_state.X(0);
    identity_state.X(1);
    identity_state.X(2);
    identity_state.MeasurePauliProduct({0, 1, 2}, {Pauli::Z, Pauli::I, Pauli::Z}, 0);
    EXPECT_FALSE(identity_state.ReadRegister(0));

    for (auto repeat_seed = std::uint64_t{0}; repeat_seed < 16; ++repeat_seed) {
        auto collapse_state = FullQuantumState(repeat_seed, 2, use_qulacs, false);
        collapse_state.H(0);
        collapse_state.H(1);
        collapse_state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 0);
        const auto parity_is_minus = collapse_state.ReadRegister(0);

        collapse_state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 1);
        EXPECT_EQ(parity_is_minus, collapse_state.ReadRegister(1));

        collapse_state.Measure(0, 2);
        collapse_state.Measure(1, 3);
        EXPECT_EQ(
                parity_is_minus,
                collapse_state.ReadRegister(2) != collapse_state.ReadRegister(3)
        );
    }
}
}  // namespace

TEST_F(FullQuantumStateTest, InitializationState) {
    // 1 qubit, save_operation_matrix = false
    auto state = FullQuantumState(seed_, 1, false, false);

    // Initial state |0>, so Prob(1) = 0
    EXPECT_NEAR(state.Calc1Prob(0), 0.0, QuantumState::Eps);

    // Superposition check
    EXPECT_FALSE(state.IsSuperposed(0));
}

TEST_F(FullQuantumStateTest, GateOperationsXHZ) {
    auto state = FullQuantumState(seed_, 1, false, false);

    // X: |0> -> |1>
    state.X(0);
    EXPECT_NEAR(state.Calc1Prob(0), 1.0, QuantumState::Eps);

    // X: |1> -> |0>
    state.X(0);
    EXPECT_NEAR(state.Calc1Prob(0), 0.0, QuantumState::Eps);

    // H: |0> -> |+>
    state.H(0);
    EXPECT_NEAR(state.Calc1Prob(0), 0.5, QuantumState::Eps);
    EXPECT_TRUE(state.IsSuperposed(0));

    // Z: |+> -> |-> (Probabilities don't change)
    state.Z(0);
    EXPECT_NEAR(state.Calc1Prob(0), 0.5, QuantumState::Eps);

    // H: |-> -> |1>
    state.H(0);
    EXPECT_NEAR(state.Calc1Prob(0), 1.0, QuantumState::Eps);
}

TEST_F(FullQuantumStateTest, EntanglementBellState) {
    // 2 qubits
    auto state = FullQuantumState(seed_, 2, false, false);

    // Create Bell pair |Phi+> = (|00> + |11>) / sqrt(2)
    state.H(0);
    state.CX(1, 0);  // Target 1, Control 0

    // Check marginal probabilities
    EXPECT_NEAR(state.Calc1Prob(0), 0.5, QuantumState::Eps);
    EXPECT_NEAR(state.Calc1Prob(1), 0.5, QuantumState::Eps);

    // Measure q0 -> r0
    state.Measure(0, 0);
    bool r0 = state.ReadRegister(0);

    // Measure q1 -> r1
    state.Measure(1, 1);
    bool r1 = state.ReadRegister(1);

    // Should be correlated
    EXPECT_EQ(r0, r1);
}

TEST_F(FullQuantumStateTest, PauliProductMeasurement) {
    auto state = FullQuantumState(seed_, 2, false, false);

    state.H(0);
    state.CX(1, 0);

    state.MeasurePauliProduct({0, 1}, {math::Pauli::Z, math::Pauli::Z}, 0);
    EXPECT_FALSE(state.ReadRegister(0));

    state.MeasurePauliProduct({0, 1}, {math::Pauli::X, math::Pauli::X}, 1);
    EXPECT_FALSE(state.ReadRegister(1));

    auto y_state = FullQuantumState(seed_, 1, false, false);
    y_state.H(0);
    y_state.S(0);
    y_state.MeasurePauliProduct({0}, {math::Pauli::Y}, 0);
    EXPECT_FALSE(y_state.ReadRegister(0));
}

TEST_F(FullQuantumStateTest, PauliProductMeasurementEdgeCases) {
    ExpectPauliProductMeasurementEdgeCases(seed_, false);
}

TEST_F(FullQuantumStateTest, PauliProductMeasurementUpdatesOperationMatrix) {
    auto state = FullQuantumState(seed_, 2, false, true);
    state.H(0);
    state.H(1);

    state.MeasurePauliProduct({0, 1}, {Pauli::Z, Pauli::Z}, 0);
    const auto parity_is_minus = state.ReadRegister(0);

    const auto* mat = state.GetOperationMatrix();
    ASSERT_NE(mat, nullptr);
    for (auto col = std::size_t{0}; col < 4; ++col) {
        for (auto row = std::size_t{0}; row < 4; ++row) {
            const auto row_parity_is_minus =
                    ((row & 0b01U) != 0U) != ((row & 0b10U) != 0U);
            if (row_parity_is_minus != parity_is_minus) {
                EXPECT_TRUE(IsClose(GetMatrixElement(mat, 4, row, col), 0.0));
            }
        }
    }
}

TEST_F(FullQuantumStateTest, QulacsPauliProductMeasurement) {
    if (!runtime::CanUseQulacs()) {
        GTEST_SKIP() << "Qulacs is not available in this build.";
    }

    auto state = FullQuantumState(seed_, 2, true, false);

    state.H(0);
    state.CX(1, 0);

    state.MeasurePauliProduct({0, 1}, {math::Pauli::Z, math::Pauli::Z}, 0);
    EXPECT_FALSE(state.ReadRegister(0));

    state.MeasurePauliProduct({0, 1}, {math::Pauli::X, math::Pauli::X}, 1);
    EXPECT_FALSE(state.ReadRegister(1));

    auto y_state = FullQuantumState(seed_, 1, true, false);
    y_state.H(0);
    y_state.S(0);
    y_state.MeasurePauliProduct({0}, {math::Pauli::Y}, 0);
    EXPECT_FALSE(y_state.ReadRegister(0));
}

TEST_F(FullQuantumStateTest, QulacsPauliProductMeasurementEdgeCases) {
    if (!runtime::CanUseQulacs()) {
        GTEST_SKIP() << "Qulacs is not available in this build.";
    }

    ExpectPauliProductMeasurementEdgeCases(seed_, true);
}

TEST_F(FullQuantumStateTest, OperationMatrixIdentity) {
    // 1 qubit, enable matrix saving
    auto state = FullQuantumState(seed_, 1, false, true);

    const auto* mat = state.GetOperationMatrix();
    ASSERT_NE(mat, nullptr);

    // Initial matrix should be Identity (2x2)
    // Row 0, Col 0 -> 1
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 0, 0), 1.0));
    // Row 1, Col 0 -> 0
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 1, 0), 0.0));
    // Row 0, Col 1 -> 0
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 0, 1), 0.0));
    // Row 1, Col 1 -> 1
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 1, 1), 1.0));
}

TEST_F(FullQuantumStateTest, OperationMatrixXGate) {
    // 1 qubit, enable matrix saving
    auto state = FullQuantumState(seed_, 1, false, true);

    // Apply X
    state.X(0);

    const auto* mat = state.GetOperationMatrix();

    // Expected: [0 1; 1 0]
    // (0,0) -> 0
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 0, 0), 0.0));
    // (1,0) -> 1
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 1, 0), 1.0));
    // (0,1) -> 1
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 0, 1), 1.0));
    // (1,1) -> 0
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, 2, 1, 1), 0.0));
}

TEST_F(FullQuantumStateTest, OperationMatrixHAndPhase) {
    // 1 qubit, enable matrix saving
    auto state = FullQuantumState(seed_, 1, false, true);
    double inv_sqrt2 = 1.0 / std::numbers::sqrt2;

    state.H(0);
    // H = 1/sqrt(2) * [1 1; 1 -1]
    const auto* matH = state.GetOperationMatrix();
    EXPECT_TRUE(IsClose(GetMatrixElement(matH, 2, 0, 0), inv_sqrt2));
    EXPECT_TRUE(IsClose(GetMatrixElement(matH, 2, 1, 1), -inv_sqrt2));

    state.S(0);
    // S = [1 0; 0 i]
    // S * H = 1/sqrt(2) * [1 1; i -i]
    const auto* matSH = state.GetOperationMatrix();

    // (0,0) = 1 * inv_sqrt2
    EXPECT_TRUE(IsClose(GetMatrixElement(matSH, 2, 0, 0), inv_sqrt2));
    // (1,0) = i * inv_sqrt2
    EXPECT_TRUE(IsClose(GetMatrixElement(matSH, 2, 1, 0), std::complex<double>(0, inv_sqrt2)));
}

TEST_F(FullQuantumStateTest, OperationMatrixTwoQubitsCX) {
    // 2 qubits, enable matrix saving
    auto state = FullQuantumState(seed_, 2, false, true);
    std::size_t dim = 4;  // 2^2

    // Apply CX(target=1, control=0)
    // Assuming q0 is LSB.
    // Basis: |00>(0), |01>(1), |10>(2), |11>(3) where |q1 q0>
    // Control q0=1 (indices 1 and 3). Target q1 flips.
    // |01> (q1=0, q0=1) -> |11> (q1=1, q0=1) => 1 -> 3
    // |11> (q1=1, q0=1) -> |01> (q1=0, q0=1) => 3 -> 1
    // Others map to themselves.
    state.CX(1, 0);

    const auto* mat = state.GetOperationMatrix();

    // Check mapping logic via matrix columns

    // Col 0 (|00>) -> Row 0
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, dim, 0, 0), 1.0));

    // Col 1 (|01>) -> Row 3 (|11>)
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, dim, 3, 1), 1.0));
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, dim, 1, 1), 0.0));

    // Col 2 (|10>) -> Row 2
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, dim, 2, 2), 1.0));

    // Col 3 (|11>) -> Row 1 (|01>)
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, dim, 1, 3), 1.0));
    EXPECT_TRUE(IsClose(GetMatrixElement(mat, dim, 3, 3), 0.0));
}

TEST_F(FullQuantumStateTest, Registers) {
    auto state = FullQuantumState(seed_, 4, false, false);

    // Test manual register write/read
    state.WriteRegister(0, true);
    EXPECT_TRUE(state.ReadRegister(0));
    state.WriteRegister(0, false);
    EXPECT_FALSE(state.ReadRegister(0));

    // Test bulk write
    // 5 (binary 101) -> r10=1, r11=0, r12=1
    qret::BigInt val = 5;
    state.WriteRegisters(10, 3, val);

    auto regs = state.ReadRegisters(10, 3);
    ASSERT_EQ(regs.size(), 3);
    EXPECT_EQ(regs[0], true);
    EXPECT_EQ(regs[1], false);
    EXPECT_EQ(regs[2], true);
}

TEST_F(FullQuantumStateTest, GlobalPhase) {
    auto state = FullQuantumState(seed_, 1, false, false);

    // Rotation shouldn't affect probability magnitude
    state.RotateGlobalPhase(Pi / 2.0);
    EXPECT_NEAR(state.Calc1Prob(0), 0.0, QuantumState::Eps);

    state.H(0);
    state.RotateGlobalPhase(Pi);
    EXPECT_NEAR(state.Calc1Prob(0), 0.5, QuantumState::Eps);
}
