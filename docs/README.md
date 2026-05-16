# Documentation

English guides for embedding and using the Tiny interpreter library.

| Document | Description |
|----------|-------------|
| [Getting started](getting-started.md) | Build, minimal host setup, running scripts |
| [Script language](script-language.md) | Syntax: variables, control flow, macros, containers |
| [C++ API](cpp-api.md) | `Interpreter`, `Value`, errors, host functions and operators |
| [Base library](base-library.md) | Optional modules: arithmetic, containers, filesystem, … |
| [Limitations](limitations.md) | Known quirks and edge cases |
| [Copy and containers](copy-and-containers.md) | `Interpreter` copy vs `Map` / `Vector` variable visibility |
| [Benchmarks](benchmarks.md) | Local `make bench` throughput (reference numbers) |

**Quick links**

- Core: [`include/interpreter.h`](../include/interpreter.h), [`src/interpreter.cpp`](../src/interpreter.cpp)
- Full sample: [`example/main.cpp`](../example/main.cpp)
- Tests: [`src/test.cpp`](../src/test.cpp)
