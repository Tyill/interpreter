
#include "../../include/interpreter.h"

namespace InterpreterBaseLib {

  class ArithmeticOperations {
  public:
    ArithmeticOperations(Interpreter& ir)
    {
      ir.addOperator("*", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd) && Interpreter::valueIsNumeric(rightOpd))
          return Interpreter::valueAsInt64(leftOpd) * Interpreter::valueAsInt64(rightOpd);
        return int64_t{0};
      }, 0);

      ir.addOperator("/", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd) && Interpreter::valueIsNumeric(rightOpd))
          return Interpreter::valueAsInt64(leftOpd) / Interpreter::valueAsInt64(rightOpd);
        return int64_t{0};
      }, 0);

      ir.addOperator("+", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd) && Interpreter::valueIsNumeric(rightOpd))
          return Interpreter::valueAsInt64(leftOpd) + Interpreter::valueAsInt64(rightOpd);
        return Interpreter::valueAsString(leftOpd) + Interpreter::valueAsString(rightOpd);
      }, 1);

      ir.addOperator("-", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd) && Interpreter::valueIsNumeric(rightOpd))
          return Interpreter::valueAsInt64(leftOpd) - Interpreter::valueAsInt64(rightOpd);
        return int64_t{0};
      }, 1);

      ir.addOperator("+=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd) && Interpreter::valueIsNumeric(rightOpd)) {
          leftOpd = Interpreter::valueAsInt64(leftOpd) + Interpreter::valueAsInt64(rightOpd);
          return leftOpd;
        }
        leftOpd = Interpreter::valueAsString(leftOpd) + Interpreter::valueAsString(rightOpd);
        return leftOpd;
      }, 4);

      ir.addOperator("-=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd) && Interpreter::valueIsNumeric(rightOpd)) {
          leftOpd = Interpreter::valueAsInt64(leftOpd) - Interpreter::valueAsInt64(rightOpd);
          return leftOpd;
        }
        leftOpd = Interpreter::valueAsString(leftOpd) + Interpreter::valueAsString(rightOpd);
        return leftOpd;
      }, 4);

      ir.addOperator("++", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd)) {
          leftOpd = Interpreter::valueAsInt64(leftOpd) + 1;
          return leftOpd;
        }
        if (Interpreter::valueIsNumeric(rightOpd)) {
          rightOpd = Interpreter::valueAsInt64(rightOpd) + 1;
        }
        return rightOpd;
      }, 4);

      ir.addOperator("--", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (Interpreter::valueIsNumeric(leftOpd)) {
          leftOpd = Interpreter::valueAsInt64(leftOpd) - 1;
          return leftOpd;
        }
        if (Interpreter::valueIsNumeric(rightOpd)) {
          rightOpd = Interpreter::valueAsInt64(rightOpd) - 1;
        }
        return rightOpd;
      }, 4);

      ir.addOperator("=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        leftOpd = rightOpd;
        return leftOpd;
      }, 100);
    }
  };
}
