# Getting started

## What this is

A small embeddable command interpreter: you register C++ functions and operators, then run script strings that call them. The core library is a single header plus one translation unit:

- [`include/interpreter.h`](../include/interpreter.h)
- [`src/interpreter.cpp`](../src/interpreter.cpp)

Optional features (math, `Vector`/`Map`, files, structs, typed variables) live under [`include/base_library/`](../include/base_library/). See [Base library](base-library.md).

## Build and test

On Debian/Ubuntu (same as [CI](../.github/workflows/ci.yml)):

```bash
sudo apt-get update && sudo apt-get install -y libgtest-dev g++
cd src && make && ./test
```

To compile your own program, add `interpreter.cpp` and include paths, C++17 or later:

```bash
g++ -std=c++17 -O2 -I. your_app.cpp src/interpreter.cpp -o your_app
```

## Minimal host program

Pattern from [`example/main.cpp`](../example/main.cpp):

```cpp
#include "include/interpreter.h"
#include "include/base_library/arithmetic_operations.h"
#include "include/base_library/comparison_operations.h"

int main() {
  Interpreter ir;

  // Register base library on the same ir you will execute.
  InterpreterBaseLib::ArithmeticOperations ao(ir);
  InterpreterBaseLib::ComparisonOperations co(ir);

  ir.addFunction("summ", [](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {
    int64_t res = 0;
    for (const auto& v : args) {
      res += Interpreter::valueAsInt64(v);
    }
    return res;
  });

  const char* script =
    "$a = 5; $b = 2; "
    "while ($a > 1) { $a = $a - 1; $b = summ($b, $a); if ($a < 4) { break; } } "
    "$b;";

  const auto result = ir.cmd(script);
  if (Interpreter::hasError(result.second)) {
    // handle parse/runtime error
    return 1;
  }
  // result.first holds the last expression value; string form often easiest:
  std::string res = Interpreter::cmdResultToString(result);  // "9"
  return 0;
}
```

Important:

- Host callbacks use [`Interpreter::Value`](../include/interpreter.h) (`std::variant` of `bool`, `int64_t`, `double`, `std::string`, and control-flow tokens), not raw `std::string`. See [C++ API](cpp-api.md#value-type).
- Scripts accept boolean literals **`true`** and **`false`** ([details](script-language.md#boolean-literals)).
- Construct every base-library helper (`Container`, `Filesystem`, …) on the **same** `Interpreter` instance you call `cmd()` on.

## Running scripts

### `cmd(script)` — usual path

`cmd` cleans the script, parses if the text changed, then runs. Returns `CmdResult` = `{ Value, Error }`.

```cpp
auto r = ir.cmd("$a = 1; $a + 2;");
if (!Interpreter::hasError(r.second)) {
  std::string s = Interpreter::cmdResultToString(r);  // "3"
}
```

### `parseScript` + `runScript` — same text many times

If you execute the **same** script string repeatedly, parse once and run many times (AST is cached when `m_prevScript` matches):

```cpp
Interpreter::Error err;
if (ir.parseScript("$a = 0; while ($a < 10) { $a += 1; } $a;", err)) {
  auto r = ir.runScript();
  // ...
}
```

Use `cmd` when the script text changes on each call.

## What to register

| Need | Register |
|------|----------|
| `+`, `-`, `*`, `/`, etc. | `ArithmeticOperations` |
| `==`, `<`, `>`, … | `ComparisonOperations` |
| `Vector`, `Map` | `Container` |
| `Struct` fields | `Structure` |
| `File`, `Dir` | `Filesystem` |
| `a: int = …`, `type(a)` | `Types` |
| Custom logic | `addFunction` / `addOperator` |

You can omit modules you do not use. Scripts that call undefined functions or operators fail at parse or runtime.

## Next steps

- [Script language reference](script-language.md)
- [C++ API details](cpp-api.md)
- [Limitations](limitations.md) (return values vs `print`, copy + containers, …)
- More examples: [`example/main.cpp`](../example/main.cpp), [`src/test.cpp`](../src/test.cpp) (`readmeExamplesTest`)

## Testing

```bash
cd src && make && ./test
```

Google Test suite (34 tests) covers language features, base library, and API edge cases. When in doubt about behavior, check the matching test in `src/test.cpp`.

## Benchmarks (local)

```bash
cd src && make bench && ./bench
```

Details and reference numbers: [Benchmarks](benchmarks.md).
