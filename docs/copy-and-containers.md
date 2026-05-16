# Interpreter copy and containers (Map / Vector / Struct)

## Summary

**On a copied interpreter, Map/Vector init expressions see the parent’s variables only if those variables were set on the same `ir` instance that `m_intr` points to.**

In other words: when `cmd()` runs on a copy (`ir2`), expressions inside `Map{ … }` / `Vector{ … }` are evaluated from a snapshot of the **original** interpreter bound in `Container` / `Structure`, not from the copy’s `m_var`.

## How it works

Base library code ([`containers.h`](../include/base_library/containers.h), [`structure.h`](../include/base_library/structure.h), [`filesystem.h`](../include/base_library/filesystem.h)) stores a reference at construction:

```cpp
Container(Interpreter& ir) : m_intr(ir) { … }
```

When a container literal is initialized:

```cpp
Interpreter intrCopy = m_intr;
intrCopy.parseScript(expression_from_braces, err);
value = intrCopy.runScript().first;
```

- `m_intr` is always the **same** `Interpreter` passed to the `Container` constructor (in tests and `example/main.cpp`, that is the first `ir`).
- A script may run on a **copy** (`Interpreter ir2 = ir`), but `intrCopy` and `currentEntity()` inside operators still use **`m_intr`**, not `ir2`.

`Map` / `Vector` storage (`m_mapContr`, `m_vectorContr`) lives on the `Container` object, not inside `Interpreter::Impl`. Copying an interpreter does not duplicate that storage; copies that share one `Container` share the same maps.

## What works

| Scenario | Result |
|----------|--------|
| `$b = 12` on **`ir`**, then `ir2 = ir`, then on **`ir2`**: `Map{ one : $b + 5 }` | OK: `m_intr` (`ir`) already has `$b = 12` → `17` |
| `$b = 12` on the **same** `ir`, then `Map{ one : $b + 5 }` on that `ir` | OK (`intrCopy` is the same interpreter) |

## What does not work (current behavior)

| Scenario | Result |
|----------|--------|
| `ir2 = ir`, then **only** `ir2.cmd("$b = 12")`, then `Map{ one : $b + 5 }` on `ir2` | `$b` is empty on `m_intr` (`ir`) → wrong or empty result |
| Two copies with different `c = Map{ … }` under the same name `c` | Shared `Container` storage → data can overwrite each other |

## Diagram

```
  ir  ←── m_intr (fixed reference in Container)
  ↑
  │  intrCopy = m_intr  →  variables read from ir
  │
  ir2  ←── cmd() runs here; m_var ($b) may exist only on ir2
```

## Tests

In [`src/test.cpp`](../src/test.cpp), `interpreterCopyMoveTest`:

- asserts **intrCopy on a single `ir`** (`$b = 12` → `Map` → `"17"`);
- does **not** claim support for “`$b` only on `ir2` after copy”.

## See also

- [Documentation index](README.md)
- [Limitations](limitations.md)
- [Base library — Container](base-library.md#containersh--container)

This behavior is **documented as current**; no change to container binding is planned unless requirements change.
