#pragma once

#include <vector>
#include <sstream>

// key words
// cycle(f()){;} // f - user func
// for(n)){;} // n - iter count 
// if(f())){;} 
// break;
// continue;

class Interpreter {
	
public:
  Interpreter(const std::string& scenar = ""):
    m_run(false),
    m_pause(false){
    if (!scenar.empty())
      parseScenar(scenar);
  }
		
	bool addFunction(const std::string& name, UserFunc ufunc){
		if ((nextOperator(name) != Operator::SEQUENCE) ||
        (m_ufunc.find(name) != m_ufunc.end())) return false;
    m_ufunc.insert(std::make_pair(name, ufunc));
    return true;
	}

	void start(){
    if (!m_run){
		  m_run = true;
		  workCycle();
    }
  }

	void stop(){
    m_run = false;
	  m_pause = false;
  }

	void pause(bool set){
    m_pause = set;
  }

	void clear(){
    m_expr.clear();
  }

	bool parseScenar(const std::string& scenar){    
    scenar.erase(std::remove(scenar.begin(), scenar.end(), '\n'), scenar.end());
    scenar.erase(std::remove(scenar.begin(), scenar.end(), '\t'), scenar.end());
    scenar.erase(std::remove(scenar.begin(), scenar.end(), '\v'), scenar.end());
    scenar.erase(std::remove(scenar.begin(), scenar.end(), '\f'), scenar.end());
    scenar.erase(std::remove(scenar.begin(), scenar.end(), '\r'), scenar.end());
    scenar.erase(std::remove(scenar.begin(), scenar.end(), ' '), scenar.end());

    bool ok = parseScenar(scenar, Operator::SEQUENCE);
    if (!ok) m_expr.clear();
    return ok;
  }

private:

	bool m_run, m_pause;

	enum class Operator{
		SEQUENCE,
		CYCLE,
		FOR,
		IF,
		ELSE,
		BREAK,
		CONTINUE,
	};

	struct Expression{
		UserFunc ufunc;
    Operator opr;
		int position;
    bool isExecute;
	  std::vector<std::string> args;
  };

	std::map<std::string, userFunc> m_ufunc;

	std::vector<Expression> m_expr;

