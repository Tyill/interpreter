# Limitations and quirks

Behaviors that are easy to misunderstand but are intentional or documented in tests. No surprises when embedding the library.

## Copy and containers

Copying `Interpreter` duplicates variables, macros, and operators, but **Map/Vector init** still uses the interpreter instance bound when `Container` / `Structure` were constructed.

**Summary:** On a copy, container literals see parent variables only if those variables were set on the same `ir` that `m_intr` points to.

Full explanation: **[Copy and containers](copy-and-containers.md)**.

Variable isolation on copy (`$a` on `ir` vs `ir2`) works; shared container storage and `m_intr` are the subtle parts.

## `cmd` return value vs stdout

- `cmd` / `runScript` return the last expression’s [`Value`](cpp-api.md#value-type), not console output.
- `print` is a **host** function; typical implementations use `printf`. Tests expect loop+print scripts to return `"0"` while lines appear on stdout (`printStdoutTest`).
- To assert output, capture stdout in tests or log inside your `print` callback.

## Container access errors

Out-of-range Vector index or missing Map key often yields an **empty string** from `cmdResultToString`, not a `Runtime` error with a message. See `containerEdgeTest` in [`src/test.cpp`](../src/test.cpp).

## Numeric types

- Values are `int64_t`, `double`, or `string` at runtime ([`Value`](cpp-api.md#value-type)).
- Mixed arithmetic follows implementation rules in base library and tests; float vs int promotion may differ from C++ (e.g. `1.5 * 2` vs `1.5 / 2` — see `comparisonSupplementTest`).
- String `"5"` and integer `5` can compare equal via `valueEquals`.

## Macros

- `setMacro("inc", body)` and `setMacro("#inc", body)` are equivalent.
- Macro bodies are whitespace-cleaned when expanded; match normal script formatting.
- Unknown `#name` at expand time is a parse error.

## Parse cache

`parseScript` skips re-parse when the script text equals the previous successful parse. After changing macros or operators, run a new script string or use `cmd` with changed text so the AST refreshes.

## C API

[`interpreter_c.h`](../include/interpreter_c.h) wraps the C++ engine for FFI. It is smoke-tested in CI but not documented in depth here; prefer the C++ API for new integrations.

## See also

- [Getting started](getting-started.md)
- [Script language](script-language.md)
- [Copy and containers](copy-and-containers.md)
