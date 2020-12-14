
#include "../include/interpreter.h"

#include <vector>
#include <sstream>
#include <map>
#include <algorithm>

class InterpreterImpl {
public:
  InterpreterImpl(std::string scenar);
  bool addFunction(const std::string& name, Interpreter::UserFunction ufunc);
  bool addOperator(const std::string& name, Interpreter::UserOperator uopr);
  void start();
  void stop();
  void pause(bool set);
  bool parseScenar(std::string scenar);
private:
	bool m_run, m_pause;
	enum class Keyword{
		SEQUENCE,
		CYCLE,
		FOR,
		IF,
		ELSE,
		BREAK,
		CONTINUE,
    FUNCTION,
    VARIABLE,
    MACRO,
    OPERATOR
	};
	struct Expression{
    Interpreter::UserFunction ufunc;
    Interpreter::UserOperator uoper;
    Keyword keyw;
    int position;
	  std::vector<std::string> args;
    Expression(Keyword _keyw, int _position, const std::vector<std::string>& _args):
      ufunc(nullptr), uoper(nullptr), keyw(_keyw), position(_position), args(_args){}
  };
  std::map<std::string, Interpreter::UserFunction> m_ufunc;
  std::map<std::string, Interpreter::UserOperator> m_uoper;
  std::map<std::string, std::string> m_var;
  std::map<std::string, std::string> m_macro;
	std::vector<Expression> m_expr;

  std::vector<std::string> split(const std::string& str, char sep);
  bool startWith(const std::string& str, int pos, const std::string& begin);
  bool endWith(const std::string& str, int pos, const std::string& end);

  std::string runUserFunction(int iExpr);
  std::string runUserOperator(int iExpr);
  void workIntroCycle(int& cPos);
  void workCycle();
  Keyword nextKeyword(const std::string& oprName);  
  std::string getNextParam(const std::string& scenar, int& stPos, int& endPos, const std::string& symb);
  std::string getIntroScenar(const std::string& scenar, int& stPos, int& endPos);
  bool parseScenar(const std::string& scenar, Keyword mainOpr);
};

InterpreterImpl::InterpreterImpl(std::string scenar) :
  m_run(false),
  m_pause(false){
  if (!scenar.empty())
    parseScenar(std::move(scenar));
}

