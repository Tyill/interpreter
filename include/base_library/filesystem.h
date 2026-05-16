
#include "../../include/interpreter.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>


namespace InterpreterBaseLib {

  class Filesystem {
  static Interpreter::Value evalInitBodyPath(Interpreter& ir, const std::string& initBody) {
    if (initBody.empty()) {
      return std::string{};
    }
    std::string script = initBody;
    if (script.front() != '"') {
      script = "\"" + initBody + "\"";
    }
    Interpreter intrCopy = ir;
    Interpreter::Error err;
    if (intrCopy.parseScript(script, err)) {
      return intrCopy.runScript().first;
    }
    return Interpreter::valueFromLiteral(initBody);
  }

  public:

    Filesystem(Interpreter& ir):
      m_intr(ir)
    {
      auto currOperator = ir.getUserOperator("=");
      ir.addOperator("=", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        std::string leftKey = Interpreter::operandName(m_intr, leftOpd);
        if (leftKey.empty()) {
          leftKey = Interpreter::valueAsString(leftOpd);
        }
        const size_t opIdx = m_intr.currentEntity().beginIndex;
        Interpreter::Entity rhsEntity;
        std::string typeName;
        for (int delta : {1, -1}) {
          const size_t idx = (delta > 0) ? opIdx + 1 : (opIdx > 0 ? opIdx - 1 : 0);
          const auto candidate = m_intr.getEntityByIndex(idx);
          if (candidate.name == "File" || candidate.name == "Dir") {
            typeName = candidate.name;
            rhsEntity = candidate;
            break;
          }
        }
        if (typeName.empty()) {
          typeName = Interpreter::valueAsString(rightOpd);
          rhsEntity = m_intr.getEntityByIndex(opIdx + 1);
        }
        const std::string initBody = Interpreter::valueAsInitBody(rhsEntity.value);

        if (typeName == "File") {
          if (!initBody.empty()) {
            m_fileHandler[leftKey] = evalInitBodyPath(m_intr, initBody);
          }
          else {
            m_fileHandler[leftKey];
          }
        }
        else if (typeName == "Dir") {
          if (!initBody.empty()) {
            m_dirHandler[leftKey] = evalInitBodyPath(m_intr, initBody);
          }
          else {
            m_dirHandler[leftKey];
          }
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd;
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string leftKey = Interpreter::valueAsString(leftOpd);
        if (m_fileHandler.count(leftKey) || m_dirHandler.count(leftKey)) {
          return rightOpd;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return Interpreter::valueAsString(leftOpd) + '.' + Interpreter::valueAsString(rightOpd);
      }, 0);

      auto currFunction = ir.getUserFunction("read");
      ir.addFunction("read", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ifstream fs(Interpreter::valueAsString(m_fileHandler[contrName]));
          if (fs.good()) {
            std::stringstream strStream;
            strStream << fs.rdbuf();
            return strStream.str();
          }
          else {
            return int64_t{0};
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return std::string{};
      });

      currFunction = ir.getUserFunction("write");
      ir.addFunction("write", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ofstream fs(Interpreter::valueAsString(m_fileHandler[contrName]));
          if (fs.good() && !args.empty()) {
            fs << Interpreter::valueToString(args[0]);
            return int64_t{1};
          }
          else
            return int64_t{0};
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return std::string{};
      });

      currFunction = ir.getUserFunction("append");
      ir.addFunction("append", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ofstream fs(Interpreter::valueAsString(m_fileHandler[contrName]), std::ios_base::app);
          if (fs.good() && !args.empty()) {
            fs << Interpreter::valueToString(args[0]);
            return int64_t{1};
          }
          else
            return int64_t{0};
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return int64_t{0};
      });

      currFunction = ir.getUserFunction("exist");
      ir.addFunction("exist", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ifstream fs(Interpreter::valueAsString(m_fileHandler[contrName]));
          return fs.good();
        }
        else if (m_dirHandler.count(contrName)) {
          struct stat info;
          if (stat(Interpreter::valueAsString(m_dirHandler[contrName]).c_str(), &info) != 0)
            return false;
          else if (info.st_mode & S_IFDIR)
            return true;
          else
            return false;
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return false;
      });

      currFunction = ir.getUserFunction("remove");
      ir.addFunction("remove", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          return remove(Interpreter::valueAsString(m_fileHandler[contrName]).c_str()) == 0;
        }
        if (m_dirHandler.count(contrName)) {
          return remove(Interpreter::valueAsString(m_dirHandler[contrName]).c_str()) == 0;
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return false;
      });
    }

    std::string getContrNameByFunction(size_t funcBeginIndex){

      std::string out;
      if ((funcBeginIndex - 1 >= 0) && (m_intr.getEntityByIndex(funcBeginIndex - 1).name == ".")){
        if (funcBeginIndex - 2 >= 0)
          out = m_intr.getEntityByIndex(funcBeginIndex - 2).name;
      }
      return out;
    }

  protected:
    Interpreter& m_intr;
    std::map<std::string, Interpreter::Value> m_fileHandler;
    std::map<std::string, Interpreter::Value> m_dirHandler;
  };
}
