
#include "../../include/interpreter.h"

#include <cctype>
#include <sstream>

namespace InterpreterBaseLib {

  class Container {
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

    Container(Interpreter& ir):
      m_intr(ir)
    {      
      auto currOperator = ir.getUserOperator("=");
      ir.addOperator("=", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
                        
        if (rightOpd == "Vector") {
          m_vectorContr[leftOpd] = std::vector<std::string>();

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

                const std::string arg = initBody.substr(cpos, cp - cpos);
                std::string err;
                if (intrCopy.parseScript(arg, err))
                  m_vectorContr[leftOpd].push_back(intrCopy.runScript());

                cpos = cp + 1;
              }
              ++cp;
            }
          }
        }
        else if (rightOpd == "Map") {
          m_mapContr[leftOpd] = std::map<std::string, std::string>();

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
                  m_mapContr[leftOpd][args[0]] = intrCopy.runScript();
                else if (!args.empty())
                  m_mapContr[leftOpd][args[0]] = "";

                cpos = cp + 1;
              }
              ++cp;
            }
          }         
        }
        else if (currOperator){
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd;
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_vectorContr.count(leftOpd) || m_mapContr.count(leftOpd)) {
          return rightOpd;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd + '.' + rightOpd;
      }, 0);

      currOperator = ir.getUserOperator("[");
      ir.addOperator("[", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_vectorContr.count(leftOpd) || m_mapContr.count(leftOpd)) {
          auto value = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex - 1).value;
          Interpreter intrCopy = m_intr;
          std::string err;
          if (!value.empty() && intrCopy.parseScript(value, err)){
            auto key = intrCopy.runScript();
            if (m_vectorContr.count(leftOpd)){
              auto ix = isNumber(key) ? stoi(key) : -1;
              if (0 <= ix && ix < m_vectorContr[leftOpd].size()){
                return m_vectorContr[leftOpd][ix];
              }
            }else if (m_mapContr[leftOpd].count(key)){
              return m_mapContr[leftOpd][key];
            }
          } else if (value.empty()){
            err = "error value.empty";
          }
          return err;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return "0";
      }, 0);

      currOperator = ir.getUserOperator(":");
      ir.addOperator(":", [this, currOperator](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_vectorContr.count(rightOpd)) {
          
          int itPos = m_intr.currentEntity().value.empty() ? 0 : stoi(m_intr.currentEntity().value);

          if (itPos < (int)m_vectorContr[rightOpd].size()) {
            leftOpd = m_vectorContr[rightOpd][itPos];
            return std::to_string(++itPos);
          }
          else return "0";
        }
        else if (m_mapContr.count(rightOpd)) {

          int itPos = m_intr.currentEntity().value.empty() ? 0 : stoi(m_intr.currentEntity().value);

          if (itPos < (int)m_mapContr[rightOpd].size()) {
            int cpos = 0;
            for (auto& v : m_mapContr[rightOpd]) {
              leftOpd = v.first + '\t' + v.second;
              ++cpos;
              if (cpos > itPos) break;
            }       
            return std::to_string(++itPos);
          }
          else return "0";
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd + ':' + rightOpd;
        }, 0);

      auto currFunction = ir.getUserFunction("push_back");
      ir.addFunction("push_back", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string vecName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(vecName)) {
          for (auto& a : args)
            m_vectorContr[vecName].push_back(a);
          ok = "1";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
      });

      currFunction = ir.getUserFunction("pop_back");
      ir.addFunction("pop_back", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string vecName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(vecName)) {
          if (!m_vectorContr[vecName].empty()) {
            m_vectorContr[vecName].pop_back();
            ok = "1";
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("insert");
      ir.addFunction("insert", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(contrName)) {
          if ((args.size() > 1) && isNumber(args[0])) {
            size_t inx = size_t(stoi(args[0]));
            if (m_vectorContr[contrName].size() > inx) {
              m_vectorContr[contrName].insert(m_vectorContr[contrName].begin() + inx, args[1]);
              ok = "1";
            }
          }
        }
        else if (m_mapContr.count(contrName)) {
          if (args.size() > 1) {
            m_mapContr[contrName][args[0]] = args[1];
            ok = "1";
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("erase");
      ir.addFunction("erase", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(contrName)) {
          if (!args.empty() && isNumber(args[0])) {
            size_t inx = size_t(stoi(args[0]));
            if (m_vectorContr[contrName].size() > inx) {
              m_vectorContr[contrName].erase(m_vectorContr[contrName].begin() + inx);
              ok = "1";
            }
          }
        }
        else if (m_mapContr.count(contrName)) {
          if (!args.empty() && m_mapContr[contrName].count(args[0])) {
            m_mapContr[contrName].erase(args[0]);
            ok = "1";
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("size");
      ir.addFunction("size", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName))
          return std::to_string(m_vectorContr[contrName].size());
        else if (m_mapContr.count(contrName))
          return std::to_string(m_mapContr[contrName].size());
        else if (currFunction) {
          return currFunction(args);
        }
        return "";
      });

      currFunction = ir.getUserFunction("empty");
      ir.addFunction("empty", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName))
          return m_vectorContr[contrName].empty() ? "1" : "0";
        else if (m_mapContr.count(contrName))
          return  m_mapContr[contrName].empty() ? "1" : "0";
        else if (currFunction) {
          return currFunction(args);
        }
        return "";
      });

      currFunction = ir.getUserFunction("clear");
      ir.addFunction("clear", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(contrName)) {
          m_vectorContr[contrName].clear();
          ok = "1";
        }
        else if (m_mapContr.count(contrName)) {
          m_mapContr[contrName].clear();
          ok = "1";
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("at");
      ir.addFunction("at", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string out;
        if (m_vectorContr.count(contrName)) {
          if (!args.empty() && isNumber(args[0])) {
            size_t inx = size_t(stoi(args[0]));
            if (m_vectorContr[contrName].size() > inx)
              out = m_vectorContr[contrName][inx];
          }
        }
        else if (m_mapContr.count(contrName)) {
          if (!args.empty() && m_mapContr[contrName].count(args[0]))
            out = m_mapContr[contrName][args[0]];
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return out;
      });

      currFunction = ir.getUserFunction("set");
      ir.addFunction("set", [this, currFunction](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(contrName)) {
          if ((args.size() > 1) && isNumber(args[0])) {
            size_t inx = size_t(stoi(args[0]));
            if (m_vectorContr[contrName].size() > inx) {
              m_vectorContr[contrName][inx] = args[1];
              ok = "1";
            }
          }
        }
        else if (m_mapContr.count(contrName)) {
          if ((args.size() > 1) && m_mapContr[contrName].count(args[0])) {
            m_mapContr[contrName][args[0]] = args[1];
            ok = "1";
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
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
    std::map<std::string, std::vector<std::string>> m_vectorContr;
    std::map<std::string, std::map<std::string, std::string>> m_mapContr;
  };
}