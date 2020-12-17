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
  enum class OperatorType {
    VARIABLE_VARIABLE,
    VALUE_VALUE,
    FUNCTION_FUNCTION,
    VARIABLE_VALUE,
    VALUE_VARIABLE,
    VARIABLE_FUNCTION,
    FUNCTION_VARIABLE,
    VALUE_FUNCTION,
    FUNCTION_VALUE,
  };
  struct Expression{    
    Keyword keyw;
    size_t iConditionEnd;
    size_t iBodyEnd;
    vector<string> params;
    OperatorType oprType;
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
  string introValue(size_t iExpr);
  void workCycle();
  Keyword keywordByName(const string& oprName);  
  string getNextParam(const string& scenar, size_t& cpos, char symb);  
  string getNextOperator(const string& scenar, size_t& cpos);
  string getNextFunction(const string& scenar, size_t& cpos);
  string getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd);
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
  if (name.empty() || (keywordByName(name) != Keyword::SEQUENCE)) return false;
  m_ufunc.insert({ name, move(ufunc) });
  return true;
}
bool InterpreterImpl::addOperator(const string& name, Interpreter::UserOperator uopr, uint32_t priority){
  if (name.empty() || (keywordByName(name) != Keyword::SEQUENCE)) return false;
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
      case Keyword::VARIABLE: {
        string result = introValue(i + 1);
        while(i < expr.iBodyEnd - 1){ 
          //const string& opr = m_expr[i + 1].args[1];
          //if (!opr.empty())
           // result = m_uoper[opr](result, introValue(i + 1));
        }
      }
      break;
      case Keyword::FUNCTION: {
        for (size_t j = i; j < i + expr.iBodyEnd; ++j){
         // expr.result = introValue(j);
        }
      }
      break;
    }
  }
}
    
