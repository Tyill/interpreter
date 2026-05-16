
#include "../../include/interpreter.h"
#include "containers.h"

#include <cctype>
#include <sstream>

namespace InterpreterBaseLib {

  class Structure {
  public:

    std::vector<std::string> split(const std::string& str, char sep) {
      std::vector<std::string> res;
      std::istringstream iss(str);
      std::string token;
      while (std::getline(iss, token, sep)) {
        res.emplace_back(token);
      }
      return res;
    }

    Structure(Interpreter& ir):
      m_intr(ir)
    {
      auto currOperator = ir.getUserOperator("=");
      ir.addOperator("=", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string leftKey = Interpreter::operandName(m_intr, leftOpd);
        const std::string rightKey = Interpreter::valueAsString(rightOpd);

        if (rightKey == "Struct") {
          m_structContr[leftKey] = std::string{};

          auto entityRight = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1);

          const std::string initBody = Interpreter::valueAsInitBody(entityRight.value);

          if (!initBody.empty()) {
            Interpreter intrCopy = m_intr;
            foreachCommaInitBody(initBody, [&](const std::string& segment) {
              auto args = split(segment, ':');
              Interpreter::Error err;
              if ((args.size() > 1) && intrCopy.parseScript(args[1], err)) {
                m_structContr[leftKey + '.' + args[0]] = intrCopy.runScript().first;
              }
              else if (!args.empty()) {
                m_structContr[leftKey + '.' + args[0]] = std::string{};
              }
            });
          }
        }
        else {
          const std::string leftFull = Interpreter::valueAsString(leftOpd);
          if (leftFull.find('.') != std::string::npos) {
            m_structContr[leftFull] = rightOpd;
            leftOpd = rightOpd;
            return rightOpd;
          }
        }
        if (m_structContr.count(leftKey)) {
          m_structContr[leftKey] = rightOpd;
          return rightOpd;
        }
        else if (currOperator){
          return currOperator(leftOpd, rightOpd);
        }
        return Interpreter::valueAsString(leftOpd) + '=' + Interpreter::valueAsString(rightOpd);
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string leftKey = Interpreter::operandName(m_intr, leftOpd);
        const std::string rightKey = Interpreter::valueAsString(rightOpd);

        if (m_structContr.count(leftKey)) {

          if (isEqualOfNextOperator(m_intr.currentEntity().beginIndex)) {
            const std::string fieldKey = leftKey + '.' + rightKey;
            if (!m_structContr.count(fieldKey))
              m_structContr[fieldKey] = std::string{};
            return fieldKey;
          }
          else {
            return m_structContr[leftKey + '.' + rightKey];
          }
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return Interpreter::valueAsString(leftOpd) + '.' + Interpreter::valueAsString(rightOpd);
      }, 0);
    }

    bool isEqualOfNextOperator(size_t beginIndex){

      return m_intr.getEntityByIndex(beginIndex + 2).name == "=";
    }

  protected:
    Interpreter& m_intr;
    std::map<std::string, Interpreter::Value> m_structContr;
  };
}
