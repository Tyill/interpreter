//
// Interpreter Project
// Copyright (C) 2020 by Contributors <https://github.com/Tyill/interpreter>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "../include/interpreter.h"

#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

class InterpreterImpl {
public:
  InterpreterImpl() = default;
  bool addFunction(const string& name, Interpreter::UserFunction ufunc);
  bool addOperator(const string& name, Interpreter::UserOperator uopr, uint32_t priority);
  string cmd(string scenar);
  bool parseScript(string script, string& outErr);
  string runScript();
  std::map<std::string, std::string> allVariables() const;
  std::string variable(const std::string& vname) const;
  std::string runFunction(const std::string& fname, const std::vector<std::string>& args);
  bool setVariable(const std::string& vname, const std::string& value);
  bool setMacro(const std::string& mname, const std::string& script);
  bool gotoOnLabel(const std::string& lname);
  void exitFromScript();
  std::vector<Interpreter::Entity> allEntities();
  Interpreter::Entity currentEntity();
  Interpreter::Entity getEntityByIndex(size_t beginIndex);
  bool gotoOnEntity(size_t iBegin);
  Interpreter::UserFunction getUserFunction(const std::string& fname);
  Interpreter::UserOperator getUserOperator(const std::string& oname);
private:
  enum class Keyword {
    INSTRUCTION,
    EXPRESSION,
    OPERATOR,
    WHILE,
    IF,
    ELSE,
    ELSE_IF,
    BREAK,
    CONTINUE,
    FUNCTION,
    ARGUMENT,
    MACRO,
    VARIABLE,
    VALUE,
    GOTO,
  };
  struct Expression {
    Keyword keyw;
    size_t iConditionEnd;
    size_t iBodyEnd;
    size_t iOperator;
    string params;
    string result;
  };
  struct Operatr {
    size_t inx, priority, iLOpr, iROpr;
  };
  map<string, Interpreter::UserFunction> m_ufunc;
  map<string, pair<Interpreter::UserOperator, uint32_t>> m_uoper; // operator, priority
  map<string, string> m_var;
  map<string, string> m_macro;
  map<string, size_t> m_label;
  map<size_t, vector<Operatr>> m_soper;
  vector<Expression> m_expr;
  string m_err, m_prevScenar;
  size_t m_gotoIndex = size_t(-1);
  size_t m_currentIndex = 0;
  bool m_exit = false;

  string calcOperation(Keyword mainKeyword, size_t iExpr);
  string calcCondition(size_t iExpr);
  string calcExpression(size_t iBegin, size_t iEnd);
  void calcOperatorPriority(size_t iBegin, size_t iEnd, vector<Operatr>& oprs);

  bool parseInstructionScenar(string& scenar, size_t gpos);
  bool parseExpressionScenar(string& scenar, size_t gpos);
  bool parseArgumentScenar(string& scenar, size_t gpos);
  bool parseMacroArgs(const string& args, string& macro);

  void cleaningScenar(string& scenar) const;
  bool checkScenar(const string& scenar, string& err) const;

  bool startWith(const string& str, size_t pos, const string& begin) const;
  bool isNumber(const string& s) const;
  bool isFindKeySymbol(const string& scenar, size_t cpos, size_t maxpos) const;
  Keyword keywordByName(const string& oprName) const;
  Interpreter::EntityType keywordToEntityType(Keyword keyw) const;
  string getNextParam(const string& scenar, size_t& cpos, char symb) const;
  string getOperatorAtFirst(const string& scenar, size_t& cpos) const;
  string getFunctionAtFirst(const string& scenar, size_t& cpos) const;
  string getMacroAtFirst(const string& scenar, size_t& cpos) const;
  string getNextOperator(const string& scenar, size_t& cpos) const;
  string getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd) const;
};

string InterpreterImpl::cmd(string scenar) {
    
  string err;
  if (!parseScript(move(scenar), err))
    return err;

  return runScript();
}

bool InterpreterImpl::parseScript(string scenar, string& err) {

  cleaningScenar(scenar);

  if (scenar.empty()) {
    err = "Error: empty scenar";
    return false;
  }

  if (scenar.back() != ';') scenar += ';';

  if (m_prevScenar != scenar) {
    m_prevScenar = scenar;
    m_expr.clear();
    m_label.clear();
    m_soper.clear();
    m_err.clear();
    if (!checkScenar(scenar, m_err) || !parseInstructionScenar(scenar, 0)) {
      m_prevScenar.clear();
      err = m_err;
      return false;
    }
  }
  return true;
}

string InterpreterImpl::runScript() {

  for (auto& ex : m_expr)
    ex.iOperator = size_t(-1);

  string result;
  m_exit = false;
  for (size_t i = 0; i < m_expr.size();) {

    result = calcOperation(m_expr[i].keyw, i);
    i = max(m_expr[i].iConditionEnd, m_expr[i].iBodyEnd);

    if (m_gotoIndex != size_t(-1)) {
      for (size_t j = m_gotoIndex; j < i; ++j)
        m_expr[j].iOperator = size_t(-1);
      i = m_gotoIndex;
      m_gotoIndex = size_t(-1);
    }
    if (m_exit) break;
  }
  return result;
}

