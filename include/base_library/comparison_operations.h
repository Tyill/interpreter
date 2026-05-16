
#include "../../include/interpreter.h"

namespace InterpreterBaseLib {

  class ComparisonOperations {
  public:
    ComparisonOperations(Interpreter& ir)
    {
      ir.addOperator("==", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (const auto* ls = std::get_if<std::string>(&leftOpd)) {
          if (const auto* rs = std::get_if<std::string>(&rightOpd)) {
            return *ls == *rs;
          }
        }
        return Interpreter::valueEquals(leftOpd, rightOpd);
      }, 2);

      ir.addOperator("!=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        if (const auto* ls = std::get_if<std::string>(&leftOpd)) {
          if (const auto* rs = std::get_if<std::string>(&rightOpd)) {
            return *ls != *rs;
          }
        }
        return !Interpreter::valueEquals(leftOpd, rightOpd);
      }, 2);

      ir.addOperator(">", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        return Interpreter::valueCompare(leftOpd, rightOpd) > 0;
      }, 2);

      ir.addOperator("<", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        return Interpreter::valueCompare(leftOpd, rightOpd) < 0;
      }, 2);

      ir.addOperator(">=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        return Interpreter::valueCompare(leftOpd, rightOpd) >= 0;
      }, 2);

      ir.addOperator("<=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        return Interpreter::valueCompare(leftOpd, rightOpd) <= 0;
      }, 2);

      ir.addOperator("=", [](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        leftOpd = rightOpd;
        return leftOpd;
      }, 100);
    }
  };
}
