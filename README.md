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

### User functions outside the script
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

### User functions inside the script
You can define any functions in script
```
$a = 1; $b = 2; 
function myFunc{ $a += $b; };  // define
myFunc();                      // call                         
```
With params
```
function myFunc{ $0 += $1; };  // define
myFunc(2, 3);                  // call       
```

### User operators
You can define any operators. Simple addition
```cpp
 ir.addOperator("=", [](string& leftOpd, string& rightOpd) ->string {
    leftOpd = rightOpd;
    return leftOpd;
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
$b = "string";

$a{12}   // initializer
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
#macro myMac{ $c = 5; $d = $c + 5 + 6; };
```
Insert a macro with '#'
```
$c = 5; #myMac;
```

Insert a macro with params (index of param begin with 0)
```
#macro myMacr{ $a = $a + $0 + $0 + $1; };
$a = 5;  #myMacr(3,4); // result 15
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

### Control keywords

|                          |                                                   |
|--------------------------|---------------------------------------------------|
|`if`(condition){body}     | Condition if expression in parentheses is nonzero |
|`while`(condition){body}  | Cycle if expression in parentheses is nonzero.    |
|`elseif`(condition){body} | If the previous condition is not met              |
|`else`{body}              | If the previous condition is not met              |
|`break;`                  | Aborts the execution of the loop                  |
|`continue;`               | Continues the cycle                               |

### Structure from [base lib](https://github.com/Tyill/interpreter/blob/main/include/base_library/structure.h) 

```
scenar = "e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one";
res = ir.cmd(scenar); // 7

scenar = "$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three";
res = ir.cmd(scenar); // 22
```

### Containers from [base lib](https://github.com/Tyill/interpreter/blob/main/include/base_library/containers.h) 

```
scenar = "a = Vector{1,2,3}; a.push_back(4); a.push_back(5); while($v : a) print($v);";
res = ir.cmd(scenar); // 1 2 3 4 5

scenar = "b = Map{myKeyOne: myValueOne}; b.insert(myKeyTwo, myValueTwo); b.at(myKeyTwo)";
res = ir.cmd(scenar); // myValueTwo
```

| Vector                | Map                 |
|-----------------------|---------------------|
|`push_back`(value)     |`insert`(key, value) |
|`pop_back`()           |`erase`(key)         |
|`insert`(index, value) |`size`()             |
|`erase`(index)         |`empty`()            |
|`size`()               |`clear`()            |
|`empty`()              |`at`(key)            |
|`clear`()              |`set`(key)           |
|`at`(index)            |                     |
|`set`(index)           |                     |

### Filesystem from [base lib](https://github.com/Tyill/interpreter/blob/main/include/base_library/filesystem.h) 

```
scenar = "file1 = File{\"main.cpp\"}; file2 = File{\"mainCopy.txt\"};  \
          if (file1.exist()) { $data = file1.read(); file2.write($data); }";
res = ir.cmd(scenar);
```

| File                  | Dir                 |
|-----------------------|---------------------|
|`read`()               |`exist`()            |
|`write`(data)          |`remove`()           |
|`exist`()              |                     |
|`remove`()             |                     |

### Example of use

```cpp

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
  
  scenar = "$a = 5; $b = 2; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;";
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
  
  scenar = "$a = 0; function myFunc{ if ($0 > 1) $a = $0 * myFunc($0 - 1); else $a = 1; $a }; myFunc(5)";
  res = ir.cmd(scenar); // 120

  scenar = "function myFunc{ $0 += $1; }; myFunc(2, 3)";
  res = ir.cmd(scenar); // 5
  
  return 0;
}
```

### [Tests](https://github.com/Tyill/interpreter/blob/main/src/test.cpp)


### License
Licensed under an [MIT-2.0]-[license](LICENSE).

