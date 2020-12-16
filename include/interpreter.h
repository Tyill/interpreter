#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

class InterpreterImpl;
class Interpreter {

public:
  using UserFunction = std::function<std::string(const std::vector<std::string>& args)>;
  using UserOperator = std::function<std::string(const std::string& operand1, const std::string& operand2)>;

  explicit 
  Interpreter(const std::map<std::string, UserFunction>& ufuncs = std::map<std::string, UserFunction>(),
              const std::map<std::string, UserOperator>& uopers = std::map<std::string, UserOperator>());
  ~Interpreter();

  Interpreter(const Interpreter&) = delete;
  Interpreter& operator=(const Interpreter&) = delete;

  bool addFunction(const std::string& name, UserFunction ufunc);

  bool addOperator(const std::string& name, UserOperator uopr);

  bool parseScenar(std::string scenar, std::string& out_err);
  
  bool start(bool asynch);

  bool stop();

  bool pause(bool set);

private:
  InterpreterImpl* m_d = nullptr;
};