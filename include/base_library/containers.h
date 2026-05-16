#pragma once

#include "../../include/interpreter.h"

#include <cctype>
#include <sstream>
#include <string>

namespace InterpreterBaseLib {

template<typename Fn>
inline void foreachCommaInitBody(const std::string& initBody, Fn&& fn) {
  const size_t ssz = initBody.size();
  size_t cpos = 0;
  int bordCnt = 0;
  for (size_t cp = 0; cp < ssz; ++cp) {
    if (initBody[cp] == '(') {
      ++bordCnt;
    }
    if (initBody[cp] == ')') {
      --bordCnt;
    }
    if (((initBody[cp] == ',') || (cp == ssz - 1)) && (bordCnt == 0)) {
      size_t end = cp;
      if (end == ssz - 1) {
        ++end;
      }
      fn(initBody.substr(cpos, end - cpos));
      cpos = cp + 1;
    }
  }
}

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
      ir.addOperator("=", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string leftKey = Interpreter::operandName(m_intr, leftOpd);
        const std::string rightKey = Interpreter::valueAsString(rightOpd);

        if (rightKey == "Vector") {
          m_vectorContr[leftKey] = std::vector<Interpreter::Value>();

          auto entityRight = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1);

          const std::string initBody = Interpreter::valueAsInitBody(entityRight.value);

          if (!initBody.empty()) {
            Interpreter intrCopy = m_intr;
            foreachCommaInitBody(initBody, [&](const std::string& arg) {
              Interpreter::Error err;
              if (intrCopy.parseScript(arg, err)) {
                m_vectorContr[leftKey].push_back(intrCopy.runScript().first);
              }
            });
          }
        }
        else if (rightKey == "Map") {
          m_mapContr[leftKey] = std::map<std::string, Interpreter::Value>();

          auto entityRight = m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex + 1);

          const std::string initBody = Interpreter::valueAsInitBody(entityRight.value);

