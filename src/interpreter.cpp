#include "../include/interpreter.h"

#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

class InterpreterImpl {
public:
  InterpreterImpl();
  bool addFunction(const string& name, Interpreter::UserFunction ufunc);
  bool addOperator(const string& name, Interpreter::UserOperator uopr, uint32_t priority);
  bool parseScenar(string scenar, string& out_err);
  bool start(bool asynch);
  bool stop();
  bool pause(bool set);
private:
  bool m_run, m_pause;
  enum class Keyword{
    SEQUENCE,
    EXPRESSION,
    OPERATOR,
    WHILE,
    FOR,
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
    vector<string> params;
    string result;
  };
  map<string, Interpreter::UserFunction> m_ufunc;
  map<string, pair<Interpreter::UserOperator, uint32_t>> m_uoper; // operator, priority
  map<string, string> m_var;
  map<string, string> m_macro;
  vector<Expression> m_expr;
  string m_err;

  vector<string> split(const string& str, char sep);
  bool startWith(const string& str, size_t pos, const string& begin);

  string runUserFunction(size_t iExpr);
  string runUserOperator(size_t iExpr);
  void workCycle();
  Keyword keywordByName(const string& oprName);  
  string getNextParam(const string& scenar, size_t& cpos, char symb);  
  string getOperatorAtFirst(const string& scenar, size_t& cpos);
  string getFunctionAtFirst(const string& scenar, size_t& cpos);
  string getNextOperator(const string& scenar, size_t& cpos);
  string getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd);
  bool isFindKeySymbol(const string& scenar, size_t cpos, size_t maxpos);
  bool parseScenar(const string& scenar, Keyword mainKeyword, size_t gpos);
  bool checkScenar(const string& scenar);
};

InterpreterImpl::InterpreterImpl() :
  m_run(false),
  m_pause(false){
}

