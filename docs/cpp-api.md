# C++ API

Reference for embedding [`Interpreter`](../include/interpreter.h) from C++. Samples match [`example/main.cpp`](../example/main.cpp).

## Lifecycle

```cpp
Interpreter ir;                          // default construct
Interpreter ir2 = ir;                    // copy state (variables, macros, operators, …)
Interpreter ir3 = std::move(ir);         // move
ir = other;                              // copy assign
```

Copy semantics for containers are special; see [Copy and containers](copy-and-containers.md).

## Registration

### Functions

```cpp
using UserFunction = std::function<Value(const std::vector<Value>&)>;

bool ok = ir.addFunction("summ", [](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {
  int64_t n = 0;
  for (const auto& v : args) n += Interpreter::valueAsInt64(v);
  return n;
});
```

Script call: `summ(1, 2, 3);`

### Operators

```cpp
using UserOperator = std::function<Value(Value& left, Value& right)>;

ir.addOperator("+=", [](Interpreter::Value& l, Interpreter::Value& r) -> Interpreter::Value {
  // update l, return result
  return l;
}, priority);
```

Higher `priority` runs earlier when multiple operators can match. Base library operators use priorities documented in [Base library](base-library.md).

### Attributes

```cpp
ir.addAttribute("myAttr");
```

Used with reflection / entity metadata (advanced).

## Execution

| Method | Purpose |
|--------|---------|
| `CmdResult cmd(std::string script)` | Parse (if needed) + run; returns `{ Value, Error }` |
| `bool parseScript(std::string script, Error& outErr)` | Parse only; cache AST when script text unchanged |
| `std::pair<Value, Error> runScript()` | Run after successful `parseScript` |

```cpp
auto r = ir.cmd("$a = 1;");
if (Interpreter::hasError(r.second)) {
  // r.second.kind == Parse or Runtime
  // r.second.position — character index for parse errors
  // r.second.message
}
```

Helpers:

| Helper | Role |
|--------|------|
| `cmdResultToString(const CmdResult&)` | String form of result value (empty on error) |
| `hasError(const Error&)` | `true` if `message` non-empty |
| `isParseError(const Error&)` | Parse vs runtime |

## Variables and macros

```cpp
ir.setVariable("$x", int64_t{42});
Interpreter::Value v = ir.variable("$x");
auto all = ir.allVariables();

ir.setMacro("inc", "$a = $a + 1;");   // stored as #inc
ir.setMacro("#twice", "2");           // also valid
// script: #inc or #twice + 3
```

`setMacro` accepts names with or without a leading `#`.

## Host control

```cpp
ir.gotoOnLabel("l_myLabel");  // must exist in parsed script
ir.exitFromScript();          // stop current run
ir.runFunction("summ", { int64_t{1}, int64_t{2} });
```

## Value type

`Interpreter::Value` is a `std::variant` of:

| Alternative | Use |
|-------------|-----|
| `bool` | Boolean literals `true` / `false` in scripts, conditions, `setVariable` |
| `int64_t` | Integers |
| `double` | Floating point |
| `std::string` | Text, names, mixed results |
| `ControlFlow::Break` / `Continue` | Loop control inside interpreter |

Common static helpers:

| Function | Notes |
|----------|--------|
| `valueToString(v)` | Display string (`bool` → `"1"` / `"0"`) |
| `valueFromLiteral("true")` / `"false"` | Parse boolean literals |
| `valueAsInt64(v)` | Coerce numeric types |
| `valueAsString(v)` | String or stringified value |
| `valueIsInteger` / `valueIsNumeric` | Type checks |
| `valueIsTruthy(v)` | Condition semantics |
| `valueFromLiteral(s)` | Parse literal text |
| `valueEquals` / `valueCompare` | Comparison |
| `makeParam("name")` | Build operator parameter lists |

Host functions should return a `Value` appropriate to the script context (often `int64_t` or `std::string`).

## Reflection (advanced)

For tools that inspect or steer execution:

- `allEntities()`, `currentEntity()`, `getEntityByIndex`
- `Entity` — `beginIndex`, `type` (`EntityType`), `name`, `value`, …
- `gotoOnEntity(index)`, `getUserFunction`, `getUserOperator`
- `getAttributeByIndex`

Most applications only need `cmd` and host callbacks.

## C API

A thin C wrapper lives in [`include/interpreter_c.h`](../include/interpreter_c.h) and [`src/interpreter_c.cpp`](../src/interpreter_c.cpp). CI smoke-compiles it; there is no separate C tutorial. Use the C++ API above for new code.

## See also

- [Getting started](getting-started.md)
- [Script language](script-language.md)
- [Limitations](limitations.md)
