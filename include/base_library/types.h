
#include "../../include/interpreter.h"

namespace InterpreterBaseLib {

  class Types {
  public:

    Types(Interpreter& ir):
      m_intr(ir)
    {
      ir.addOperator(":", [this](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        m_types[Interpreter::valueAsString(leftOpd)] = rightOpd;
        return leftOpd;
      }, 0);

      ir.addFunction("type", [this](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {
        if (!args.empty()) {
          const std::string name = Interpreter::valueAsString(args[0]);
          if (m_types.count(name)) {
            return m_types[name];
          }
        }
        return std::string{};
      });
    }
  protected:
    Interpreter& m_intr;
    std::map<std::string, Interpreter::Value> m_types;
  };
}
