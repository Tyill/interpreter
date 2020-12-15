
#include "../include/interpreter.h"

using namespace std;

int main(int argc, char* argv[])
{
  string scenar = "$a = 4;",
         err = "";

  Interpreter ir(scenar, err);


	return 0;
}

