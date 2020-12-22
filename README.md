# Tiny interpreter
Simple and fast command interpreter.  
Library [one header](https://github.com/Tyill/interpreter/tree/main/include/interpreter.h) and [source file](https://github.com/Tyill/interpreter/tree/main/src/interpreter.cpp).

Example:
```cpp
stringstream ss;
ss << "$a = 5;"
      "$b = 2;"
      "while($a > 1){"
      "  $a -= 1;"
      "  $b = summ($b, $a);"
      "  if($a < 4){"
      "    break;"
      "  }"
      "}"
      "$b";
string res = ir.cmd(ss.str()); // 9
```

### User functions
You can define any functions. Can be passed as parameters to other functions.  
Simple addition
```cpp
 ir.addFunction("summ", [](const vector<string>& args) ->string {
    int res = 0;
    for (auto& v : args) {
      if (isNumber(v)) res += stoi(v);
    }
    return to_string(res);
  });
```
Use in script
```
$c = 5; $d = $c + 5; summ($c, $d,  summ(6 + 5, 3));
```

### User operators
You can define any operators. Simple addition
```cpp
 ir.addOperator("=", [](string& opd1, string& opd2) ->string {
    opd1 = opd2;
    return opd1;
  }, 100);

 ir.addOperator("+=", [](string& leftOpd, string& rightOpd) ->string {
   if (isNumber(leftOpd) && isNumber(rightOpd)){
     leftOpd = to_string(stoi(leftOpd) + stoi(rightOpd));
     return leftOpd;
   }     
   else{
     leftOpd += rightOpd;
     return leftOpd;
   }
 }, 100);
```
Use in script
```
$c = 5;
$c += 5;
```

### Variables
Must start with '$'
```
$a = 5;
```

### Expressions
Start with any characters.  Must end with ';'.  
Parentheses increase the priority of the operation.  
Can be passed as parameters to functions.
```
$a = 5; $b = 2; $c = $a * (2 + $b);
$d = summ($a, $b, $c + 3, 4);
```

### Macros
Macro declaration with '#macro name {body}'
```
#macro myMac{ $c = 5; $d = $c + 5 + 6; } ;
```
Insert a macro with '#'
```
$c = 5; #myMac;
```

### Goto
Jump on label.
Name of label must start with 'l_' and end with ':'
```
if ($a == 3){
  goto l_myLabel1;
}
l_myLabel1: $a = 4;
```

### Keywords

|                          |                                                   |
|--------------------------|---------------------------------------------------|
|`if`(condition){body}     | Condition if expression in parentheses is nonzero |
|`while`(condition){body}  | Cycle if expression in parentheses is nonzero.    |
|`elseif`(condition){body} | If the previous condition is not met              |
|`else`{body}              | If the previous condition is not met              |
|`break;`                  | Aborts the execution of the loop                  |
|`continue;`               | Continues the cycle                               |

### Example of use

```cpp

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
```

### [Tests](https://github.com/Tyill/interpreter/blob/main/src/test.cpp)


### License
Licensed under an [MIT-2.0]-[license](LICENSE).

