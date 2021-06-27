
#include "../../include/interpreter.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>


namespace InterpreterBaseLib {

  class Filesystem {
  public:
        
    Filesystem(Interpreter& ir):
      m_intr(ir)
    {      
      auto currOperator = ir.getUserOperator("=");
      ir.addOperator("=", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
                        
        if (rightOpd == "File") {          
          auto initBody = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1).value;
          if (!initBody.empty()) {
            Interpreter intrCopy = m_intr;
            std::string err;
            if (intrCopy.parseScript(initBody, err))
              m_fileHandler[leftOpd] = intrCopy.runScript();
          }
          else
            m_fileHandler[leftOpd];
        }
        else if (rightOpd == "Dir") {
          auto initBody = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1).value;
          if (!initBody.empty()) {
            Interpreter intrCopy = m_intr;
            std::string err;
            if (intrCopy.parseScript(initBody, err))
              m_dirHandler[leftOpd] = intrCopy.runScript();
          }
          else
            m_dirHandler[leftOpd];
        }
        else if (currOperator){
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd;
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_fileHandler.count(leftOpd) || m_dirHandler.count(leftOpd)) {
          return rightOpd;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd + '.' + rightOpd;
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
            return "0";
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
        return "0";
      });

      currFunction = ir.getUserFunction("exist");
      ir.addFunction("exist", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName)) {
          std::ifstream fs(m_fileHandler[contrName]);
          return fs.good() ? "1" : "0";
        }
        else if (m_dirHandler.count(contrName)) {
          struct stat info;
          if (stat(m_dirHandler[contrName].c_str(), &info) != 0) // cannot access
            return "0";
          else if (info.st_mode & S_IFDIR)
            return "1";
          else
            return "0";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return "0";
      });

      currFunction = ir.getUserFunction("remove");
      ir.addFunction("remove", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_fileHandler.count(contrName) || m_dirHandler.count(contrName)) {
          return remove(m_fileHandler[contrName].c_str()) == 0 ? "1" : "0";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return "0";
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
    std::map<std::string, std::string> m_dirHandler;
  };
}