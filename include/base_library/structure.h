
#include "../../include/interpreter.h"

#include <cctype>
#include <sstream>

namespace InterpreterBaseLib {

  class Structure {
  public:

    bool isNumber(const std::string& s) const {
      for (auto c : s) {
        if (!std::isdigit(c)) {
          return false;
        }
      }
      return !s.empty();
    }

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
      ir.addOperator("=", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
        
        if (rightOpd == "Struct") {
          m_structContr[leftOpd] = "";

          auto entityRight = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1);

          std::string& initBody = entityRight.value;

          if (!initBody.empty()) {

            size_t ssz = initBody.size(),
              cpos = 0,
              cp = 0;
            int bordCnt = 0;

            Interpreter intrCopy = m_intr;

            while (cp < ssz) {
              if (initBody[cp] == '(') ++bordCnt;
              if (initBody[cp] == ')') --bordCnt;
              if (((initBody[cp] == ',') || (cp == ssz - 1)) && (bordCnt == 0)) {

                if (cp == ssz - 1) ++cp;

                auto args = split(initBody.substr(cpos, cp - cpos), ':');
                std::string err;
                if ((args.size() > 1) && intrCopy.parseScript(args[1], err))
                  m_structContr[leftOpd + '.' + args[0]] = intrCopy.runScript();
                else if (!args.empty())
                  m_structContr[leftOpd + '.' + args[0]] = "";

                cpos = cp + 1;
              }
              ++cp;
            }
          }         
        }
        else if (m_structContr.count(leftOpd)) {
          m_structContr[leftOpd] = rightOpd;
          return rightOpd;
        }
        else if (currOperator){
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd + '=' + rightOpd;
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](std::string& leftOpd, std::string& rightOpd) -> std::string {
        
        if (m_structContr.count(leftOpd)) {

          if (isEqualOfNextOperator(m_intr.currentEntity().beginIndex)) {
            if (!m_structContr.count(leftOpd + '.' + rightOpd))
              m_structContr[leftOpd + '.' + rightOpd] = "";
            return leftOpd + '.' + rightOpd;
          }
          else {
            return m_structContr[leftOpd + '.' + rightOpd];
          }          
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd + '.' + rightOpd;
      }, 0);
    }

    bool isEqualOfNextOperator(size_t beginIndex){
            
      return m_intr.getEntityByIndex(beginIndex + 2).name == "=";        
    }

  protected:
    Interpreter& m_intr;
    std::map<std::string, std::string> m_structContr;
  };
}