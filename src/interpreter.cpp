
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
    OPERATOR
	};

	struct Expression{
    Interpreter::UserFunction ufunc;
    Interpreter::UserOperator uoper;
    Keyword keyw;
	  std::vector<std::string> args;
  };

  std::map<std::string, Interpreter::UserFunction> m_ufunc;
  std::map<std::string, Interpreter::UserOperator> m_uoper;
  std::map<std::string, std::string> m_var;

	std::vector<Expression> m_expr;

  std::string runUserFunction(int iExpr);

  std::string runUserOperator(int iExpr);

  void workIntroCycle(int& cPos);

  void workCycle();

  Keyword nextKeyword(const std::string& oprName);
	
  std::vector<std::string> split(const std::string& str, char sep);

  std::string getNextParam(const std::string& scenar, int& stPos, int& endPos, const std::string& symb);

  std::string getIntroScenar(const std::string& scenar, int& stPos, int& endPos, int offs);

  void addExpression(const std::string& fName, const std::string& args, Keyword mainCmd);

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
  m_ufunc[name] = std::move(ufunc);
  return true;
}

bool InterpreterImpl::addOperator(const std::string& name, Interpreter::UserOperator uopr){
  m_uoper[name] = std::move(uopr);
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

std::string InterpreterImpl::runUserFunction(int inx){

  return std::string();
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

InterpreterImpl::Keyword InterpreterImpl::nextKeyword(const std::string& oprName){
  Keyword nextOpr = Keyword::SEQUENCE;
  if (oprName == "if") nextOpr = Keyword::IF;
  else if (oprName == "cycle") nextOpr = Keyword::CYCLE;
  else if (oprName == "for") nextOpr = Keyword::FOR;
  else if (oprName == "break") nextOpr = Keyword::BREAK;
  else if (oprName == "continue") nextOpr = Keyword::CONTINUE;
  return nextOpr;
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

std::string InterpreterImpl::getNextParam(const std::string& scenar, int& stPos, int& endPos, const std::string& symb){
  stPos = endPos + 1;
  endPos = scenar.find(symb, stPos);
  return (endPos > 0) ? scenar.substr(stPos, endPos - stPos) : "err";
}

std::string InterpreterImpl::getIntroScenar(const std::string& scenar, int& stPos, int& endPos, int offs){
  stPos = endPos + offs;
  int ssz = scenar.size(),
      cPos = stPos,
      bordCnt = 0;
  while (cPos < ssz){
    if (scenar[cPos] == '{') ++bordCnt;
    if (scenar[cPos] == '}') --bordCnt;
    if (bordCnt < 0) break;
    ++cPos;
  }
  if (bordCnt >= 0) return "err";
  endPos = cPos;
  return scenar.substr(stPos, endPos - stPos);
}

void InterpreterImpl::addExpression(const std::string& fName, const std::string& args, Keyword mainCmd){
  Expression expr{ 0 };
  expr.ufunc = m_ufunc[fName];
  expr.args = split(args, ',');
  m_expr.push_back(expr);
}

bool InterpreterImpl::parseScenar(const std::string& scenar, Keyword mainOpr){

  int stPos = 0,
      endPos = -1,
      ssz = scenar.size(),
      iExpr = m_expr.size(),
      position = 0;
  while (stPos < ssz){
    int stp = stPos, enp = endPos;
    std::string fName = getNextParam(scenar, stp, enp, ";");

    if ((fName != "break") && (fName != "continue")) {
      fName = getNextParam(scenar, stPos, endPos, "(");
      if (fName == "err"){
        if (mainOpr != Keyword::SEQUENCE)
          m_expr[iExpr].position = m_expr.size() - iExpr;
        break;
      }
    }

    Keyword nxtOpr = nextKeyword(fName);
    if (nxtOpr != Keyword::SEQUENCE){

      if (nxtOpr == Keyword::FOR){
        std::string args = getNextParam(scenar, stPos, endPos, ")");
        if (args == "err") return false;
        if (args.empty()) args.push_back('0');
        addExpression(fName, args, nxtOpr);
        --endPos;
      }
      else if (nxtOpr == Keyword::BREAK){
        getNextParam(scenar, stPos, endPos, ";");
        addExpression(fName, "", nxtOpr);
        ++position;
        continue;
      }
      else if (nxtOpr == Keyword::CONTINUE){
        getNextParam(scenar, stPos, endPos, ";");
        addExpression(fName, "", nxtOpr);
        ++position;
        continue;
      }
      else{
        fName = getNextParam(scenar, stPos, endPos, "(");
        if (m_ufunc.find(fName) == m_ufunc.end()) return false;
        std::string args = getNextParam(scenar, stPos, endPos, ")");
        if (args == "err") return false;
        addExpression(fName, args, nxtOpr);
      }

      int positionMem = position;
      ++position;

      std::string introScenar = getIntroScenar(scenar, stPos, endPos, 3);
      if (introScenar == "err") return false;
      ++endPos;

      if (!parsScenar(introScenar, nxtOpr)) return false;

      int stp = stPos, enp = endPos;
      if (getNextParam(scenar, stp, enp, "{") == "else"){

        fName = getNextParam(scenar, stPos, endPos, "{");
        addExpression(fName, to_string(bPos + positionMem + 1) + ", 0, 0", Keyword::ELSE);
        ++position;

        std::string introScenar = getIntroScenar(scenar, stPos, endPos, 1);
        if (introScenar == "err") return false;
        endPos++;

        if (!parsScenar(introScenar, elseCmd)) return false;
      }
      continue;
    }
    if (m_ufunc.find(fName) == m_ufunc.end())	return false;
    std::string args = getNextParam(scenar, stPos, endPos, ")");
    if (args == "err") return false;
    addExpression(fName, args, Keyword::SEQUENCE);
    ++position;
    ++endPos;
  }
  return true;
}