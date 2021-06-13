
#include "../../include/interpreter.h"

#include <cctype>

namespace InterpreterBaseLib {

  class BaseContainer {
  public:

    bool isNumber(const std::string& s) const {
      for (auto c : s) {
        if (!std::isdigit(c)) {
          return false;
        }
      }
      return !s.empty();
    }

    BaseContainer(Interpreter& ir):
      m_intr(ir)
    {      
      ir.addOperator("=", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {        
        if (rightOpd == "Vector") {
          m_vectorContr[leftOpd] = std::vector<std::string>();
        }
        else if (rightOpd == "Map") {
          m_mapContr[leftOpd] = std::map<std::string, std::string>();
        }
        else {
          leftOpd = rightOpd;
        }
        return leftOpd;
      }, 100);

      ir.addOperator(".", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_vectorContr.count(leftOpd) || m_mapContr.count(leftOpd)) {
          return rightOpd;
        }
        return leftOpd;
      }, 0);

      ir.addOperator(":", [this](std::string& leftOpd, std::string& rightOpd) ->std::string {
        if (m_vectorContr.count(rightOpd)) {
          
          int itPos = m_intr.currentEntity().value.empty() ? 0 : stoi(m_intr.currentEntity().value);

          if (itPos < m_vectorContr[rightOpd].size()) {
            leftOpd = m_vectorContr[rightOpd][itPos];
            return std::to_string(++itPos);
          }
          else return "0";
        }
        else if (m_mapContr.count(rightOpd)) {

          int itPos = m_intr.currentEntity().value.empty() ? 0 : stoi(m_intr.currentEntity().value);

          if (itPos < m_mapContr[rightOpd].size()) {
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
        return leftOpd;
        }, 0);

      ir.addFunction("push_back", [this](const std::vector<std::string>& args) ->std::string {

        std::string vecName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(vecName) && !args.empty()) {
          for (auto& a : args)
            m_vectorContr[vecName].push_back(a);
          ok = "1";
        }
        return ok;
        });

      ir.addFunction("pop_back", [this](const std::vector<std::string>& args) ->std::string {

        std::string vecName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(vecName) && !m_vectorContr[vecName].empty()) {
          m_vectorContr[vecName].pop_back();
          ok = "1";
        }
        return ok;
        });

      ir.addFunction("insert", [this](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(contrName) && (args.size() > 1) && isNumber(args[0])) {
          size_t inx = size_t(stoi(args[0]));
          if (m_vectorContr[contrName].size() > inx) {
            m_vectorContr[contrName].insert(m_vectorContr[contrName].begin() + inx, args[1]);
            ok = "1";
          }
        }
        else if (m_mapContr.count(contrName) && (args.size() > 1)) {
          m_mapContr[contrName][args[0]] = args[1];
          ok = "1";
        }
        return ok;
        });

      ir.addFunction("erase", [this](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string ok = "0";
        if (m_vectorContr.count(contrName) && !args.empty() && isNumber(args[0])) {
          size_t inx = size_t(stoi(args[0]));
          if (m_vectorContr[contrName].size() > inx) {
            m_vectorContr[contrName].erase(m_vectorContr[contrName].begin() + inx);
            ok = "1";
          }
        }
        else if (m_mapContr.count(contrName) && !args.empty() && m_mapContr[contrName].count(args[0])) {
          m_mapContr[contrName].erase(args[0]);
          ok = "1";
        }
        return ok;
        });

      ir.addFunction("size", [this](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName))
          return std::to_string(m_vectorContr[contrName].size());
        else if (m_mapContr.count(contrName))
          return std::to_string(m_mapContr[contrName].size());
        else
          return "";
      });

      ir.addFunction("empty", [this](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName))
          return m_vectorContr[contrName].empty() ? "1" : "0";
        else if (m_mapContr.count(contrName))
          return  m_mapContr[contrName].empty() ? "1" : "0";
        else
          return "";
      });

      ir.addFunction("clear", [this](const std::vector<std::string>& args) ->std::string {

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
        return ok;
        });

      ir.addFunction("at", [this](const std::vector<std::string>& args) ->std::string {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        std::string out;
        if (m_vectorContr.count(contrName) && !args.empty() && isNumber(args[0])) {
          size_t inx = size_t(stoi(args[0]));
          if (m_vectorContr[contrName].size() > inx)
            out = m_vectorContr[contrName][inx];
        }
        else if (m_mapContr.count(contrName) && !args.empty()) {
          if (m_mapContr[contrName].count(args[0]))
            out = m_mapContr[contrName][args[0]];
        }
        return out;
        });
    }

    std::string getContrNameByFunction(size_t funcBeginIndex){

      auto entities = m_intr.allEntities();

      std::string out;
      if ((funcBeginIndex - 1 >= 0) && (entities[funcBeginIndex - 1].name == ".")){
        if (funcBeginIndex - 2 >= 0)
          out = entities[funcBeginIndex - 2].name;
      }
      return out;
    }


  protected:
    Interpreter& m_intr;
    std::map<std::string, std::vector<std::string>> m_vectorContr;
    std::map<std::string, std::map<std::string, std::string>> m_mapContr;
  };
}