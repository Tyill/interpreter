
#include "../../include/interpreter.h"

#include <cctype>

namespace InterpreterBaseLib {

  class ArithmeticOperations {
  public:
    bool isNumber(const std::string& s) const {
      for (auto c : s) {
        if (!std::isdigit(c)) {
          return false;
        }
      }
      return !s.empty();
    }
        
    ArithmeticOperations(Interpreter& ir)
    {
      ir.addOperator("*", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
      if (isNumber(leftOpd) && isNumber(rightOpd))
        return std::to_string(stoi(leftOpd) * stoi(rightOpd));
      else
        return "0";
      }, 0);

    ir.addOperator("/", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
      if (isNumber(leftOpd) && isNumber(rightOpd))
        return std::to_string(stoi(leftOpd) / stoi(rightOpd));
      else
        return "0";
      }, 0);

    ir.addOperator("+", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
      if (isNumber(leftOpd) && isNumber(rightOpd))
        return std::to_string(stoi(leftOpd) + stoi(rightOpd));
      else
        return leftOpd + rightOpd;
      }, 1);

    ir.addOperator("-", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
      if (isNumber(leftOpd) && isNumber(rightOpd))
        return std::to_string(stoi(leftOpd) - stoi(rightOpd));
      else
        return "0";
      }, 1);

    ir.addOperator("+=", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
    if (isNumber(leftOpd) && isNumber(rightOpd)){
      leftOpd = std::to_string(stoi(leftOpd) + stoi(rightOpd));
      return leftOpd;
    }    
    else{
      leftOpd += rightOpd;
      return leftOpd;
    }
  }, 4);

  ir.addOperator("-=", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
    if (isNumber(leftOpd) && isNumber(rightOpd)){
      leftOpd = std::to_string(stoi(leftOpd) - stoi(rightOpd));
      return leftOpd;
    }     
    else{
      leftOpd += rightOpd;
      return leftOpd;
    }
  }, 4);
  ir.addOperator("++", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
    if (isNumber(leftOpd)){
      leftOpd = std::to_string(stoi(leftOpd) + 1);
      return leftOpd;
    }    
    if (isNumber(rightOpd)){
      rightOpd = std::to_string(stoi(rightOpd) + 1);
    }
    return rightOpd;
  }, 4);

  ir.addOperator("--", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
    if (isNumber(leftOpd)){
      leftOpd = std::to_string(stoi(leftOpd) - 1);
      return leftOpd;
    }    
    if (isNumber(rightOpd)){
      rightOpd = std::to_string(stoi(rightOpd) - 1);
    }
    return rightOpd;
  }, 4);
    
  ir.addOperator("=", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
    leftOpd = rightOpd;
    return leftOpd;
  }, 100);

  }
  };
}