\page qret_target target module

Implements instructions and passes specific to backend quantum computers.

## Code map

The following files implement functionality for registering quantum-computer-specific information and passes.

* `qret/target/target_enum.h`
* `qret/target/target_machine.h`
  * `qret::TargetMachine` : class that holds information about a target machine
* `qret/target/target_registry.h`
  * `qret::Target` : class that holds target-specific information
    * `name` : target identifier, for example `sc_ls_fixed_v0` or `tutorial_nisq_v0`
    * `short_desc` : short description of the target
    * `asm_printer_ctor` : function that creates `qret::AsmPrinter` (optional capability)
    * `CreateAsmStreamer()` : creates the `qret::AsmStreamer` used for asm output
    * `HasAsmPrinter()` / `TryCreateAsmPrinter()` : safely checks whether asm functionality is available
  * `qret::TargetRegistry` : target registry class
    * `target_map` : target name -> `qret::Target`
    * `compile_backend_map` : target name -> `qret::TargetCompileBackend`
    * Provides `RegisterTarget`, `RegisterCompileBackend`, and `RegisterAsmPrinter`.
    * Centrally manages compile backends and optional asm printer capability.

## Backends

* SC_LS_FIXED_V0: implemented in `qret/target/sc_ls_fixed_v0/`
* TUTORIAL_NISQ_V0: implemented in `qret/target/tutorial_nisq_v0/`