bool InterpreterImpl::parseScenar(string scenar, string& out_err) {

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

  if (!checkScenar(scenar) || !parseScenar(move(scenar), Keyword::SEQUENCE, 0)) {
    m_expr.clear();
    out_err = m_err;
  }
  return out_err.empty();
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
  CHECK(scenar.find(",(") != string::npos);
  CHECK(scenar.find(",}") != string::npos);
  CHECK(scenar.find("{,") != string::npos);
  CHECK(scenar.find(",{") != string::npos);
  CHECK(scenar.find(";)") != string::npos);
  CHECK(scenar.find("(;") != string::npos);
  CHECK(scenar.find(";(") != string::npos);
  CHECK(scenar.find("{;") != string::npos);
  CHECK(scenar.find(";{") != string::npos);
  CHECK(scenar.find("((") != string::npos);
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
bool InterpreterImpl::start(bool async){
  if (m_expr.empty()) return false;
  if (!m_run){
    m_run = true;
    workCycle();
  }
  return true;
}
bool InterpreterImpl::stop(){
  if (m_expr.empty()) return false;
  m_run = false;
  m_pause = false;
  return true;
}
bool InterpreterImpl::pause(bool set){
  if (m_expr.empty()) return false;
  m_pause = set;
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

void InterpreterImpl::workCycle(){

  size_t esz = m_expr.size();
  for (size_t i = 0; i < esz; ++i){

    auto& expr = m_expr[i];
    switch (expr.keyw){
      case Keyword::EXPRESSION: {
        
      }
      break;
      case Keyword::FUNCTION: {
        
      }
      break;
    }
  }
}

string InterpreterImpl::runUserFunction(size_t iExpr) {
  return "";// function<string(const vector<string>&)>(m_expr[iExpr].ufunc)(m_expr[iExpr].args);
}
string InterpreterImpl::runUserOperator(size_t iExpr) {
  return "";//m_expr[iExpr].uoper(m_expr[iExpr].args[0], m_expr[iExpr].args[1]);
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
      while (cpos < ssz) {        
        if (scenar[cpos] == '$') {             // variable                            
          m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, { } });

          const string expr = getNextParam(scenar, cpos, ';');
          CHECK(expr.empty() || !parseScenar(expr, Keyword::EXPRESSION, gpos + cpos - expr.size() - 1));

          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
        }
        else if (scenar[cpos] == ';') {
          ++cpos;
        }        
        else if (startWith(scenar, cpos, "while") || startWith(scenar, cpos, "for") || startWith(scenar, cpos, "if") || startWith(scenar, cpos, "elseif")) {
          const string kname = getNextParam(scenar, cpos, '(');
          CHECK(kname.empty());
                    
          m_expr.emplace_back<Expression>({ keywordByName(kname), iExpr, iExpr, { "0"/*current index*/ } });
                    
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
          
          m_expr.emplace_back<Expression>({ Keyword::ELSE, iExpr, iExpr, {} });

          const string body = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(body.empty() || !parseScenar(body, Keyword::SEQUENCE, gpos + cpos - body.size() - 2));
                    
          iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (startWith(scenar, cpos, "break") || startWith(scenar, cpos, "continue")) {
          const string kname = getNextParam(scenar, cpos, ';');
          CHECK(kname.empty());
          const Keyword keyw = keywordByName(kname);
          m_expr.emplace_back<Expression>({ keyw, iExpr, iExpr, {} });
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
          string fName, oprName;
          if (!(fName = getFunctionAtFirst(scenar, cpos)).empty()) {

            CHECK(m_ufunc.find(fName) == m_ufunc.end());

            m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, iExpr, { fName } });

            size_t cposMem = cpos;
            const string args = getIntroScenar(scenar, cpos, '(', ')');
            CHECK(args.empty() && (cposMem + 2 != cpos));
            if (!args.empty())
              CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));

            iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();

            if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
          }
          else if (!(oprName = getOperatorAtFirst(scenar, cpos)).empty()) {

            CHECK(m_uoper.find(oprName) == m_uoper.end());

            m_expr.emplace_back<Expression>({ Keyword::EXPRESSION, iExpr, iExpr, { } });

            const string expr = getNextParam(scenar, cpos, ';');
            CHECK(expr.empty() || !parseScenar(expr, Keyword::EXPRESSION, gpos + cpos - expr.size() - 1));

            iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
          }
          else {
            m_err = "Error scenar pos " + to_string(cpos + gpos) + " src line " + to_string(__LINE__) + ": unknown operator";
            return false;
          }
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

            m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, { vName } }); ++iExpr;
            m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, { oprName } }); ++iExpr;
          }
          else {
            string vName = scenar.substr(cpos);

            if (vName.back() == ';') vName.pop_back();
            
            if (m_var.find(vName) == m_var.end())
              m_var.insert({ vName, "" });

            m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, { vName } });
            break;
          }
        }
        else if (!(fName = getFunctionAtFirst(scenar, cpos)).empty()){
          CHECK(m_ufunc.find(fName) == m_ufunc.end());

          m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, iExpr, { fName } });

          size_t cposMem = cpos;
          const string args = getIntroScenar(scenar, cpos, '(', ')');
          CHECK(args.empty() && (cposMem + 2 != cpos));
          if (!args.empty())
            CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));

          iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();

          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (!(oprName = getOperatorAtFirst(scenar, cpos)).empty()) {
          CHECK(m_uoper.find(oprName) == m_uoper.end());

          m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, { oprName } }); ++iExpr;
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
           
            m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, { value } }); ++iExpr;
            m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, { oprName } }); ++iExpr;
          }
          else {
            if (isFindKeySymbol(scenar, cpos, scenar.size())){
              m_err = "Error scenar pos " + to_string(cpos + gpos) + " src line " + to_string(__LINE__) + ": unknown operator";
              return false;
            }
            string value = scenar.substr(cpos);

            if (value.back() == ';') value.pop_back();
                        
            m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr, { value } });
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
          m_expr.emplace_back<Expression>({ Keyword::ARGUMENT, iExpr, iExpr, {} });

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
  else if (oprName == "for") nextOpr = Keyword::FOR;
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
bool Interpreter::parseScenar(string scenar, string& out_err){   
  return m_d->parseScenar(scenar, out_err);
}
bool Interpreter::addFunction(const string& name, UserFunction ufunc){
  return m_d->addFunction(name, ufunc);
}
bool Interpreter::addOperator(const string& name, UserOperator uoper, uint32_t priority){
  return m_d->addOperator(name, uoper, priority);
}
bool Interpreter::start(bool asynch){
  return m_d->start(asynch);
}
bool Interpreter::stop(){
  return m_d->stop();
}
bool Interpreter::pause(bool set){
  return m_d->pause(set);
}