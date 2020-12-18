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

  bool parseScenar(std::string scenar, std::string& out_err);
  
  bool start(bool async);

  bool stop();

  bool pause(bool set);

  /// return result
//  string cmd(std::string scenar, std::string& out_err);

private:
  InterpreterImpl* m_d = nullptr;
};