bool InterpreterImpl::addFunction(const std::string& name, Interpreter::UserFunction ufunc){
  if (nextKeyword(name) != Keyword::SEQUENCE) return false;
  m_ufunc.insert(std::make_pair(name, std::move(ufunc)));
  return true;
}
bool InterpreterImpl::addOperator(const std::string& name, Interpreter::UserOperator uopr){
  m_uoper.insert(std::make_pair(name, std::move(uopr)));
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

std::vector<std::string> InterpreterImpl::split(const std::string& str, char sep) {
  std::vector<std::string> res;
  std::istringstream iss(str);
  std::string token;
  while (getline(iss, token, sep)){
    res.emplace_back(token);
  }
  return res;
}
bool InterpreterImpl::startWith(const std::string& str, int pos, const std::string& begin){
  return str.find(begin, pos) == 0;
}
bool InterpreterImpl::endWith(const std::string& str, int pos, const std::string& end){
  auto p = str.find(end, pos);
  return (p != std::string::npos) && (p == (str.size() - end.size()));
}

std::string InterpreterImpl::runUserFunction(int iExpr){
  auto& args = m_expr[iExpr].args;
  auto& ufunc = m_expr[iExpr].ufunc;
  switch (args.size()){
    case 0:  return std::function<std::string()>(ufunc)();
    case 1:  return std::function<std::string(const std::string&)>(ufunc)(args[0]);
    case 2:  return std::function<std::string(const std::string&, const std::string&)>(ufunc)(args[0], args[1]);
    case 3:  return std::function<std::string(const std::string&, const std::string&, const std::string&)>(ufunc)(args[0], args[1], args[2]);
    case 4:  return std::function<std::string(const std::string&, const std::string&, const std::string&, const std::string&)>(ufunc)(args[0], args[1], args[2], args[3]);
    case 5:  return std::function<std::string(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&)>(ufunc)(args[0], args[1], args[2], args[3], args[4]);
    case 6:  return std::function<std::string(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&)>(ufunc)(args[0], args[1], args[2], args[3], args[4], args[5]);
    case 7:  return std::function<std::string(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&)>(ufunc)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
    default: return std::function<std::string(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&)>(ufunc)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
  }
}
std::string InterpreterImpl::runUserOperator(int iExpr){

  return m_expr[iExpr].uoper(m_expr[iExpr].args[0], m_expr[iExpr].args[1]);
}
void InterpreterImpl::workIntroCycle(int& cPos){

  int bPos = cPos,
    leng = bPos + m_expr[cPos].position;
  cPos += m_expr[cPos].position - 1;
  bool _break = false,
    _continue = false;
  while (true){

    for (int i = bPos; i < leng; ++i){

      //  while (m_pause) Sleep(10);

      if (!m_run) return;

      if (_break || _continue){
        if ((m_expr[bPos].opr == Keyword::CYCLE) || (m_expr[bPos].cmd == Keyword::FOR)){
          if (_continue){
            _continue = false;
            break;
          }
        }
        return;
      }

      if (i == bPos){
        switch (m_expr[bPos].opr){
        case Keyword::IF:
          if (runUserFunc(bPos)){
            m_expr[bPos].isExecute = true;
            continue;
          }
          break;
        case Keyword::FOR:
          if (stoi(m_expr[bPos].args[0]) < stoi(m_expr[bPos].args[1])){
            m_expr[bPos].args[0] = std::to_string(stoi(m_expr[bPos].args[0]) + 1);
            m_expr[bPos].isExecute = true;
            continue;
          }
          else{
            m_expr[bPos].args[0] = "0";
          }
          break;
        case Keyword::CYCLE:
          if (runUserFunc(bPos)){
            m_expr[bPos].isExecute = true;
            continue;
          }
          break;
        case Keyword::ELSE:
          if (!m_expr[p0].isExecute){
            m_expr[bPos].isExecute = true;
            continue;
          }
          break;
        case Keyword::BREAK:
          _break = true;
          continue;
        case Keyword::CONTINUE:
          _continue = true;
          continue;
        }
        m_expr[bPos].isExecute = false;
        return;
      }

      if (m_expr[i].opr != Keyword::SEQUENCE){
        workIntroCycle(i);

        if (i == (leng - 1) && (m_expr[bPos].opr != Keyword::CYCLE) && (m_expr[bPos].opr != Keyword::FOR)) return;
        continue;
      }
      runUserFunc(i);

      if (i == (leng - 1) && (m_expr[bPos].opr != Keyword::CYCLE) && (m_expr[bPos].opr != Keyword::FOR)) return;
    }
  }
}
void InterpreterImpl::workCycle(){

  int esz = m_expr.size();
  for (int i = 0; i < esz; ++i){

    while (m_pause) Sleep(10);

    if (!m_run) break;

    if (m_expr[i].opr != Keyword::SEQUENCE){
      workIntroCycle(i);
      continue;
    }
    runUserFunc(i);
  }
  m_run = false;
}

bool InterpreterImpl::parseScenar(std::string scenar){
  scenar.erase(std::remove(scenar.begin(), scenar.end(), '\n'), scenar.end());
  scenar.erase(std::remove(scenar.begin(), scenar.end(), '\t'), scenar.end());
  scenar.erase(std::remove(scenar.begin(), scenar.end(), '\v'), scenar.end());
  scenar.erase(std::remove(scenar.begin(), scenar.end(), '\f'), scenar.end());
  scenar.erase(std::remove(scenar.begin(), scenar.end(), '\r'), scenar.end());
  scenar.erase(std::remove(scenar.begin(), scenar.end(), ' '), scenar.end());

  if (m_run) stop();

  m_expr.clear();

  bool ok = parseScenar(scenar, Keyword::SEQUENCE);
  if (!ok) m_expr.clear();
  return ok;
}
bool InterpreterImpl::parseScenar(const std::string& scenar, Keyword ckeyword){

  int stp = 0,
      enp = -1,
      ssz = scenar.size(),
      iExpr = m_expr.size();

  while (stp < ssz){
    if (ckeyword == Keyword::SEQUENCE){
      if (startWith(scenar, stp, "$")){             // variable
        std::string vname = getNextParam(scenar, stp, enp, "=");
        if (m_var.find(vname) == m_var.end())
          m_var.insert(std::make_pair(vname, ""));
        m_expr.push_back(Expression(Keyword::VARIABLE, iExpr, { vname, ""/*for value*/ }));
        std::string value = getNextParam(scenar, stp, enp, ";");
        if (value.empty() && !parseScenar(value, Keyword::VARIABLE)) return false;
      }
      else if (startWith(scenar, stp, "#define")){  // macro
        stp += 7;
        std::string mname = getNextParam(scenar, stp, enp, "{");
        std::string mvalue = getIntroScenar(scenar, stp, enp);
        m_macro[mname] = mvalue;
      }
      else if (startWith(scenar, stp, "cycle") || startWith(scenar, stp, "for") || startWith(scenar, stp, "if")){
        auto keyw = nextKeyword(getNextParam(scenar, stp, enp, "("));
        m_expr.push_back(Expression(keyw, iExpr, { ""/*for condition*/ }));
        std::string condition = getNextParam(scenar, stp, enp, ")");
        if (condition.empty() || !parseScenar(condition, keyw)) return false;
      }
      else if (startWith(scenar, stp, "break") || startWith(scenar, stp, "continue")){
        auto keyw = nextKeyword(getNextParam(scenar, stp, enp, ";"));
        m_expr.push_back(Expression(keyw, iExpr, {}));
      }
      else {
        std::string param = getNextParam(scenar, stp, enp, "(");
        if (m_ufunc.find(param) != m_ufunc.end()){ // user function
          m_expr.push_back(Expression(Keyword::FUNCTION, iExpr, { param }));
          std::string args = getNextParam(scenar, stp, enp, ")");
          if (args.empty() || !parseScenar(args, Keyword::FUNCTION)) return false;
        }
        if (param.empty()){
          param = getNextParam(scenar, stp, enp, ";");
          if (m_macro.find(param) != m_macro.end()){ // macro
            if (m_macro[param].empty() || !parseScenar(m_macro[param], Keyword::SEQUENCE)) return false;
          }
        }
        if (param.empty()){
          param = getNextParam(scenar, stp, enp, "$");
          if (m_uoper.find(param) != m_uoper.end()){ // operator
            std::string vname = getNextParam(scenar, stp, enp, ";");
            if (vname.empty()) 
              return false;
            if (m_var.find(vname) == m_var.end())
              m_var.insert(std::make_pair(vname, ""));
            m_var[vname] = m_uoper[param](m_var[vname], m_var[vname]);
          }
        }
        else return false;
      }
    }
  }
}

InterpreterImpl::Keyword InterpreterImpl::nextKeyword(const std::string& oprName){
  Keyword nextOpr = Keyword::SEQUENCE;
  if (oprName == "if") nextOpr = Keyword::IF;
  else if (oprName == "cycle") nextOpr = Keyword::CYCLE;
  else if (oprName == "for") nextOpr = Keyword::FOR;
  else if (oprName == "break") nextOpr = Keyword::BREAK;
  else if (oprName == "continue") nextOpr = Keyword::CONTINUE;
  return nextOpr;
}
std::string InterpreterImpl::getNextParam(const std::string& scenar, int& stPos, int& endPos, const std::string& symb){
  stPos = endPos + 1;
  endPos = scenar.find(symb, stPos) + 1;
  return (endPos > 0) ? scenar.substr(stPos, endPos - stPos) : "";
}
std::string InterpreterImpl::getIntroScenar(const std::string& scenar, int& stPos, int& endPos){
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