string InterpreterImpl::introValue(size_t iExpr){

  auto& cval = m_expr[iExpr];
  switch (cval.keyw){
    case Keyword::VARIABLE: {

    }
    break;
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
          size_t cposFunc = cpos, cposOpr = cpos;
          if (!(fName = getNextFunction(scenar, cposFunc)).empty() && ((cposFunc - fName.size()) == cpos)) {

            CHECK(m_ufunc.find(fName) == m_ufunc.end());

            m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, iExpr, { fName } });

            cpos = cposFunc;
            const string args = getIntroScenar(scenar, cpos, '(', ')');
            if (!args.empty())
              CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));

            iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();

            if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
          }
          else if (!(oprName = getNextOperator(scenar, cposOpr)).empty() && ((cposOpr - oprName.size()) == cpos)) {

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
      Keyword prevType = Keyword::VARIABLE;      
      while (cpos < ssz) {

        if (startWith(scenar, cpos, "$")) {  // variable
          string vname = "";
          if (m_expr[iExpr].keyw != Keyword::OPERATOR) {
            size_t posmem = cpos;
            auto opr = getNextOperator(scenar, cpos);
            if (!opr.empty()) {
              vname = scenar.substr(posmem, cpos - posmem - opr.size());

              //  m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, iExpr, { opr, vname } });
            }
            else {
              vname = scenar.substr(cpos);
              //    m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr,{ vname } });
            }
          }
          else { // m_expr[iExpr].keyw == Keyword::OPERATOR
            size_t posmem = cpos;
            auto opr = getNextOperator(scenar, cpos);
            if (!opr.empty()) {
              vname = scenar.substr(posmem, cpos - posmem - opr.size());
            }
            else {
              vname = scenar.substr(cpos);
              if (vname.back() == ';') vname.pop_back();
              cpos = ssz; // to end
            }
          }
          if (vname.back() == ';') vname.pop_back();
          if (m_var.find(vname) != m_var.end())
            m_var.insert({ vname, "" });

          m_expr[iExpr].params.push_back(vname);
          if (prevType == Keyword::FUNCTION)
            m_expr[iExpr].oprType = OperatorType::FUNCTION_VARIABLE;
          else if (prevType == Keyword::VARIABLE)
            m_expr[iExpr].oprType = OperatorType::VARIABLE_VARIABLE;
          else
            m_expr[iExpr].oprType = OperatorType::VALUE_VARIABLE;
        }
      }

      // 

      //      //string vname = !opr.empty() ? scenar.substr(posmem, cpos - posmem - opr.size()) : scenar.substr(cpos);
      //      //if (!vname.empty() && (vname.back() == ';')) vname.pop_back();
      //      //if (m_var.find(vname) != m_var.end())
      //      //  m_var.insert({ vname, "" });
      //      //        
      //      //if (opr.empty()) {
      //      //  m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr, { vname } });
      //      //  ++iExpr;
      //      //  break;
      //      //}
      //    }
      //    else {
      //      auto fName = getNextParam(scenar, cpos, '(');
      //      if (!fName.empty()) {            // function
      //        CHECK(m_ufunc.find(fName) == m_ufunc.end());

      //        m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, iExpr,{ fName } }); // fName, opr

      //        --cpos;
      //        const string args = getIntroScenar(scenar, cpos, '(', ')');
      //        if (!args.empty()) {
      //          CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));
      //        }

      //        auto opr = getNextOperator(scenar, cpos);
      //        if (opr.empty()) cpos = ssz; // end

      //        m_expr[iExpr].args.push_back(opr);

      //        iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();
      //      }
      //      else {                            // value
      //        size_t posmem = cpos;
      //        auto opr = getNextOperator(scenar, cpos);

      //        string value = !opr.empty() ? scenar.substr(posmem, cpos - posmem - opr.size()) : scenar.substr(cpos);
      //        if (!value.empty() && (value.back() == ';')) value.pop_back();

      //        if (opr.empty()) cpos = ssz;    // end

      //        m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr,{ value, opr } });    // value, opr
      //        ++iExpr;
      //      }
      //    }
      //  }
      //} 
    }
    break;
    case InterpreterImpl::Keyword::ARGUMENT:{
      const string auxScenar = "," + scenar + ",";
      while (cpos < ssz){

        m_expr.emplace_back<Expression>({ Keyword::ARGUMENT, iExpr, iExpr, { } });  

        const string arg = getIntroScenar(auxScenar, cpos, ',', ',');
        CHECK(!arg.empty() && !parseScenar(arg, Keyword::EXPRESSION, gpos + cpos - arg.size() - 2));
         
        iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
       
        --cpos;
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
    if ((pos != string::npos) && ((pos < minp) || (minp == string::npos))) {
      minp = pos;
      opr = op.first;
    }
  }
  if (minp != string::npos) {
    cpos = minp + opr.size();
  }
  return opr;
}
string InterpreterImpl::getNextFunction(const string& scenar, size_t& cpos) {
  size_t minp = string::npos;
  string fName = "";
  for (const auto& f : m_ufunc) {
    size_t pos = scenar.find(f.first, cpos);
    if ((pos != string::npos) && ((pos < minp) || (minp == string::npos))) {
      minp = pos;
      fName = f.first;
    }
  }
  if (minp != string::npos) {
    cpos = minp + fName.size();
  }
  return fName;
}
string InterpreterImpl::getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd){
  size_t ssz = scenar.size(),
         cp = cpos;
  int bordCnt = 0;
  if (symbBegin != symbEnd) {
    while (cp < ssz) {
      if (scenar[cp] == symbBegin) ++bordCnt;
      if (scenar[cp] == symbEnd) --bordCnt;
      if (bordCnt == 0) break;
      ++cp;
    }
  }
  else {
    while (cp < ssz) {
      if (scenar[cp] == symbBegin) ++bordCnt;
      if (bordCnt == 2) {
        bordCnt = 0;
        break;
      }
      ++cp;
    }
  }
  string res = "";
  if ((bordCnt == 0) && (cp > cpos)) {
    res = scenar.substr(cpos + 1, cp - cpos - 1);
    cpos = cp + 1;
  }  
  return res;
}
InterpreterImpl::Keyword InterpreterImpl::keywordByName(const string& oprName) {
  Keyword nextOpr = Keyword::SEQUENCE;
  if (oprName == "if") nextOpr = Keyword::IF;
  else if (oprName == "else") nextOpr = Keyword::ELSE;
  else if (oprName == "elseif") nextOpr = Keyword::ELSE_IF;
  else if (oprName == "while") nextOpr = Keyword::WHILE;
  else if (oprName == "for") nextOpr = Keyword::FOR;
  else if (oprName == "break") nextOpr = Keyword::BREAK;
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