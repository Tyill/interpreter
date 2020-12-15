
#include "../include/interpreter.h"

#include <vector>
#include <sstream>
#include <map>
#include <algorithm>

using namespace std;

class InterpreterImpl {
public:
  InterpreterImpl(string scenar, string& err);
  bool addFunction(const string& name, Interpreter::UserFunction ufunc);
  bool addOperator(const string& name, Interpreter::UserOperator uopr);
  void start();
  void stop();
  void pause(bool set);
private:
	bool m_run, m_pause;
	enum class Keyword{
		SEQUENCE,
		WHILE,
		FOR,
		IF,
		ELSE,
		BREAK,
		CONTINUE,
    FUNCTION,
    VARIABLE,
    VALUE,
    ARGS,
    MACRO,
    OPERATOR
	};
	struct Expression{    
    Keyword keyw;
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
  Keyword nextKeyword(const string& oprName);  
  string getNextParam(const string& scenar, int& stPos, int& endPos, const string& symb);
  pair<string, string> // pname, selSymb
  getNextParam(const string& scenar, int& stPos, int& endPos, const vector<string>& symb);
  string getIntroScenar(const string& scenar, int& stPos, int& endPos);
  bool parseScenar(const string& scenar, Keyword mainOpr);
};

InterpreterImpl::InterpreterImpl(string scenar, string& err) :
  m_run(false),
  m_pause(false){
  
  scenar.erase(remove(scenar.begin(), scenar.end(), '\n'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\t'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\v'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\f'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), '\r'), scenar.end());
  scenar.erase(remove(scenar.begin(), scenar.end(), ' '), scenar.end());
 
  if (!parseScenar(move(scenar), Keyword::SEQUENCE)){
    m_expr.clear();
    err = m_err;
  }
}

bool InterpreterImpl::addFunction(const string& name, Interpreter::UserFunction ufunc){
  if (nextKeyword(name) != Keyword::SEQUENCE) return false;
  m_ufunc.insert({ name, move(ufunc) });
  return true;
}
bool InterpreterImpl::addOperator(const string& name, Interpreter::UserOperator uopr){
  m_uoper.insert({ name, move(uopr) });
  return true;
}
void InterpreterImpl::start(){
  if (!m_run){
    m_run = true;
    workCycle();
  }
}
void InterpreterImpl::stop(){
  m_run = false;
  m_pause = false;
}
void InterpreterImpl::pause(bool set){
  m_pause = set;
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
  return str.find(begin, pos) == 0;
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

bool InterpreterImpl::parseScenar(const string& scenar, Keyword ckeyword){

  size_t ssz = scenar.size(),
         iExpr = m_expr.size();
  int stp = 0,
      enp = 0;

#define CHECK(condition, pos)                                                                                  \
  if (condition){                                                                                              \
    m_err = "Error scenar pos " + to_string(pos) + " src line " + to_string(__LINE__ - 1) + ": " + #condition; \
    return false;                                                                                              \
  }   

  while (enp < ssz){
    switch (ckeyword){
      case InterpreterImpl::Keyword::SEQUENCE: {
        if (startWith(scenar, stp, "$")) {             // variable
          const string vname = getNextParam(scenar, stp, enp, "=");
          CHECK(vname.empty(), stp);
          if (m_var.find(vname) == m_var.end())
            m_var.insert({ vname, "" });

          const size_t iBeginValue = iExpr + 1;
          const string value = getNextParam(scenar, stp, enp, ";");
          CHECK(value.empty() || !parseScenar(value + ";", Keyword::VALUE), stp);
          const size_t iEndValue = m_expr.size();
                
          m_expr.emplace_back<Expression>({ Keyword::VARIABLE, { vname, to_string(iBeginValue), to_string(iEndValue) } });
          iExpr = iEndValue + 1;
        }
        else if (startWith(scenar, stp, "#macro")) {  // macro
          stp += 6;
          const string mname = getNextParam(scenar, stp, enp, "{");
          const string mvalue = getIntroScenar(scenar, stp, enp);
          CHECK(mname.empty() || mvalue.empty(), stp);
          m_macro.insert({ "#" + mname, mvalue });
        }
        else if (startWith(scenar, stp, "while") || startWith(scenar, stp, "for") || startWith(scenar, stp, "if")) {
          const string kname = getNextParam(scenar, stp, enp, "(");
          CHECK(kname.empty(), stp);
          const Keyword keyw = nextKeyword(kname); 

          const size_t iBeginCondition = iExpr + 1;
          const string condition = getNextParam(scenar, stp, enp, ")");
          CHECK(condition.empty() || !parseScenar(condition, Keyword::VALUE), stp);
               
          const size_t iBeginBody = m_expr.size();
          const string body = getIntroScenar(scenar, stp, enp);
          CHECK(body.empty() || !parseScenar(body, Keyword::SEQUENCE), stp);        
          const size_t iEndBody = m_expr.size();

          m_expr.emplace_back<Expression>({ keyw, { to_string(iBeginCondition), to_string(iBeginBody), to_string(iEndBody) } });
          if (keyw == Keyword::FOR)
            m_expr.back().args.push_back("0"); // current index
          iExpr = iEndBody + 1;
        }
        else if (startWith(scenar, stp, "break") || startWith(scenar, stp, "continue")) {
          const string kname = getNextParam(scenar, stp, enp, ";");
          CHECK(kname.empty(), stp);
          const Keyword keyw = nextKeyword(kname);
          m_expr.emplace_back<Expression>({ keyw });
          ++iExpr;
        }
        else {
          const auto param = getNextParam(scenar, stp, enp, vector<string>{ "(", ";" });
          const string& pname = param.first;
          if (m_ufunc.find(pname) != m_ufunc.end()) {                 // user function
            const size_t iBeginArgs = iExpr + 1;
            const string args = getNextParam(scenar, stp, enp, ")");
            CHECK(args.empty() || !parseScenar(args, Keyword::ARGS), stp);
            const size_t iEndArgs = m_expr.size();
          
            m_expr.emplace_back<Expression>({ Keyword::FUNCTION, { pname, to_string(iBeginArgs), to_string(iEndArgs) } });
            iExpr = iEndArgs + 1;
            if ((stp < scenar.size()) && (scenar[stp] == ';')) ++stp;
          }
          else if (m_macro.find(pname) != m_macro.end()) {            // macro
            CHECK(m_macro[pname].empty() || !parseScenar(m_macro[pname], Keyword::SEQUENCE), stp);
            iExpr = m_expr.size();
          }        
          else{
            m_err = "Error scenar pos " + to_string(stp) + " src line " + to_string(__LINE__) + ": parsing error";
            return false;
          }
        }
      }
      break;
      case InterpreterImpl::Keyword::VALUE: {
        vector<pair<string, string>> retOprs;
        vector<string> oprs{ ";" };
        for (const auto& o : m_uoper) oprs.emplace_back(o.first);
        auto param = getNextParam(scenar, stp, enp, oprs);
        while (!param.first.empty()) {
          retOprs.emplace_back<pair<string, string>>({param.first, param.second});
          param = getNextParam(scenar, stp, enp, oprs);
        }
        if (enp < scenar.size())
          retOprs.emplace_back<pair<string, string>>({ scenar.substr(enp), ";" });
        CHECK(retOprs.empty(), stp);

        for (size_t i = 0; i < retOprs.size(); ++i){
          const auto& opr = retOprs[i];
          if (startWith(opr.first, 0, "$")) {  // variable
            CHECK(m_var.find(opr.first) == m_var.end(), stp);
            m_expr.emplace_back<Expression>({ Keyword::OPERATOR, { opr.first, opr.second, to_string(iExpr) } }); // varName, opr, inx
            ++iExpr;
          }else{ 
            stp = enp = 0;
            auto fname = getNextParam(opr.first, stp, enp, "(");
            if (!fname.empty() && (m_ufunc.find(fname) != m_ufunc.end())){ // function
              const size_t iBeginArgs = iExpr + 1;
              const string args = getNextParam(opr.first, stp, enp, ")");
              CHECK(args.empty() || !parseScenar(args, Keyword::ARGS), stp);
              const size_t iEndArgs = m_expr.size();

              m_expr.emplace_back<Expression>({ Keyword::FUNCTION, { fname, to_string(iBeginArgs), to_string(iEndArgs) } });
              iExpr = iEndArgs + 1;
            }else{                           // value
              m_expr.emplace_back<Expression>({ Keyword::VALUE, { opr.first, to_string(iExpr) } });
              ++iExpr;
            }
          }
        }      
      }
      return true;
      case InterpreterImpl::Keyword::ARGS:{
        vector<string> retOprs;
        auto param = getNextParam(scenar, stp, enp, ",");
        while (!param.empty()) {
          retOprs.emplace_back(param);
          param = getNextParam(scenar, stp, enp, ",");
        }
        retOprs.emplace_back(scenar.substr(stp));

        for (size_t i = 0; i < retOprs.size(); ++i){
          const auto& opr = retOprs[i];

          const size_t iBeginValue = iExpr + 1;
          CHECK(opr.empty() || !parseScenar(opr, Keyword::VALUE), stp);
          const size_t iEndValue = m_expr.size();

          m_expr.emplace_back<Expression>({ Keyword::ARGS, { opr, to_string(iBeginValue), to_string(iEndValue) } });
          ++iExpr;
        }
      }
      return true;
    }    
  }
}
InterpreterImpl::Keyword InterpreterImpl::nextKeyword(const string& oprName){
  Keyword nextOpr = Keyword::SEQUENCE;
  if (oprName == "if") nextOpr = Keyword::IF;
  else if (oprName == "while") nextOpr = Keyword::WHILE;
  else if (oprName == "for") nextOpr = Keyword::FOR;
  else if (oprName == "break") nextOpr = Keyword::BREAK;
  else if (oprName == "continue") nextOpr = Keyword::CONTINUE;
  return nextOpr;
}
string InterpreterImpl::getNextParam(const string& scenar, int& stp, int& enp, const string& symb) {
  size_t pos = scenar.find(symb, enp);  
  string res = "";
  if (pos != string::npos) {
    stp = enp;
    res = scenar.substr(stp, pos - stp);
    enp = pos + symb.size();
  }
  return res;
}
pair<string, string> InterpreterImpl::getNextParam(const string& scenar, int& stp, int& enp, const vector<string>& symb){
  size_t minp = string::npos;
  string selSym;
  for (auto& s : symb) {
    size_t pos = scenar.find(s, enp);
    if ((pos != string::npos) && ((pos < minp) || (minp == string::npos))) {
      minp = pos;
      selSym = s;
    }
  }
  string res = "";
  if (minp != string::npos) {
    stp = enp;
    res = scenar.substr(stp, minp - stp);
    enp = minp + selSym.size();
  }
  return make_pair(res, selSym);
}
string InterpreterImpl::getIntroScenar(const string& scenar, int& stPos, int& endPos){
  stPos = endPos + 1;
  int ssz = scenar.size(),
      cp = stPos,
      bordCnt = 0;
  while (cp < ssz){
    if (scenar[cp] == '{') ++bordCnt;
    if (scenar[cp] == '}') --bordCnt;
    if (bordCnt == 0) break;
    ++cp;
  }
  endPos = cp;
  return (bordCnt == 0) ? scenar.substr(stPos, endPos - stPos) : "";  
}

Interpreter::Interpreter(std::string scenar, std::string& err){
  m_d = new InterpreterImpl(scenar, err);
}
Interpreter::~Interpreter(){
  delete m_d;
}
bool Interpreter::addFunction(const std::string& name, UserFunction ufunc){
  return m_d->addFunction(name, ufunc);
}
bool Interpreter::addOperator(const std::string& name, UserOperator uoper){
  return m_d->addOperator(name, uoper);
}
void Interpreter::start(){
  return m_d->start();
}
void Interpreter::stop(){
  return m_d->stop();
}
void Interpreter::pause(bool set){
  return m_d->pause(set);
}