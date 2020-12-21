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

#include <map>
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
 
private:
  enum class Keyword{
    SEQUENCE,
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
  }; 
  struct Expression{    
    Keyword keyw;
    size_t iConditionEnd;
    size_t iBodyEnd;
    size_t iOperator;
    string params;
    string result;
  };
  struct Operatr{
    size_t inx, priority, iLOpr, iROpr;
  };
  map<string, Interpreter::UserFunction> m_ufunc;
  map<string, pair<Interpreter::UserOperator, uint32_t>> m_uoper; // operator, priority
  map<string, string> m_var;
  map<string, string> m_macro;
  map<size_t, vector<Operatr>> m_soper;
  vector<Expression> m_expr;  
  string m_err, m_prevScenar, m_result;

  vector<string> split(const string& str, char sep);
  bool startWith(const string& str, size_t pos, const string& begin);
  bool isNumber(const string& s);

  string calcOperation(Keyword mainKeyword, size_t iExpr);
  string calcExpression(size_t iBegin, size_t iEnd);
  Keyword keywordByName(const string& oprName);  
  string getNextParam(const string& scenar, size_t& cpos, char symb);  
  string getOperatorAtFirst(const string& scenar, size_t& cpos);
  string getFunctionAtFirst(const string& scenar, size_t& cpos);
  string getMacroAtFirst(const string& scenar, size_t& cpos);
  string getNextOperator(const string& scenar, size_t& cpos);
  string getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd);
  bool isFindKeySymbol(const string& scenar, size_t cpos, size_t maxpos);
  bool parseScenar(const string& scenar, Keyword mainKeyword, size_t gpos);
  bool checkScenar(const string& scenar);
};

string InterpreterImpl::cmd(string scenar) {

  size_t commp = scenar.find("//");
  while (commp != string::npos) {
    size_t endstr = scenar.find("\n");
    if (endstr != string::npos)
      scenar.erase(commp, endstr - commp + 1);
    else
      scenar.erase(commp);
    commp = scenar.find("//");
  }

  scenar.erase(remove(scenar.begin(), scenar.end(), '\n'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\t'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\v'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\f'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\r'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), ' '), scenar.end());

  if (scenar.empty())
    return "Error: empty scenar";

  if (scenar.back() != ';') scenar += ';';

  if (m_prevScenar != scenar){
    m_prevScenar.clear();
    m_expr.clear();
    m_soper.clear();
    if (!checkScenar(scenar) || !parseScenar(scenar, Keyword::SEQUENCE, 0)) {
      return m_err;
    }
    m_prevScenar = scenar;
  }
  else {
    for (auto& ex : m_expr)
      ex.iOperator = size_t(-1);
  }

  for (size_t i = 0; i < m_expr.size();) {
    m_result = calcOperation(m_expr[i].keyw, i);
    i = max(m_expr[i].iConditionEnd, m_expr[i].iBodyEnd);
  }
  return m_result;
}

