#include "../include/interpreter.h"

#include <sstream>
#include <algorithm>

using namespace std;

class InterpreterImpl {
public:
  InterpreterImpl(const map<string, Interpreter::UserFunction>& ufuncs, const map<string, Interpreter::UserOperator>& uopers);
  bool addFunction(const string& name, Interpreter::UserFunction ufunc);
  bool addOperator(const string& name, Interpreter::UserOperator uopr);
  bool parseScenar(string scenar, string& out_err);
  bool start(bool asynch);
  bool stop();
  bool pause(bool set);
private:
  bool m_run, m_pause;
  enum class Keyword{
    SEQUENCE,
    WHILE,
    FOR,
    IF,
    ELSE,
    ELSE_IF,
    BREAK,
    CONTINUE,
    FUNCTION,
    VARIABLE,
    VALUE,
    ARGUMENT,
    MACRO,
  };
  struct Expression{    
    Keyword keyw;
    size_t beginInx;
    size_t endInx;
    vector<string> args;
    string result;
  };
  map<string, Interpreter::UserFunction> m_ufunc;
  map<string, Interpreter::UserOperator> m_uoper;
  map<string, string> m_var;
  map<string, string> m_macro;
  vector<Expression> m_expr;
  string m_err;

  vector<string> split(const string& str, char sep);
  bool startWith(const string& str, int pos, const string& begin);

  string runUserFunction(int iExpr);
  string runUserOperator(int iExpr);
  void workIntroCycle(int& cPos);
  void workCycle();
  Keyword keywordByName(const string& oprName);  
  string getNextParam(const string& scenar, size_t& cpos, char symb);  
  string getNextOperator(const string& scenar, size_t& cpos);
  string getIntroScenar(const string& scenar, size_t& cpos, char symbBegin, char symbEnd);
  bool parseScenar(const string& scenar, Keyword mainKeyword, size_t gpos);
  bool checkScenar(const string& scenar);
};

InterpreterImpl::InterpreterImpl(const map<string, Interpreter::UserFunction>& ufuncs, const map<string, Interpreter::UserOperator>& uopers) :
  m_ufunc(ufuncs),
  m_uoper(uopers),
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
#undef CHECK
  
  return true;
}

