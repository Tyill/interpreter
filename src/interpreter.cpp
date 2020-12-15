
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
    int position;
	  vector<string> args;
    string result;
    Expression(Keyword _keyw, int _position, const vector<string>& _args):
      keyw(_keyw), position(_position), args(_args){}
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
  return function<string(const vector<string>&)>(m_expr[iExpr].ufunc)(m_expr[iExpr].args);
}
string InterpreterImpl::runUserOperator(int iExpr) {
  return m_expr[iExpr].uoper(m_expr[iExpr].args[0], m_expr[iExpr].args[1]);
}

bool InterpreterImpl::parseScenar(const string& scenar, Keyword ckeyword){

  size_t ssz = scenar.size(),
         iExpr = m_expr.size();
  int stp = 0,
      enp = ssz;

  while (stp < ssz){
    switch (ckeyword){
    case InterpreterImpl::Keyword::SEQUENCE: {
      if (startWith(scenar, stp, "$")) {             // variable
        const string vname = getNextParam(scenar, stp, enp, "=");
        if (vname.empty()) return false;
        if (m_var.find(vname) == m_var.end())
          m_var.insert({ vname, "" });
        const string value = getNextParam(scenar, stp, enp, ";");
        if (value.empty() || !parseScenar(value, Keyword::VALUE)) return false;
        const size_t iValue = m_expr.size() - 1;
        m_expr.emplace_back<Expression>({ Keyword::VARIABLE, iExpr, { vname, to_string(iValue)/*value*/}});
        iExpr = m_expr.size();
      }
      else if (startWith(scenar, stp, "#macro")) {  // macro
        stp += 6;
        const string mname = getNextParam(scenar, stp, enp, "{");
        const string mvalue = getIntroScenar(scenar, stp, enp);
        if (mname.empty() || mvalue.empty()) return false;
        m_macro.insert({ "#" + mname, mvalue });
      }
      else if (startWith(scenar, stp, "while") || startWith(scenar, stp, "for") || startWith(scenar, stp, "if")) {
        const string kname = getNextParam(scenar, stp, enp, "(");
        if (kname.empty()) return false;
        const Keyword keyw = nextKeyword(kname); 

        const string condition = getNextParam(scenar, stp, enp, ")");
        if (condition.empty() || !parseScenar(condition, Keyword::VALUE)) return false;
        const size_t iCondition = m_expr.size() - 1;

        const string body = getIntroScenar(scenar, stp, enp);
        if (body.empty() || !parseScenar(body, Keyword::SEQUENCE)) return false;
        const size_t iBody = m_expr.size() - 1;

        m_expr.emplace_back<Expression>({ keyw, iExpr, { to_string(iCondition), to_string(iBody) } });
        if (keyw == Keyword::FOR)
          m_expr[iExpr].args.push_back("0"); // current index
        iExpr = m_expr.size();        
      }
      else if (startWith(scenar, stp, "break") || startWith(scenar, stp, "continue")) {
        const string kname = getNextParam(scenar, stp, enp, ";");
        if (kname.empty()) return false;
        const Keyword keyw = nextKeyword(kname);
        m_expr.emplace_back<Expression>({ keyw, iExpr, {} });
        iExpr = m_expr.size();
      }
      else {
        const auto param = getNextParam(scenar, stp, enp, { "(", ";", "$" });
        const string& pname = param.first;
        if (m_ufunc.find(pname) != m_ufunc.end()) {      // user function
          const string args = getNextParam(scenar, stp, enp, ")");
          if (args.empty() || !parseScenar(args, Keyword::ARGS)) return false;
          
          const size_t iArgs = m_expr.size() - 1;
          m_expr.emplace_back<Expression>({ Keyword::FUNCTION, iExpr, { pname, to_string(iArgs) } });
          iExpr = m_expr.size();
          if ((stp < scenar.size()) && (scenar[stp] == ';')) ++stp; // append ';'
        }
        else if (m_macro.find(pname) != m_macro.end()) { // macro
          if (m_macro[pname].empty() || !parseScenar(m_macro[pname], Keyword::SEQUENCE)) return false;
          iExpr = m_expr.size();
        }        
        else return false;
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
      if (retOprs.empty()) return false;
      auto prevOpr = retOprs.front();
      for (size_t i = 1; i < retOprs.size(); ++i){
        const auto& currOpr = retOprs[i];
        if (startWith(prevOpr.first, 0, "$") && startWith(currOpr.first, 0, "$")) {
          if ((m_var.find(prevOpr.first) == m_var.end()) || (m_var.find(currOpr.first) == m_var.end()))
            return false;
          m_expr.emplace_back<Expression>({ Keyword::OPERATOR, iExpr, { prevOpr.second, iExpr - 1, m_var[currOpr.first] } });
          ++iExpr;
          prevOpr = currOpr;
        }
      }        
    }
    break;
    case InterpreterImpl::Keyword::WHILE:
      break;
    case InterpreterImpl::Keyword::FOR:
      break;
    case InterpreterImpl::Keyword::IF:
      break;
    case InterpreterImpl::Keyword::ELSE:
      break;
    case InterpreterImpl::Keyword::BREAK:
      break;
    case InterpreterImpl::Keyword::CONTINUE:
      break;
    case InterpreterImpl::Keyword::FUNCTION:
      break;
    case InterpreterImpl::Keyword::MACRO:
      break;
    case InterpreterImpl::Keyword::OPERATOR:
      break;
    default:
      break;
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

