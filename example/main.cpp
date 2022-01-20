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

  InterpreterBaseLib::ArithmeticOperations ao(ir);
  InterpreterBaseLib::ComparisonOperations co(ir);
  InterpreterBaseLib::Container bc(ir);
  InterpreterBaseLib::Filesystem fs(ir);
  InterpreterBaseLib::Structure st(ir);

  ir.addFunction("summ", [](const vector<string>& args) ->string {
    int res = 0;
    for (auto& v : args) {
      if (isNumber(v)) res += stoi(v);
    }
    return to_string(res);
  });

  ir.addFunction("print", [](const vector<string>& args) ->string {
    for (auto& v : args) {
      printf("%s\n", v.c_str());
    }
    return "";
  });

  string scenar = "$a = 5; $b = 2; while ($a > 1){ $a = $a - 1; $b = summ($b, $a); if ($a < 4){ break;} } $b;";
  string res = ir.cmd(scenar); // 9
  
  scenar = "$a{5}; $b{2}; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;";
  res = ir.cmd(scenar); // 50

  scenar = "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); a.size()";
  res = ir.cmd(scenar); // 3

  scenar = "b = Map; b.insert(myKeyOne, myValueOne); b.insert(myKeyTwo, myValueTwo); b.at(myKeyTwo)";
  res = ir.cmd(scenar); // myValueTwo

  scenar = "a = Vector; a.push_back(1); a.push_back(2); a.push_back(3); while($v : a) print($v);";
  res = ir.cmd(scenar); // 1 2 3

  scenar = "a = Vector{1 + 2, 2 + 3, 3 + 4}; while($v : a) print($v);";
  res = ir.cmd(scenar); // 3 5 7

  scenar = "$b = 12; c = Map{ one : $b + 5, two : 2}; while($v : c) print($v);";
  res = ir.cmd(scenar); // one 17 two 2

  scenar = "e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one";
  res = ir.cmd(scenar); // 7

  scenar = "$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three";
  res = ir.cmd(scenar); // 22

  scenar = "file1 = File{\"main.cpp\"}; file2 = File{\"mainCopy.txt\"}; if (file1.exist()) { $data = file1.read(); file2.write($data); }";
  res = ir.cmd(scenar);

  scenar = "$a = 1; $b = 2; function myFunc{ $a += $b; }; myFunc()";
  res = ir.cmd(scenar); // 3

  scenar = "$a = 1; $b = 2; function myFunc{ $a += $b; function myFunc2{ $a += $b; }; myFunc2(); }; myFunc()";
  res = ir.cmd(scenar); // 5
  
  scenar = "function myFunc{ $arg0 += $arg1; }; myFunc(2, 3)";
  res = ir.cmd(scenar); // 5
  
  return 0;
}