void InterpreterImpl::cleaningScenar(string& scenar) const {

  // del comments
  size_t commp = scenar.find("//");
  while (commp != string::npos) {
    size_t endStr = scenar.find("\n");
    if (endStr != string::npos)
      scenar.erase(commp, endStr - commp + 1);
    else
      scenar.erase(commp);
    commp = scenar.find("//");
  }

  // save string "value"
  vector<string> strVals;
  size_t beginStr = scenar.find("\"");
  while (beginStr != string::npos) {
    size_t endStr = scenar.find("\"", beginStr + 1);
    if (endStr != string::npos) {
      strVals.emplace_back(scenar.substr(beginStr, endStr - beginStr + 1));
      beginStr = scenar.find("\"", endStr + 1);
    }
    else break;
  }

  // cleaning
  scenar.erase(remove(scenar.begin(), scenar.end(), '\n'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\t'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\v'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\f'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\r'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), ' '), scenar.end());

  // restore string "value"
  beginStr = scenar.find("\"");
  size_t iVal = 0;
  while ((beginStr != string::npos) && (iVal < strVals.size())) {
    size_t endStr = scenar.find("\"", beginStr + 1);
    if (endStr != string::npos) {
      scenar.replace(beginStr, endStr - beginStr + 1, strVals[iVal]);
      beginStr = scenar.find("\"", beginStr + strVals[iVal].size() + 1);
      ++iVal;
    }
    else break;
  }
}

