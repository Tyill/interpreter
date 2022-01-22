//
// Interpreter Project
// Copyright (C) 2020 by Contributors <https://github.com/Tyill/interpreter>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <gtest/gtest.h>
#include "../include/interpreter.h"
#include "../include/base_library/arithmetic_operations.h"
#include "../include/base_library/comparison_operations.h"
#include "../include/base_library/containers.h"
#include "../include/base_library/structure.h"

using namespace std;

bool isNumber(const string& s) {
  for (auto c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return !s.empty();
}

class InprTest : public ::testing::Test {
public:
  InprTest(): 
  ao_ir(ir),
  co_ir(ir),
  bc_ir(ir),
  st_ir(ir){
    
  ir.addOperator("->", [](string& leftOpd, string& rightOpd) ->string {
    rightOpd = leftOpd;
    return rightOpd;
  }, 5);
  ir.addFunction("summ", [](const vector<string>& args) ->string {
    int res = 0;
    for (auto& v : args) {
      if (isNumber(v)) res += stoi(v);
    }
    return to_string(res);
  });
  Interpreter* pIr = &ir;
  ir.addFunction("setB", [pIr](const vector<string>& args) ->string {
    int res = 0;
    for (auto& v : args) {
      if (isNumber(v)) res += stoi(v);
    }
    pIr->setVariable("$b", to_string(res));
    return to_string(res);
  });
  ir.addFunction("range", [pIr](const vector<string>& args) ->string {
    int maxv = 0;
    if (!args.empty() && isNumber(args[0])) 
      maxv = stoi(args[0]); 

    auto entity = pIr->currentEntity();
    int cval = 0;
    if (isNumber(entity.value)) 
      cval = stoi(entity.value);
    
    return cval < maxv ? to_string(cval + 1) : "0";
  });
  }
  ~InprTest() {   
  }
  Interpreter ir;
  InterpreterBaseLib::ArithmeticOperations ao_ir;
  InterpreterBaseLib::ComparisonOperations co_ir;
  InterpreterBaseLib::Container bc_ir;
  InterpreterBaseLib::Structure st_ir;
};

TEST_F(InprTest, operatorTest){   
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; $a + $b") == "7");
  EXPECT_TRUE(ir.cmd("$a = 0; $a += 5; $b = 2; $a + $b") == "7");
  EXPECT_TRUE(ir.cmd("$a = 4; ++$a; $b = 2; $a + $b") == "7");
  EXPECT_TRUE(ir.cmd("$a = 4; $a++; $b = 2; $a + $b") == "7");
  EXPECT_TRUE(ir.cmd("$a = 0; $b = 0; $c = ($a == $b); $c") == "1");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; $a->$b; $b") == "5");
  EXPECT_TRUE(ir.cmd("$b = \"summ = \"; $a = $b + \"$a + $b\"; $a") == "summ = $a + $b");
  EXPECT_TRUE(ir.cmd("$b = \"$a + $b\"; $a = \"summ = \" + $b; $a") == "summ = $a + $b"); 
}
TEST_F(InprTest, conditionTest){   
  EXPECT_TRUE(ir.cmd("$a = 5; if ($a == 5){ $b = 2; } $a + $b") == "7");
  EXPECT_TRUE(ir.cmd("$a = 5; if ($a == 3){ $b = 2;} else { $a = 3;}; $a") == "3");
  EXPECT_TRUE(ir.cmd("$a = 5; if ($a == 3){ $b = 2;} elseif($a == 5){ $a = 3;}; $a") == "3");
  EXPECT_TRUE(ir.cmd("$a = 5; if ($a == 3){ $b = 2;} elseif($a != 4){ $a = 3;}; $a") == "3");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; if ($a == 1 + $b){ $b = 2; } elseif($a != 4){ $a = 3;}; $a") == "3");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ $a -= 1; $b += 1; } $b") == "5");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ while($b < $a){ $b += 1; break;} $a -= 1;} $b") == "3");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ $a -= 1; $b += 1; if ($a == 1){ break;} } $b") == "4");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ $a -= 1; $b += 1; if ($a == 1){ if ($a == 1){ break;}} } $b") == "4");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ $a -= 1; if ($a == 2){continue;} $b += 1;} $b") == "4");
  EXPECT_TRUE(ir.cmd("$a = 5; if($a == 3){ $b = 3;} elseif($a == 5){ $b = 5;} elseif($a == 5){ $b = 4;} $b") == "5");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ $a -= 1; if ($a == 2) continue; $b += 1;} $b") == "4");
  EXPECT_TRUE(ir.cmd("$a = 5; if($a == 3) $b = 3; elseif($a == 5) $b = 5; elseif($a == 5) $b = 4; $b") == "5");
}
TEST_F(InprTest, functionTest){   
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; summ($a, $b)") == "7");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; 1 + summ($a, $b)") == "8");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; 1 + summ($a, $b) - 1") == "7");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; 1 + summ($a, $b, 1) - 1") == "8");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; 1 + summ() + $b") == "3");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; setB($a - 1)") == "4");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; 1 + summ(1, summ($a)) + $b") == "9");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; summ(summ($a, summ($b)), summ($b, summ($a)))") == "14");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; summ(summ($a + summ($b + 1, 1) + 3, summ($b)))") == "14");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 0; while($a > 0){ $a -= summ(1); if ($a == 2){continue;} $b += summ(1);} $b") == "4");
}
TEST_F(InprTest, macrosTest){   
  EXPECT_TRUE(ir.cmd("$a = 5; #macro myMacr{$a = $a + 2;} #myMacr; #myMacr; #myMacr;") == "11");
  EXPECT_TRUE(ir.cmd("$a = 5; #macro myMacr{ $a = $a + $1 + $1 + $2; } #myMacr(3,4);") == "15");
  EXPECT_TRUE(ir.cmd("#macro RANGE{while(range($1))}; $a = 0; #RANGE(100) $a += 1; $a;") == "100");
}
TEST_F(InprTest, gotoTest){   
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; goto l_jmp; $a = summ($a, $b); l_jmp: $a;") == "5");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; while($a > 0){ $a -= 1; if ($a == 2){ goto l_jmp;}} l_jmp: $a; ") == "2");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; goto l_cyc; l_jmp: goto l_exit; l_cyc: while($a > 0){ $a -= 1; if ($a == 2){ goto l_jmp;};}; l_exit: $a;") == "2");
  EXPECT_TRUE(ir.cmd("$a = 5; $b = 2; while($a > 0){goto l_jmp1; l_jmp: $a = 10; goto l_exit; l_jmp1: $a -= 1; if ($a == 2){ goto l_jmp;}} l_exit: $a; ") == "10");
}
TEST_F(InprTest, reflectionTest){ 
  EXPECT_TRUE(ir.cmd("$a = 0; while(range(100)) $a += 1; $a;") == "100");
}
TEST_F(InprTest, containerTest){ 
  EXPECT_TRUE(ir.cmd("a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); a.size()") == "3");
  EXPECT_TRUE(ir.cmd("b = Map; b.insert(myKeyOne, myValueOne); b.insert(myKeyTwo, myValueTwo); b.at(myKeyTwo)") == "myValueTwo");
}
TEST_F(InprTest, structureTest){ 
  EXPECT_TRUE(ir.cmd("e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one") == "7");
  EXPECT_TRUE(ir.cmd("$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = $b; e.three") == "12");
  EXPECT_TRUE(ir.cmd("$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three") == "22");
}
TEST_F(InprTest, internFuncTest){ 
  EXPECT_TRUE(ir.cmd("$a = 1; $b = 2; function myFunc{ $a += $b; }; myFunc()") == "3");
  EXPECT_TRUE(ir.cmd("$a = 1; $b = 2; function myFunc{ $a += $b; function myFunc2{ $a += $b; }; myFunc2(); }; myFunc()") == "5");
  EXPECT_TRUE(ir.cmd("$a = 0; function myFunc{ if ($arg0 > 1) $a = $arg0 * myFunc($arg0 - 1); else $a = 1; $a }; myFunc(5)") == "120");
  EXPECT_TRUE(ir.cmd("function myFunc{ $arg0 += $arg1; }; myFunc(2, 3)") == "5");
}

int main(int argc, char* argv[]){
 
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}