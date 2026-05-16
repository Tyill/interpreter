# Script language

Syntax reference for scripts passed to `Interpreter::cmd` or `parseScript`. Behavior is covered by [`src/test.cpp`](../src/test.cpp) (especially `readmeExamplesTest`).

## Statements and expressions

- Scripts are a sequence of statements.
- Statements end with `;`.
- Whitespace outside strings is ignored. Line comments start with `//` (to end of line).
- The value of the last expression in a script is the result of `cmd` / `runScript`.
- Parentheses `()` group expressions and raise precedence inside them.

## Variables

Names start with `$`:

```text
$a = 5;
$b = "hello";
```

Initializer syntax:

```text
$a{12};    // same idea as $a = 12 with parse-time init
```

Non-`$` names are ordinary identifiers (containers, struct fields, labels). Names like `trueVar` are **not** the boolean literals — only the exact tokens `true` and `false` count.

## Boolean literals

The keywords **`true`** and **`false`** are boolean literals (no `$` prefix):

```text
$x = true;
$y = false;
if (false) { $a = 1; }
while (true) { break; }
```

- Stored as `bool` in [`Value`](cpp-api.md#value-type).
- `cmdResultToString` prints them as **`1`** and **`0`** (same as numeric truthiness).
- Usable anywhere an expression is expected, including `if` / `while` conditions and comparisons (`$a == true`).
- **Reserved:** you cannot `addFunction("true", …)` or `addOperator("false", …)`; registration fails.

## Arithmetic and comparisons

Require [ArithmeticOperations](base-library.md#arithmetic_operationsh) and [ComparisonOperations](base-library.md#comparison_operationsh):

```text
$a = 5; $b = 2;
$c = $a * (2 + $b);
$d = $a == $b;
```

## Host functions

Registered in C++ with `addFunction`. Called from script:

```text
$c = summ($a, $b, summ(5, 3));
```

Arguments are expressions; the host receives evaluated [`Value`](cpp-api.md#value-type) values.

## Script functions

Define with `function` and a braced body:

```text
$a = 1; $b = 2;
function myFunc { $a += $b; };
myFunc();
```

Parameters are positional: `$0`, `$1`, …

```text
function add { $0 += $1; };
add(2, 3);
```

Nested functions are allowed. Recursion works if the host function is visible inside the script function body.

## Host operators

Registered in C++ with `addOperator(name, callback, priority)`. Base library registers `=`, `.`, `[`, `:`, etc.

```text
$c = 5;
$c += 5;
```

## Control flow

| Construct | Meaning |
|-----------|---------|
| `if (condition) { body }` | Run body if condition is truthy |
| `elseif (condition) { body }` | Else branch chain |
| `else { body }` | Final else |
| `while (condition) { body }` | Loop while truthy |
| `break;` | Exit innermost loop |
| `continue;` | Next loop iteration |

Conditions use [truthy](cpp-api.md#value-type) rules (non-zero numbers, non-empty strings, `true`, …).

**Container iteration**

```text
while ($v : a) print($v);
```

`$v` receives each element (Vector) or key/value pair context (Map) depending on container type.

## Macros

**Declare in script**

```text
#macro myMac { $c = 5; $d = $c + 5; };
$c = 1; #myMac;
```

**Invoke**

```text
#myMac;
```

**With parameters** (`$0`, `$1`, …)

```text
#macro myMacr { $a = $a + $0 + $0 + $1; };
$a = 5; #myMacr(3, 4);
```

**From C++**

```cpp
ir.setMacro("inc", "$a = $a + 1;");
// script: #inc   (name with or without #)
```

Macros are expanded before parse. Bodies are cleaned the same way as normal scripts.

## Goto

Labels must start with `l_` and end with `:`:

```text
if ($a == 3) {
  goto l_done;
}
l_done: $a = 0;
```

## Containers

Requires [Container](base-library.md#containersh--container). See [Base library](base-library.md) for method tables.

**Vector**

```text
a = Vector;
a.push_back(1);
a.push_back(2);
a[2];

a = Vector{ 1 + 2, 2 + 3, 3 + 4 };
```

**Map**

```text
b = Map;
b.insert(myKey, myValue);
b["myKey"];

$b = 12;
c = Map{ one : $b + 5, two : 2 };
c["one"];
```

**Literals**

Comma-separated elements in `{ … }`. Map entries use `key : value` segments.

## Struct

Requires [Structure](base-library.md#structureh--structure):

```text
e = Struct{ one : 5, two : 2 };
e.one = summ(e.one, e.two);
e.three = 22;
```

## Filesystem

Requires [Filesystem](base-library.md#filesystemh--filesystem):

```text
file1 = File{"main.cpp"};
if (file1.exist()) {
  $data = file1.read();
}
```

## Types

Requires [Types](base-library.md#typesh--types):

```text
a: int = 123;
b: str = "abc";
type(a);
```

## Attributes

Optional metadata on entities (advanced). Register names with `addAttribute` in C++.

## Print and script results

`print` is not built in; the host defines it (see [`example/main.cpp`](../example/main.cpp)). Scripts that only call `print` in a loop often return `0` from `cmd` because the last value is from the loop, not the printed text. Output goes to the host’s `printf` (or similar). See [Limitations](limitations.md).

## See also

- [Getting started](getting-started.md)
- [C++ API](cpp-api.md)
- [Limitations](limitations.md)
