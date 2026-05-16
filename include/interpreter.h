//
// Interpreter Project
// Copyright (C) 2020 by Contributors <https://github.com/Tyill/interpreter>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>


class Interpreter {

public:
  enum class ControlFlow { Break, Continue };

  struct Error {
    enum class Kind { Parse, Runtime } kind = Kind::Parse;
    size_t position = 0;
    std::string message;
    explicit operator bool() const { return !message.empty(); }
  };

  using Value = std::variant<bool, int64_t, double, std::string, ControlFlow>;
  using CmdResult = std::pair<Value, Error>;
  using UserFunction = std::function<Value(const std::vector<Value>& args)>;
  using UserOperator = std::function<Value(Value& ioLeftOperand, Value& ioRightOperand)>;

  explicit
  Interpreter();
  ~Interpreter();

  Interpreter(const Interpreter&);
  Interpreter(Interpreter&&) noexcept;
  Interpreter& operator=(const Interpreter&);  
  Interpreter& operator=(Interpreter&&) noexcept;

  /// Add function
  /// @param name
  /// @param ufunc function
  /// return true - ok
  bool addFunction(const std::string& name, UserFunction ufunc);

  /// Add operator
  /// @param name
  /// @param uopr operator
  /// @param priority
  /// return true - ok
  bool addOperator(const std::string& name, UserOperator uopr, uint32_t priority);

  /// Add attribute
  /// @param name
  /// return true - ok
  bool addAttribute(const std::string& name);
   
  /// Execute script (== parseScript + runScript). Re-parses when script text changes.
  /// @param script
  /// @return result and parse error (if any)
  CmdResult cmd(std::string script);

  /// Parse script; on failure outErr carries kind, position, and message.
  bool parseScript(std::string script, Error& outErr);

  /// Run parsed script; second is runtime error (empty on success).
  /// Use after parseScript on the same script text (AST cached via m_prevScript).
  CmdResult runScript();

  /// All variables
  /// @return vname, value
  std::map<std::string, Value> allVariables() const;

  /// Value of variable
  /// @param vname
  /// @return value
  Value variable(const std::string& vname) const;

  /// Run of user function
  /// @param fname
  /// @param args
  /// @return result
  Value runFunction(const std::string& fname, const std::vector<Value>& args);

  /// Set value of variable
  /// @param vname
  /// @param value
  /// @return true - ok
  bool setVariable(const std::string& vname, const Value& value);

  /// Set macro
  /// @param mname
  /// @param script
  /// @return true - ok
  bool setMacro(const std::string& mname, const std::string& script);

  /// Go-to on label
  /// @param lname label name
  /// @return true - ok
  bool gotoOnLabel(const std::string& lname);
    
  /// Exit from script
  void exitFromScript();
  

  //// Reflection part ////////////////////////////////////

  enum class EntityType {
    EXPRESSION,
    OPERATOR,
    WHILE,
    IF,
    ELSE,
    ELSE_IF,
    BREAK,
    CONTINUE,
    FUNCTION,
    ARGUMENT,
    VARIABLE,
    VALUE,
    GOTO,
    MACRO,
  };

  static constexpr size_t NoLinkIndex = static_cast<size_t>(-1);

  /// Internal object
  struct Entity {
    size_t beginIndex;
    size_t conditionEndIndex;
    size_t bodyEndIndex;
    EntityType type;
    std::string name;
    Value value{std::string{}};
    size_t linkIndex = NoLinkIndex;
  };

  /// Get all entities
  std::vector<Entity> allEntities();

  /// Current entity
  Entity currentEntity();

  /// Entity by index
  Entity getEntityByIndex(size_t beginIndex);

  /// Attribute by index
  std::vector<std::string> getAttributeByIndex(size_t beginIndex);

  /// Go-to on entity
  /// @return true - ok
  bool gotoOnEntity(size_t beginIndex);

  UserFunction getUserFunction(const std::string& fname);

  UserOperator getUserOperator(const std::string& oname);

  //// Reflection part ////////////////////////////////////

  //// Value helpers ////////////////////////////////////

  static std::string valueToString(const Value& v);
  static Value valueFromLiteral(std::string_view s);
  static bool valueIsTruthy(const Value& v);
  static std::string valueAsString(const Value& v);
  static std::string valueAsInitBody(const Value& v);
  static std::string operandName(Interpreter& ir, const Value& opd);
  static bool valueIsInteger(const Value& v);
  static bool valueIsNumeric(const Value& v);
  static int64_t valueAsInt64(const Value& v);
  static std::vector<Value> makeParam(const std::string& s);
  static std::vector<Value> makeParam(std::string_view s);
  static std::string paramAsString(const std::vector<Value>& params);
  static Value valueFromParams(const std::vector<Value>& params, const Value& fallback);
  static std::string cmdResultToString(const CmdResult& r);
  static bool hasError(const Error& err);
  static bool isParseError(const Error& err);
  static std::string_view valueAsStringView(const Value& v);
  static bool valueEquals(const Value& a, const Value& b);
  static int valueCompare(const Value& a, const Value& b);

private:
  class Impl;
  std::unique_ptr<Impl> m_d;
};