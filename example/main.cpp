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

  ir.addOperator("*", [](string& leftOpd, string& rightOpd) ->string {
    if (isNumber(leftOpd) && isNumber(rightOpd))
      return to_string(stoi(leftOpd) * stoi(rightOpd));
    else
      return "0";
  }, 0);

  ir.addOperator("/", [](string& leftOpd, string& rightOpd) ->string {
    if (isNumber(leftOpd) && isNumber(rightOpd))
      return to_string(stoi(leftOpd) / stoi(rightOpd));
    else
      return "0";
  }, 0);

  ir.addOperator("+", [](string& leftOpd, string& rightOpd) ->string {
    if (isNumber(leftOpd) && isNumber(rightOpd))
      return to_string(stoi(leftOpd) + stoi(rightOpd));
    else
      return leftOpd + rightOpd;
  }, 1);

  ir.addOperator("-", [](string& leftOpd, string& rightOpd) ->string {
    if (isNumber(leftOpd) && isNumber(rightOpd))
      return to_string(stoi(leftOpd) - stoi(rightOpd));
    else
      return "0";
  }, 1);

  ir.addOperator("==", [](string& leftOpd, string& rightOpd) ->string {
    return leftOpd == rightOpd ? "1" : "0";
  }, 2);

  ir.addOperator("!=", [](string& leftOpd, string& rightOpd) ->string {
    return leftOpd != rightOpd ? "1" : "0";
  }, 2);

  ir.addOperator(">", [](string& leftOpd, string& rightOpd) ->string {
    if (isNumber(leftOpd) && isNumber(rightOpd))
      return stoi(leftOpd) > stoi(rightOpd) ? "1" : "0";
    else
      return leftOpd.size() > rightOpd.size() ? "1" : "0";
  }, 2);

  ir.addOperator("<", [](string& leftOpd, string& rightOpd) ->string {
    if (isNumber(leftOpd) && isNumber(rightOpd))
      return stoi(leftOpd) < stoi(rightOpd) ? "1" : "0";
    else
      return leftOpd.size() < rightOpd.size() ? "1" : "0";
  }, 2);
   
  ir.addOperator("=", [](string& leftOpd, string& rightOpd) ->string {
    leftOpd = rightOpd;
    return leftOpd;
  }, 100);

  ir.addFunction("summ", [](const vector<string>& args) ->string {
    int res = 0;
    for (auto& v : args) {
      if (isNumber(v)) res += stoi(v);
    }
    return to_string(res);
  });

  string scenar = "$a = 5; $b = 2; while ($a > 1){ $a = $a - 1; $b = summ($b, $a); if ($a < 4){ break;} } $b;";

  string res = ir.cmd(scenar); // 9

  scenar = "$a = 5; $b = 2; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;";

  res = ir.cmd(scenar); // 50

  return 0;
}

