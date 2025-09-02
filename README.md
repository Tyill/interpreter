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
script = "e = Struct{ one : 5, two : 2}; e.one = summ(e.one, e.two); e.one";
res = ir.cmd(script); // 7

script = "$b = 12; e = Struct{ one : $b + 5, two : 2}; e.three = e.one + e.two + 3; e.three";
res = ir.cmd(script); // 22
```

### Containers from [base lib](https://github.com/Tyill/interpreter/blob/main/include/base_library/containers.h) 

```
script = "a = Vector{1,2,3}; a.push_back(4); a.push_back(5); while($v : a) print($v);";
res = ir.cmd(script); // 1 2 3 4 5

script = "b = Map{myKeyOne: myValueOne}; b.insert(myKeyTwo, myValueTwo); b.at(myKeyTwo)";
res = ir.cmd(script); // myValueTwo
```

| Vector                | Map                 |
|-----------------------|---------------------|
|`insert`(index, value) |`insert`(key, value) |
|`push_back`(value)     |                     |
|`pop_back`()           |                     |
|`erase`(index)         |`erase`(key)         |
|`size`()               |`size`()             |
|`empty`()              |`empty`()            |
|`clear`()              |`clear`()            |
|`at`(index) OR `[index]` |`at`(key) OR `[key]` |
|`set`(index)           |`set`(key)           |

### Filesystem from [base lib](https://github.com/Tyill/interpreter/blob/main/include/base_library/filesystem.h) 

```
script = "file1 = File{\"main.cpp\"}; file2 = File{\"mainCopy.txt\"};  \
          if (file1.exist()) { $data = file1.read(); file2.write($data); }";
res = ir.cmd(script);
```

| File                  | Dir                 |
|-----------------------|---------------------|
|`read`()               |`exist`()            |
|`write`(data)          |`remove`()           |
|`exist`()              |                     |
|`remove`()             |                     |

### Types from [base lib](https://github.com/Tyill/interpreter/blob/main/include/base_library/types.h) 

```
script = "a: int = 123; type(a)";
res = ir.cmd(script); // int

script = "b: str = "abc"; type(b)";
res = ir.cmd(script); // str
```

### Example of use

```cpp

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
  
  script = "$a = 5; $b = 2; $c = summ($a, ($a + ($a * ($b + $a))), summ(5)); $c;";
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
  
  script = "$a = 0; function myFunc{ if ($0 > 1) $a = $0 * myFunc($0 - 1); else $a = 1; $a }; myFunc(5)";
  res = ir.cmd(script); // 120

  script = "function myFunc{ $0 += $1; }; myFunc(2, 3)";
  res = ir.cmd(script); // 5

  script = "b: str = \"abc\"; type(b)";
  res = ir.cmd(script); // str
  
  return 0;
}
```

### [Tests](https://github.com/Tyill/interpreter/blob/main/src/test.cpp)


### License
Licensed under an [MIT-2.0]-[license](LICENSE).


