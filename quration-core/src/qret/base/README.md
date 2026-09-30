\page qret_base base module

Provides basic functionality used widely across all modules.

## Code map

* `qret/base/cast.h` : functions for explicit casts
* `qret/base/graph.h` : graph library
  * `qret::DiGraph` : undirected graph class
  * `qret::Graph` : directed graph class
* `qret/base/gridsynth.h` : wrapper for `GridSynth`
  * The wrapper functions can be used only when the `GRIDSYNTH_PATH` environment variable is set appropriately.
* `qret/base/iterator.h` : iterator for `std::list<std::unique_ptr<T>>`
  * Dereferencing a `std::list` iterator yields `std::unique_ptr<T>&` or `const std::unique_ptr<T>&`, while dereferencing this iterator yields `T&` or `const T&`.
* `qret/base/json.h` : serializer and deserializer for the `qret::PortableFunction` class
* `qret/base/list.h` : utility functions for lists
* `qret/base/log.h` : logger
* `qret/base/option.h` : provides a way to set option values from command-line arguments
* `qret/base/portable_function.h` : classical functions that can be serialized to and deserialized from JSON
  * `qret::PortableFunction` : classical function class
  * `qret::PortableFunctionBuilder` : class for creating `qret::PortableFunction`
  * `qret::PortableAnnotatedFunction` : `qret::PortableFunction` that takes an array of bool values and outputs an array of bool values
* `qret/base/string.h` : string-related functions
* `qret/base/system.h` : wrapper functions for executing binaries
* `qret/base/time.h` : time-related functions
* `qret/base/type.h` : basic types `qret::Int`, `qret::BigInt`, and `qret::BigFraction`, and their conversion functions
