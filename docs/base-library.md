# Base library

Optional headers under [`include/base_library/`](../include/base_library/) extend the core interpreter with operators and functions. Each class takes `Interpreter&` in its constructor and registers handlers on that instance.

**Rule:** construct every helper on the **same** `Interpreter` you pass scripts to. Example from [`example/main.cpp`](../example/main.cpp):

```cpp
Interpreter ir;
InterpreterBaseLib::ArithmeticOperations ir_ao(ir);
InterpreterBaseLib::ComparisonOperations ir_co(ir);
InterpreterBaseLib::Container ir_bc(ir);
InterpreterBaseLib::Types ir_tp(ir);
InterpreterBaseLib::Filesystem ir_fs(ir);
InterpreterBaseLib::Structure ir_st(ir);
```

Order among helpers does not matter; all must exist before running scripts that need them.

## arithmetic_operations.h

Registers standard arithmetic and compound assignment operators (`+`, `-`, `*`, `/`, `+=`, …) on numeric [`Value`](../include/interpreter.h) types.

Required for almost all numeric scripts.

## comparison_operations.h

Registers comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`). Results are usable in `if` / `while` conditions.

## containers.h — `Container`

Enables `Vector` and `Map` in scripts.

**Construction / literals**

```text
a = Vector;
a = Vector{ 1, 2, 3 };
a = Vector{ 1 + 2, 2 + 3 };

c = Map;
c = Map{ keyOne : valueOne, keyTwo : 2 };
```

Expressions inside `{ … }` are evaluated in a snapshot of the bound interpreter (see [Copy and containers](copy-and-containers.md) when using copies).

**Vector methods**

| Method | Description |
|--------|-------------|
| `push_back(value)` | Append |
| `pop_back()` | Remove last |
| `insert(index, value)` | Insert at index |
| `erase(index)` | Remove at index |
| `size()`, `empty()`, `clear()` | Queries / reset |
| `at(index)` or `[index]` | Access |

**Map methods**

| Method | Description |
|--------|-------------|
| `insert(key, value)` | Insert or replace |
| `erase(key)` | Remove key |
| `size()`, `empty()`, `clear()` | Queries / reset |
| `at(key)` or `[key]` | Access |

**Iteration**

```text
while ($v : a) print($v);
```

For `Map`, printed lines are typically `key` and value (tab-separated in host `print` output). See [Limitations](limitations.md).

Syntax details: [Script language — Containers](script-language.md#containers).

## structure.h — `Structure`

Struct-like objects with dotted fields:

```text
e = Struct{ one : 5, two : 2 };
e.one = summ(e.one, e.two);
e.three = e.one + e.two + 3;
```

Fields can be added after construction. Init expressions in `{ … }` follow the same snapshot rules as containers.

## filesystem.h — `Filesystem`

**File**

```text
f = File{"path.txt"};
f.exist();
$data = f.read();
f.write($data);
f.remove();
```

**Dir**

```text
d = Dir{"path"};
d.exist();
d.remove();
```

Paths in literals are usually quoted strings. Init bodies can be expressions.

## types.h — `Types`

Typed variable declarations and `type()`:

```text
a: int = 123;
b: str = "abc";
type(a);   // host function returns type name
```

Registers `:` for declarations and the `type` function.

## Linking

Add `interpreter.cpp` once per binary. Base library is header-only (templates and inline registration in constructors); no extra `.cpp` files.

## See also

- [Getting started](getting-started.md)
- [Script language](script-language.md)
- [C++ API](cpp-api.md)
