#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

class InterpreterImpl;
class Interpreter {

public:
  using UserFunction = std::function<std::string(const std::vector<std::string>& args)>;
  using UserOperator = std::function<std::string(std::string& ioOperandOne, std::string& ioOperandTwo)>;

  explicit 
  Interpreter();
  ~Interpreter();

  Interpreter(const Interpreter&) = delete;
  Interpreter& operator=(const Interpreter&) = delete;

  bool addFunction(const std::string& name, UserFunction ufunc);

  bool addOperator(const std::string& name, UserOperator uopr, uint32_t priority);
    
  /// return result or error
  std::string cmd(std::string scenar);

private:
  InterpreterImpl* m_d = nullptr;
};