  bool runUserFunc(int fPos){
    auto& args = m_expr[fPos].args;
    switch (args.size()):{
      case 0 :    return m_expr[fPos].ufunc();
      case 1 :    return m_expr[fPos].ufunc(args[0]);
      case 2 :    return m_expr[fPos].ufunc(args[0], args[1]);
      case 3 :    return m_expr[fPos].ufunc(args[0], args[1], args[2]);
      case 4 :    return m_expr[fPos].ufunc(args[0], args[1], args[2], args[3]);
      case 5 :    return m_expr[fPos].ufunc(args[0], args[1], args[2], args[3], args[4]);
      case 6 :    return m_expr[fPos].ufunc(args[0], args[1], args[2], args[3], args[4], args[5]);
      case 7 :    return m_expr[fPos].ufunc(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
      default 8 : return m_expr[fPos].ufunc(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    }
  }

	void workIntroCycle(int& cPos){
    
    int bPos = cPos, 
        leng = bPos + m_expr[cPos].position;
    cPos += m_expr[cPos].position - 1;
    bool _break = false,
         _continue = false;
    while (true){

      for (int i = bPos; i < leng; ++i){

        while (m_pause) Sleep(10);

        if (!m_run) return;

        if (_break || _continue){
          if ((m_expr[bPos].opr == Operator::CYCLE) || (m_expr[bPos].cmd == Operator::FOR)){
            if (_continue){
              _continue = false;
              break;
            }
          }
          return;
        }
     
        if (i == bPos){
          switch (m_expr[bPos].opr){
            case Operator::IF:
              if (runUserFunc(bPos)){
                m_expr[bPos].isExecute = true;
                continue;
              }
              break;
            case Operator::FOR:
              if (stoi(m_expr[bPos].args[0]) < stoi(m_expr[bPos].args[1])){
                m_expr[bPos].args[0] = std::to_string(stoi(m_expr[bPos].args[0]) + 1);
                m_expr[bPos].isExecute = true;
                continue;
              }else{
                m_expr[bPos].args[0] = "0";
              }
              break;
            case Operator::CYCLE:
              if (runUserFunc(bPos)){
                m_expr[bPos].isExecute = true;
                continue;
              }
              break;
            case Operator::ELSE:
              if (!m_expr[p0].isExecute){
                m_expr[bPos].isExecute = true;
                continue;
              }
              break;
            case Operator::BREAK:
              _break = true;
              continue;
            case Operator::CONTINUE:
              _continue = true;
              continue;
          }         
          m_expr[bPos].isExecute = false;
          return;
        }

        if (m_expr[i].opr != Operator::SEQUENCE){
          workIntroCycle(i);

          if (i == (leng - 1) && (m_expr[bPos].opr != Operator::CYCLE) && (m_expr[bPos].opr != Operator::FOR)) return;
          continue;
        }
        runUserFunc(i);

        if (i == (leng - 1) && (m_expr[bPos].opr != Operator::CYCLE) && (m_expr[bPos].opr != Operator::FOR)) return;
      }
    }
  }

	void workCycle(){
    
    int esz = m_expr.size();
    for (int i = 0; i < esz; ++i){

      while (m_pause) Sleep(10);

      if (!m_run) break;

      if (m_expr[i].opr != Operator::SEQUENCE){
        workIntroCycle(i);
        continue;
      }
      runUserFunc(i);
    }
    m_run = false;
  }

	Operator nextOperator(const std::string& cName){
    Operator nextOpr = Operator::SEQUENCE;
    if (cName == "if") nextOpr = Operator::IF;
    else if (cName == "cycle") nextOpr = Operator::CYCLE;
    else if (cName == "for") nextOpr = Operator::FOR;
    else if (cName == "break") nextOpr = Operator::BREAK;
    else if (cName == "continue") nextOpr = Operator::CONTINUE;
    return nextOpr;
  }
	
  std::vector<std::string> split(const std::string& str, char sep) {  
    std::vector<std::string> res;
    std::istringstream iss(str);
    std::string token;
    while (getline(iss, token, sep)){
      res.emplace_back(token);
    }
    return res;
  }

	std::string getNextParam(const std::string& scenar, int& stPos, int& endPos, const std::string& symb){
    stPos = endPos + 1;
    endPos = scenar.find(symb, stPos);
    return (endPos > 0) ? scenar.substr(stPos, endPos - stPos) : "err";
  }

	std::string getIntroScenar(const std::string& scenar, int& stPos, int& endPos, int offs){
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

	void addExpression(const std::string& fName, const std::string& args, Operator mainCmd){
    Expression expr{ 0 };
    expr.cmd = mainCmd;
    expr.position = 1;
    expr.ufunc = m_ufunc[fName];
    expr.args = split(args, ",");
    m_expr.push_back(expr);
  }

	bool parseScenar(const std::string& scenar, Operator mainOpr){
    
    int stPos = 0,
        endPos = -1, 
        ssz = scenar.size(),
        bPos = m_expr.size() - 1, 
        position = 0;
    while (stPos < ssz){
      int stp = stPos, enp = endPos;
      std::string fName = getNextParam(scenar, stp, enp, ";");

      if ((fName != "break") && (fName != "continue")) {
        fName = getNextParam(scenar, stPos, endPos, "(");
        if (fName == "err"){
          if (mainOpr != Operator::SEQUENCE)
            m_expr[bPos].position = m_expr.size() - bPos;
          break;
        }
      }

      Operator nxtOpr = nextOperator(fName);
      if (nxtOpr != Operator::SEQUENCE){

        if (nxtOpr == Operator::FOR){
          std::string args = getNextParam(scenar, stPos, endPos, ")");
          if (args == "err") return false;
          if (args.empty()) args.push_back('0');
          addExpression(fName, args, nxtOpr);
          --endPos;
        }
        else if (nxtOpr == Operator::BREAK){
          getNextParam(scenar, stPos, endPos, ";");
          addExpression(fName, "", nxtOpr);
          ++position;				
          continue;
        }
        else if (nxtOpr == Operator::CONTINUE){
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
          addExpression(fName, to_string(bPos + positionMem + 1) + ", 0, 0", Operator::ELSE);
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
      addExpression(fName, args, Operator::SEQUENCE);
      ++position;
      ++endPos;
    }
    return true;
  }
};
