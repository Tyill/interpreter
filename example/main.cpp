
#include "../include/interpreter.h"

using namespace std;

int main(int argc, char* argv[])
{  
  Interpreter ir;

  bool  ok = ir.addOperator("+", [](string& opd1, string& opd2) ->string {
    return "";
  }, 0);

  ok = ir.addOperator("++", [](string& opd1, string& opd2) ->string {
    return "";
  }, 0);

  ok = ir.addOperator("=", [](string& opd1, string& opd2) ->string {
    return "";
  }, 0);

  ok = ir.addFunction("myFunc", [](const vector<string>& args) ->string {
    return "";
  });

  string scenar = "++$a = 4; $b = $a + 14; myFunc($a, myFunc($a + myFunc($b = 23),$b = 23 + $a,$b)); for($b + myFunc($a)){ $b = $b + 1;} ; // abcd \n #macro dd{ $c = 4; }; #dd; // abcd \n  ",
         err = "";
  ir.parseScenar(scenar, err);

	return 0;
}

