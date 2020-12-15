#pragma once

#include <string>
#include <functional>

class InterpreterImpl;
class Interpreter {

public:
  using UserFunction = std::function<std::string(const std::vector<std::string>& args)>;
  using UserOperator = std::function<std::string(const std::string& operand1, const std::string& operand2)>;

  Interpreter(std::string scenar, std::string& err);

  bool addFunction(const std::string& name, UserFunction ufunc);

  bool addOperator(const std::string& name, UserOperator uopr);

  void start();

  void stop();

  void pause(bool set);

private:
  InterpreterImpl* m_d = nullptr;
};