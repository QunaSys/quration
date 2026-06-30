# Data {#examples_data}

- `circuit/` : intermediate representation and SC_LS_FIXED_V0 intermediate files
- `OpenQASM2/` : examples of quantum circuits copied from the OpenQASM2 paper [1]
- `topology/` : topology files
- `pipeline/` : pipeline files

## `circuit/`

- `circuit/add_craig_5.json` : JSON file output by running `create_add_craig.cpp`
  - Defines the intermediate representation of a width-5 Craig adder circuit.
- `circuit/add_cuccaro_5.json` : JSON file output by running `create_add_cuccaro.cpp`
  - Defines the intermediate representation of a width-5 Cuccaro adder circuit.
- `circuit/add_cuccaro_5_pbc.json` : version of `circuit/add_cuccaro_5.json` in which all qubits are measured
  - When compiling with `sc_ls_fixed_v0_enable_pbc_mode=true`, all qubits must be measured.
- `circuit/branch.json` : circuit that branches according to measured values
- `circuit/decompose_using_external_pass.json`
- `circuit/mapping_using_external_pass.json`
- `circuit/random.json` : JSON file output by running `create_random.cpp`

## `pipeline/`

- `pipeline/decompose_using_external_pass.yaml`
- `pipeline/mapping_based_on_topology_file.yaml`
- `pipeline/mapping_using_external_pass.yaml`
- `pipeline/qret_compile_branch.yaml` : example of a pass that compiles branches depending on measured values
- `pipeline/qret_compile_cultivation.yaml`
- `pipeline/qret_compile_pbc.yaml` : example of a pass that compiles in PBC mode (`sc_ls_fixed_v0_enable_pbc_mode=true`)
- `pipeline/qret_compile_random.yaml`
- `pipeline/qret_compile.yaml` : example of a pass that compiles to SC_LS_FIXED_V0
- `pipeline/qret_opt.yaml` : example of an optimization pass for the intermediate representation
- `pipeline/runtime_simulation_pruning.yaml` : example of a pass that determines the value of `PROBABILITY_HINT` according to `sc_ls_fixed_v0_runtime_simulation_pruning_seed` and estimates resources based on that branch

## `topology/`

- `topology/dist_all.yaml`
- `topology/dist_circle.yaml`
- `topology/dist_line.yaml`
- `topology/grid.yaml`
- `topology/plane.yaml`
- `topology/tutorial.yaml`

## References

- [1] : Open Quantum Assembly Language
  - <https://arxiv.org/abs/1707.03429>
