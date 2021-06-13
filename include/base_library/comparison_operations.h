
#include "../../include/interpreter.h"

#include <cctype>

namespace InterpreterBaseLib {

  class ComparisonOperations {
  public:
    bool isNumber(const std::string& s) const {
      for (auto c : s) {
        if (!std::isdigit(c)) {
          return false;
        }
      }
      return !s.empty();
    }
        
    ComparisonOperations(Interpreter& ir)
    {      
      ir.addOperator("==", [](std::string& leftOpd, std::string& rightOpd) ->std::string {
        return leftOpd == rightOpd ? "1" : "0";
        }, 2);

      ir.addOperator("!=", [](std::string& leftOpd, std::string& rightOpd) ->std::string {
        return leftOpd != rightOpd ? "1" : "0";
        }, 2);

      ir.addOperator(">", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (isNumber(leftOpd) && isNumber(rightOpd))
          return stoi(leftOpd) > stoi(rightOpd) ? "1" : "0";
        else
          return leftOpd.size() > rightOpd.size() ? "1" : "0";
        }, 2);

      ir.addOperator("<", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (isNumber(leftOpd) && isNumber(rightOpd))
          return stoi(leftOpd) < stoi(rightOpd) ? "1" : "0";
        else
          return leftOpd.size() < rightOpd.size() ? "1" : "0";
        }, 2);

      ir.addOperator(">=", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (isNumber(leftOpd) && isNumber(rightOpd))
          return stoi(leftOpd) >= stoi(rightOpd) ? "1" : "0";
        else
          return leftOpd.size() >= rightOpd.size() ? "1" : "0";
        }, 2);

      ir.addOperator("<=", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (isNumber(leftOpd) && isNumber(rightOpd))
          return stoi(leftOpd) <= stoi(rightOpd) ? "1" : "0";
        else
          return leftOpd.size() <= rightOpd.size() ? "1" : "0";
        }, 2);

      ir.addOperator("=", [](std::string& leftOpd, std::string& rightOpd) ->std::string {
        leftOpd = rightOpd;
        return leftOpd;
        }, 100);
    }
  };
}