\page qret_ir IR module

Implements the intermediate representation for quantum circuits in the QRET library.
For the design of the intermediate representation and the instruction set, see "Intermediate Representation for Quantum Circuits".

## Code map

* `qret/ir/basic_block.h`
  * `qret::ir::BasicBlock` : basic block class
    * A basic block is an instruction list that belongs to a function.
    * Instructions in a basic block execute sequentially without branching.
    * In a well-formed basic block, the instruction list ends with a terminator instruction.
    * In a well-formed basic block, no terminator instruction appears in the middle of the instruction list.
* `qret/ir/function.h`
  * `qret::ir::Function` : function class
* `qret/ir/context.h`
  * `qret::ir::IRContext` : context for the intermediate representation
* `qret/ir/function_pass.h`
  * `qret::ir::FunctionPass` : parent class for function passes
    * Passes that optimize functions inherit from this class.
* `qret/ir/instruction.h`
  * `qret::ir::Instruction` : parent class for instruction classes
* `qret/ir/instructions.h` : various instruction classes
  * `qret::ir::MeasurementInst`
  * `qret::ir::PauliProductMeasurementInst`
  * `qret::ir::UnaryInst`
  * `qret::ir::ParametrizedRotationInst`
  * `qret::ir::BinaryInst`
  * `qret::ir::TernaryInst`
  * `qret::ir::MultiControlInst`
  * `qret::ir::GlobalPhaseInst`
  * `qret::ir::CallInst`
  * `qret::ir::CallCFInst`
  * `qret::ir::DiscreteDistInst`
  * `qret::ir::BranchInst`
  * `qret::ir::SwitchInst`
  * `qret::ir::ReturnInst`
  * `qret::ir::CleanInst`
  * `qret::ir::CleanProbInst`
  * `qret::ir::DirtyInst`
* `qret/ir/json.h` : JSON serializer and deserializer for the intermediate representation
* `qret/ir/metadata.h` : metadata attached to instructions
* `qret/ir/module.h`
  * `qret::ir::Module` : module
* `qret/ir/value.h` : defines values in the intermediate representation
