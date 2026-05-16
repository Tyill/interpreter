//
// Interpreter Project
// Copyright (C) 2020 by Contributors <https://github.com/Tyill/interpreter>
//
// This code is licensed under the MIT License.
//
#include <gtest/gtest.h>
#include "../include/interpreter.h"
#include "../include/base_library/arithmetic_operations.h"
#include "../include/base_library/comparison_operations.h"
#include "../include/base_library/containers.h"
#include "../include/base_library/structure.h"
#include "../include/base_library/types.h"
#include "../include/base_library/filesystem.h"

#include <cstdio>
#include <ostream>
#include <unistd.h>

using namespace std;

bool cmdIs(Interpreter& ir, const string& script, const string& expected) {
  return Interpreter::cmdResultToString(ir.cmd(script)) == expected;
}

class InprTest : public ::testing::Test {
public:
  InprTest():
  ao_ir(ir),
  co_ir(ir),
  ts_ir(ir),
  bc_ir(ir),
  fs_ir(ir),
  st_ir(ir){

  ir.addOperator("->", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
    rightOpd = leftOpd;
    return rightOpd;
  }, 5);
  ir.addFunction("summ", [](const vector<Interpreter::Value>& args) -> Interpreter::Value {
    int64_t res = 0;
    for (auto& v : args) {
      res += Interpreter::valueAsInt64(v);
    }
    return res;
  });
  ir.addFunction("print", [](const vector<Interpreter::Value>& args) -> Interpreter::Value {
    for (auto& v : args) {
      printf("%s ", Interpreter::valueToString(v).c_str());
    }
    printf("\n");
    return std::string{};
  });
  Interpreter* pIr = &ir;
  ir.addFunction("setB", [pIr](const vector<Interpreter::Value>& args) -> Interpreter::Value {
    int64_t res = 0;
    for (auto& v : args) {
      if (Interpreter::valueIsInteger(v)) res += Interpreter::valueAsInt64(v);
    }
    pIr->setVariable("$b", res);
    return res;
  });
  ir.addFunction("range", [pIr](const vector<Interpreter::Value>& args) -> Interpreter::Value {
    int64_t maxv = 0;
    if (!args.empty() && Interpreter::valueIsInteger(args[0]))
      maxv = Interpreter::valueAsInt64(args[0]);

    auto entity = pIr->currentEntity();
    int64_t cval = 0;
    if (Interpreter::valueIsInteger(entity.value))
      cval = Interpreter::valueAsInt64(entity.value);

    return cval < maxv ? static_cast<int64_t>(cval + 1) : int64_t{0};
  });
  ir.addFunction("getAttr", [pIr](const vector<Interpreter::Value>&) -> Interpreter::Value {
    auto attrs = pIr->getAttributeByIndex(pIr->currentEntity().beginIndex - 1);
    std::ostringstream out;
    std::copy(attrs.begin(), attrs.end(), std::ostream_iterator<std::string>(out, ","));
    auto str = out.str();
    return str.substr(0, str.size()-1);
  });
  ir.addAttribute("attr1");
  ir.addAttribute("attr2");
  ir.addAttribute("attr3");
  }
  ~InprTest() {
  }
  Interpreter ir;
  InterpreterBaseLib::ArithmeticOperations ao_ir;
  InterpreterBaseLib::ComparisonOperations co_ir;
  InterpreterBaseLib::Types ts_ir;
  InterpreterBaseLib::Container bc_ir;
  InterpreterBaseLib::Filesystem fs_ir;
  InterpreterBaseLib::Structure st_ir;
};