bool InterpreterImpl::addFunction(const string& name, Interpreter::UserFunction ufunc){
  if (name.empty() || (keywordByName(name) != Keyword::SEQUENCE)) return false;
  m_ufunc.insert({ name, move(ufunc) });
  return true;
}
bool InterpreterImpl::addOperator(const string& name, Interpreter::UserOperator uopr){
  if (name.empty()) return false;
  m_uoper.insert({ name, move(uopr) });
  return true;
}
bool InterpreterImpl::start(bool asynch){
  if (!m_run){
    m_run = true;
    workCycle();
  }
  return true;
}
bool InterpreterImpl::stop(){
  m_run = false;
  m_pause = false;
  return true;
}
bool InterpreterImpl::pause(bool set){
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
bool InterpreterImpl::startWith(const string& str, int pos, const string& begin){
    
  return (str.find(begin, pos) - pos) == 0;
}

void InterpreterImpl::workIntroCycle(int& cPos){

  //int bPos = cPos,
  //  leng = bPos + m_expr[cPos].position;
  //cPos += m_expr[cPos].position - 1;
  //bool _break = false,
  //  _continue = false;
  //while (true){

  //  for (int i = bPos; i < leng; ++i){

  //    //  while (m_pause) Sleep(10);

  //    if (!m_run) return;

  //    if (_break || _continue){
  //      if ((m_expr[bPos].opr == Keyword::CYCLE) || (m_expr[bPos].cmd == Keyword::FOR)){
  //        if (_continue){
  //          _continue = false;
  //          break;
  //        }
  //      }
  //      return;
  //    }

  //    if (i == bPos){
  //      switch (m_expr[bPos].opr){
  //      case Keyword::IF:
  //        if (runUserFunc(bPos)){
  //          m_expr[bPos].isExecute = true;
  //          continue;
  //        }
  //        break;
  //      case Keyword::FOR:
  //        if (stoi(m_expr[bPos].args[0]) < stoi(m_expr[bPos].args[1])){
  //          m_expr[bPos].args[0] = to_string(stoi(m_expr[bPos].args[0]) + 1);
  //          m_expr[bPos].isExecute = true;
  //          continue;
  //        }
  //        else{
  //          m_expr[bPos].args[0] = "0";
  //        }
  //        break;
  //      case Keyword::CYCLE:
  //        if (runUserFunc(bPos)){
  //          m_expr[bPos].isExecute = true;
  //          continue;
  //        }
  //        break;
  //      case Keyword::ELSE:
  //        if (!m_expr[p0].isExecute){
  //          m_expr[bPos].isExecute = true;
  //          continue;
  //        }
  //        break;
  //      case Keyword::BREAK:
  //        _break = true;
  //        continue;
  //      case Keyword::CONTINUE:
  //        _continue = true;
  //        continue;
  //      }
  //      m_expr[bPos].isExecute = false;
  //      return;
  //    }

  //    if (m_expr[i].opr != Keyword::SEQUENCE){
  //      workIntroCycle(i);

  //      if (i == (leng - 1) && (m_expr[bPos].opr != Keyword::CYCLE) && (m_expr[bPos].opr != Keyword::FOR)) return;
  //      continue;
  //    }
  //    runUserFunc(i);

  //    if (i == (leng - 1) && (m_expr[bPos].opr != Keyword::CYCLE) && (m_expr[bPos].opr != Keyword::FOR)) return;
  //  }
  //}
}
void InterpreterImpl::workCycle(){

  /*int esz = m_expr.size();
  for (int i = 0; i < esz; ++i){

    while (m_pause) Sleep(10);

    if (!m_run) break;

    if (m_expr[i].opr != Keyword::SEQUENCE){
      workIntroCycle(i);
      continue;
    }
    runUserFunc(i);
  }
  m_run = false;*/
}
string InterpreterImpl::runUserFunction(int iExpr) {
  return "";// function<string(const vector<string>&)>(m_expr[iExpr].ufunc)(m_expr[iExpr].args);
}
string InterpreterImpl::runUserOperator(int iExpr) {
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
        if (startWith(scenar, cpos, "$")) {             // variable
          const string vname = getNextParam(scenar, cpos, '=');
          CHECK(vname.empty());
          if (m_var.find(vname) == m_var.end())
            m_var.insert({ vname, "" });

          const string value = getNextParam(scenar, cpos, ';');
          CHECK(value.empty() || !parseScenar(value, Keyword::VALUE, gpos + cpos - value.size() - 1));

          const size_t iBeginValue = iExpr,
                       iEndValue = m_expr.size();
          m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iBeginValue, iEndValue, { vname } });
          iExpr = iEndValue + 1;
        }
        else if (startWith(scenar, cpos, "#macro")) {  // macro
          cpos += 6;
          const string mname = getNextParam(scenar, cpos, '{');
          cpos -= 1;
          const string mvalue = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(mname.empty() || mvalue.empty());
          m_macro.insert({ "#" + mname, mvalue });
          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (startWith(scenar, cpos, "while") || startWith(scenar, cpos, "for") || startWith(scenar, cpos, "if") || startWith(scenar, cpos, "elseif")) {
          const string kname = getNextParam(scenar, cpos, '(');
          CHECK(kname.empty());
          const Keyword keyw = keywordByName(kname);

          const size_t iBeginCondition = iExpr;
          --cpos;
          const string condition = getIntroScenar(scenar, cpos, '(', ')');
          CHECK(condition.empty() || !parseScenar(condition, Keyword::VALUE, gpos + cpos - condition.size() - 2));

          const size_t iBeginBody = m_expr.size();
          const string body = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(body.empty() || !parseScenar(body, Keyword::SEQUENCE, gpos + cpos - body.size() - 2));
          const size_t iEndBody = m_expr.size();

          m_expr.emplace_back<Expression>({ keyw, iBeginBody, iEndBody, { to_string(iBeginCondition) } });
          if (keyw == Keyword::FOR)
            m_expr.back().args.push_back("0"); // current index
          iExpr = iEndBody + 1;
          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (startWith(scenar, cpos, "else")) {
          cpos += 4;
          const string body = getIntroScenar(scenar, cpos, '{', '}');
          CHECK(body.empty() || !parseScenar(body, Keyword::SEQUENCE, gpos + cpos - body.size() - 2));

          const size_t iBeginBody = iExpr,
                       iEndBody = m_expr.size();
          m_expr.emplace_back<Expression>({ Keyword::ELSE, iBeginBody, iEndBody, { } });
          iExpr = iEndBody + 1;
          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
        else if (startWith(scenar, cpos, "break") || startWith(scenar, cpos, "continue")) {
          const string kname = getNextParam(scenar, cpos, ';');
          CHECK(kname.empty());
          const Keyword keyw = keywordByName(kname);
          m_expr.emplace_back<Expression>({ keyw, iExpr, iExpr + 1, {} });
          ++iExpr;
        }
        else if (startWith(scenar, cpos, "#")) {  // macro
          const string mname = getNextParam(scenar, cpos, ';');
          CHECK(mname.empty() || (m_macro.find(mname) == m_macro.end()));
          CHECK(!parseScenar(m_macro[mname], Keyword::SEQUENCE, gpos + cpos - mname.size() - 1));
          iExpr = m_expr.size();
        }
        else if (startWith(scenar, cpos, ";")) {
          ++cpos;
        }
        else {
          const string fname = getNextParam(scenar, cpos, '('); // user function
          CHECK(fname.empty() || (m_ufunc.find(fname) == m_ufunc.end()));
          --cpos;
          const string args = getIntroScenar(scenar, cpos, '(', ')');
          if (!args.empty())
            CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));

          const size_t iBeginArgs = iExpr,
                       iEndArgs = m_expr.size();
          m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iBeginArgs, iEndArgs, { fname } });
          iExpr = iEndArgs + 1;
          if ((cpos < scenar.size()) && (scenar[cpos] == ';')) ++cpos;
        }
      }
    }
    break;
    case InterpreterImpl::Keyword::VALUE: {
      while (cpos < ssz){                 
        if (startWith(scenar, cpos, "$")) {  // variable
          size_t posmem = cpos;
          auto opr = getNextOperator(scenar, cpos);

          string vname = !opr.empty() ? scenar.substr(posmem, cpos - posmem - opr.size()) : scenar.substr(cpos);
          if (!vname.empty() && (vname.back() == ';')) vname.pop_back();

          CHECK(m_var.find(vname) == m_var.end());
            
          if (opr.empty()) cpos = ssz;      // end

          m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, iExpr + 1,{ vname, opr } }); // varName, opr
          ++iExpr;
        }
        else {            
          auto fname = getNextParam(scenar, cpos, '(');
          if (!fname.empty()) {            // function
            CHECK(m_ufunc.find(fname) == m_ufunc.end());
            --cpos;
            const string args = getIntroScenar(scenar, cpos, '(', ')');
            if (!args.empty())
              CHECK(!parseScenar(args, Keyword::ARGUMENT, gpos + cpos - args.size() - 2));
            const size_t iBeginArgs = iExpr,
                          iEndArgs = m_expr.size();

            auto opr = getNextOperator(scenar, cpos);
            if (opr.empty()) cpos = ssz; // end
             
            m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iBeginArgs, iEndArgs, { fname, opr } }); // fName, opr
            iExpr = iEndArgs + 1;
          }
          else {                            // value
            size_t posmem = cpos;
            auto opr = getNextOperator(scenar, cpos);

            string value = !opr.empty() ? scenar.substr(posmem, cpos - posmem - opr.size()) : scenar.substr(cpos);
            if (!value.empty() && (value.back() == ';')) value.pop_back();

            if (opr.empty()) cpos = ssz;    // end

            m_expr.emplace_back<Expression>({ Keyword::VALUE, iExpr, iExpr + 1,{ value, opr } });    // value, opr
            ++iExpr;
          }
        }
      } 
    }
    break;
    case InterpreterImpl::Keyword::ARGUMENT:{
      const string auxScenar = "," + scenar + ",";
      while (cpos < ssz){
        const string arg = getIntroScenar(auxScenar, cpos, ',', ',');
        CHECK(!arg.empty() && !parseScenar(arg, Keyword::VALUE, gpos + cpos - arg.size() - 2));
          
        const size_t iBeginArg = iExpr,
                     iEndArg = m_expr.size();
        m_expr.emplace_back<Expression>({ Keyword::ARGUMENT, iBeginArg, iEndArg, { arg } });    // value, opr
        iExpr = iEndArg + 1;

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

Interpreter::Interpreter(const map<string, UserFunction>& ufuncs, const map<string, UserOperator>& uopers){
  m_d = new InterpreterImpl(ufuncs, uopers);
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
bool Interpreter::addOperator(const string& name, UserOperator uoper){
  return m_d->addOperator(name, uoper);
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