bool InterpreterImpl::checkScenar(const string& scenar, string& err) const {

#define CHECK_SCENAR_RETURN(condition)                                                \
  if (condition){                                                       \
    if (err.empty()) err = "Error check scenar: " + string(#condition); \
    return false;                                                       \
  }  

  CHECK_SCENAR_RETURN(std::count(scenar.begin(), scenar.end(), '{') != std::count(scenar.begin(), scenar.end(), '}'));
  CHECK_SCENAR_RETURN(std::count(scenar.begin(), scenar.end(), '(') != std::count(scenar.begin(), scenar.end(), ')'));
  CHECK_SCENAR_RETURN(std::count(scenar.begin(), scenar.end(), '"') % 2 != 0);

#undef CHECK_SCENAR_RETURN

  return true;
}

bool InterpreterImpl::addFunction(const string& name, Interpreter::UserFunction ufunc) {
  if (name.empty() || (keywordByName(name) != Keyword::INSTRUCTION) || isFindKeySymbol(name, 0, name.size())) return false;
  m_ufunc[name] = move(ufunc);
  return true;
}
bool InterpreterImpl::addOperator(const string& name, Interpreter::UserOperator uopr, uint32_t priority) {
  if (name.empty() || (keywordByName(name) != Keyword::INSTRUCTION) || isFindKeySymbol(name, 0, name.size())) return false;
  m_uoper[name] = {move(uopr), priority};
  return true;
}

std::map<std::string, std::string> InterpreterImpl::allVariables() const {
  return m_var;
}
std::string InterpreterImpl::variable(const std::string& vname) const {
  return m_var.find(vname) != m_var.end() ? m_var.at(vname) : "";
}
bool InterpreterImpl::setVariable(const std::string& vname, const std::string& value) {
  m_var[vname] = value;
  return true;
}
std::string InterpreterImpl::runFunction(const std::string& fname, const std::vector<std::string>& args) {
  return m_ufunc.count(fname) ? m_ufunc[fname](args) : "";
}
bool InterpreterImpl::setMacro(const std::string& mname, const std::string& script) {
  m_macro[mname] = script;
  return true;
}
bool InterpreterImpl::gotoOnLabel(const std::string& lname) {
  bool exist = m_label.find(lname) != m_label.end();
  if (exist)
    m_gotoIndex = m_label[lname];
  return exist;
}
void InterpreterImpl::exitFromScript() {
  m_exit = true;
}
std::vector<Interpreter::Entity> InterpreterImpl::allEntities() {
  std::vector<Interpreter::Entity> res;
  for (size_t i = 0; i < m_expr.size(); ++i) {    
    const auto& exp = m_expr[i];
    res.emplace_back(Interpreter::Entity{
      i, exp.iConditionEnd, exp.iBodyEnd, keywordToEntityType(exp.keyw), exp.params, exp.result
    });
  }
  return res;
}
Interpreter::Entity InterpreterImpl::currentEntity() {
  if (m_currentIndex >= m_expr.size())
    return Interpreter::Entity{0};
  const auto& exp = m_expr[m_currentIndex];
  return Interpreter::Entity{
      m_currentIndex, exp.iConditionEnd, exp.iBodyEnd, keywordToEntityType(exp.keyw), exp.params, exp.result
  };
}
Interpreter::Entity InterpreterImpl::getEntityByIndex(size_t beginIndex) {
  if (beginIndex >= m_expr.size())
    return Interpreter::Entity{ 0 };
  const auto& exp = m_expr[beginIndex];
  return Interpreter::Entity{
      beginIndex, exp.iConditionEnd, exp.iBodyEnd, keywordToEntityType(exp.keyw), exp.params, exp.result
  };
}
bool InterpreterImpl::gotoOnEntity(size_t beginIndex) {
  if (beginIndex < m_expr.size()) {
    m_gotoIndex = beginIndex;
    return true;
  }
  return false;
}
Interpreter::UserFunction InterpreterImpl::getUserFunction(const std::string& fname) {
  return m_ufunc.count(fname) ? m_ufunc[fname] : nullptr;
}
Interpreter::UserOperator InterpreterImpl::getUserOperator(const std::string& oname) {
  return m_uoper.count(oname) ? m_uoper[oname].first : nullptr;
}

string InterpreterImpl::calcOperation(Keyword mainKeyword, size_t iExpr) {

  string g_result;
  switch (mainKeyword) {
  case Keyword::VARIABLE:
    g_result = m_var[m_expr[iExpr].params];
    break;
  case Keyword::VALUE:
    g_result = m_expr[iExpr].params;
    break;
  case Keyword::EXPRESSION:
    g_result = m_expr[iExpr].result = calcExpression(iExpr + 1, m_expr[iExpr].iBodyEnd);
    break;
  case Keyword::FUNCTION: {
    size_t iBegin = iExpr + 1;
    size_t iEnd = m_expr[iExpr].iConditionEnd;
    vector<string> args;
    for (size_t i = iBegin; i < iEnd;) {
      if ((i + 1 == m_expr[i].iBodyEnd - 1) && ((m_expr[i + 1].keyw == Keyword::VARIABLE) || (m_expr[i + 1].keyw == Keyword::VALUE))) {
        if (m_expr[i + 1].keyw == Keyword::VARIABLE)
          m_expr[i].result = m_var[m_expr[i + 1].params];
        else
          m_expr[i].result = m_expr[i + 1].params;
      }
      else{
        m_expr[i].result = calcExpression(i + 1, m_expr[i].iBodyEnd); 
      }
      args.emplace_back(m_expr[i].result);
      i = m_expr[i].iBodyEnd;
    }
    m_currentIndex = iExpr;
    g_result = m_expr[iExpr].result = m_ufunc[m_expr[iExpr].params](args);
  }
    break;
  case Keyword::WHILE:
  case Keyword::IF:
  case Keyword::ELSE:
  case Keyword::ELSE_IF:
    g_result = calcCondition(iExpr);
    break;
  case Keyword::GOTO: {
    if (m_label.find(m_expr[iExpr].params) != m_label.end())
      m_gotoIndex = m_label[m_expr[iExpr].params];
  }
    break;
  default:
    break;
  }
  return g_result;
}
string InterpreterImpl::calcCondition(size_t iExpr) {

  string g_result;
  size_t iBegin = iExpr + 1;
  size_t iCondEnd = m_expr[iExpr].iConditionEnd;
  size_t iBodyEnd = m_expr[iExpr].iBodyEnd;
  if ((m_expr[iExpr].keyw == Keyword::ELSE) || (m_expr[iExpr].keyw == Keyword::ELSE_IF)) {
    size_t iIF = stoul(m_expr[iExpr].params);
    if (iIF != size_t(-1)) {
      string ifCondn = m_expr[iIF].result;
      bool isNum = isNumber(ifCondn);
      if ((isNum && (stoi(ifCondn) != 0)) || (!isNum && !ifCondn.empty())) {
        return g_result;
      }
    }
    else return g_result;
  }
  string condn;
  if (iBegin < iCondEnd) {
    condn = m_expr[iExpr].result = calcExpression(iBegin, iCondEnd);
  }
  bool isNum = isNumber(condn);
  if ((m_expr[iExpr].keyw == Keyword::ELSE) || (isNum && (stoi(condn) != 0)) || (!isNum && !condn.empty())) {
    bool isContinue = false,
      isBreak = false;
    for (size_t i = iCondEnd; i < iBodyEnd;) {
      switch (m_expr[i].keyw) {
      case Keyword::EXPRESSION: {
        m_expr[i].result = calcExpression(i + 1, m_expr[i].iBodyEnd);
        i = m_expr[i].iBodyEnd;
      }
        break;
      case Keyword::WHILE:
      case Keyword::IF:
      case Keyword::ELSE:
      case Keyword::ELSE_IF: {
        string res = calcCondition(i);
        if (m_expr[i].keyw != Keyword::WHILE) {
          isBreak = res == "break";
          isContinue = res == "continue";
          g_result = res;
        }
        i = m_expr[i].iBodyEnd;
      }
        break;
      case Keyword::BREAK: {
        isBreak = true;
        if (m_expr[iExpr].keyw != Keyword::WHILE)
          g_result = "break";
      }
        break;
      case Keyword::CONTINUE: {
        isContinue = true;
        if (m_expr[iExpr].keyw != Keyword::WHILE)
          g_result = "continue";
      }
        break;
      case Keyword::GOTO: {
        if (m_label.find(m_expr[i].params) != m_label.end()) {
          m_gotoIndex = m_label[m_expr[i].params];
        }
      }
        break;
      default:
        break;
      }
      if (isBreak || m_exit) break;

      if (m_gotoIndex != size_t(-1)) {
        if ((iCondEnd <= m_gotoIndex) && (m_gotoIndex < iBodyEnd)) {
          for (size_t j = m_gotoIndex; j < i; ++j)
            m_expr[j].iOperator = size_t(-1);
          i = m_gotoIndex;
          m_gotoIndex = size_t(-1);
        }
        else break;
      }

      if (isContinue) i = iBodyEnd;

      if ((m_expr[iExpr].keyw == Keyword::WHILE) && (i >= iBodyEnd)) {
        isContinue = false;

        for (size_t j = iBegin; j < iCondEnd; ++j)
          m_expr[j].iOperator = size_t(-1);

        const string& condn = m_expr[iExpr].result = calcExpression(iBegin, iCondEnd);
        bool isNum = isNumber(condn);
        if ((isNum && (stoi(condn) != 0)) || (!isNum && !condn.empty())) {
          for (size_t j = iCondEnd; j < iBodyEnd; ++j)
            m_expr[j].iOperator = size_t(-1);
          i = iCondEnd;
        }
      }
    }
  }
  return g_result;
}
string InterpreterImpl::calcExpression(size_t iBegin, size_t iEnd) {

  if (iBegin + 1 == iEnd) {
    if (m_expr[iBegin].keyw == Keyword::VARIABLE)
      return m_var[m_expr[iBegin].params];
    if (m_expr[iBegin].keyw == Keyword::VALUE)
      return m_expr[iBegin].params;
    return calcOperation(m_expr[iBegin].keyw, iBegin);
  }

  bool firstRun = m_soper.find(iBegin) == m_soper.end();
  if (firstRun)
    m_soper.insert({ iBegin, vector<Operatr>() });

  vector<Operatr>& oprs = m_soper[iBegin];
  if (firstRun) {
    calcOperatorPriority(iBegin, iEnd, oprs);
  }

  if (oprs.empty()) {
    return calcOperation(m_expr[iBegin].keyw, iBegin);
  }

  string g_result;
  for (auto& op : oprs) {
    size_t iOp = op.inx;
    Expression* pLeftOperd = nullptr,
      * pRightOperd = nullptr;
    string lValue, rValue;
    if (op.iLOpr != size_t(-1)) { // left operand
      pLeftOperd = &m_expr[op.iLOpr];
      if (pLeftOperd->iOperator == size_t(-1)) {
        if (pLeftOperd->keyw == Keyword::VARIABLE)
          lValue = m_var[m_expr[op.iLOpr].params];
        else if (pLeftOperd->keyw == Keyword::VALUE)
          lValue = m_expr[op.iLOpr].params;
        else
          lValue = calcOperation(pLeftOperd->keyw, op.iLOpr);
      }
      else
        lValue = m_expr[pLeftOperd->iOperator].result;
    }
    if (op.iROpr != size_t(-1)) { // right operand
      pRightOperd = &m_expr[op.iROpr];
      if (pRightOperd->iOperator == size_t(-1)) {
        if (pRightOperd->keyw == Keyword::VARIABLE)
          rValue = m_var[m_expr[op.iROpr].params];
        else if (pRightOperd->keyw == Keyword::VALUE)
          rValue = m_expr[op.iROpr].params;
        else
          rValue = calcOperation(pRightOperd->keyw, op.iROpr);
      }
      else
        rValue = m_expr[pRightOperd->iOperator].result;
    }
    m_currentIndex = iOp;
    g_result = m_expr[iOp].result = m_uoper[m_expr[iOp].params].first(lValue, rValue);

    if (pLeftOperd && (pLeftOperd->keyw == Keyword::VARIABLE) && (pLeftOperd->iOperator == size_t(-1))) {
      pLeftOperd->result = m_var[pLeftOperd->params] = lValue;
    }
    if (pRightOperd && (pRightOperd->keyw == Keyword::VARIABLE) && (pRightOperd->iOperator == size_t(-1))) {
      pRightOperd->result = m_var[pRightOperd->params] = rValue;
    }
    if (pLeftOperd) {
      if (pLeftOperd->iOperator != size_t(-1)) {
        size_t iLOp = pLeftOperd->iOperator;
        for (size_t i = iBegin; i < iEnd; ++i) {
          if (m_expr[i].iOperator == iLOp)
            m_expr[i].iOperator = iOp;
        }
      }
      else pLeftOperd->iOperator = iOp;
    }
    if (pRightOperd) {
      if (pRightOperd->iOperator != size_t(-1)) {
        size_t iROp = pRightOperd->iOperator;
        for (size_t i = iBegin; i < iEnd; ++i) {
          if (m_expr[i].iOperator == iROp)
            m_expr[i].iOperator = iOp;
        }
      }
      else pRightOperd->iOperator = iOp;
    }
  }
  return g_result;
}
void InterpreterImpl::calcOperatorPriority(size_t iBegin, size_t iEnd, vector<Operatr>& oprs) {

  size_t iLOpr = size_t(-1);
  for (size_t i = iBegin; i < iEnd;) {
    if (m_expr[i].keyw == Keyword::FUNCTION) {
      iLOpr = i;
      i = m_expr[i].iConditionEnd;
      continue;
    }
    if (m_expr[i].keyw == Keyword::EXPRESSION) {
      iLOpr = i;
      i = m_expr[i].iBodyEnd;
      continue;
    }
    if (m_expr[i].keyw == Keyword::OPERATOR) {
      uint32_t priority = m_uoper[m_expr[i].params].second;
      size_t iROpr = (i < iEnd - 1) ? i + 1 : size_t(-1);
      oprs.emplace_back<Operatr>({ i, priority, iLOpr, iROpr });  // inx, priority
    }
    iLOpr = i;
    ++i;
  }

  size_t osz = oprs.size();
  if (osz > 1) {
    if (osz == 2) {
      if (oprs[0].priority > oprs[1].priority) swap(oprs[0], oprs[1]);
    }
    else if (osz == 3) {
      if (oprs[0].priority < oprs[1].priority) {
        if (oprs[2].priority < oprs[0].priority) swap(oprs[0], oprs[2]);
      }
      else {
        if (oprs[1].priority < oprs[2].priority) swap(oprs[0], oprs[1]);
        else swap(oprs[0], oprs[2]);
      }
      if (oprs[2].priority < oprs[1].priority) swap(oprs[1], oprs[2]);
    }
    else if (osz < 10) { // faster than std::sort on small distance
      for (size_t i = 0; i < osz - 2; ++i) {
        size_t iMin = i;
        size_t minPriort = oprs[i].priority;
        for (size_t j = i + 1; j < osz; ++j) {
          if (oprs[j].priority < minPriort) {
            minPriort = oprs[j].priority;
            iMin = j;
          }
        }
        if (iMin != i) swap(oprs[i], oprs[iMin]);
      }
      if (oprs[osz - 2].priority > oprs[osz - 1].priority) swap(oprs[osz - 2], oprs[osz - 1]);
    }
    else {
      sort(oprs.begin(), oprs.end(), [](const Operatr& l, const Operatr& r) {
        return l.priority < r.priority;
        });
    }
  }
}

bool InterpreterImpl::parseInstructionScenar(string& scenar, size_t gpos) {

  size_t iExpr = m_expr.size(),
         cpos = 0;

#define CHECK_PARSE_RETURN(condition)                                                                                                              \
    if (condition){                                                                                                                       \
        if (m_err.empty()) m_err = "Error scenar pos " + to_string(cpos + gpos) + " src line " + to_string(__LINE__) + ": " + #condition; \
        return false;                                                                                                                     \
    }   

  size_t iIF = size_t(-1);
  while (cpos < scenar.size()) {
    if (scenar[cpos] == ';') {
      ++cpos;
      continue;
    }

    size_t cposFunc = cpos,
           cposOpr = cpos;
    if (!getFunctionAtFirst(scenar, cposFunc).empty() || !getOperatorAtFirst(scenar, cposOpr).empty()) {
        m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, size_t(-1) });

        string expr = getNextParam(scenar, cpos, ';');
        CHECK_PARSE_RETURN(expr.empty() || !parseExpressionScenar(expr, gpos + cpos - expr.size() - 1));

        iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
    }     
    else if (startWith(scenar, cpos, "while") || startWith(scenar, cpos, "if") || startWith(scenar, cpos, "elseif")) {
      const string kname = getNextParam(scenar, cpos, '(');
      CHECK_PARSE_RETURN(kname.empty());

      Keyword keyw = keywordByName(kname);
      m_expr.emplace_back<Expression>({ keyw, iExpr, iExpr, size_t(-1) });

      if (keyw == Keyword::IF) {
        iIF = iExpr;
      }
      else if (keyw == Keyword::ELSE_IF) {
        m_expr[iExpr].params = to_string(iIF);
        iIF = iExpr;
      }

      --cpos;
      string condition = getIntroScenar(scenar, cpos, '(', ')');
      CHECK_PARSE_RETURN(condition.empty() || !parseExpressionScenar(condition, gpos + cpos - condition.size() - 2));

      m_expr[iExpr].iConditionEnd = m_expr.size();

      if (scenar[cpos] == '{') {
        string body = getIntroScenar(scenar, cpos, '{', '}');
        CHECK_PARSE_RETURN(body.empty() || !parseInstructionScenar(body, gpos + cpos - body.size() - 2));
      }
      else {
        string body = getNextParam(scenar, cpos, ';') + ';';
        CHECK_PARSE_RETURN((body.size() == 1) || !parseInstructionScenar(body, gpos + cpos - body.size()));
      }
      iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

      if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
    }
    else if (startWith(scenar, cpos, "else")) {
      cpos += 4;

      m_expr.emplace_back<Expression>({ Keyword::ELSE, iExpr, iExpr, size_t(-1) });

      m_expr[iExpr].params = to_string(iIF);

      if (scenar[cpos] == '{') {
        string body = getIntroScenar(scenar, cpos, '{', '}');
        CHECK_PARSE_RETURN(body.empty() || !parseInstructionScenar(body, gpos + cpos - body.size() - 2));
      }
      else {
        string body = getNextParam(scenar, cpos, ';') + ';';
        CHECK_PARSE_RETURN((body.size() == 1) || !parseInstructionScenar(body, gpos + cpos - body.size()));
      }
      m_expr[iExpr].iConditionEnd = iExpr + 1;
      iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

      if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
    }
    else if (startWith(scenar, cpos, "break") || startWith(scenar, cpos, "continue")) {
      const string kname = getNextParam(scenar, cpos, ';');
      CHECK_PARSE_RETURN(kname.empty());
      const Keyword keyw = keywordByName(kname);

      m_expr.emplace_back<Expression>({ keyw, iExpr, iExpr, size_t(-1) });
      ++iExpr;
    }
    else if (startWith(scenar, cpos, "#macro")) {  // macro declaration
      cpos += 6;
      const string mname = getNextParam(scenar, cpos, '{');

      cpos -= 1;
      const string mvalue = getIntroScenar(scenar, cpos, '{', '}');
      CHECK_PARSE_RETURN(mname.empty() || mvalue.empty());

      m_macro["#" + mname] = mvalue;

      if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
    }
    else if (scenar[cpos] == '#') {               // macro definition
      size_t cposMName = cpos;
      const string mname = getMacroAtFirst(scenar, cposMName);
      CHECK_PARSE_RETURN(mname.empty() || (m_macro.find(mname) == m_macro.end()));

      size_t cposArg = cposMName;
      const string args = getIntroScenar(scenar, cposArg, '(', ')');
      string macro = m_macro[mname];
      if (!args.empty())
        CHECK_PARSE_RETURN(!parseMacroArgs(args, macro));
      
      if (cposArg == cposMName)
        scenar.replace(cpos, mname.size(), macro);
      else
        scenar.replace(cpos, (mname + "(" + args + ")").size(), macro);
    }
    else if (startWith(scenar, cpos, "goto")) {
      cpos += 4;
      const string lname = getNextParam(scenar, cpos, ';');
      CHECK_PARSE_RETURN(lname.empty());

      if (m_label.find(lname) == m_label.end())
        m_label.insert({ lname, size_t(-1) });

      m_expr.emplace_back<Expression>({ Keyword::GOTO, iExpr, iExpr, size_t(-1), lname });
      ++iExpr;
    }
    else if (startWith(scenar, cpos, "l_")) {
      const string lname = getNextParam(scenar, cpos, ':');
      CHECK_PARSE_RETURN(lname.empty());

      m_label[lname] = iExpr;
    }
    else {
      m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, size_t(-1) });

      string expr = getNextParam(scenar, cpos, ';');
      CHECK_PARSE_RETURN(expr.empty() || !parseExpressionScenar(expr, gpos + cpos - expr.size() - 1));

      iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
    }
  }
  return true;
}
bool InterpreterImpl::parseExpressionScenar(string& scenar, size_t gpos) {

  size_t iExpr = m_expr.size(),
         cpos = 0;

  string oprName, fName;
  while (cpos < scenar.size()) {
    if (scenar[cpos] == '$') {
      size_t posmem = cpos;
      oprName = getNextOperator(scenar, cpos);

      size_t bodyBegin = scenar.find('{', posmem);

      if ((!oprName.empty() && (bodyBegin < cpos)) || (oprName.empty() && (bodyBegin != string::npos))) {
        const string vName = scenar.substr(posmem, bodyBegin - posmem);
        string value = getIntroScenar(scenar, bodyBegin, '{', '}');
        if (!value.empty()) {
          if (value[0] == '"') 
            value = value.substr(1);
          if (!value.empty() && (value.back() == '"'))
            value.pop_back();
        }

        m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, size_t(-1), vName, value }); ++iExpr;
        m_var[vName] = value;

        cpos = bodyBegin;
      }
      else if (!oprName.empty()) {
        string vName = scenar.substr(posmem, cpos - posmem - oprName.size());
        if (m_var.find(vName) == m_var.end())
          m_var.insert({ vName, "" });

        m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, size_t(-1), vName }); ++iExpr;
        m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, size_t(-1), oprName }); ++iExpr;
      }
      else {        
        string vName = scenar.substr(cpos);

        if (vName.back() == ';') vName.pop_back();

        if (m_var.find(vName) == m_var.end())
          m_var.insert({ vName, "" });

        m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, size_t(-1), vName });
        
        break;
      }
    }
    else if (!(fName = getFunctionAtFirst(scenar, cpos)).empty()) {
      CHECK_PARSE_RETURN(m_ufunc.find(fName) == m_ufunc.end());

      m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, iExpr, size_t(-1), fName });

      size_t cposMem = cpos;
      string args = getIntroScenar(scenar, cpos, '(', ')');
      CHECK_PARSE_RETURN(args.empty() && (cposMem + 2 != cpos));
      if (!args.empty())
        CHECK_PARSE_RETURN(!parseArgumentScenar(args, gpos + cpos - args.size() - 2));

      iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();

      if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
    }
    else if (scenar[cpos] == '(') {
      string expr = getIntroScenar(scenar, cpos, '(', ')');

      m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, size_t(-1) });

      CHECK_PARSE_RETURN(expr.empty() || !parseExpressionScenar(expr, gpos + cpos - expr.size() - 2));

      iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

      if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
    }
    else if (scenar[cpos] == '#') {
      size_t cposMName = cpos;
      const string mname = getMacroAtFirst(scenar, cposMName);
      CHECK_PARSE_RETURN(mname.empty() || (m_macro.find(mname) == m_macro.end()));

      size_t cposArg = cposMName;
      const string args = getIntroScenar(scenar, cposArg, '(', ')');
      string macro = m_macro[mname];
      if (!args.empty())
        CHECK_PARSE_RETURN(!parseMacroArgs(args, macro));

      if (cposArg == cposMName)
          scenar.replace(cpos, mname.size(), macro);
      else
          scenar.replace(cpos, (mname + "(" + args + ")").size(), macro);
    }
    else if (!(oprName = getOperatorAtFirst(scenar, cpos)).empty()) {
      CHECK_PARSE_RETURN(m_uoper.find(oprName) == m_uoper.end());

      m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, size_t(-1), oprName }); ++iExpr;
    }
    else {  // value
      if (scenar[cpos] == '"') {
        ++cpos;
        const string vName = getNextParam(scenar, cpos, '"');
        m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), vName }); ++iExpr;
      }
      else if (scenar[cpos] == '{') {
        const string value = getIntroScenar(scenar, cpos, '{', '}');
        m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), "", value }); ++iExpr; // empty name
      }
      else {
        size_t posmem = cpos;
        oprName = getNextOperator(scenar, cpos);

        size_t bodyBegin = scenar.find('{', posmem);

        if ((!oprName.empty() && (bodyBegin < cpos)) || (oprName.empty() && (bodyBegin != string::npos))) {
          const string vName = scenar.substr(posmem, bodyBegin - posmem);         
          const string value = getIntroScenar(scenar, bodyBegin, '{', '}');
          m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), vName, value }); ++iExpr;

          cpos = bodyBegin;
        }
        else if (!oprName.empty()) {
          const string vName = scenar.substr(posmem, cpos - posmem - oprName.size());

          m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), vName }); ++iExpr;
          m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, size_t(-1), oprName }); ++iExpr;
        }
        else {
          string vName = scenar.substr(cpos);

          if (vName.back() == ';') vName.pop_back();

          m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), vName });
          break;
        }
      }
    }
  }
  return true;
}
bool InterpreterImpl::parseArgumentScenar(string& scenar, size_t gpos) {

  size_t iExpr = m_expr.size(),
         cpos = 0,
         cp = 0;
  int bordCnt = 0;

  while (cp < scenar.size()) {
    if (scenar[cp] == '(') ++bordCnt;
    if (scenar[cp] == ')') --bordCnt;
    if (((scenar[cp] == ',') || (cp == scenar.size() - 1)) && (bordCnt == 0)) {
      m_expr.emplace_back<Expression>({ Keyword::ARGUMENT, iExpr, iExpr, size_t(-1) });

      if (cp == scenar.size() - 1) ++cp;

      string arg = scenar.substr(cpos, cp - cpos);
      CHECK_PARSE_RETURN(!arg.empty() && !parseExpressionScenar(arg, gpos + cpos));

      iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
      cpos = cp + 1;
    }
    ++cp;
  }
  return true;
}
bool InterpreterImpl::parseMacroArgs(const string& args, string& macro) {

  size_t ssz = args.size(),
         cpos = 0,
         cp = 0;
  int bordCnt = 0;
  int argIndex = 1;

  while (cp < ssz) {
    if (args[cp] == '(') ++bordCnt;
    if (args[cp] == ')') --bordCnt;
    if (((args[cp] == ',') || (cp == ssz - 1)) && (bordCnt == 0)) {
     
      if (cp == ssz - 1) ++cp;

      const string arg = args.substr(cpos, cp - cpos);
      
      size_t argPos = 0;
      argPos = macro.find("$" + to_string(argIndex), argPos);
      while (argPos != string::npos) {
      
        macro.replace(argPos, ("$" + to_string(argIndex)).size(), arg);

        argPos = macro.find("$" + to_string(argIndex), argPos);
      }

      cpos = cp + 1;
      ++argIndex;
    }
    ++cp;
  }
  return true;
}