bool InterpreterImpl::checkScenar(const string& scenar) {

#define CHECK(condition)                                                    \
  if (condition){                                                           \
    if (m_err.empty()) m_err = "Error check scenar: " + string(#condition); \
    return false;                                                           \
  }  

  CHECK(std::count(scenar.begin(), scenar.end(), '{') != std::count(scenar.begin(), scenar.end(), '}'));
  CHECK(std::count(scenar.begin(), scenar.end(), '(') != std::count(scenar.begin(), scenar.end(), ')'));
  CHECK(scenar.find(",)") != string::npos);
  CHECK(scenar.find("(,") != string::npos);
  CHECK(scenar.find(",}") != string::npos);
  CHECK(scenar.find("{,") != string::npos);
  CHECK(scenar.find(",{") != string::npos);
  CHECK(scenar.find(";)") != string::npos);
  CHECK(scenar.find("(;") != string::npos);
  CHECK(scenar.find("{;") != string::npos);
  CHECK(scenar.find(";{") != string::npos);
  CHECK(scenar.find("{{") != string::npos);
  CHECK(scenar.find("{}") != string::npos);
#undef CHECK
  
  return true;
}

bool InterpreterImpl::addFunction(const string& name, Interpreter::UserFunction ufunc){
  if (name.empty() || (keywordByName(name) != Keyword::SEQUENCE) || isFindKeySymbol(name, 0, name.size())) return false;
  m_ufunc.insert({ name, move(ufunc) });
  return true;
}
bool InterpreterImpl::addOperator(const string& name, Interpreter::UserOperator uopr, uint32_t priority){
  if (name.empty() || (keywordByName(name) != Keyword::SEQUENCE) || isFindKeySymbol(name, 0, name.size())) return false;
  m_uoper.insert({ name, {move(uopr), priority} });
  return true;
}

vector<string> InterpreterImpl::split(const string& str, char sep) {
  vector<string> res;
  istringstream iss(str);
  string token;
  while (getline(iss, token, sep)){
    res.emplace_back(token);
  }
  return res;
}
bool InterpreterImpl::startWith(const string& str, size_t pos, const string& begin){
  return (str.find(begin, pos) - pos) == 0;
}
bool InterpreterImpl::isNumber(const string& s) {
  for (auto c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return !s.empty();
}

string InterpreterImpl::calcOperation(Keyword mainKeyword, size_t iExpr){
         
  string g_result;
  switch (mainKeyword){
    case Keyword::VARIABLE: {
      g_result = m_var[m_expr[iExpr].params];
    }
    break;
    case Keyword::VALUE: {
      g_result = m_expr[iExpr].params;
    }
    break;
    case Keyword::EXPRESSION: {
      g_result = calcExpression(iExpr + 1, m_expr[iExpr].iBodyEnd);
    }
    break;
    case Keyword::FUNCTION: {
      size_t iBegin = iExpr + 1;
      size_t iEnd = m_expr[iExpr].iConditionEnd;
      vector<string> args;
      for (size_t i = iBegin; i < iEnd;) {
        if ((i + 1 == m_expr[i].iBodyEnd - 1) && ((m_expr[i + 1].keyw == Keyword::VARIABLE) || (m_expr[i + 1].keyw == Keyword::VALUE))){
          if (m_expr[i + 1].keyw == Keyword::VARIABLE)
            args.emplace_back(m_var[m_expr[i + 1].params]);
          else
            args.emplace_back(m_expr[i + 1].params);
        }
        else{
          args.emplace_back(calcExpression(i + 1, m_expr[i].iBodyEnd));
        }
        i = m_expr[i].iBodyEnd;
      }      
      g_result = m_ufunc[m_expr[iExpr].params](args);
    }
    break;
    case Keyword::WHILE:
    case Keyword::IF:
    case Keyword::ELSE:
    case Keyword::ELSE_IF: {
      size_t iBegin = iExpr + 1;
      size_t iCondEnd = m_expr[iExpr].iConditionEnd;
      size_t iBodyEnd = m_expr[iExpr].iBodyEnd;
      if ((m_expr[iExpr].keyw == Keyword::ELSE) || (m_expr[iExpr].keyw == Keyword::ELSE_IF)) {
        size_t iIF = stoul(m_expr[iExpr].params);
        if (iIF != size_t(-1)) {
          string ifCondn = m_expr[iIF].result;
          if ((isNumber(ifCondn) && (stoi(ifCondn) != 0)) || (!isNumber(ifCondn) && !ifCondn.empty())) {
            break;
          }
        }
        else break;
      }     
      string condn;
      if (iBegin < iCondEnd) {
        condn = m_expr[iExpr].result = calcExpression(iBegin, iCondEnd);
      }      
      if ((m_expr[iExpr].keyw == Keyword::ELSE) || (isNumber(condn) && (stoi(condn) != 0)) || (!isNumber(condn) && !condn.empty())) {
        bool isContinue = false,
             isBreak = false;
        for (size_t i = iCondEnd; i < iBodyEnd;) {
          switch (m_expr[i].keyw) {            
            case Keyword::EXPRESSION: {
              calcExpression(i + 1, m_expr[i].iBodyEnd);
              i = m_expr[i].iBodyEnd;
            }
            break;           
            case Keyword::WHILE:
            case Keyword::IF:
            case Keyword::ELSE:
            case Keyword::ELSE_IF: {
              string res = calcOperation(m_expr[i].keyw, i);
              if (m_expr[i].keyw != Keyword::WHILE){
                isBreak = res == "break";
                isContinue = res == "continue";
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
            default:
            break;
          }  
          if (isContinue || isBreak) i = iBodyEnd;

          if ((m_expr[iExpr].keyw == Keyword::WHILE) && (i >= iBodyEnd)){
            isContinue = false;
            if (isBreak) break;

            for (size_t j = iBegin; j < iCondEnd; ++j)
              m_expr[j].iOperator = size_t(-1);

            string condn = calcExpression(iBegin, iCondEnd);
            if ((isNumber(condn) && (stoi(condn) != 0)) || (!isNumber(condn) && !condn.empty())) {
              for (size_t j = iCondEnd; j < iBodyEnd; ++j)
                m_expr[j].iOperator = size_t(-1);
              i = iCondEnd;
              g_result.clear();
            }            
          }           
        }
      }
    }
    break;
    default:
    break;
  }
  return g_result;
}
string InterpreterImpl::calcExpression(size_t iBegin, size_t iEnd) {
   
  if (iBegin + 1 == iEnd){
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
  if (firstRun){
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
    if (osz > 1){
      if (osz == 2){
        if (oprs[0].priority > oprs[1].priority) swap(oprs[0], oprs[1]);
      }
      else if (osz == 3){
        if (oprs[0].priority < oprs[1].priority){
          if (oprs[2].priority < oprs[0].priority) swap(oprs[0], oprs[2]);
        }
        else {
          if (oprs[1].priority < oprs[2].priority) swap(oprs[0], oprs[1]);
          else swap(oprs[0], oprs[2]);
        }
        if (oprs[2].priority < oprs[1].priority) swap(oprs[1], oprs[2]);
      }
      else if (osz < 10) { // faster than std::sort on small distance
        for (size_t i = 0; i < osz - 2; ++i){
          size_t iMin = i;
          size_t minPriort = oprs[i].priority;
          for (size_t j = i + 1; j < osz; ++j){
            if (oprs[j].priority < minPriort){
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

  if (oprs.empty()){
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
      if (pLeftOperd->iOperator == size_t(-1))
        lValue = calcOperation(pLeftOperd->keyw, op.iLOpr);
      else
        lValue = m_expr[pLeftOperd->iOperator].result;
    }
    if (op.iROpr != size_t(-1)) { // right operand
      pRightOperd = &m_expr[op.iROpr];
      if (pRightOperd->iOperator == size_t(-1))
        rValue = calcOperation(pRightOperd->keyw, op.iROpr);
      else
        rValue = m_expr[pRightOperd->iOperator].result;
    }
    g_result = m_expr[iOp].result = m_uoper[m_expr[iOp].params].first(lValue, rValue);

    if (pLeftOperd && (pLeftOperd->keyw == Keyword::VARIABLE) && (pLeftOperd->iOperator == size_t(-1))) {
      m_var[pLeftOperd->params] = lValue;
    }
    if (pRightOperd && (pRightOperd->keyw == Keyword::VARIABLE) && (pRightOperd->iOperator == size_t(-1))) {
      m_var[pRightOperd->params] = rValue;
    }
    if (pLeftOperd){
      if (pLeftOperd->iOperator != size_t(-1)){
        size_t iLOp = pLeftOperd->iOperator;
        for (auto& ex : m_expr){
          if (ex.iOperator == iLOp)
            ex.iOperator = iOp;
        }
      }
      else pLeftOperd->iOperator = iOp;
    }
    if (pRightOperd){
      if (pRightOperd->iOperator != size_t(-1)){
        size_t iROp = pRightOperd->iOperator;
        for (auto& ex : m_expr){
          if (ex.iOperator == iROp)
            ex.iOperator = iOp;
        }
      }
      else pRightOperd->iOperator = iOp;
    }
  }
  return g_result;
}

bool InterpreterImpl::parseScenar(const string& scenar, Keyword mainKeyword, size_t gpos){

  size_t ssz = scenar.size(),
         iExpr = m_expr.size(),
         cpos = 0;
 
#define CHECK(condition)                                                                                                              \
  if (condition){                                                                                                                     \
    if (m_err.empty()) m_err = "Error scenar pos " + to_string(cpos + gpos) + " src line " + to_string(__LINE__) + ": " + #condition; \
    return false;                                                                                                                     \
  }   
  
  switch (mainKeyword){
    case InterpreterImpl::Keyword::SEQUENCE: {
      size_t iIF = size_t(-1);
      while (cpos < ssz) {        
        if (scenar[cpos] == ';') {
          ++cpos;
        }        
        else if (startWith(scenar, cpos, "while") || startWith(scenar, cpos, "if") || startWith(scenar, cpos, "elseif")) {
          const string kname = getNextParam(scenar, cpos, '(');
          CHECK(kname.empty());
                    
          Keyword keyw = keywordByName(kname);
          m_expr.emplace_back<Expression>({ keyw, iExpr, iExpr, size_t(-1) });
          
          if (keyw == Keyword::IF) iIF = iExpr;
          else if (keyw == Keyword::ELSE_IF) m_expr[iExpr].params = to_string(iIF);
                    
          --cpos;
          const string condition = getIntroScenar(scenar, cpos, '(', ')');
          CHECK(condition.empty() || !parseScenar(condition, Keyword::EXPRESSION, gpos + cpos - condition.size() - 2));
          
          m_expr[iExpr].iConditionEnd = m_expr.size();
          
          const string body = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(body.empty() || !parseScenar(body, Keyword::SEQUENCE, gpos + cpos - body.size() - 2));
          
          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
          
          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (startWith(scenar, cpos, "else")) {
          cpos += 4;
          
          m_expr.emplace_back<Expression>({ Keyword::ELSE, iExpr, iExpr, size_t(-1) });

          m_expr[iExpr].params = to_string(iIF);

          const string body = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(body.empty() || !parseScenar(body, Keyword::SEQUENCE, gpos + cpos - body.size() - 2));
                    
          m_expr[iExpr].iConditionEnd = iExpr + 1;
          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (startWith(scenar, cpos, "break") || startWith(scenar, cpos, "continue")) {
          const string kname = getNextParam(scenar, cpos, ';');
          CHECK(kname.empty());
          const Keyword keyw = keywordByName(kname);
          m_expr.emplace_back<Expression>({ keyw, iExpr, iExpr, size_t(-1) });
          ++iExpr;
        }
        else if (startWith(scenar, cpos, "#macro")) {  // macro declaration
          cpos += 6;
          const string mname = getNextParam(scenar, cpos, '{');
          cpos -= 1;
          const string mvalue = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(mname.empty() || mvalue.empty());
          m_macro.insert({ "#" + mname, mvalue });
          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (scenar[cpos] == '#') {               // macro definition
          const string mname = getNextParam(scenar, cpos, ';');
          CHECK(mname.empty() || (m_macro.find(mname) == m_macro.end()));
          CHECK(!parseScenar(m_macro[mname], Keyword::SEQUENCE, gpos + cpos - mname.size() - 1));
          iExpr = m_expr.size();
        }       
        else {
          m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, size_t(-1) });

          const string expr = getNextParam(scenar, cpos, ';');
          CHECK(expr.empty() || !parseScenar(expr, Keyword::EXPRESSION, gpos + cpos - expr.size() - 1));

          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
        }
      }
    }
    break;
    case InterpreterImpl::Keyword::EXPRESSION: {
      while (cpos < ssz) {
        string oprName, fName;
        if (scenar[cpos] == '$') {    
          size_t posmem = cpos;
          oprName = getNextOperator(scenar, cpos);

          if (!oprName.empty()) {
            string vName = scenar.substr(posmem, cpos - posmem - oprName.size());
            if (m_var.find(vName) == m_var.end())
              m_var.insert({ vName, "" });

            m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, size_t(-1), vName}); ++iExpr;
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
        else if (!(fName = getFunctionAtFirst(scenar, cpos)).empty()){
          CHECK(m_ufunc.find(fName) == m_ufunc.end());

          m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, iExpr, size_t(-1), fName });

          size_t cposMem = cpos;
          const string args = getIntroScenar(scenar, cpos, '(', ')');
          CHECK(args.empty() && (cposMem + 2 != cpos));
          if (!args.empty())
            CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));

          iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();

          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (scenar[cpos] == '('){          
          const string expr = getIntroScenar(scenar, cpos, '(', ')');

          m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, size_t(-1) });

          CHECK(expr.empty() || !parseScenar(expr, Keyword::EXPRESSION, gpos + cpos - expr.size() - 2));

          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;          
        }
        else if (scenar[cpos] == '#'){
          const string mname = getMacroAtFirst(scenar, cpos);
          CHECK(mname.empty() || (m_macro.find(mname) == m_macro.end()));

          m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, size_t(-1) });

          CHECK(!parseScenar(m_macro[mname], Keyword::EXPRESSION, gpos + cpos - mname.size()));

          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (!(oprName = getOperatorAtFirst(scenar, cpos)).empty()) {
          CHECK(m_uoper.find(oprName) == m_uoper.end());

          m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, size_t(-1), oprName }); ++iExpr;
        }
        else {  // value
          size_t posmem = cpos;
          oprName = getNextOperator(scenar, cpos);

          if (!oprName.empty()) {
            if (isFindKeySymbol(scenar, posmem, cpos - oprName.size())){
              m_err = "Error scenar pos " + to_string(posmem + gpos) + " src line " + to_string(__LINE__) + ": unknown operator";
              return false;
            }
            string value = scenar.substr(posmem, cpos - posmem - oprName.size());
           
            m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), value }); ++iExpr;
            m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, size_t(-1), oprName }); ++iExpr;
          }
          else {
            if (isFindKeySymbol(scenar, cpos, scenar.size())){
              m_err = "Error scenar pos " + to_string(cpos + gpos) + " src line " + to_string(__LINE__) + ": unknown operator";
              return false;
            }
            string value = scenar.substr(cpos);

            if (value.back() == ';') value.pop_back();
                        
            m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, size_t(-1), value });
            break;
          }
        }
      }
    }
    break;
    case InterpreterImpl::Keyword::ARGUMENT:{
      size_t cp = cpos;
      int bordCnt = 0;
      while (cp < ssz){
        if (scenar[cp] == '(') ++bordCnt;
        if (scenar[cp] == ')') --bordCnt;
        if (((scenar[cp] == ',') || (cp == ssz - 1)) && (bordCnt == 0)){
          m_expr.emplace_back<Expression>({ Keyword::ARGUMENT, iExpr, iExpr, size_t(-1) });

          if (cp == ssz - 1) ++cp;
          
          const string arg = scenar.substr(cpos, cp - cpos);
          CHECK(!arg.empty() && !parseScenar(arg, Keyword::EXPRESSION, gpos + cpos));

          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
          cpos = cp + 1;
        }        
        ++cp;
      }
    }
    break;
    default:
    break;
  } 
  return true;
}
string InterpreterImpl::getNextParam(const string& scenar, size_t& cpos, char symb) {
  size_t pos = scenar.find(symb, cpos);
  string res = "";
  if (pos != string::npos) {
    res = scenar.substr(cpos, pos - cpos);
    cpos = pos + 1;
  }
  return res;
}
string InterpreterImpl::getNextOperator(const string& scenar, size_t& cpos) {
  size_t minp = string::npos;
  string opr = "";
  for (const auto& op : m_uoper) {
    size_t pos = scenar.find(op.first, cpos);
    if ((pos != string::npos) && ((pos <= minp) || (minp == string::npos))) {
      if (opr.empty() || (pos < minp) || (opr.size() < op.first.size())) // for '+' and '++'
        opr = op.first;
      minp = pos;
    }
  }
  if (minp != string::npos) {
    cpos = minp + opr.size();
  }
  return opr;
}
string InterpreterImpl::getOperatorAtFirst(const string& scenar, size_t& cpos) {
  string opr = "";
  for (const auto& op : m_uoper) {
    if (startWith(scenar, cpos, op.first)){      
      if (opr.empty() || (opr.size() < op.first.size())) // for '+' and '++'
        opr = op.first;
    }
  }
  cpos += opr.size();
  return opr;
}
string InterpreterImpl::getFunctionAtFirst(const string& scenar, size_t& cpos) {
  string fName = "";
  for (const auto& f : m_ufunc) {
    if (startWith(scenar, cpos, f.first)){
      fName = f.first;
      break;
    }
  }
  cpos += fName.size();
  return fName;
}
string InterpreterImpl::getMacroAtFirst(const string& scenar, size_t& cpos) {
  string mName = "";
  for (const auto& m : m_macro) {
    if (startWith(scenar, cpos, m.first)){
      mName = m.first;
      break;
    }
  }
  cpos += mName.size();
  return mName;
}
string InterpreterImpl::getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd){
  size_t ssz = scenar.size(),
         cp = cpos;
  int bordCnt = 0;
  while (cp < ssz){
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
bool InterpreterImpl::isFindKeySymbol(const string& scenar, size_t cpos, size_t maxpos){
  return (scenar.find('(', cpos) < maxpos) ||
         (scenar.find(')', cpos) < maxpos) ||
         (scenar.find('{', cpos) < maxpos) ||
         (scenar.find('}', cpos) < maxpos) ||
         (scenar.find(',', cpos) < maxpos) ||
         (scenar.find(';', cpos) < maxpos) ||
         (scenar.find('#', cpos) < maxpos) ||
         (scenar.find('$', cpos) < maxpos);
}
InterpreterImpl::Keyword InterpreterImpl::keywordByName(const string& oprName) {
  Keyword nextOpr = Keyword::SEQUENCE;
  if (oprName == "if") nextOpr = Keyword::IF;
  else if (oprName == "else") nextOpr = Keyword::ELSE;
  else if (oprName == "elseif") nextOpr = Keyword::ELSE_IF;
  else if (oprName == "while") nextOpr = Keyword::WHILE;
  else if (oprName == "break") nextOpr = Keyword::BREAK;
  else if (oprName == "#macro") nextOpr = Keyword::MACRO;
  else if (oprName == "continue") nextOpr = Keyword::CONTINUE;
  return nextOpr;
}

Interpreter::Interpreter(){
  m_d = new InterpreterImpl();
}
Interpreter::~Interpreter(){
  delete m_d;
}
string Interpreter::cmd(string scenar){
  return m_d->cmd(move(scenar));
}
bool Interpreter::addFunction(const string& name, UserFunction ufunc){
  return m_d->addFunction(name, ufunc);
}
bool Interpreter::addOperator(const string& name, UserOperator uoper, uint32_t priority){
  return m_d->addOperator(name, uoper, priority);
}