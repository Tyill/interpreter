# Benchmarks

Informal local throughput checks for regression comparison. **Not run in CI.**

Build and run:

```bash
cd src && make bench && ./bench
```

Source: [`src/bench.cpp`](../src/bench.cpp). The host keeps `ArithmeticOperations`, `ComparisonOperations`, and `Container` alive for the whole run (same lifetime pattern as tests).

## Scenarios

| Label | What it measures |
|-------|------------------|
| cmd short loop | `while` + `summ` + `break` (README-style loop) |
| cmd nested expr | Nested `summ` and arithmetic |
| cmd Vector literal | `Vector{1+2, …}` and index |
| cmd Map literal | `Map{one:$b+5, …}` and lookup |
| cmd setMacro #inc | `setMacro("inc", …)` then `#inc` twice |
| cmd (varying script) | Rotating three similar scripts (no AST cache reuse) |
| parseScript+runScript (cached) | Same script text: parse once per call path, then `runScript` |
| copy | `Interpreter` copy constructor only |

Default iteration counts: **5000** per `cmd` case (1000 for Vector/Map, 500 for varying script). Each case warms up **50** iterations before timing.

## Reference results

Snapshot from `g++ -std=c++17 -O2` on Linux x86_64 (single run; **your numbers will differ** by CPU and load):

```
interpreter bench (O2, 5000 iterations per case)

  cmd short loop                   5000 iter     14.424 ms      346648 ops/s
  cmd nested expr                  5000 iter      9.555 ms      523299 ops/s
  cmd Vector literal               1000 iter      8.718 ms      114700 ops/s
  cmd Map literal                  1000 iter      8.550 ms      116965 ops/s
  cmd setMacro #inc                5000 iter      6.557 ms      762500 ops/s
  cmd (varying script)              500 iter      1.529 ms      327061 ops/s
  parseScript+runScript (cached)     5000 iter     15.092 ms      331305 ops/s

copy: 500 Interpreter copies in 1.018 ms (490951 /s)
```

### How to read this

- **ops/s** — how many times per second the scenario completed (one `cmd` or one parse+run pair).
- Use results only to compare **before vs after** a change on the **same** machine and `bench` binary.
- These micro-benchmarks do **not** compare this library to Lua, Python, or other engines.

## See also

- [Getting started](getting-started.md)
- [Limitations](limitations.md)
