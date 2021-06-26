
#include "../../include/interpreter.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <set>

namespace InterpreterBaseLib {

  class Filesystem {
  public:
        
    Filesystem(Interpreter& ir):
      m_intr(ir)
    {      
      auto currOperator = ir.getUserOperator("=");
      ir.addOperator("=", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
                        
        if (rightOpd == "File") {
          
          auto entityRight = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1);

          m_fileHandler[leftOpd] = entityRight.value; // fpath
        }
        else if (currOperator){
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd;
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_fileHandler.count(leftOpd)) {
          return rightOpd;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd;
      }, 0);

      auto currFunction = ir.getUserFunction("read");
      ir.addFunction("read", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {          
          std::ifstream fs(m_fileHandler[contrName]);
          if (fs.good()) {
            std::stringstream strStream;
            strStream << fs.rdbuf();
            return strStream.str();
          }
          else {
            return "Error open file " + args[0];
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
          return "";
        });

      currFunction = ir.getUserFunction("write");
      ir.addFunction("write", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ofstream fs(m_fileHandler[contrName]);
          if (fs.good() && !args.empty()) {
            fs << args[0];
            return "1";
          }
          else
            return "0";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return "";
      });

      currFunction = ir.getUserFunction("append");
      ir.addFunction("append", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ofstream fs(m_fileHandler[contrName], std::ios_base::app);
          if (fs.good() && !args.empty()) {
            fs << args[0];
            return "1";
          }
          else
            return "0";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return "";
        });

      currFunction = ir.getUserFunction("exist");
      ir.addFunction("exist", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ifstream fs(m_fileHandler[contrName]);
          return fs.good() ? "1" : "0";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return "";
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
    std::map<std::string, std::string> m_fileHandler;
  };
}