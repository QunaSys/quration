\page qret_transforms_scalar scalar transforms module

The scalar module implements optimization passes for individual qubits and local gates.

## Code map

- `qret/transforms/scalar/decomposition.cpp` : converts the intermediate representation into one composed of simpler instructions
  - Converts RX, RY, RZ, CY, CZ, CCX, CCY, and CCZ into combinations of {X, Y, Z, S, SDag, T, TDag, CX}.
- `qret/transforms/scalar/delete_consecutive_same_pauli.cpp` : removes consecutive applications of the same Pauli operator
- `qret/transforms/scalar/delete_opt_hint.cpp` : removes optimization hint instructions
- `qret/transforms/scalar/ignore_global_phase.cpp` : removes `qret::ir::GlobalPhaseInst` instructions
  - Removes instructions that rotate the global phase, which is not essential to the quantum circuit.
- `qret/transforms/static_condition_pruning.cpp` : removes statically determined branches