string InterpreterImpl::getNextParam(const string& scenar, size_t& cpos, char symb) const {
  size_t pos = scenar.find(symb, cpos);
  string res = "";
  if (pos != string::npos) {
    res = scenar.substr(cpos, pos - cpos);
    cpos = pos + 1;
  }
  return res;
}
string InterpreterImpl::getNextOperator(const string& scenar, size_t& cpos) const {
  size_t minp = string::npos;
  string opr = "";
  for (const auto& op : m_uoper) {
    size_t pos = scenar.find(op.first, cpos);
    if ((pos != string::npos) && ((pos <= minp) || (minp == string::npos))) {
      if (opr.empty() || (pos < minp) || (opr.size() < op.first.size()))
        opr = op.first;
      minp = pos;
    }
  }
  if (minp != string::npos) {
    cpos = minp + opr.size();
  }
  return opr;
}
string InterpreterImpl::getOperatorAtFirst(const string& scenar, size_t& cpos) const {
  string opr = "";
  for (const auto& op : m_uoper) {
    if (startWith(scenar, cpos, op.first)) {
      if (opr.empty() || (opr.size() < op.first.size()))
        opr = op.first;
    }
  }
  cpos += opr.size();
  return opr;
}
string InterpreterImpl::getFunctionAtFirst(const string& scenar, size_t& cpos) const {
  string fName = "";
  for (const auto& f : m_ufunc) {
    if (startWith(scenar, cpos, f.first)) {
      if (fName.empty() || (fName.size() < f.first.size()))
        fName = f.first;
    }
  }
  cpos += fName.size();
  return fName;
}
string InterpreterImpl::getMacroAtFirst(const string& scenar, size_t& cpos) const {
  string mName = "";
  for (const auto& m : m_macro) {
    if (startWith(scenar, cpos, m.first)) {
      if (mName.empty() || (mName.size() < m.first.size()))
        mName = m.first;
    }
  }
  cpos += mName.size();
  return mName;
}
string InterpreterImpl::getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd) const {
  size_t ssz = scenar.size(),
    cp = cpos;
  int bordCnt = 0;
  while (cp < ssz) {
    if (scenar[cp] == symbBegin) ++bordCnt;
    if (scenar[cp] == symbEnd) --bordCnt;
    if (bordCnt == 0) break;
    ++cp;
  }
  string res = "";
  if ((bordCnt == 0) && (cp > cpos)) {
    res = scenar.substr(cpos + 1, cp - cpos - 1);
    cpos = cp + 1;
  }
  return res;
}

