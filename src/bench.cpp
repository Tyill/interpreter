//
// Interpreter Project — informal micro-benchmarks (local use).
//
#include "../include/interpreter.h"
#include "../include/base_library/arithmetic_operations.h"
#include "../include/base_library/comparison_operations.h"
#include "../include/base_library/containers.h"

#include <chrono>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct BenchHost {
  Interpreter ir;
  InterpreterBaseLib::ArithmeticOperations ao;
  InterpreterBaseLib::ComparisonOperations co;
  InterpreterBaseLib::Container bc;

  BenchHost()
      : ir(),
        ao(ir),
        co(ir),
        bc(ir) {
    ir.addFunction("summ", [](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {
      int64_t res = 0;
      for (const auto& v : args) {
        res += Interpreter::valueAsInt64(v);
      }
      return res;
    });
    ir.setMacro("inc", "$a = $a + 1;");
  }
};

void runBench(const char* label, int iterations, const std::function<void()>& fn) {
  constexpr int kWarmup = 50;
  for (int i = 0; i < kWarmup; ++i) {
    fn();
  }

  const auto t0 = Clock::now();
  for (int i = 0; i < iterations; ++i) {
    fn();
  }
  const auto t1 = Clock::now();

  const double sec = std::chrono::duration<double>(t1 - t0).count();
  const double ops = iterations / sec;
  std::printf("  %-28s %8d iter  %9.3f ms  %10.0f ops/s\n",
    label, iterations, sec * 1000.0, ops);
}

void benchCmdChanging(Interpreter& ir, int iterations) {
  static const char* scripts[] = {
    "$a=0;$a+=1;$a",
    "$a=0;$a+=2;$a",
    "$a=0;$a+=3;$a",
  };
  runBench("cmd (varying script)", iterations, [&]() {
    static int idx = 0;
    const auto r = ir.cmd(scripts[idx % 3]);
    ++idx;
    if (Interpreter::hasError(r.second)) {
      return;
    }
  });
}

void benchCmdSame(Interpreter& ir, const char* script, const char* label, int iterations) {
  runBench(label, iterations, [&]() {
    const auto r = ir.cmd(script);
    if (Interpreter::hasError(r.second)) {
      return;
    }
  });
}

void benchParseRunSame(Interpreter& ir, const char* script, int iterations) {
  runBench("parseScript+runScript (cached)", iterations, [&]() {
    Interpreter::Error err;
    if (!ir.parseScript(script, err)) {
      return;
    }
    ir.runScript();
  });
}

} // namespace

int main() {
  BenchHost host;
  Interpreter& ir = host.ir;

  static const char kShortLoop[] =
    "$a = 5; $b = 2; while ($a > 1) { $a = $a - 1; $b = summ($b, $a); if ($a < 4) { break; } } $b;";
  static const char kNested[] =
    "$a = 5; $b = 2; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;";
  static const char kVector[] =
    "a = Vector{1 + 2, 2 + 3, 3 + 4}; a[2];";
  static const char kMap[] =
    "$b = 12; c = Map{ one : $b + 5, two : 2 }; c[\"one\"];";
  static const char kMacro[] =
    "$a = 5; #inc; #inc; $a;";

  constexpr int kIter = 5000;

  std::printf("interpreter bench (O2, %d iterations per case)\n\n", kIter);

  benchCmdSame(ir, kShortLoop, "cmd short loop", kIter);
  benchCmdSame(ir, kNested, "cmd nested expr", kIter);
  benchCmdSame(ir, kVector, "cmd Vector literal", kIter / 5);
  benchCmdSame(ir, kMap, "cmd Map literal", kIter / 5);
  benchCmdSame(ir, kMacro, "cmd setMacro #inc", kIter);
  benchCmdChanging(ir, kIter / 10);
  benchParseRunSame(ir, kShortLoop, kIter);

  std::printf("\ncopy: ");
  const auto t0 = Clock::now();
  constexpr int kCopyIter = 500;
  for (int i = 0; i < kCopyIter; ++i) {
    Interpreter copy = ir;
    (void)copy;
  }
  const auto t1 = Clock::now();
  const double copySec = std::chrono::duration<double>(t1 - t0).count();
  std::printf("%d Interpreter copies in %.3f ms (%.0f /s)\n",
    kCopyIter, copySec * 1000.0, kCopyIter / copySec);

  std::printf("\n(done — for regression comparison only, not CI)\n");
  return 0;
}
