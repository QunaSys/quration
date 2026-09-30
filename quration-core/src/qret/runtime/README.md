\page qret_runtime runtime module

Implements simulators for the intermediate representation.
For details about how to use the simulator, see "Verification with the Quantum Circuit Simulator".

## Code map

* `qret/runtime/clifford_state.h`
* `qret/runtime/dd.h`
* `qret/runtime/full_quantum_state.h`
  * `qret::runtime::FullQuantumState` : quantum state class that can execute arbitrary gates
* `qret/runtime/quantum_state.h`
  * `qret::runtime::QuantumState` : parent class for quantum states
    * Concrete quantum states should be implemented by inheriting from this class.
    * Examples: `qret::runtime::ToffoliState`, `qret::runtime::FullQuantumState`
* `qret/runtime/simulator.h`
  * `qret::runtime::Simulator` : simulator class
    * Operates on `qret::runtime::QuantumState` to simulate quantum circuits.
  * `qret::runtime::SimulatorConfig` : simulator configuration
* `qret/runtime/toffoli_state.h`
  * `qret::runtime::ToffoliState` : efficient quantum state class when the number of computational-basis superpositions is small