TEST_F(InprTest, operatorTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; $a + $b", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; $a - $b", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 0; $a += 5; $b = 2; $a + $b", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 4; ++$a; $b = 2; $a + $b", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 4; $a++; $b = 2; $a + $b", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 0; $b = 0; $c = ($a == $b); $c", "1"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; $a->$b; $b", "5"));
  EXPECT_TRUE(cmdIs(ir, "$b = \"summ = \"; $a = $b + \"$a + $b\"; $a", "summ = $a + $b"));
  EXPECT_TRUE(cmdIs(ir, "$b = \"$a + $b\"; $a = \"summ = \" + $b; $a", "summ = $a + $b"));
}
TEST_F(InprTest, conditionTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if ($a == 5){ $b = 2; } $a + $b", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if ($a == 3){ $b = 2;} else { $a = 3;}; $a", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if ($a == 3){ $b = 2;} elseif($a == 5){ $a = 3;}; $a", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if ($a == 3){ $b = 2;} elseif($a != 4){ $a = 3;}; $a", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; if ($a == 1 + $b){ $b = 2; } elseif($a != 4){ $a = 3;}; $a", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ $a -= 1; $b += 1; } $b", "5"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ while($b < $a){ $b += 1; break;} $a -= 1;} $b", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ $a -= 1; $b += 1; if ($a == 1){ break;} } $b", "4"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ $a -= 1; $b += 1; if ($a == 1){ if ($a == 1){ break;}} } $b", "4"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ $a -= 1; if ($a == 2){continue;} $b += 1;} $b", "4"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if($a == 3){ $b = 3;} elseif($a == 5){ $b = 5;} elseif($a == 5){ $b = 4;} $b", "5"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ $a -= 1; if ($a == 2) continue; $b += 1;} $b", "4"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if($a == 3) $b = 3; elseif($a == 5) $b = 5; elseif($a == 5) $b = 4; $b", "5"));
}
TEST_F(InprTest, functionTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; summ($a, $b)", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; 1 + summ($a, $b)", "8"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; 1 + summ($a, $b) - 1", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; 1 + summ($a, $b, 1) - 1", "8"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; 1 + summ() + $b", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; setB($a - 1)", "4"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; 1 + summ(1, summ($a)) + $b", "9"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; summ(summ($a, summ($b)), summ($b, summ($a)))", "14"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; summ(summ($a + summ($b + 1, 1) + 3, summ($b)))", "14"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 0; while($a > 0){ $a -= summ(1); if ($a == 2){continue;} $b += summ(1);} $b", "4"));
}
TEST_F(InprTest, macrosTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 5; #macro myMacr{$a = $a + 2;} #myMacr; #myMacr; #myMacr;", "11"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; #macro myMacr{ $a = $a + $0 + $0 + $1; } #myMacr(3,4);", "15"));
  EXPECT_TRUE(cmdIs(ir, "#macro RANGE{while(range($0))}; $a = 0; #RANGE(100) $a += 1; $a;", "100"));
}
TEST_F(InprTest, gotoTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; goto l_jmp; $a = summ($a, $b); l_jmp: $a;", "5"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; while($a > 0){ $a -= 1; if ($a == 2){ goto l_jmp;}} l_jmp: $a; ", "2"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; goto l_cyc; l_jmp: goto l_exit; l_cyc: while($a > 0){ $a -= 1; if ($a == 2){ goto l_jmp;};}; l_exit: $a;", "2"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; while($a > 0){goto l_jmp1; l_jmp: $a = 10; goto l_exit; l_jmp1: $a -= 1; if ($a == 2){ goto l_jmp;}} l_exit: $a; ", "10"));
}
TEST_F(InprTest, reflectionTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 0; while(range(100)) $a += 1; $a;", "100"));
}
TEST_F(InprTest, containerTest){
  EXPECT_TRUE(cmdIs(ir, "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); a.size()", "3"));
  EXPECT_TRUE(cmdIs(ir, "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); a[1 + 1]", "3"));
  EXPECT_TRUE(cmdIs(ir, "b = Map; b.insert(myKeyOne, myValueOne); b.insert(myKeyTwo, myValueTwo); b.at(myKeyTwo)", "myValueTwo"));
  EXPECT_TRUE(cmdIs(ir, "b = Map; b.insert(myKeyOne, myValueOne); b.insert(myKeyTwo, myValueTwo); b[\"myKeyTwo\"]", "myValueTwo"));
}
TEST_F(InprTest, structureTest){
  EXPECT_TRUE(cmdIs(ir, "e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one", "7"));
  EXPECT_TRUE(cmdIs(ir, "$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = $b; e.three", "12"));
  EXPECT_TRUE(cmdIs(ir, "$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three", "22"));
}
TEST_F(InprTest, internFuncTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 1; $b = 2; function myFunc{ $a += $b; }; myFunc()", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 1; $b = 2; function myFunc{ $a += $b; function myFunc2{ $a += $b; }; myFunc2(); }; myFunc()", "5"));
  EXPECT_TRUE(cmdIs(ir, "function myFunc{ if ($0 > 1) $a = $0 * myFunc($0 - 1); else $a = 1; $a }; myFunc(5)", "120"));
  EXPECT_TRUE(cmdIs(ir, "function myFunc{ $0 += $1; }; myFunc(2, 3)", "5"));
}
TEST_F(InprTest, typesTest){
  EXPECT_TRUE(cmdIs(ir, "$a: int = 123; type($a)", "int"));
  EXPECT_TRUE(cmdIs(ir, "$b: str = \"abc\"; type($b)", "str"));
}
TEST_F(InprTest, attributesTest){
  EXPECT_TRUE(cmdIs(ir, "[attr1,attr2,attr3] getAttr()", "attr1,attr2,attr3"));
}
TEST_F(InprTest, literalTest){
  EXPECT_TRUE(cmdIs(ir, "$a = -42; $a;", "-42"));
  EXPECT_TRUE(cmdIs(ir, "$a = 3.14; $a;", "3.14"));
}
TEST_F(InprTest, parseTimeValueTest){
  EXPECT_TRUE(cmdIs(ir, "\"42\" + 1", "421"));
  EXPECT_TRUE(cmdIs(ir, "-5 + 10", "5"));
  EXPECT_TRUE(cmdIs(ir, "-3.14 + 3.14", "0"));
  EXPECT_TRUE(cmdIs(ir, "1.5 + 2", "3"));
  EXPECT_TRUE(cmdIs(ir, "$x = true; $x", "1"));
  EXPECT_TRUE(cmdIs(ir, "$y = false; $y", "0"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; $c = ($a == $b); $c", "0"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $c = ($a == 5); $c", "1"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; if (false) { $a = 1; } $a", "5"));
  EXPECT_FALSE(ir.addFunction("true", [](const vector<Interpreter::Value>&) -> Interpreter::Value {
    return int64_t{0};
  }));
}
TEST_F(InprTest, literalCmdRepeatTest) {
  const string script = "1 + 2;";
  const string expected = "3";
  EXPECT_EQ(Interpreter::cmdResultToString(ir.cmd(script)), expected);
  EXPECT_EQ(Interpreter::cmdResultToString(ir.cmd(script)), expected);
  EXPECT_EQ(Interpreter::cmdResultToString(ir.cmd(script)), expected);

  const string floatScript = "-5 + 10;";
  EXPECT_EQ(Interpreter::cmdResultToString(ir.cmd(floatScript)), "5");
  EXPECT_EQ(Interpreter::cmdResultToString(ir.cmd(floatScript)), "5");

  Interpreter::Error err;
  ASSERT_TRUE(ir.parseScript("-3.14 + 3.14;", err));
  EXPECT_FALSE(Interpreter::hasError(err));
  EXPECT_EQ(Interpreter::valueToString(ir.runScript().first), "0");
  EXPECT_EQ(Interpreter::valueToString(ir.runScript().first), "0");
}
TEST_F(InprTest, cleaningTest){
  EXPECT_TRUE(cmdIs(ir, "$a = \"a b\"; $a;", "a b"));
  EXPECT_TRUE(cmdIs(ir, "1+2;", "3"));
  EXPECT_TRUE(cmdIs(ir, "1 + 2;", "3"));
}
TEST_F(InprTest, unaryMinusTest){
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = -$a; $b;", "-5"));
  EXPECT_TRUE(cmdIs(ir, "$a = -(1 + 2); $a;", "-3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $b = 2; 1 + summ($a, $b) - 1", "7"));
}
TEST_F(InprTest, parseErrorTest){
  auto r = ir.cmd("if (");
  EXPECT_TRUE(r.second);
  EXPECT_FALSE(r.second.message.empty());
  EXPECT_TRUE(Interpreter::isParseError(r.second));

  r = ir.cmd("unknownFunc();");
  EXPECT_TRUE(r.second);
  EXPECT_FALSE(r.second.message.empty());

  r = ir.cmd("$a = 1; $a;");
  EXPECT_FALSE(r.second);
}
TEST_F(InprTest, parseScriptErrorTest) {
  Interpreter::Error err;
  EXPECT_FALSE(ir.parseScript("if (", err));
  EXPECT_TRUE(Interpreter::hasError(err));
  EXPECT_TRUE(Interpreter::isParseError(err));

  EXPECT_FALSE(ir.parseScript("$a = 1; unknownFunc();", err));
  EXPECT_TRUE(Interpreter::isParseError(err));
  EXPECT_GT(err.position, 0u);

  EXPECT_TRUE(ir.parseScript("$a = 5;", err));
  EXPECT_FALSE(Interpreter::hasError(err));
  const auto r1 = ir.runScript();
  EXPECT_FALSE(Interpreter::hasError(r1.second));
  EXPECT_EQ(Interpreter::valueToString(r1.first), "5");

  EXPECT_TRUE(ir.parseScript("$a = 5;", err));
  EXPECT_FALSE(Interpreter::hasError(err));
  const auto r2 = ir.runScript();
  EXPECT_EQ(Interpreter::valueToString(r1.first), Interpreter::valueToString(r2.first));
}
TEST_F(InprTest, filesystemTest){
  char path[] = "/tmp/ir_fs_test_XXXXXX";
  const int fd = mkstemp(path);
  ASSERT_NE(fd, -1);
  close(fd);
  remove(path);

  const string p = path;
  EXPECT_TRUE(cmdIs(ir, "f = File{\"" + p + "\"}; f.write(\"hello\"); f.exist();", "1"));
  EXPECT_TRUE(cmdIs(ir, "f = File{\"" + p + "\"}; f.read();", "hello"));
  EXPECT_TRUE(cmdIs(ir, "f = File{\"" + p + "\"}; f.write(\"hel\"); f.append(\"lo\"); f.read();", "hello"));
  EXPECT_TRUE(cmdIs(ir, "f = File{\"" + p + "\"}; f.remove(); f.exist();", "0"));
  EXPECT_TRUE(cmdIs(ir, "d = Dir{\"/tmp\"}; d.exist();", "1"));
}

TEST_F(InprTest, valueSemanticsTest) {
  EXPECT_TRUE(cmdIs(ir, "$x = \"5\"; $x + 2", "52"));
  EXPECT_TRUE(cmdIs(ir, "$a = 0; if (true) { $a = 1; } $a", "1"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; while (false) { $a = 1; } $a", "5"));
  EXPECT_FALSE(ir.addOperator("false", [](Interpreter::Value&, Interpreter::Value&) -> Interpreter::Value {
    return int64_t{0};
  }, 1));
  ir.setVariable("$t", true);
  EXPECT_TRUE(cmdIs(ir, "$t", "1"));
}

TEST_F(InprTest, comparisonAndArithmeticTest) {
  EXPECT_TRUE(cmdIs(ir, "$a = 3; $a < 5", "1"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $a != 5", "0"));
  EXPECT_TRUE(cmdIs(ir, "$a = 3.14; $a > 3", "1"));
  EXPECT_TRUE(cmdIs(ir, "$a = 6; $a / 2", "3"));
  EXPECT_TRUE(cmdIs(ir, "$a = 2; $a * 3", "6"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; --$a; $a", "4"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $a -= 2; $a", "3"));
  EXPECT_TRUE(cmdIs(ir, "\"5\" == \"5\"", "1"));
  // Cross-type string/int compare: both coerce to comparable values.
  EXPECT_TRUE(cmdIs(ir, "\"5\" == 5", "1"));
}

TEST_F(InprTest, containerExtendedTest) {
  EXPECT_TRUE(cmdIs(ir, "a = Vector{1, 2, 3}; a.size()", "3"));
  EXPECT_TRUE(cmdIs(ir, "a = Vector{1, 2, 3}; $s = 0; while ($v : a) $s += $v; $s", "6"));
  EXPECT_TRUE(cmdIs(ir, "m = Map{ k : 10 }; m[\"k\"]", "10"));
  EXPECT_TRUE(cmdIs(ir, "$b = 12; c = Map{ one : $b + 5, two : 2}; $s = 0; while ($v : c) $s += 1; $s", "2"));
  EXPECT_TRUE(cmdIs(ir, "a = Vector; a.push_back(1); a.pop_back(); a.empty()", "1"));
}

TEST_F(InprTest, interpreterApiTest) {
  EXPECT_EQ(Interpreter::valueAsInt64(ir.runFunction("summ", {int64_t{2}, int64_t{3}})), 5);
  EXPECT_TRUE(cmdIs(ir, "$a = 42;", "42"));
  EXPECT_EQ(Interpreter::valueAsInt64(ir.variable("$a")), 42);
  EXPECT_TRUE(cmdIs(ir, "1+2;", "3"));
  EXPECT_TRUE(cmdIs(ir, "10;", "10"));

  Interpreter::Error err;
  EXPECT_FALSE(ir.parseScript("", err));
  EXPECT_TRUE(Interpreter::hasError(err));
  EXPECT_FALSE(ir.parseScript("$a = \"", err));
  EXPECT_TRUE(Interpreter::hasError(err));
}

TEST_F(InprTest, readmeExamplesTest) {
  EXPECT_TRUE(cmdIs(ir,
    "$a = 5; $b = 2; while($a > 1){ $a -= 1; $b = summ($b, $a); if($a < 4){ break;} } $b;",
    "9"));

  struct Case { const char* script; const char* expected; };
  const Case cases[] = {
    {"$a = 5; $b = 2; while ($a > 1){ $a = $a - 1; $b = summ($b, $a); if ($a < 4){ break;} } $b;",
     "9"},
    {"$a = 5; $b = 2; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;", "50"},
    {"a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); a[2]", "3"},
    {"b = Map; b.insert(myKeyOne, myValueOne); b.insert(myKeyTwo, myValueTwo); b[\"myKeyTwo\"]",
     "myValueTwo"},
    // README comments say "1 2 3" etc.; last while-iter value is 0.
    {"a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); while($v : a) print($v);", "0"},
    {"a = Vector{1 + 2, 2 + 3, 3 + 4}; while($v : a) print($v);", "0"},
    {"$b = 12; c = Map{ one : $b + 5, two : 2}; while($v : c) print($v);", "0"},
    {"e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one", "7"},
    {"$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three", "22"},
    {"$a = 1; $b = 2; function myFunc{ $a += $b; }; myFunc()", "3"},
    {"$a = 1; $b = 2; function myFunc{ $a += $b; function myFunc2{ $a += $b; }; myFunc2(); }; myFunc()",
     "5"},
    {"$a = 0; function myFunc{ if ($0 > 1) $a = $0 * myFunc($0 - 1); else $a = 1; $a }; myFunc(5)",
     "120"},
    {"function myFunc{ $0 += $1; }; myFunc(2, 3)", "5"},
    {"b: str = \"abc\"; type(b)", "str"},
  };
  for (const auto& c : cases) {
    EXPECT_TRUE(cmdIs(ir, c.script, c.expected)) << c.script;
  }

  char srcPath[] = "/tmp/ir_readme_src_XXXXXX";
  char dstPath[] = "/tmp/ir_readme_dst_XXXXXX";
  const int srcFd = mkstemp(srcPath);
  const int dstFd = mkstemp(dstPath);
  ASSERT_NE(srcFd, -1);
  ASSERT_NE(dstFd, -1);
  close(srcFd);
  close(dstFd);
  remove(dstPath);

  const string src = srcPath;
  const string dst = dstPath;
  EXPECT_TRUE(cmdIs(ir, "file1 = File{\"" + src + "\"}; file1.write(\"readme\"); file1.exist();", "1"));
  const string copyScript =
    "file1 = File{\"" + src + "\"}; file2 = File{\"" + dst + "\"}; "
    "if (file1.exist()) { $data = file1.read(); file2.write($data); }";
  const auto copyResult = ir.cmd(copyScript);
  EXPECT_FALSE(Interpreter::hasError(copyResult.second));
  EXPECT_TRUE(cmdIs(ir, "f = File{\"" + dst + "\"}; f.read();", "readme"));
  remove(srcPath);
  remove(dstPath);
}

TEST(valueHelpersTest, staticValueApi) {
  EXPECT_TRUE(std::holds_alternative<bool>(Interpreter::valueFromLiteral("true")));
  EXPECT_TRUE(std::holds_alternative<bool>(Interpreter::valueFromLiteral("false")));
  EXPECT_TRUE(std::holds_alternative<int64_t>(Interpreter::valueFromLiteral("42")));
  EXPECT_TRUE(std::holds_alternative<double>(Interpreter::valueFromLiteral("3.14")));
  EXPECT_TRUE(std::holds_alternative<std::string>(Interpreter::valueFromLiteral("")));

  EXPECT_FALSE(Interpreter::valueIsTruthy(false));
  EXPECT_FALSE(Interpreter::valueIsTruthy(int64_t{0}));
  EXPECT_FALSE(Interpreter::valueIsTruthy(0.0));
  EXPECT_FALSE(Interpreter::valueIsTruthy(std::string{}));
  EXPECT_TRUE(Interpreter::valueIsTruthy(true));
  EXPECT_TRUE(Interpreter::valueIsTruthy(int64_t{1}));
  EXPECT_TRUE(Interpreter::valueIsTruthy(std::string{"x"}));
  EXPECT_TRUE(Interpreter::valueIsTruthy(Interpreter::ControlFlow::Break));

  EXPECT_TRUE(Interpreter::valueEquals(int64_t{5}, int64_t{5}));
  EXPECT_TRUE(Interpreter::valueEquals(std::string{"5"}, int64_t{5}));

  EXPECT_GT(Interpreter::valueCompare(3.14, int64_t{3}), 0);
  EXPECT_LT(Interpreter::valueCompare(int64_t{3}, int64_t{5}), 0);

  EXPECT_EQ(Interpreter::valueToString(true), "1");
  EXPECT_EQ(Interpreter::valueToString(Interpreter::ControlFlow::Break), "break");
}

TEST_F(InprTest, comparisonSupplementTest) {
  EXPECT_TRUE(cmdIs(ir, "$a = 3; $a <= 3", "1"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; $a >= 6", "0"));
  EXPECT_TRUE(cmdIs(ir, "$a = 3.14; $a == 3.14", "1"));
  // Float literals: * promotes like +; / uses integer division on numeric values.
  EXPECT_TRUE(cmdIs(ir, "1.5 * 2", "2"));
  EXPECT_TRUE(cmdIs(ir, "1.5 / 2", "0"));
}

TEST_F(InprTest, variableInitTest) {
  EXPECT_TRUE(cmdIs(ir, "$a{10}; $a", "10"));
  EXPECT_TRUE(cmdIs(ir,
    "$a{5}; $b{2}; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;",
    "50"));
}

TEST_F(InprTest, interpreterStateTest) {
  EXPECT_TRUE(cmdIs(ir, "$a = 7;", "7"));
  EXPECT_TRUE(cmdIs(ir, "$a + 1;", "8"));

  ir.cmd("$a = 1; $b = 2;");
  const auto vars = ir.allVariables();
  EXPECT_GE(vars.size(), 2u);
  EXPECT_EQ(Interpreter::valueAsInt64(vars.at("$a")), 1);

  ASSERT_TRUE(ir.setMacro("#two", "2"));
  EXPECT_TRUE(cmdIs(ir, "#two + 3;", "5"));

  ir.setVariable("$d", 3.14);
  ir.setVariable("$s", std::string{"hi"});
  EXPECT_TRUE(std::holds_alternative<double>(ir.variable("$d")));
  EXPECT_EQ(Interpreter::valueAsString(ir.variable("$s")), "hi");
  EXPECT_TRUE(cmdIs(ir, "$d", "3.14"));
  EXPECT_TRUE(cmdIs(ir, "$s", "hi"));
}

TEST_F(InprTest, containerEdgeTest) {
  // OOB / missing keys: error string in result, not CmdResult.second.
  EXPECT_TRUE(cmdIs(ir, "a = Vector{1}; a[99]", ""));
  EXPECT_TRUE(cmdIs(ir, "m = Map{ k : 1 }; m[\"missing\"]", ""));
  EXPECT_TRUE(cmdIs(ir, "m = Map{}; m.at(x)", ""));
  EXPECT_TRUE(cmdIs(ir, "m = Map{ k : 10 }; m.at(k)", "10"));
  EXPECT_TRUE(cmdIs(ir, "a = Vector; a.push_back(1); a.clear(); a.empty()", "1"));
}

TEST_F(InprTest, hostReturnTypesTest) {
  ir.addFunction("retBool", [](const vector<Interpreter::Value>&) -> Interpreter::Value {
    return true;
  });
  ir.addFunction("retDouble", [](const vector<Interpreter::Value>&) -> Interpreter::Value {
    return 3.14;
  });
  ir.addFunction("retStr", [](const vector<Interpreter::Value>&) -> Interpreter::Value {
    return std::string{"ok"};
  });

  EXPECT_TRUE(cmdIs(ir, "retBool()", "1"));
  EXPECT_TRUE(cmdIs(ir, "retDouble()", "3.14"));
  EXPECT_TRUE(cmdIs(ir, "retStr()", "ok"));
  EXPECT_TRUE(cmdIs(ir, "summ(1, retBool())", "2"));
  EXPECT_TRUE(cmdIs(ir, "retStr() + \"!\"", "ok!"));
}

TEST_F(InprTest, keywordBoundaryTest) {
  EXPECT_TRUE(cmdIs(ir, "m = Map{ trueVar : 5 }; m[\"trueVar\"]", "5"));
  EXPECT_TRUE(cmdIs(ir, "e = Struct{ falseFlag : 2 }; e.falseFlag", "2"));
  // trueVar without $ is not the bool literal; comparison yields false (0).
  EXPECT_TRUE(cmdIs(ir, "if (trueVar == 5) { 1; }", "0"));
}

TEST_F(InprTest, miscApiTest) {
  Interpreter::Error err;
  ASSERT_TRUE(ir.parseScript("$a = \"\";", err));
  EXPECT_FALSE(Interpreter::hasError(err));
  ir.runScript();
  EXPECT_EQ(Interpreter::valueAsString(ir.variable("$a")), "");

  ASSERT_TRUE(ir.setMacro("#inc", "$a = $a + 1;"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; #inc; $a", "6"));
  ASSERT_TRUE(ir.setMacro("inc", "$a = $a + 1;"));
  EXPECT_TRUE(cmdIs(ir, "$a = 5; #inc; $a", "6"));
}

TEST_F(InprTest, printStdoutTest) {
  auto runPrint = [this](const char* script) {
    testing::internal::CaptureStdout();
    const auto r = ir.cmd(script);
    const std::string captured = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(Interpreter::hasError(r.second)) << Interpreter::cmdResultToString(r);
    EXPECT_EQ(Interpreter::cmdResultToString(r), "0");
    return captured;
  };

  const std::string out = runPrint(
    "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); while($v : a) print($v);");
  EXPECT_NE(out.find("1"), std::string::npos);
  EXPECT_NE(out.find("2"), std::string::npos);
  EXPECT_NE(out.find("3"), std::string::npos);

  const std::string out2 = runPrint(
    "a = Vector{1 + 2, 2 + 3, 3 + 4}; while($v : a) print($v);");
  EXPECT_NE(out2.find("3"), std::string::npos);
  EXPECT_NE(out2.find("5"), std::string::npos);
  EXPECT_NE(out2.find("7"), std::string::npos);

  const std::string out3 = runPrint(
    "$b = 12; c = Map{ one : $b + 5, two : 2}; while($v : c) print($v);");
  EXPECT_NE(out3.find("one"), std::string::npos);
  EXPECT_NE(out3.find("17"), std::string::npos);
  EXPECT_NE(out3.find("two"), std::string::npos);
  EXPECT_NE(out3.find("2"), std::string::npos);
}

TEST_F(InprTest, interpreterCopyMoveTest) {
  EXPECT_TRUE(cmdIs(ir, "$a = 1;", "1"));
  Interpreter ir2 = ir;
  EXPECT_TRUE(cmdIs(ir2, "$a = 2;", "2"));
  EXPECT_TRUE(cmdIs(ir, "$a", "1"));
  EXPECT_TRUE(cmdIs(ir2, "$a", "2"));

  EXPECT_TRUE(cmdIs(ir, "$b = 12;", "12"));
  EXPECT_TRUE(cmdIs(ir, "c = Map{ one : $b + 5 }; c[\"one\"]", "17"));

  Interpreter ir3 = std::move(ir);
  EXPECT_TRUE(cmdIs(ir3, "1 + 2;", "3"));
}

int main(int argc, char* argv[]){

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
