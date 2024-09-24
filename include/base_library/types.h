
#include "../../include/interpreter.h"

namespace InterpreterBaseLib {

  class Types {
  public:
      
    Types(Interpreter& ir):
      m_intr(ir)
    {      
      auto currOperator = ir.getUserOperator(":");
      ir.addOperator(":", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {

        m_types[leftOpd] = rightOpd;
        return leftOpd;
      }, 0);

      ir.addFunction("type", [this](const std::vector<std::string>& args) ->std::string {

        if (!args.empty() && m_types.count(args[0])){
          return m_types[args[0]];
        }
        return "";
      });
    }
  protected:
    Interpreter& m_intr;
    std::map<std::string, std::string> m_types;
  };
}