          if (!initBody.empty()) {
            Interpreter intrCopy = m_intr;
            foreachCommaInitBody(initBody, [&](const std::string& segment) {
              auto args = split(segment, ':');
              Interpreter::Error err;
              if ((args.size() > 1) && intrCopy.parseScript(args[1], err)) {
                m_mapContr[leftKey][args[0]] = intrCopy.runScript().first;
              }
              else if (!args.empty()) {
                m_mapContr[leftKey][args[0]] = std::string{};
              }
            });
          }
        }
        else if (currOperator){
          return currOperator(leftOpd, rightOpd);
        }
        return leftOpd;
      }, 100);

      currOperator = ir.getUserOperator(".");
      ir.addOperator(".", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string leftKey = Interpreter::operandName(m_intr, leftOpd);
        if (m_vectorContr.count(leftKey) || m_mapContr.count(leftKey)) {
          return rightOpd;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return Interpreter::valueAsString(leftOpd) + '.' + Interpreter::valueAsString(rightOpd);
      }, 0);

      currOperator = ir.getUserOperator("[");
      ir.addOperator("[", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string leftKey = Interpreter::operandName(m_intr, leftOpd);
        if (m_vectorContr.count(leftKey) || m_mapContr.count(leftKey)) {
          const std::string value = Interpreter::valueAsString(m_intr.getEntityByIndex(m_intr.currentEntity().beginIndex - 1).value);
          Interpreter intrCopy = m_intr;
          Interpreter::Error err;
          if (!value.empty() && intrCopy.parseScript(value, err)){
            const std::string key = Interpreter::valueToString(intrCopy.runScript().first);
            if (m_vectorContr.count(leftKey)){
              auto ix = isNumber(key) ? stoi(key) : -1;
              if (0 <= ix && ix < (int)m_vectorContr[leftKey].size()){
                return m_vectorContr[leftKey][ix];
              }
            }else if (m_mapContr[leftKey].count(key)){
              return m_mapContr[leftKey][key];
            }
          } else if (value.empty()){
            return std::string{"error value.empty"};
          }
          return err.message;
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return int64_t{0};
      }, 0);

      currOperator = ir.getUserOperator(":");
      ir.addOperator(":", [this, currOperator](Interpreter::Value& leftOpd, Interpreter::Value& rightOpd) -> Interpreter::Value {
        const std::string rightKey = Interpreter::valueAsString(rightOpd);
        if (m_vectorContr.count(rightKey)) {

          int itPos = Interpreter::valueIsInteger(m_intr.currentEntity().value) ? (int)Interpreter::valueAsInt64(m_intr.currentEntity().value) : 0;

          if (itPos < (int)m_vectorContr[rightKey].size()) {
            leftOpd = m_vectorContr[rightKey][itPos];
            return static_cast<int64_t>(++itPos);
          }
          else return int64_t{0};
        }
        else if (m_mapContr.count(rightKey)) {

          int itPos = Interpreter::valueIsInteger(m_intr.currentEntity().value) ? (int)Interpreter::valueAsInt64(m_intr.currentEntity().value) : 0;

          if (itPos < (int)m_mapContr[rightKey].size()) {
            int cpos = 0;
            for (auto& v : m_mapContr[rightKey]) {
              leftOpd = Interpreter::valueAsString(v.first) + '\t' + Interpreter::valueAsString(v.second);
              ++cpos;
              if (cpos > itPos) break;
            }
            return static_cast<int64_t>(++itPos);
          }
          else return int64_t{0};
        }
        else if (currOperator) {
          return currOperator(leftOpd, rightOpd);
        }
        return Interpreter::valueAsString(leftOpd) + ':' + Interpreter::valueAsString(rightOpd);
        }, 0);

      auto currFunction = ir.getUserFunction("push_back");
      ir.addFunction("push_back", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string vecName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        Interpreter::Value ok = int64_t{0};
        if (m_vectorContr.count(vecName)) {
          for (auto& a : args)
            m_vectorContr[vecName].push_back(a);
          ok = int64_t{1};
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
      });

      currFunction = ir.getUserFunction("pop_back");
      ir.addFunction("pop_back", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string vecName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        Interpreter::Value ok = int64_t{0};
        if (m_vectorContr.count(vecName)) {
          if (!m_vectorContr[vecName].empty()) {
            m_vectorContr[vecName].pop_back();
            ok = int64_t{1};
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("insert");
      ir.addFunction("insert", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        Interpreter::Value ok = int64_t{0};
        if (m_vectorContr.count(contrName)) {
          if ((args.size() > 1) && Interpreter::valueIsInteger(args[0])) {
            size_t inx = size_t(Interpreter::valueAsInt64(args[0]));
            if (m_vectorContr[contrName].size() > inx) {
              m_vectorContr[contrName].insert(m_vectorContr[contrName].begin() + inx, args[1]);
              ok = int64_t{1};
            }
          }
        }
        else if (m_mapContr.count(contrName)) {
          if (args.size() > 1) {
            m_mapContr[contrName][Interpreter::valueAsString(args[0])] = args[1];
            ok = int64_t{1};
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("erase");
      ir.addFunction("erase", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        Interpreter::Value ok = int64_t{0};
        if (m_vectorContr.count(contrName)) {
          if (!args.empty() && Interpreter::valueIsInteger(args[0])) {
            size_t inx = size_t(Interpreter::valueAsInt64(args[0]));
            if (m_vectorContr[contrName].size() > inx) {
              m_vectorContr[contrName].erase(m_vectorContr[contrName].begin() + inx);
              ok = int64_t{1};
            }
          }
        }
        else if (m_mapContr.count(contrName)) {
          const std::string key = Interpreter::valueAsString(args[0]);
          if (!args.empty() && m_mapContr[contrName].count(key)) {
            m_mapContr[contrName].erase(key);
            ok = int64_t{1};
          }
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("size");
      ir.addFunction("size", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName))
          return static_cast<int64_t>(m_vectorContr[contrName].size());
        else if (m_mapContr.count(contrName))
          return static_cast<int64_t>(m_mapContr[contrName].size());
        else if (currFunction) {
          return currFunction(args);
        }
        return std::string{};
      });

      currFunction = ir.getUserFunction("empty");
      ir.addFunction("empty", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName))
          return m_vectorContr[contrName].empty();
        else if (m_mapContr.count(contrName))
          return m_mapContr[contrName].empty();
        else if (currFunction) {
          return currFunction(args);
        }
        return std::string{};
      });

      currFunction = ir.getUserFunction("clear");
      ir.addFunction("clear", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        Interpreter::Value ok = int64_t{0};
        if (m_vectorContr.count(contrName)) {
          m_vectorContr[contrName].clear();
          ok = int64_t{1};
        }
        else if (m_mapContr.count(contrName)) {
          m_mapContr[contrName].clear();
          ok = int64_t{1};
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return ok;
        });

      currFunction = ir.getUserFunction("at");
      ir.addFunction("at", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        if (m_vectorContr.count(contrName)) {
          if (!args.empty() && Interpreter::valueIsInteger(args[0])) {
            size_t inx = size_t(Interpreter::valueAsInt64(args[0]));
            if (m_vectorContr[contrName].size() > inx)
              return m_vectorContr[contrName][inx];
          }
        }
        else if (m_mapContr.count(contrName)) {
          const std::string key = Interpreter::valueAsString(args[0]);
          if (!args.empty() && m_mapContr[contrName].count(key))
            return m_mapContr[contrName][key];
        }
        else if (currFunction) {
          return currFunction(args);
        }
        return std::string{};
      });

      currFunction = ir.getUserFunction("set");
      ir.addFunction("set", [this, currFunction](const std::vector<Interpreter::Value>& args) -> Interpreter::Value {

        std::string contrName = getContrNameByFunction(m_intr.currentEntity().beginIndex);

        Interpreter::Value ok = int64_t{0};
        if (m_vectorContr.count(contrName)) {
          if ((args.size() > 1) && Interpreter::valueIsInteger(args[0])) {
            size_t inx = size_t(Interpreter::valueAsInt64(args[0]));
            if (m_vectorContr[contrName].size() > inx) {
              m_vectorContr[contrName][inx] = args[1];
              ok = int64_t{1};
            }
          }
        }
        else if (m_mapContr.count(contrName)) {
          const std::string key = Interpreter::valueAsString(args[0]);
          if ((args.size() > 1) && m_mapContr[contrName].count(key)) {
            m_mapContr[contrName][key] = args[1];
            ok = int64_t{1};
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
    std::map<std::string, std::vector<Interpreter::Value>> m_vectorContr;
    std::map<std::string, std::map<std::string, Interpreter::Value>> m_mapContr;
  };
}