bool InterpreterImpl::isFindKeySymbol(const string& scenar, size_t cpos, size_t maxpos) const {
  return (scenar.find('(', cpos) < maxpos) ||
    (scenar.find(')', cpos) < maxpos) ||
    (scenar.find('{', cpos) < maxpos) ||
    (scenar.find('}', cpos) < maxpos) ||
    (scenar.find(',', cpos) < maxpos) ||
    (scenar.find(';', cpos) < maxpos) ||
    (scenar.find('#', cpos) < maxpos) ||
    (scenar.find('"', cpos) < maxpos) ||
    (scenar.find('$', cpos) < maxpos);
}
bool InterpreterImpl::startWith(const string& str, size_t pos, const string& begin) const {
  return (str.find(begin, pos) - pos) == 0;
}
bool InterpreterImpl::isNumber(const string& s) const {
  for (auto c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return !s.empty();
}
InterpreterImpl::Keyword InterpreterImpl::keywordByName(const string& oprName) const {
  Keyword nextOpr = Keyword::INSTRUCTION;
  if (oprName == "if") nextOpr = Keyword::IF;
  else if (oprName == "else") nextOpr = Keyword::ELSE;
  else if (oprName == "elseif") nextOpr = Keyword::ELSE_IF;
  else if (oprName == "while") nextOpr = Keyword::WHILE;
  else if (oprName == "break") nextOpr = Keyword::BREAK;
  else if (oprName == "goto") nextOpr = Keyword::GOTO;
  else if (oprName == "#macro") nextOpr = Keyword::MACRO;
  else if (oprName == "continue") nextOpr = Keyword::CONTINUE;
  return nextOpr;
}
Interpreter::EntityType InterpreterImpl::keywordToEntityType(Keyword keyw) const {
  switch (keyw){
  case InterpreterImpl::Keyword::EXPRESSION: return Interpreter::EntityType::EXPRESSION;
  case InterpreterImpl::Keyword::OPERATOR:   return Interpreter::EntityType::OPERATOR;
  case InterpreterImpl::Keyword::WHILE:      return Interpreter::EntityType::WHILE;
  case InterpreterImpl::Keyword::IF:         return Interpreter::EntityType::IF;
  case InterpreterImpl::Keyword::ELSE:       return Interpreter::EntityType::ELSE;
  case InterpreterImpl::Keyword::ELSE_IF:    return Interpreter::EntityType::ELSE_IF;
  case InterpreterImpl::Keyword::BREAK:      return Interpreter::EntityType::BREAK;
  case InterpreterImpl::Keyword::CONTINUE:   return Interpreter::EntityType::CONTINUE;
  case InterpreterImpl::Keyword::FUNCTION:   return Interpreter::EntityType::FUNCTION;
  case InterpreterImpl::Keyword::ARGUMENT:   return Interpreter::EntityType::ARGUMENT;
  case InterpreterImpl::Keyword::VARIABLE:   return Interpreter::EntityType::VARIABLE;
  case InterpreterImpl::Keyword::VALUE:      return Interpreter::EntityType::VALUE;
  case InterpreterImpl::Keyword::GOTO:       return Interpreter::EntityType::GOTO;
  default:                                   return Interpreter::EntityType::EXPRESSION;
  }  
}

Interpreter::Interpreter() {
  m_d = new InterpreterImpl();
}
Interpreter::~Interpreter() {
  if (m_d) delete m_d;
}
Interpreter::Interpreter(const Interpreter& other) {
  m_d = new InterpreterImpl();
  if (other.m_d)
    *m_d = *other.m_d;
}
Interpreter::Interpreter(Interpreter&& other) {
  m_d = other.m_d;
  other.m_d = nullptr;
}
Interpreter& Interpreter::operator=(const Interpreter& other) {
  if ((this != &other) && m_d && other.m_d)
    *m_d = *other.m_d;
  return *this;
}
Interpreter& Interpreter::operator=(Interpreter&& other) {
  if (this != &other) {
    if (m_d) delete m_d;

    m_d = other.m_d;
    other.m_d = nullptr;
  }
  return *this;
}
string Interpreter::cmd(string scenar) {
  return m_d ? m_d->cmd(move(scenar)) : "";
}
bool Interpreter::parseScript(std::string scenar, string& outErr) {
  return m_d ? m_d->parseScript(move(scenar), outErr) : false;
}
std::string Interpreter::runScript() {
  return m_d ? m_d->runScript() : "";
}
bool Interpreter::addFunction(const string& name, UserFunction ufunc) {
  return m_d ? m_d->addFunction(name, ufunc) : false;
}
bool Interpreter::addOperator(const string& name, UserOperator uoper, uint32_t priority) {
  return m_d ? m_d->addOperator(name, uoper, priority) : false;
}
std::map<std::string, std::string> Interpreter::allVariables() const {
  return m_d ? m_d->allVariables() : std::map<std::string, std::string>();
}
std::string Interpreter::variable(const std::string& vname) const {
  return m_d ? m_d->variable(vname) : "";
}
std::string Interpreter::runFunction(const std::string& fname, const std::vector<std::string>& args) {
  return m_d ? m_d->runFunction(fname, args) : "";
}
bool Interpreter::setVariable(const std::string& vname, const std::string& value) {
  return m_d ? m_d->setVariable(vname, value) : false;
}
bool Interpreter::setMacro(const std::string& mname, const std::string& script) {
  return m_d ? m_d->setMacro(mname, script) : false;
}
bool Interpreter::gotoOnLabel(const std::string& lname) {
  return m_d ? m_d->gotoOnLabel(lname) : false;
}
void Interpreter::exitFromScript() {
  if (m_d) m_d->exitFromScript();
}
std::vector<Interpreter::Entity> Interpreter::allEntities() {
  return m_d ? m_d->allEntities() : std::vector<Interpreter::Entity>();
}
Interpreter::Entity Interpreter::currentEntity() {
  return m_d ? m_d->currentEntity() : Interpreter::Entity{ 0 };
}
Interpreter::Entity Interpreter::getEntityByIndex(size_t beginIndex) {
  return m_d ? m_d->getEntityByIndex(beginIndex) : Interpreter::Entity{ 0 };
}
bool Interpreter::gotoOnEntity(size_t beginIndex) {
  return m_d ? m_d->gotoOnEntity(beginIndex) : false;
}
Interpreter::UserFunction Interpreter::getUserFunction(const std::string& fname) {
  return m_d ? m_d->getUserFunction(fname) : nullptr;
}
Interpreter::UserOperator Interpreter::getUserOperator(const std::string& oname) {
  return m_d ? m_d->getUserOperator(oname) : nullptr;
}
