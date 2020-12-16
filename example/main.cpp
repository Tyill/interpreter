
#include "../include/interpreter.h"

using namespace std;

int main(int argc, char* argv[])
{  
  Interpreter ir;

  ir.addOperator("+", [](const string& opd1, const string& opd2) ->string {
    return "";
  });

  ir.addFunction("myFunc", [](const vector<string>& args) ->string {
    return "";
  });

  string scenar = "$a = 4; $b = $a + 14; myFunc(); for($b + myFunc($a)){ $b = $b + 1;} ; // abcd \n #macro dd{ $c = 4; }; #dd; // abcd \n  ",
         err = "";
  ir.parseScenar(scenar, err);

	return 0;
}

