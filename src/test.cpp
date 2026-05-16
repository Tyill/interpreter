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

int main(int argc, char* argv[]){

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
