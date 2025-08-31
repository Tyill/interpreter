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

#include "../include/interpreter.h"
#include "../include/base_library/arithmetic_operations.h"
#include "../include/base_library/comparison_operations.h"
#include "../include/base_library/containers.h"
#include "../include/base_library/filesystem.h"
#include "../include/base_library/structure.h"
#include "../include/base_library/types.h"
#include <cctype>

using namespace std;

bool isNumber(const string& s) {
  for (auto c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return !s.empty();
}

int main(int argc, char* argv[])
{  
  Interpreter ir;

  InterpreterBaseLib::ArithmeticOperations ir_ao(ir);
  InterpreterBaseLib::ComparisonOperations ir_co(ir);
  InterpreterBaseLib::Types ir_tp(ir);
  InterpreterBaseLib::Container ir_bc(ir);
  InterpreterBaseLib::Filesystem ir_fs(ir);
  InterpreterBaseLib::Structure ir_st(ir);

  ir.addFunction("summ", [](const vector<string>& args) ->string {
    int res = 0;
    for (auto& v : args) {
      if (isNumber(v)) res += stoi(v);
    }
    return to_string(res);
  });

  ir.addFunction("print", [](const vector<string>& args) ->string {
    for (auto& v : args) {
      printf("%s ", v.c_str());
    }
    printf("\n");
    return "";
  });

  string script = "$a = 5; $b = 2; while ($a > 1){ $a = $a - 1; $b = summ($b, $a); if ($a < 4){ break;} } $b;";
  string res = ir.cmd(script); // 9
  
  script = "$a{5}; $b{2}; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;";
  res = ir.cmd(script); // 50
  
  script = "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); a[2]";
  res = ir.cmd(script); // 3
  
  script = "b = Map; b.insert(myKeyOne, myValueOne); b.insert(myKeyTwo, myValueTwo); b[\"myKeyTwo\"]";
  res = ir.cmd(script); // myValueTwo
  
  script = "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); while($v : a) print($v);";
  res = ir.cmd(script); // 1 2 3
  
  script = "a = Vector{1 + 2, 2 + 3, 3 + 4}; while($v : a) print($v);";
  res = ir.cmd(script); // 3 5 7
  
  script = "$b = 12; c = Map{ one : $b + 5, two : 2}; while($v : c) print($v);";
  res = ir.cmd(script); // one 17 two 2
  
  script = "e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one";
  res = ir.cmd(script); // 7
  
  script = "$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three";
  res = ir.cmd(script); // 22
  
  script = "file1 = File{\"main.cpp\"}; file2 = File{\"mainCopy.txt\"}; if (file1.exist()) { $data = file1.read(); file2.write($data); }";
  res = ir.cmd(script);
  
  script = "$a = 1; $b = 2; function myFunc{ $a += $b; }; myFunc()";
  res = ir.cmd(script); // 3
  
  script = "$a = 1; $b = 2; function myFunc{ $a += $b; function myFunc2{ $a += $b; }; myFunc2(); }; myFunc()";
  res = ir.cmd(script); // 5

  script = "function myFunc{ if ($0 > 1) $a = $0 * myFunc($0 - 1); else $a = 1; $a }; myFunc(5)";
  res = ir.cmd(script); // 120
  
  script = "function myFunc{ $0 += $1; }; myFunc(2, 3)";
  res = ir.cmd(script); // 5
  
  script = "b: str = 123; type(b)";
  res = ir.cmd(script); // str

  return 0;
}

