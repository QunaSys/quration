\page examples Examples

Implements examples that use the qret library.

## Overview

### Circuit Description (core)

| File | Description |
| ---- | ----------- |
| `quration-core/examples/grover.cpp` | Implements Grover search. |
| `quration-core/examples/trotter.cpp` | Implements a circuit that simulates Hamiltonian time evolution using Trotter decomposition. |
| `quration-core/examples/portable_function.cpp` | Shows an example of using `PortableFunction`. |
| `quration-core/examples/external_decompose_pass.cpp` | Shows an example of using an external pass. |
| `quration-core/examples/external_mapping_pass.cpp` | Shows an example of using an external mapping pass. |
| `quration-core/examples/create_random.cpp` | Shows an example of random circuit generation. |

#### `quration-core/examples/trotter.cpp`

Implements a circuit that simulates the time evolution of a Hamiltonian composed of Pauli strings using Trotter decomposition.

### Circuit Description (algorithm)

Examples for the algorithm library are placed under `quration-algorithm/examples/`.

### Intermediate Representation

| File | Description |
| ---- | ----------- |
| `examples/external_decompose_pass.cpp` | Uses an external pass to decompose Toffoli gates into T gates and other gates. |

### Compilation to SC_LS_FIXED_V0

| File | Description |
| ---- | ----------- |
| `examples/compile_adder_to_distributed_chip.cpp` | Compiles an adder circuit to SC_LS_FIXED_V0 (distributed instruction set). |

### Other

| File | Description |
| ---- | ----------- |
| `examples/portable_function.cpp` | Implements the Collatz function using `PortableFunction`. |
