\page qret_codegen codegen module

Provides parent classes shared by target quantum computers.
Target-specific implementations are placed under `qret/target/`.

## Overview

* `qret/codegen/asm_printer.h`
  * `qret::AsmPrinter` : class that outputs assembly to a stream
* `qret/codegen/asm_streamer.h`
  * `qret::AsmStreamer` : stream class for assembly
  * Also stores output formatting such as separators, comment markers, and indentation width.
  * `ToString()` returns the generated text; file I/O is handled by the caller.
* `qret/codegen/compile_info.h`
* `qret/codegen/dummy.h`
* `qret/codegen/machine_function_pass.h`
  * `qret::MachineFunctionPass` : parent class for passes over target functions
  * `qret::MFAnalysis` : records the execution order and execution time of `qret::MachineFunctionPass`
  * `qret::MFPassManager`
    * Holds multiple `qret::MachineFunctionPass` objects and runs passes in an appropriate order.
    * Returns `qret::MFAnalysis` after execution.
* `qret/codegen/machine_function.h`
  * `qret::MachineInstruction` : parent class for target instructions
  * `qret::MachineBasicBlock` : parent class for target basic blocks
  * `qret::MachineFunction` : parent class for target functions
