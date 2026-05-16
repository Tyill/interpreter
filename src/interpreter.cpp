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
#include "../include/interpreter.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <type_traits>
#include <utility>

// --- ir::detail (declarations) ---

namespace ir {
namespace detail {

constexpr size_t kNoIndex = static_cast<size_t>(-1);

bool isDigits(std::string_view s);
std::optional<int64_t> parseInteger(std::string_view s);
std::optional<Interpreter::Value> parseNumber(std::string_view s);
bool isIdentContinue(unsigned char c);
bool matchKeywordAt(std::string_view script, size_t cpos, std::string_view kw);
Interpreter::Value parseLiteralText(std::string_view s);
Interpreter::Value parseQuotedLiteral(std::string s);
Interpreter::Value valueOfValueExpr(const Interpreter::Value& result,
    const std::vector<Interpreter::Value>& params);
const std::string& paramKey(const std::vector<Interpreter::Value>& params);
size_t paramIndex(const std::vector<Interpreter::Value>& params);
bool isBreakValue(const Interpreter::Value& v);
bool isContinueValue(const Interpreter::Value& v);

} // namespace detail
} // namespace ir

class Interpreter::Impl {
public:
  Impl() = default;
  bool addFunction(const std::string& name, Interpreter::UserFunction ufunc);
  bool addOperator(const std::string& name, Interpreter::UserOperator uopr, uint32_t priority);
  bool addAttribute(const std::string& name);
  Interpreter::CmdResult cmd(std::string script);
  bool parseScript(std::string script, Interpreter::Error& outErr);
  Interpreter::Value evalScript();
  std::pair<Interpreter::Value, Interpreter::Error> runScript();
  std::map<std::string, Interpreter::Value> allVariables() const;
  Interpreter::Value variable(const std::string& vname) const;
  Interpreter::Value runFunction(const std::string& fname, const std::vector<Interpreter::Value>& args);
  bool setVariable(const std::string& vname, const Interpreter::Value& value);
  bool setMacro(const std::string& mname, const std::string& script);
  bool gotoOnLabel(const std::string& lname);
  void exitFromScript();
  std::vector<Interpreter::Entity> allEntities() const;
  Interpreter::Entity currentEntity() const;
  Interpreter::Entity getEntityByIndex(size_t beginIndex) const;
  std::vector<std::string> getAttributeByIndex(size_t beginIndex) const;
  bool gotoOnEntity(size_t iBegin);
  Interpreter::UserFunction getUserFunction(const std::string& fname);
  Interpreter::UserOperator getUserOperator(const std::string& oname);
private:
  enum class Keyword {
    INSTRUCTION,
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
    MACRO,
    VARIABLE,
    VALUE,
    GOTO,
  };
  struct Expression {
    Keyword keyw;
    size_t iConditionEnd;
    size_t iBodyEnd;
    size_t iOperator;
    std::vector<Interpreter::Value> params;
    Interpreter::Value result{std::string{}};
  };
  struct Operator {
    size_t inx, priority, iLOpr, iROpr;
  };
  std::map<std::string, Interpreter::UserFunction> m_ufunc;
  std::map<std::string, std::pair<Interpreter::UserOperator, uint32_t>> m_uoper;
  std::map<std::string, Interpreter::Value> m_var;
  std::map<std::string, std::string> m_macro;
  std::map<std::string, size_t> m_label;
  std::set<std::string> m_attribute;
  std::map<size_t, std::vector<std::string>> m_exprAttribute;
  std::map<size_t, std::vector<Operator>> m_soper;
  std::map<std::string, Impl> m_internFunc;
  std::vector<Expression> m_expr;
  Interpreter::Error m_parseErr;
  std::string m_prevScript;
  size_t m_gotoIndex = ir::detail::kNoIndex;
  size_t m_currentIndex = 0;
  bool m_exit = false;

  Interpreter::Value calcOperation(Keyword mainKeyword, size_t iExpr);
  Interpreter::Value calcFunction(size_t iExpr);
  Interpreter::Value calcCondition(size_t iExpr);
  Interpreter::Value calcExpression(size_t iBegin, size_t iEnd);
  void calcOperatorPriority(size_t iBegin, size_t iEnd, std::vector<Operator>& oprs);

  bool parseInstructionScript(std::string& script, size_t gpos);
  bool parseExpressionScript(std::string& script, size_t gpos);
  bool parseArgumentScript(std::string& script, size_t gpos);
  bool parseExprPrimary(std::string& script, size_t& cpos, size_t& iExpr, size_t gpos, bool& breakLoop);
  bool parseExprUnary(std::string& script, size_t& cpos, size_t& iExpr, size_t gpos);
  bool parseExprOperator(std::string& script, size_t& cpos, size_t& iExpr, size_t gpos);
  bool skipOneSpareSymbol(const std::string& script, size_t& cpos);
  bool parseControlBody(std::string& script, size_t& cpos, size_t gpos,
    const char* emptyBodyMsg, const char* invalidBodyMsg);
  bool parseInstrExprOrOpCall(std::string& script, size_t& cpos, size_t gpos, size_t& iExpr);
  bool parseInstrControlFlow(std::string& script, size_t& cpos, size_t gpos, size_t& iExpr, size_t& iIF);
  bool parseInstrElse(std::string& script, size_t& cpos, size_t gpos, size_t& iExpr, size_t iIF);
  bool parseInstrMacro(std::string& script, size_t& cpos, size_t gpos);
  bool expandAllMacros(std::string& script);
  bool parseInstrGotoLabel(std::string& script, size_t& cpos, size_t gpos, size_t& iExpr);
  bool parseInstrFunctionDecl(std::string& script, size_t& cpos, size_t gpos);
  bool parseInstrStatementExpr(std::string& script, size_t& cpos, size_t gpos, size_t& iExpr);
  void emplaceSyntheticNegatePrefix(size_t& iExpr);
  void mergeInternFuncs(Impl& callee) const;
  void bindCallerScope(Impl& callee, const std::vector<Interpreter::Value>& args,
    std::set<std::string>& outScopeVars) const;
  void writeBackScope(Impl& callee, const std::set<std::string>& scopeVars);
  bool parseMacroArgs(const std::string& args, std::string& macro);

  void emplaceExpr(size_t& iExpr, Keyword keyw,
    std::vector<Interpreter::Value> params = {},
    Interpreter::Value result = std::string{});
  void emplaceExprAt(size_t iExpr, Keyword keyw,
    std::vector<Interpreter::Value> params = {},
    Interpreter::Value result = std::string{});
  enum class ParseInitOutcome { NotHandled, Handled, BreakLoop };
  ParseInitOutcome parseNamedInitBody(Keyword entityKw, bool bindVariable,
    std::string& script, size_t& cpos, size_t& iExpr,
    size_t posmem, const std::string& oprName);
  Interpreter::Entity makeEntity(size_t index, const Expression& exp) const;
  Impl makeChildImplForParse() const;
  bool failParse(size_t cpos, size_t gpos, const char* what);
  static bool failCheck(std::string& err, const char* what);

  void cleaningScript(std::string& script) const;
  bool checkScript(const std::string& script, std::string& err) const;

  bool startWith(std::string_view str, size_t pos, std::string_view begin) const;
  bool isFindKeySymbol(const std::string& script, size_t cpos, size_t maxpos) const;
  Interpreter::Value evalOperand(size_t iExpr);
  Interpreter::Value variableByKey(const std::string& key) const;
  Keyword keywordByName(std::string_view oprName) const;
  Interpreter::EntityType keywordToEntityType(Keyword keyw) const;
  std::string getNextParam(const std::string& script, size_t& cpos, char symb) const;
  std::string getOperatorAtFirst(const std::string& script, size_t& cpos) const;
  std::string getFunctionAtFirst(const std::string& script, size_t& cpos) const;
  std::string peekFunctionCall(const std::string& script, size_t cpos) const;
  std::string getMacroAtFirst(const std::string& script, size_t& cpos) const;
  std::string getAttributeAtFirst(const std::string& script, size_t& cpos) const;
  std::string getNextOperator(const std::string& script, size_t& cpos) const;
  std::string getIntroScript(const std::string& script, size_t& cpos, char symbBegin, char symbEnd) const;
  bool isUnaryMinusAt(const std::string& script, size_t cpos) const;
  bool isUnaryMinusContext(const std::string& script, size_t cpos) const;
};

void Interpreter::Impl::emplaceExpr(size_t& iExpr, Keyword keyw,
    std::vector<Interpreter::Value> params, Interpreter::Value result) {
  m_expr.emplace_back(Expression{keyw, iExpr, iExpr, ir::detail::kNoIndex,
    std::move(params), std::move(result)});
  ++iExpr;
}

void Interpreter::Impl::emplaceExprAt(size_t iExpr, Keyword keyw,
    std::vector<Interpreter::Value> params, Interpreter::Value result) {
  m_expr.emplace_back(Expression{keyw, iExpr, iExpr, ir::detail::kNoIndex,
    std::move(params), std::move(result)});
}

// --- eval ---

Interpreter::CmdResult Interpreter::Impl::cmd(std::string script) {
  Interpreter::Error err;
  if (!parseScript(std::move(script), err)) {
    return {Interpreter::Value{std::string{}}, std::move(err)};
  }
  auto run = runScript();
  return {std::move(run.first), std::move(run.second)};
}

bool Interpreter::Impl::parseScript(std::string script, Interpreter::Error& outErr) {
  outErr = {};
  m_parseErr = {};

  cleaningScript(script);

  if (script.empty()) {
    outErr.kind = Interpreter::Error::Kind::Parse;
    outErr.message = "Error: empty script";
    return false;
  }

  if (script.back() != ';') {
    script += ';';
  }

  if (!expandAllMacros(script)) {
    m_prevScript.clear();
    if (!m_parseErr.message.empty()) {
      outErr = m_parseErr;
    }
    else {
      outErr.kind = Interpreter::Error::Kind::Parse;
      outErr.message = "Error: macro expansion failed";
    }
    return false;
  }

  if (m_prevScript != script) {
    m_prevScript = script;
    m_expr.clear();
    m_label.clear();
    m_soper.clear();
    m_parseErr = {};
    std::string checkErr;
    if (!checkScript(script, checkErr) || !parseInstructionScript(script, 0)) {
      m_prevScript.clear();
      if (!m_parseErr.message.empty()) {
        outErr = m_parseErr;
      }
      else {
        outErr.kind = Interpreter::Error::Kind::Parse;
        outErr.message = checkErr;
      }
      return false;
    }
  }
  return true;
}

std::pair<Interpreter::Value, Interpreter::Error> Interpreter::Impl::runScript() {
  return {evalScript(), {}};
}

Interpreter::Value Interpreter::Impl::evalScript() {

  for (auto& ex : m_expr)
    ex.iOperator = ir::detail::kNoIndex;

  Interpreter::Value result;
  m_exit = false;
  for (size_t i = 0; i < m_expr.size();) {

    result = calcOperation(m_expr[i].keyw, i);
    i = std::max(m_expr[i].iConditionEnd, m_expr[i].iBodyEnd);

    if (m_gotoIndex != ir::detail::kNoIndex) {
      for (size_t j = m_gotoIndex; j < i; ++j)
        m_expr[j].iOperator = ir::detail::kNoIndex;
      i = m_gotoIndex;
      m_gotoIndex = ir::detail::kNoIndex;
    }
    if (m_exit) break;
  }
  return result;
}

void Interpreter::Impl::cleaningScript(std::string& script) const {

  std::string out;
  out.reserve(script.size());
  bool inString = false;
  for (size_t i = 0; i < script.size();) {
    if (!inString && i + 1 < script.size() && script[i] == '/' && script[i + 1] == '/') {
      i += 2;
      while (i < script.size() && script[i] != '\n') {
        ++i;
      }
      continue;
    }
    if (script[i] == '"') {
      if (inString && i > 0 && script[i - 1] == '\\') {
        out += script[i++];
        continue;
      }
      inString = !inString;
      out += script[i++];
      continue;
    }
    if (inString) {
      out += script[i++];
      continue;
    }
    const char c = script[i];
    if (c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r') {
      ++i;
      continue;
    }
    out += c;
    ++i;
  }
  script = std::move(out);
}

bool Interpreter::Impl::failCheck(std::string& err, const char* what) { // NOLINT(readability-make-member-function-const)
  if (err.empty()) {
    err = std::string("Error check script: ") + what;
  }
  return false;
}

bool Interpreter::Impl::checkScript(const std::string& script, std::string& err) const {
  if (std::count(script.begin(), script.end(), '{') != std::count(script.begin(), script.end(), '}')) {
    return failCheck(err, "{ } mismatch");
  }
  if (std::count(script.begin(), script.end(), '(') != std::count(script.begin(), script.end(), ')')) {
    return failCheck(err, "( ) mismatch");
  }
  if (std::count(script.begin(), script.end(), '"') % 2 != 0) {
    return failCheck(err, "unclosed string");
  }
  return true;
}

bool Interpreter::Impl::addFunction(const std::string& name, Interpreter::UserFunction ufunc) {
  if (name.empty() || (keywordByName(name) != Keyword::INSTRUCTION) || isFindKeySymbol(name, 0, name.size())) return false;
  m_ufunc[name] = std::move(ufunc);
  return true;
}
bool Interpreter::Impl::addOperator(const std::string& name, Interpreter::UserOperator uopr, uint32_t priority) {
  if (name.empty() || (keywordByName(name) != Keyword::INSTRUCTION) || isFindKeySymbol(name, 0, name.size())) return false;
  m_uoper[name] = {std::move(uopr), priority};
  return true;
}
bool Interpreter::Impl::addAttribute(const std::string& name) {
  m_attribute.insert(name);
  return true;
}

std::map<std::string, Interpreter::Value> Interpreter::Impl::allVariables() const {
  return m_var;
}
Interpreter::Value Interpreter::Impl::variable(const std::string& vname) const {
  auto it = m_var.find(vname);
  return it != m_var.end() ? it->second : Interpreter::Value{std::string{}};
}
bool Interpreter::Impl::setVariable(const std::string& vname, const Interpreter::Value& value) {
  m_var[vname] = value;
  return true;
}
Interpreter::Value Interpreter::Impl::runFunction(const std::string& fname, const std::vector<Interpreter::Value>& args) {
  return m_ufunc.count(fname) ? m_ufunc[fname](args) : Interpreter::Value{std::string{}};
}
bool Interpreter::Impl::setMacro(const std::string& mname, const std::string& script) {
  if (mname.empty()) {
    return false;
  }
  const std::string key = (mname.front() == '#') ? mname : "#" + mname;
  m_macro[key] = script;
  return true;
}
bool Interpreter::Impl::gotoOnLabel(const std::string& lname) {
  const auto it = m_label.find(lname);
  if (it == m_label.end()) {
    return false;
  }
  m_gotoIndex = it->second;
  return true;
}
void Interpreter::Impl::exitFromScript() {
  m_exit = true;
}

Interpreter::Entity Interpreter::Impl::makeEntity(size_t index, const Expression& exp) const {
  Interpreter::Entity entity{
    index, exp.iConditionEnd, exp.iBodyEnd,
    keywordToEntityType(exp.keyw), ir::detail::paramKey(exp.params), exp.result
  };
  if (exp.keyw == Keyword::ELSE || exp.keyw == Keyword::ELSE_IF) {
    entity.linkIndex = ir::detail::paramIndex(exp.params);
  }
  return entity;
}

std::vector<Interpreter::Entity> Interpreter::Impl::allEntities() const {
  std::vector<Interpreter::Entity> res;
  res.reserve(m_expr.size());
  for (size_t i = 0; i < m_expr.size(); ++i) {
    res.push_back(makeEntity(i, m_expr[i]));
  }
  return res;
}
Interpreter::Entity Interpreter::Impl::currentEntity() const {
  if (m_currentIndex >= m_expr.size()) {
    return Interpreter::Entity{0};
  }
  return makeEntity(m_currentIndex, m_expr[m_currentIndex]);
}
Interpreter::Entity Interpreter::Impl::getEntityByIndex(size_t beginIndex) const {
  if (beginIndex >= m_expr.size()) {
    return Interpreter::Entity{0};
  }
  return makeEntity(beginIndex, m_expr[beginIndex]);
}
std::vector<std::string> Interpreter::Impl::getAttributeByIndex(size_t index) const {
  const auto it = m_exprAttribute.find(index);
  return it != m_exprAttribute.end() ? it->second : std::vector<std::string>{};
}
bool Interpreter::Impl::gotoOnEntity(size_t beginIndex) {
  if (beginIndex < m_expr.size()) {
    m_gotoIndex = beginIndex;
    return true;
  }
  return false;
}
Interpreter::UserFunction Interpreter::Impl::getUserFunction(const std::string& fname) {
  return m_ufunc.count(fname) ? m_ufunc[fname] : nullptr;
}
Interpreter::UserOperator Interpreter::Impl::getUserOperator(const std::string& oname) {
  return m_uoper.count(oname) ? m_uoper[oname].first : nullptr;
}

Interpreter::Value Interpreter::Impl::calcOperation(Keyword mainKeyword, size_t iExpr) {

  Interpreter::Value g_result;
  switch (mainKeyword) {
  case Keyword::VARIABLE:
    g_result = variableByKey(ir::detail::paramKey(m_expr[iExpr].params));
    break;
  case Keyword::VALUE:
    g_result = ir::detail::valueOfValueExpr(m_expr[iExpr].result, m_expr[iExpr].params);
    break;
  case Keyword::EXPRESSION:
    g_result = m_expr[iExpr].result = calcExpression(iExpr + 1, m_expr[iExpr].iBodyEnd);
    break;
  case Keyword::FUNCTION:
    g_result = m_expr[iExpr].result = calcFunction(iExpr);
    break;
  case Keyword::WHILE:
  case Keyword::IF:
  case Keyword::ELSE:
  case Keyword::ELSE_IF:
    g_result = calcCondition(iExpr);
    break;
  case Keyword::GOTO: {
    const std::string& gotoLabel = ir::detail::paramKey(m_expr[iExpr].params);
    auto it = m_label.find(gotoLabel);
    if (it != m_label.end()) {
      m_gotoIndex = it->second;
    }
  }
    break;
  default:
    break;
  }
  return g_result;
}
Interpreter::Value Interpreter::Impl::calcFunction(size_t iExpr) {

  Interpreter::Value g_result;
  size_t iBegin = iExpr + 1;
  size_t iEnd = m_expr[iExpr].iConditionEnd;
  std::vector<Interpreter::Value> args;
  for (size_t i = iBegin; i < iEnd;) {
    if ((i + 1 == m_expr[i].iBodyEnd - 1) && ((m_expr[i + 1].keyw == Keyword::VARIABLE) || (m_expr[i + 1].keyw == Keyword::VALUE))) {
      if (m_expr[i + 1].keyw == Keyword::VARIABLE)
        m_expr[i].result = variableByKey(ir::detail::paramKey(m_expr[i + 1].params));
      else
        m_expr[i].result = ir::detail::valueOfValueExpr(m_expr[i + 1].result, m_expr[i + 1].params);
    }
    else {
      m_expr[i].result = calcExpression(i + 1, m_expr[i].iBodyEnd);
    }
    args.emplace_back(m_expr[i].result);
    i = m_expr[i].iBodyEnd;
  }
  m_currentIndex = iExpr;
  const std::string& fname = ir::detail::paramKey(m_expr[iExpr].params);
  if (m_internFunc.count(fname)) {
    auto& impl = m_internFunc[fname];
    mergeInternFuncs(impl);
    std::set<std::string> scopeVars;
    bindCallerScope(impl, args, scopeVars);
    g_result = impl.evalScript();
    writeBackScope(impl, scopeVars);
  }
  else {
    g_result = m_ufunc[fname](args);
  }
  return g_result;
}
Interpreter::Value Interpreter::Impl::calcCondition(size_t iExpr) {

  Interpreter::Value g_result;
  size_t iBegin = iExpr + 1;
  size_t iCondEnd = m_expr[iExpr].iConditionEnd;
  size_t iBodyEnd = m_expr[iExpr].iBodyEnd;
  if ((m_expr[iExpr].keyw == Keyword::ELSE) || (m_expr[iExpr].keyw == Keyword::ELSE_IF)) {
    const size_t iIF = ir::detail::paramIndex(m_expr[iExpr].params);
    if (iIF == ir::detail::kNoIndex) {
      return g_result;
    }
    if (Interpreter::valueIsTruthy(m_expr[iIF].result)) {
      return g_result;
    }
  }
  Interpreter::Value condn;
  if (iBegin < iCondEnd) {
    condn = m_expr[iExpr].result = calcExpression(iBegin, iCondEnd);
  }
  if ((m_expr[iExpr].keyw == Keyword::ELSE) || Interpreter::valueIsTruthy(condn)) {
    bool isContinue = false,
      isBreak = false;
    for (size_t i = iCondEnd; i < iBodyEnd;) {
      switch (m_expr[i].keyw) {
      case Keyword::EXPRESSION: {
        m_expr[i].result = calcExpression(i + 1, m_expr[i].iBodyEnd);
        i = m_expr[i].iBodyEnd;
      }
        break;
      case Keyword::WHILE:
      case Keyword::IF:
      case Keyword::ELSE:
      case Keyword::ELSE_IF: {
        Interpreter::Value res = calcCondition(i);
        if (m_expr[i].keyw != Keyword::WHILE) {
          isBreak = ir::detail::isBreakValue(res);
          isContinue = ir::detail::isContinueValue(res);
          g_result = res;
        }
        i = m_expr[i].iBodyEnd;
      }
        break;
      case Keyword::BREAK: {
        isBreak = true;
        if (m_expr[iExpr].keyw != Keyword::WHILE) {
          g_result = Interpreter::ControlFlow::Break;
        }
      }
        break;
      case Keyword::CONTINUE: {
        isContinue = true;
        if (m_expr[iExpr].keyw != Keyword::WHILE) {
          g_result = Interpreter::ControlFlow::Continue;
        }
      }
        break;
      case Keyword::GOTO: {
        const std::string& gotoLabel = ir::detail::paramKey(m_expr[i].params);
        auto it = m_label.find(gotoLabel);
        if (it != m_label.end()) {
          m_gotoIndex = it->second;
        }
      }
        break;
      default:
        break;
      }
      if (isBreak || m_exit) break;

      if (m_gotoIndex != ir::detail::kNoIndex) {
        if ((iCondEnd <= m_gotoIndex) && (m_gotoIndex < iBodyEnd)) {
          for (size_t j = m_gotoIndex; j < i; ++j)
            m_expr[j].iOperator = ir::detail::kNoIndex;
          i = m_gotoIndex;
          m_gotoIndex = ir::detail::kNoIndex;
        }
        else break;
      }

      if (isContinue) i = iBodyEnd;

      if ((m_expr[iExpr].keyw == Keyword::WHILE) && (i >= iBodyEnd)) {
        isContinue = false;

        for (size_t j = iBegin; j < iCondEnd; ++j)
          m_expr[j].iOperator = ir::detail::kNoIndex;

        condn = m_expr[iExpr].result = calcExpression(iBegin, iCondEnd);
        if (Interpreter::valueIsTruthy(condn)) {
          for (size_t j = iCondEnd; j < iBodyEnd; ++j)
            m_expr[j].iOperator = ir::detail::kNoIndex;
          i = iCondEnd;
        }
      }
    }
  }
  return g_result;
}
Interpreter::Value Interpreter::Impl::calcExpression(size_t iBegin, size_t iEnd) {

  if (iBegin + 1 == iEnd) {
    return evalOperand(iBegin);
  }

  if (m_soper.find(iBegin) == m_soper.end()) {
    m_soper.insert({ iBegin, std::vector<Operator>() });
  }
  std::vector<Operator>& oprs = m_soper[iBegin];
  oprs.clear();
  calcOperatorPriority(iBegin, iEnd, oprs);

  if (oprs.empty()) {
    return calcOperation(m_expr[iBegin].keyw, iBegin);
  }

  Interpreter::Value g_result;
  for (auto& op : oprs) {
    size_t iOp = op.inx;
    Expression* pLeftOperd = nullptr,
      * pRightOperd = nullptr;
    Interpreter::Value lValue, rValue;
    if (op.iLOpr != ir::detail::kNoIndex) {
      pLeftOperd = &m_expr[op.iLOpr];
      lValue = evalOperand(op.iLOpr);
    }
    if (op.iROpr != ir::detail::kNoIndex) {
      pRightOperd = &m_expr[op.iROpr];
      rValue = evalOperand(op.iROpr);
    }
    m_currentIndex = iOp;
    g_result = m_expr[iOp].result = m_uoper[ir::detail::paramKey(m_expr[iOp].params)].first(lValue, rValue);

    if (pLeftOperd && (pLeftOperd->keyw == Keyword::VARIABLE) && (pLeftOperd->iOperator == ir::detail::kNoIndex)) {
      pLeftOperd->result = m_var[ir::detail::paramKey(pLeftOperd->params)] = lValue;
    }
    if (pRightOperd && (pRightOperd->keyw == Keyword::VARIABLE) && (pRightOperd->iOperator == ir::detail::kNoIndex)) {
      pRightOperd->result = m_var[ir::detail::paramKey(pRightOperd->params)] = rValue;
    }
    if (pLeftOperd) {
      if (pLeftOperd->iOperator != ir::detail::kNoIndex) {
        size_t iLOp = pLeftOperd->iOperator;
        for (size_t i = iBegin; i < iEnd; ++i) {
          if (m_expr[i].iOperator == iLOp)
            m_expr[i].iOperator = iOp;
        }
      }
      else pLeftOperd->iOperator = iOp;
    }
    if (pRightOperd) {
      if (pRightOperd->iOperator != ir::detail::kNoIndex) {
        size_t iROp = pRightOperd->iOperator;
        for (size_t i = iBegin; i < iEnd; ++i) {
          if (m_expr[i].iOperator == iROp)
            m_expr[i].iOperator = iOp;
        }
      }
      else pRightOperd->iOperator = iOp;
    }
  }
  return g_result;
}
void Interpreter::Impl::calcOperatorPriority(size_t iBegin, size_t iEnd, std::vector<Operator>& oprs) {

  size_t iLOpr = ir::detail::kNoIndex;
  for (size_t i = iBegin; i < iEnd;) {
    if (m_expr[i].keyw == Keyword::FUNCTION) {
      iLOpr = i;
      i = m_expr[i].iConditionEnd;
      continue;
    }
    if (m_expr[i].keyw == Keyword::EXPRESSION) {
      iLOpr = i;
      i = m_expr[i].iBodyEnd;
      continue;
    }
    if (m_expr[i].keyw == Keyword::OPERATOR) {
      uint32_t priority = m_uoper[ir::detail::paramKey(m_expr[i].params)].second;
      size_t iROpr = (i < iEnd - 1) ? i + 1 : ir::detail::kNoIndex;
      oprs.emplace_back<Operator>({ i, priority, iLOpr, iROpr });  // inx, priority
    }
    iLOpr = i;
    ++i;
  }
  const auto osz = oprs.size();
  if (osz > 1) {
    if (osz == 2) {
      if (oprs[0].priority > oprs[1].priority) std::swap(oprs[0], oprs[1]);
    }
    else if (osz == 3) {
      if (oprs[0].priority < oprs[1].priority) {
        if (oprs[2].priority < oprs[0].priority) std::swap(oprs[0], oprs[2]);
      }
      else {
        if (oprs[1].priority < oprs[2].priority) std::swap(oprs[0], oprs[1]);
        else std::swap(oprs[0], oprs[2]);
      }
      if (oprs[2].priority < oprs[1].priority) std::swap(oprs[1], oprs[2]);
    }
    else{
      std::sort(oprs.begin(), oprs.end(), [](const Operator& l, const Operator& r) {
        return l.priority < r.priority;
      });
    }
  }
}

// --- parse ---

bool Interpreter::Impl::failParse(size_t cpos, size_t gpos, const char* what) {
  if (m_parseErr.message.empty()) {
    m_parseErr.kind = Interpreter::Error::Kind::Parse;
    m_parseErr.position = cpos + gpos;
    m_parseErr.message = "Error script pos " + std::to_string(m_parseErr.position) + ": " + what;
  }
  return false;
}

bool Interpreter::Impl::skipOneSpareSymbol(const std::string& script, size_t& cpos) {
  if (cpos >= script.size()) {
    return false;
  }
  const char c = script[cpos];
  if (c == ';' || c == ',' || c == ']' || c == '[') {
    ++cpos;
    return true;
  }
  return false;
}

bool Interpreter::Impl::parseControlBody(std::string& script, size_t& cpos, size_t gpos,
    const char* emptyBodyMsg, const char* invalidBodyMsg) {
  if (script[cpos] == '{') {
    std::string body = getIntroScript(script, cpos, '{', '}');
    if (body.empty() || !parseInstructionScript(body, gpos + cpos - body.size() - 2)) {
      return failParse(cpos, gpos, emptyBodyMsg);
    }
  }
  else {
    std::string body = getNextParam(script, cpos, ';') + ';';
    if ((body.size() != 1) && !parseInstructionScript(body, gpos + cpos - body.size())) {
      return failParse(cpos, gpos, invalidBodyMsg);
    }
  }
  return true;
}

bool Interpreter::Impl::parseInstrExprOrOpCall(std::string& script, size_t& cpos, size_t gpos,
    size_t& iExpr) {
  size_t cposFunc = cpos;
  size_t cposOpr = cpos;
  if (getFunctionAtFirst(script, cposFunc).empty() && getOperatorAtFirst(script, cposOpr).empty()) {
    return false;
  }

  emplaceExprAt(iExpr, Keyword::EXPRESSION);

  std::string expr = getNextParam(script, cpos, ';');
  if (expr.empty() || !parseExpressionScript(expr, gpos + cpos - expr.size() - 1)) {
    return failParse(cpos, gpos, "empty expression");
  }

  iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
  return true;
}

bool Interpreter::Impl::parseInstrControlFlow(std::string& script, size_t& cpos, size_t gpos,
    size_t& iExpr, size_t& iIF) {
  if (!startWith(script, cpos, "while") && !startWith(script, cpos, "if")
      && !startWith(script, cpos, "elseif")) {
    if (!startWith(script, cpos, "break") && !startWith(script, cpos, "continue")) {
      return false;
    }

    const std::string kname = getNextParam(script, cpos, ';');
    if (kname.empty()) {
      return failParse(cpos, gpos, "expected break or continue");
    }
    emplaceExpr(iExpr, keywordByName(kname));
    return true;
  }

  const std::string kname = getNextParam(script, cpos, '(');
  if (kname.empty()) {
    return failParse(cpos, gpos, "expected keyword");
  }

  const Keyword keyw = keywordByName(kname);
  emplaceExprAt(iExpr, keyw);

  if (keyw == Keyword::IF) {
    iIF = iExpr;
  }
  else if (keyw == Keyword::ELSE_IF) {
    m_expr[iExpr].params = { static_cast<int64_t>(iIF) };
    iIF = iExpr;
  }

  --cpos;
  std::string condition = getIntroScript(script, cpos, '(', ')');
  if (condition.empty() || !parseExpressionScript(condition, gpos + cpos - condition.size() - 2)) {
    return failParse(cpos, gpos, "empty condition");
  }

  m_expr[iExpr].iConditionEnd = m_expr.size();

  if (!parseControlBody(script, cpos, gpos, "empty body", "invalid instruction body")) {
    return true;
  }

  iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

  if ((cpos < script.size()) && (script[cpos] == ';')) {
    ++cpos;
  }
  return true;
}

bool Interpreter::Impl::parseInstrElse(std::string& script, size_t& cpos, size_t gpos,
    size_t& iExpr, size_t iIF) {
  if (!startWith(script, cpos, "else")) {
    return false;
  }

  cpos += 4;

  emplaceExprAt(iExpr, Keyword::ELSE);
  m_expr[iExpr].params = { static_cast<int64_t>(iIF) };

  if (!parseControlBody(script, cpos, gpos, "empty else body", "invalid else body")) {
    return true;
  }

  m_expr[iExpr].iConditionEnd = iExpr + 1;
  iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

  if ((cpos < script.size()) && (script[cpos] == ';')) {
    ++cpos;
  }
  return true;
}

bool Interpreter::Impl::expandAllMacros(std::string& script) {
  size_t cpos = 0;
  while (cpos < script.size()) {
    if (skipOneSpareSymbol(script, cpos)) {
      continue;
    }

    if (!startWith(script, cpos, "#macro")) {
      ++cpos;
      continue;
    }

    const size_t declStart = cpos;
    cpos += 6;
    const std::string mname = getNextParam(script, cpos, '{');

    --cpos;
    const std::string mvalue = getIntroScript(script, cpos, '{', '}');
    if (mname.empty() || mvalue.empty()) {
      return failParse(cpos, 0, "empty macro declaration");
    }

    m_macro["#" + mname] = mvalue;

    size_t declEnd = cpos;
    if ((declEnd < script.size()) && (script[declEnd] == ';')) {
      ++declEnd;
    }
    script.erase(declStart, declEnd - declStart);
    cpos = declStart;
  }

  cpos = 0;
  while (cpos < script.size()) {
    if (skipOneSpareSymbol(script, cpos)) {
      continue;
    }

    if (script[cpos] != '#') {
      ++cpos;
      continue;
    }

    size_t cposMName = cpos;
    const std::string mname = getMacroAtFirst(script, cposMName);
    if (mname.empty() || (m_macro.find(mname) == m_macro.end())) {
      return failParse(cpos, 0, "unknown macro");
    }

    size_t cposArg = cposMName;
    const std::string args = getIntroScript(script, cposArg, '(', ')');
    std::string macro = m_macro[mname];
    if (!args.empty() && !parseMacroArgs(args, macro)) {
      return failParse(cpos, 0, "invalid macro arguments");
    }
    cleaningScript(macro);

    if (cposArg == cposMName) {
      script.replace(cpos, mname.size(), macro);
    }
    else {
      script.replace(cpos, (mname + "(" + args + ")").size(), macro);
    }
  }
  return true;
}

bool Interpreter::Impl::parseInstrMacro(std::string& script, size_t& cpos, size_t gpos) {
  if (startWith(script, cpos, "#macro")) {
    cpos += 6;
    const std::string mname = getNextParam(script, cpos, '{');

    --cpos;
    const std::string mvalue = getIntroScript(script, cpos, '{', '}');
    if (mname.empty() || mvalue.empty()) {
      return failParse(cpos, gpos, "empty macro declaration");
    }

    m_macro["#" + mname] = mvalue;

    if ((cpos < script.size()) && (script[cpos] == ';')) {
      ++cpos;
    }
    return true;
  }

  if (cpos >= script.size() || script[cpos] != '#') {
    return false;
  }

  size_t cposMName = cpos;
  const std::string mname = getMacroAtFirst(script, cposMName);
  if (mname.empty() || (m_macro.find(mname) == m_macro.end())) {
    return failParse(cpos, gpos, "unknown macro");
  }

  size_t cposArg = cposMName;
  const std::string args = getIntroScript(script, cposArg, '(', ')');
  std::string macro = m_macro[mname];
  if (!args.empty() && !parseMacroArgs(args, macro)) {
    return failParse(cpos, gpos, "invalid macro arguments");
  }
  cleaningScript(macro);

  if (cposArg == cposMName) {
    script.replace(cpos, mname.size(), macro);
  }
  else {
    script.replace(cpos, (mname + "(" + args + ")").size(), macro);
  }
  return true;
}

bool Interpreter::Impl::parseInstrGotoLabel(std::string& script, size_t& cpos, size_t gpos,
    size_t& iExpr) {
  if (startWith(script, cpos, "goto")) {
    cpos += 4;
    const std::string lname = getNextParam(script, cpos, ';');
    if (lname.empty()) {
      return failParse(cpos, gpos, "empty goto label");
    }

    if (m_label.find(lname) == m_label.end()) {
      m_label.insert({ lname, ir::detail::kNoIndex });
    }

    emplaceExpr(iExpr, Keyword::GOTO, Interpreter::makeParam(lname));
    return true;
  }

  if (!startWith(script, cpos, "l_")) {
    return false;
  }

  const std::string lname = getNextParam(script, cpos, ':');
  if (lname.empty()) {
    return failParse(cpos, gpos, "empty label name");
  }

  m_label[lname] = iExpr;
  return true;
}

Interpreter::Impl Interpreter::Impl::makeChildImplForParse() const {
  Impl child;
  child.m_ufunc = m_ufunc;
  child.m_uoper = m_uoper;
  child.m_macro = m_macro;
  child.m_attribute = m_attribute;
  child.m_internFunc = m_internFunc;
  return child;
}

bool Interpreter::Impl::parseInstrFunctionDecl(std::string& script, size_t& cpos, size_t gpos) {
  if (!startWith(script, cpos, "function")) {
    return false;
  }

  cpos += 8;
  const std::string fname = getNextParam(script, cpos, '{');
  if (fname.empty()) {
    return failParse(cpos, gpos, "empty function name");
  }

  --cpos;
  const std::string fbody = getIntroScript(script, cpos, '{', '}');
  if (fbody.empty()) {
    return failParse(cpos, gpos, "empty function body");
  }

  Impl fImpl = makeChildImplForParse();
  fImpl.m_internFunc[fname] = {};

  if (!fImpl.parseScript(fbody, m_parseErr)) {
    return failParse(cpos, gpos, "invalid function body");
  }

  m_internFunc[fname] = std::move(fImpl);
  return true;
}

bool Interpreter::Impl::parseInstrStatementExpr(std::string& script, size_t& cpos, size_t gpos,
    size_t& iExpr) {
  emplaceExprAt(iExpr, Keyword::EXPRESSION);

  std::string expr = getNextParam(script, cpos, ';');
  if (expr.empty() || !parseExpressionScript(expr, gpos + cpos - expr.size() - 1)) {
    return failParse(cpos, gpos, "empty expression");
  }

  iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
  return true;
}

bool Interpreter::Impl::parseInstructionScript(std::string& script, size_t gpos) {
  size_t iExpr = m_expr.size();
  size_t cpos = 0;
  size_t iIF = ir::detail::kNoIndex;

  while (cpos < script.size()) {
    if (skipOneSpareSymbol(script, cpos)) {
      continue;
    }

    std::string attr = getAttributeAtFirst(script, cpos);
    if (!attr.empty()) {
      m_exprAttribute[iExpr].push_back(attr);
      if (skipOneSpareSymbol(script, cpos)) {
        continue;
      }
    }

    if (parseInstrExprOrOpCall(script, cpos, gpos, iExpr)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (parseInstrControlFlow(script, cpos, gpos, iExpr, iIF)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (parseInstrElse(script, cpos, gpos, iExpr, iIF)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (parseInstrMacro(script, cpos, gpos)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (parseInstrGotoLabel(script, cpos, gpos, iExpr)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (parseInstrFunctionDecl(script, cpos, gpos)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (parseInstrStatementExpr(script, cpos, gpos, iExpr)) {
      if (!m_parseErr.message.empty()) {
        return false;
      }
      continue;
    }
    if (!m_parseErr.message.empty()) {
      return false;
    }
  }
  m_prevScript = script;
  return true;
}

void Interpreter::Impl::emplaceSyntheticNegatePrefix(size_t& iExpr) {
  emplaceExpr(iExpr, Keyword::VALUE, {}, int64_t{0});
  emplaceExpr(iExpr, Keyword::OPERATOR, Interpreter::makeParam(std::string{"-"}));
}

void Interpreter::Impl::mergeInternFuncs(Impl& callee) const {
  for (const auto& f : m_internFunc) {
    if (!callee.m_internFunc.count(f.first) || callee.m_internFunc[f.first].m_prevScript.empty()) {
      callee.m_internFunc[f.first] = f.second;
    }
  }
}

void Interpreter::Impl::bindCallerScope(Impl& callee, const std::vector<Interpreter::Value>& args,
    std::set<std::string>& outScopeVars) const {
  for (const auto& var : m_var) {
    if (callee.m_var.count(var.first)) {
      callee.m_var[var.first] = var.second;
      outScopeVars.insert(var.first);
    }
  }
  for (size_t i = 0; i < args.size(); ++i) {
    callee.m_var["$" + std::to_string(i)] = args[i];
  }
}

void Interpreter::Impl::writeBackScope(Impl& callee, const std::set<std::string>& scopeVars) {
  for (const auto& var : callee.m_var) {
    if (scopeVars.count(var.first)) {
      m_var[var.first] = var.second;
    }
  }
}

Interpreter::Impl::ParseInitOutcome Interpreter::Impl::parseNamedInitBody(
    Keyword entityKw, bool bindVariable,
    std::string& script, size_t& cpos, size_t& iExpr,
    size_t posmem, const std::string& oprName) {
  const size_t bodyBeginF = script.find('{', posmem);
  const size_t bodyBeginQ = script.find('[', posmem);
  size_t bodyBegin = bodyBeginF;
  char bodyBeginSym = '{';
  char bodyEndSym = '}';
  if (bodyBeginQ < bodyBeginF) {
    bodyBegin = bodyBeginQ;
    bodyBeginSym = '[';
    bodyEndSym = ']';
  }
  if ((!oprName.empty() && (bodyBegin < cpos)) ||
      (oprName.empty() && (bodyBegin != std::string::npos))) {
    const std::string vName = script.substr(posmem, bodyBegin - posmem);
    std::string value = getIntroScript(script, bodyBegin, bodyBeginSym, bodyEndSym);
    if (!value.empty()) {
      if (value.front() == '"') {
        value.erase(value.begin());
      }
      if (!value.empty() && value.back() == '"') {
        value.pop_back();
      }
    }
    if (entityKw == Keyword::VARIABLE) {
      emplaceExpr(iExpr, Keyword::VARIABLE, Interpreter::makeParam(vName), Interpreter::valueFromLiteral(value));
    }
    else {
      emplaceExpr(iExpr, Keyword::VALUE, Interpreter::makeParam(vName), std::string{value});
    }
    if (oprName == "[" && bodyBeginSym == '[') {
      emplaceExpr(iExpr, Keyword::OPERATOR, Interpreter::makeParam(oprName));
      emplaceExpr(iExpr, Keyword::VALUE, Interpreter::makeParam(vName), std::string{value});
    }
    if (bindVariable) {
      m_var[vName] = Interpreter::valueFromLiteral(value);
    }
    cpos = bodyBegin;
    return ParseInitOutcome::Handled;
  }
  if (!oprName.empty()) {
    const std::string vName = script.substr(posmem, cpos - posmem - oprName.size());
    if (bindVariable && m_var.find(vName) == m_var.end()) {
      m_var.insert({vName, std::string{}});
    }
    emplaceExpr(iExpr, entityKw, Interpreter::makeParam(vName));
    emplaceExpr(iExpr, Keyword::OPERATOR, Interpreter::makeParam(oprName));
    return ParseInitOutcome::Handled;
  }
  std::string vName = script.substr(cpos);
  if (!vName.empty() && vName.back() == ';') {
    vName.pop_back();
  }
  if (bindVariable && m_var.find(vName) == m_var.end()) {
    m_var.insert({vName, std::string{}});
  }
  emplaceExprAt(iExpr, entityKw, Interpreter::makeParam(vName));
  return ParseInitOutcome::BreakLoop;
}

bool Interpreter::Impl::parseExprPrimary(std::string& script, size_t& cpos, size_t& iExpr,
    size_t gpos, bool& breakLoop) {
  breakLoop = false;
  std::string oprName, fName;

  if (script[cpos] == '$') {
    const size_t posmem = cpos;
    oprName = getNextOperator(script, cpos);
    const auto outcome = parseNamedInitBody(
      Keyword::VARIABLE, true, script, cpos, iExpr, posmem, oprName);
    if (outcome == ParseInitOutcome::BreakLoop) {
      breakLoop = true;
      return true;
    }
    if (outcome == ParseInitOutcome::Handled) {
      return true;
    }
    return false;
  }
  if (!(fName = peekFunctionCall(script, cpos)).empty()) {
    if (!m_ufunc.count(fName) && !m_internFunc.count(fName)) {
      failParse(cpos, gpos, "unknown function");
      return false;
    }
    cpos += fName.size();

    emplaceExprAt(iExpr, Keyword::FUNCTION, Interpreter::makeParam(fName));

    const size_t cposMem = cpos;
    std::string args = getIntroScript(script, cpos, '(', ')');
    if (args.empty() && (cposMem + 2 != cpos)) {
      failParse(cpos, gpos, "empty function arguments");
      return false;
    }
    if (!args.empty() && !parseArgumentScript(args, gpos + cpos - args.size() - 2)) {
      failParse(cpos, gpos, "invalid function arguments");
      return false;
    }

    iExpr = m_expr[iExpr].iConditionEnd = m_expr.size();

    if ((cpos < script.size()) && (script[cpos] == ';')) {
      ++cpos;
    }
    return true;
  }
  if (script[cpos] == '(') {
    std::string expr = getIntroScript(script, cpos, '(', ')');

    emplaceExprAt(iExpr, Keyword::EXPRESSION);

    if (expr.empty() || !parseExpressionScript(expr, gpos + cpos - expr.size() - 2)) {
      failParse(cpos, gpos, "empty parenthesized expression");
      return false;
    }

    iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();

    if ((cpos < script.size()) && (script[cpos] == ';')) {
      ++cpos;
    }
    return true;
  }
  if (script[cpos] == '#') {
    size_t cposMName = cpos;
    const std::string mname = getMacroAtFirst(script, cposMName);
    if (mname.empty() || (m_macro.find(mname) == m_macro.end())) {
      failParse(cpos, gpos, "unknown macro");
      return false;
    }

    size_t cposArg = cposMName;
    const std::string args = getIntroScript(script, cposArg, '(', ')');
    std::string macro = m_macro[mname];
    if (!args.empty() && !parseMacroArgs(args, macro)) {
      failParse(cpos, gpos, "invalid macro arguments");
      return false;
    }
    cleaningScript(macro);

    if (cposArg == cposMName) {
      script.replace(cpos, mname.size(), macro);
    }
    else {
      script.replace(cpos, (mname + "(" + args + ")").size(), macro);
    }
    return true;
  }
  if (script[cpos] == '"') {
    ++cpos;
    const std::string vName = getNextParam(script, cpos, '"');
    emplaceExpr(iExpr, Keyword::VALUE, {}, ir::detail::parseQuotedLiteral(vName));
    return true;
  }
  if (script[cpos] == '{') {
    const std::string value = getIntroScript(script, cpos, '{', '}');
    emplaceExpr(iExpr, Keyword::VALUE, Interpreter::makeParam(std::string_view{}), std::string{value});
    return true;
  }
  if (std::isdigit(static_cast<unsigned char>(script[cpos]))) {
    size_t end = cpos;
    while (end < script.size()) {
      const unsigned char c = static_cast<unsigned char>(script[end]);
      if (!std::isdigit(c) && script[end] != '.') {
        break;
      }
      ++end;
    }
    emplaceExpr(iExpr, Keyword::VALUE, {},
      ir::detail::parseLiteralText(script.substr(cpos, end - cpos)));
    cpos = end;
    return true;
  }
  if (ir::detail::matchKeywordAt(script, cpos, "true")) {
    emplaceExpr(iExpr, Keyword::VALUE, {}, bool{true});
    cpos += 4;
    return true;
  }
  if (ir::detail::matchKeywordAt(script, cpos, "false")) {
    emplaceExpr(iExpr, Keyword::VALUE, {}, bool{false});
    cpos += 5;
    return true;
  }
  if (isUnaryMinusAt(script, cpos)) {
    return false;
  }
  {
    const size_t cposMem = cpos;
    if (!getOperatorAtFirst(script, cpos).empty()) {
      cpos = cposMem;
      return false;
    }
  }
  const size_t posmem = cpos;
  oprName = getNextOperator(script, cpos);
  const auto outcome = parseNamedInitBody(
    Keyword::VALUE, false, script, cpos, iExpr, posmem, oprName);
  if (outcome == ParseInitOutcome::BreakLoop) {
    breakLoop = true;
    return true;
  }
  if (outcome == ParseInitOutcome::Handled) {
    return true;
  }
  return false;
}

bool Interpreter::Impl::parseExprUnary(std::string& script, size_t& cpos, size_t& iExpr, size_t gpos) {
  if (!isUnaryMinusAt(script, cpos)) {
    return false;
  }

  const size_t minusPos = cpos;
  ++cpos;

  if (cpos < script.size() && std::isdigit(static_cast<unsigned char>(script[cpos]))) {
    size_t end = cpos;
    while (end < script.size()) {
      const unsigned char c = static_cast<unsigned char>(script[end]);
      if (!std::isdigit(c) && script[end] != '.') {
        break;
      }
      ++end;
    }
    emplaceExpr(iExpr, Keyword::VALUE, {},
      ir::detail::parseLiteralText(script.substr(minusPos, end - minusPos)));
    cpos = end;
    return true;
  }

  if (cpos < script.size() && script[cpos] == '$') {
    emplaceSyntheticNegatePrefix(iExpr);
    bool breakLoop = false;
    if (!parseExprPrimary(script, cpos, iExpr, gpos, breakLoop)) {
      return failParse(cpos, gpos, "expected variable after unary minus");
    }
    return true;
  }

  if (cpos < script.size() && script[cpos] == '(') {
    emplaceSyntheticNegatePrefix(iExpr);
    std::string expr = getIntroScript(script, cpos, '(', ')');
    emplaceExprAt(iExpr, Keyword::EXPRESSION);
    if (expr.empty() || !parseExpressionScript(expr, gpos + cpos - expr.size() - 2)) {
      return failParse(cpos, gpos, "empty parenthesized expression");
    }
    iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
    return true;
  }

  return failParse(minusPos, gpos, "invalid unary minus");
}

bool Interpreter::Impl::parseExprOperator(std::string& script, size_t& cpos, size_t& iExpr, size_t gpos) {
  std::string oprName = getOperatorAtFirst(script, cpos);
  if (oprName.empty()) {
    return false;
  }
  if (m_uoper.find(oprName) == m_uoper.end()) {
    return failParse(cpos, gpos, "unknown operator");
  }
  emplaceExpr(iExpr, Keyword::OPERATOR, Interpreter::makeParam(oprName));
  return true;
}

bool Interpreter::Impl::parseExpressionScript(std::string& script, size_t gpos) {

  size_t iExpr = m_expr.size(),
         cpos = 0;

  while (cpos < script.size()) {
    if (skipOneSpareSymbol(script, cpos)) {
      continue;
    }
    std::string attr = getAttributeAtFirst(script, cpos);
    if (!attr.empty()) {
      m_exprAttribute[iExpr].push_back(attr);
      if (skipOneSpareSymbol(script, cpos)) {
        continue;
      }
    }
    bool breakLoop = false;
    if (parseExprPrimary(script, cpos, iExpr, gpos, breakLoop)) {
      if (breakLoop) {
        break;
      }
      continue;
    }
    if (!m_parseErr.message.empty()) {
      return false;
    }
    if (parseExprUnary(script, cpos, iExpr, gpos)) {
      continue;
    }
    if (!m_parseErr.message.empty()) {
      return false;
    }
    if (parseExprOperator(script, cpos, iExpr, gpos)) {
      continue;
    }
    if (!m_parseErr.message.empty()) {
      return false;
    }
  }
  return true;
}
bool Interpreter::Impl::parseArgumentScript(std::string& script, size_t gpos) {

  size_t iExpr = m_expr.size(),
         cpos = 0,
         cp = 0;
  int bordCnt = 0;

  while (cp < script.size()) {
    if (script[cp] == '(') ++bordCnt;
    if (script[cp] == ')') --bordCnt;
    if (((script[cp] == ',') || (cp == script.size() - 1)) && (bordCnt == 0)) {
      emplaceExprAt(iExpr, Keyword::ARGUMENT);

      if (cp == script.size() - 1) ++cp;

      std::string arg = script.substr(cpos, cp - cpos);
      if (!arg.empty() && !parseExpressionScript(arg, gpos + cpos)) {
        return failParse(cpos, gpos, "invalid argument expression");
      }

      iExpr = m_expr[iExpr].iBodyEnd = m_expr.size();
      cpos = cp + 1;
    }
    ++cp;
  }
  return true;
}
bool Interpreter::Impl::parseMacroArgs(const std::string& args, std::string& macro) {

  size_t ssz = args.size(),
         cpos = 0,
         cp = 0;
  int bordCnt = 0;
  int argIndex = 0;

  while (cp < ssz) {
    if (args[cp] == '(') ++bordCnt;
    if (args[cp] == ')') --bordCnt;
    if (((args[cp] == ',') || (cp == ssz - 1)) && (bordCnt == 0)) {
     
      if (cp == ssz - 1) ++cp;

      const std::string arg = args.substr(cpos, cp - cpos);
      
      size_t argPos = 0;
      argPos = macro.find("$" + std::to_string(argIndex), argPos);
      while (argPos != std::string::npos) {
      
        macro.replace(argPos, ("$" + std::to_string(argIndex)).size(), arg);

        argPos = macro.find("$" + std::to_string(argIndex), argPos);
      }

      cpos = cp + 1;
      ++argIndex;
    }
    ++cp;
  }
  return true;
}

std::string Interpreter::Impl::getNextParam(const std::string& script, size_t& cpos, char symb) const {
  size_t pos = script.find(symb, cpos);
  std::string res;
  if (pos != std::string::npos) {
    res = script.substr(cpos, pos - cpos);
    cpos = pos + 1;
  }
  return res;
}
std::string Interpreter::Impl::getNextOperator(const std::string& script, size_t& cpos) const {
  size_t minp = std::string::npos;
  std::string opr;
  for (const auto& op : m_uoper) {
    size_t pos = script.find(op.first, cpos);
    if ((pos != std::string::npos) && ((pos <= minp) || (minp == std::string::npos))) {
      if (opr.empty() || (pos < minp) || (opr.size() < op.first.size()))
        opr = op.first;
      minp = pos;
    }
  }
  if (minp != std::string::npos) {
    cpos = minp + opr.size();
  }
  return opr;
}
std::string Interpreter::Impl::getOperatorAtFirst(const std::string& script, size_t& cpos) const {
  std::string opr;
  for (const auto& op : m_uoper) {
    if (startWith(script, cpos, op.first)) {
      if (opr.empty() || (opr.size() < op.first.size()))
        opr = op.first;
    }
  }
  cpos += opr.size();
  return opr;
}
std::string Interpreter::Impl::peekFunctionCall(const std::string& script, size_t cpos) const {
  if (cpos >= script.size()) {
    return {};
  }
  size_t end = cpos;
  while (end < script.size()) {
    const unsigned char c = static_cast<unsigned char>(script[end]);
    if (!std::isalnum(c) && script[end] != '_') {
      break;
    }
    ++end;
  }
  if (end == cpos || end >= script.size() || script[end] != '(') {
    return {};
  }
  return script.substr(cpos, end - cpos);
}

std::string Interpreter::Impl::getFunctionAtFirst(const std::string& script, size_t& cpos) const {
  std::string fName;
  for (const auto& f : m_ufunc) {
    if (startWith(script, cpos, f.first)) {
      if (fName.empty() || (fName.size() < f.first.size()))
        fName = f.first;
    }
  }
  if (fName.empty()){
    for (const auto& f : m_internFunc) {
      if (startWith(script, cpos, f.first)) {
        if (fName.empty() || (fName.size() < f.first.size()))
          fName = f.first;
      }
    }
  }
  cpos += fName.size();
  return fName;
}
std::string Interpreter::Impl::getMacroAtFirst(const std::string& script, size_t& cpos) const {
  std::string mName;
  for (const auto& m : m_macro) {
    if (startWith(script, cpos, m.first)) {
      if (mName.empty() || (mName.size() < m.first.size()))
        mName = m.first;
    }
  }
  cpos += mName.size();
  return mName;
}
std::string Interpreter::Impl::getAttributeAtFirst(const std::string& script, size_t& cpos) const {
  std::string mName;
  for (const auto& m : m_attribute) {
    if (startWith(script, cpos, m)) {
      if (mName.empty() || (mName.size() < m.size()))
        mName = m;
    }
  }
  cpos += mName.size();
  return mName;
}
bool Interpreter::Impl::isUnaryMinusContext(const std::string& script, size_t cpos) const {
  if (cpos == 0) {
    return true;
  }
  const char prev = script[cpos - 1];
  if (prev == '(' || prev == ',' || prev == ';' || prev == '[' || prev == '{') {
    return true;
  }
  for (const auto& op : m_uoper) {
    if (cpos >= op.first.size() &&
        script.compare(cpos - op.first.size(), op.first.size(), op.first) == 0) {
      return true;
    }
  }
  return false;
}

bool Interpreter::Impl::isUnaryMinusAt(const std::string& script, size_t cpos) const {
  if (cpos >= script.size() || script[cpos] != '-' || cpos + 1 >= script.size()) {
    return false;
  }
  if (!isUnaryMinusContext(script, cpos)) {
    return false;
  }
  const char next = script[cpos + 1];
  return std::isdigit(static_cast<unsigned char>(next)) || next == '$' || next == '(';
}

std::string Interpreter::Impl::getIntroScript(const std::string& script, size_t& cpos, char symbBegin, char symbEnd) const {
  size_t ssz = script.size(),
    cp = cpos;
  int bordCnt = 0;
  while (cp < ssz) {
    if (script[cp] == symbBegin) ++bordCnt;
    if (script[cp] == symbEnd) --bordCnt;
    if (bordCnt == 0) break;
    ++cp;
  }
  std::string res;
  if ((bordCnt == 0) && (cp > cpos)) {
    res = script.substr(cpos + 1, cp - cpos - 1);
    cpos = cp + 1;
  }
  return res;
}

bool Interpreter::Impl::isFindKeySymbol(const std::string& script, size_t cpos, size_t maxpos) const {
  return (script.find('(', cpos) < maxpos) ||
    (script.find(')', cpos) < maxpos) ||
    (script.find('{', cpos) < maxpos) ||
    (script.find('}', cpos) < maxpos) ||
    (script.find(',', cpos) < maxpos) ||
    (script.find(';', cpos) < maxpos) ||
    (script.find('#', cpos) < maxpos) ||
    (script.find('"', cpos) < maxpos) ||
    (script.find('$', cpos) < maxpos);
}
bool Interpreter::Impl::startWith(std::string_view str, size_t pos, std::string_view begin) const {
  if (pos + begin.size() > str.size()) {
    return false;
  }
  return str.compare(pos, begin.size(), begin) == 0;
}

Interpreter::Value Interpreter::Impl::variableByKey(const std::string& key) const {
  auto it = m_var.find(key);
  return it != m_var.end() ? it->second : Interpreter::Value{std::string{}};
}

Interpreter::Value Interpreter::Impl::evalOperand(size_t iExpr) {
  Expression& exp = m_expr[iExpr];
  if (exp.iOperator != ir::detail::kNoIndex) {
    return m_expr[exp.iOperator].result;
  }
  switch (exp.keyw) {
  case Keyword::VARIABLE:
    return variableByKey(ir::detail::paramKey(exp.params));
  case Keyword::VALUE:
    return ir::detail::valueOfValueExpr(exp.result, exp.params);
  default:
    return calcOperation(exp.keyw, iExpr);
  }
}
Interpreter::Impl::Keyword Interpreter::Impl::keywordByName(std::string_view oprName) const {
  if (oprName == "if") return Keyword::IF;
  if (oprName == "else") return Keyword::ELSE;
  if (oprName == "elseif") return Keyword::ELSE_IF;
  if (oprName == "while") return Keyword::WHILE;
  if (oprName == "break") return Keyword::BREAK;
  if (oprName == "goto") return Keyword::GOTO;
  if (oprName == "#macro") return Keyword::MACRO;
  if (oprName == "continue") return Keyword::CONTINUE;
  if (oprName == "true" || oprName == "false") return Keyword::VALUE;
  return Keyword::INSTRUCTION;
}
Interpreter::EntityType Interpreter::Impl::keywordToEntityType(Keyword keyw) const {
  static constexpr Interpreter::EntityType kMap[] = {
    Interpreter::EntityType::EXPRESSION,  // INSTRUCTION
    Interpreter::EntityType::EXPRESSION,
    Interpreter::EntityType::OPERATOR,
    Interpreter::EntityType::WHILE,
    Interpreter::EntityType::IF,
    Interpreter::EntityType::ELSE,
    Interpreter::EntityType::ELSE_IF,
    Interpreter::EntityType::BREAK,
    Interpreter::EntityType::CONTINUE,
    Interpreter::EntityType::FUNCTION,
    Interpreter::EntityType::ARGUMENT,
    Interpreter::EntityType::MACRO,
    Interpreter::EntityType::VARIABLE,
    Interpreter::EntityType::VALUE,
    Interpreter::EntityType::GOTO,
  };
  const size_t i = static_cast<size_t>(keyw);
  return i < sizeof(kMap) / sizeof(kMap[0]) ? kMap[i] : Interpreter::EntityType::EXPRESSION;
}

Interpreter::Interpreter() : m_d(std::make_unique<Impl>()) {}
Interpreter::~Interpreter() = default;
Interpreter::Interpreter(const Interpreter& other)
    : m_d(std::make_unique<Impl>()) {
  if (other.m_d) {
    *m_d = *other.m_d;
  }
}
Interpreter::Interpreter(Interpreter&& other) noexcept = default;
Interpreter& Interpreter::operator=(const Interpreter& other) {
  if (this != &other) {
    if (other.m_d) {
      if (!m_d) {
        m_d = std::make_unique<Impl>();
      }
      *m_d = *other.m_d;
    }
  }
  return *this;
}
Interpreter& Interpreter::operator=(Interpreter&& other) noexcept = default;
Interpreter::CmdResult Interpreter::cmd(std::string script) {
  return m_d ? m_d->cmd(std::move(script))
             : Interpreter::CmdResult{Interpreter::Value{std::string{}}, {}};
}
bool Interpreter::parseScript(std::string script, Error& outErr) {
  return m_d ? m_d->parseScript(std::move(script), outErr) : false;
}
std::pair<Interpreter::Value, Interpreter::Error> Interpreter::runScript() {
  return m_d ? m_d->runScript()
             : std::pair<Value, Error>{Value{std::string{}}, {}};
}
bool Interpreter::addFunction(const std::string& name, UserFunction ufunc) {
  return m_d ? m_d->addFunction(name, ufunc) : false;
}
bool Interpreter::addOperator(const std::string& name, UserOperator uoper, uint32_t priority) {
  return m_d ? m_d->addOperator(name, uoper, priority) : false;
}
bool Interpreter::addAttribute(const std::string& name) {
  return m_d ? m_d->addAttribute(name) : false;
}
std::map<std::string, Interpreter::Value> Interpreter::allVariables() const {
  return m_d ? m_d->allVariables() : std::map<std::string, Interpreter::Value>();
}
Interpreter::Value Interpreter::variable(const std::string& vname) const {
  return m_d ? m_d->variable(vname) : Interpreter::Value{std::string{}};
}
Interpreter::Value Interpreter::runFunction(const std::string& fname, const std::vector<Interpreter::Value>& args) {
  return m_d ? m_d->runFunction(fname, args) : Interpreter::Value{std::string{}};
}
bool Interpreter::setVariable(const std::string& vname, const Interpreter::Value& value) {
  return m_d ? m_d->setVariable(vname, value) : false;
}
bool Interpreter::setMacro(const std::string& mname, const std::string& script) {
  return m_d ? m_d->setMacro(mname, script) : false;
}
bool Interpreter::gotoOnLabel(const std::string& lname) {
  return m_d ? m_d->gotoOnLabel(lname) : false;
}
void Interpreter::exitFromScript() {
  if (m_d) m_d->exitFromScript();
}
std::vector<Interpreter::Entity> Interpreter::allEntities() {
  return m_d ? m_d->allEntities() : std::vector<Interpreter::Entity>();
}
Interpreter::Entity Interpreter::currentEntity() {
  return m_d ? m_d->currentEntity() : Interpreter::Entity{ 0 };
}
Interpreter::Entity Interpreter::getEntityByIndex(size_t beginIndex) {
  return m_d ? m_d->getEntityByIndex(beginIndex) : Interpreter::Entity{ 0 };
}
std::vector<std::string> Interpreter::getAttributeByIndex(size_t beginIndex) {
  return m_d ? m_d->getAttributeByIndex(beginIndex) : std::vector<std::string>();
}
bool Interpreter::gotoOnEntity(size_t beginIndex) {
  return m_d ? m_d->gotoOnEntity(beginIndex) : false;
}
Interpreter::UserFunction Interpreter::getUserFunction(const std::string& fname) {
  return m_d ? m_d->getUserFunction(fname) : nullptr;
}
Interpreter::UserOperator Interpreter::getUserOperator(const std::string& oname) {
  return m_d ? m_d->getUserOperator(oname) : nullptr;
}

// --- ir::detail (definitions) ---

namespace ir {
namespace detail {

bool isDigits(std::string_view s) {
  if (s.empty()) {
    return false;
  }
  for (unsigned char c : s) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return true;
}

std::optional<int64_t> parseInteger(std::string_view s) {
  if (!isDigits(s)) {
    return std::nullopt;
  }
  int64_t v = 0;
  for (char c : s) {
    v = v * 10 + (c - '0');
  }
  return v;
}

std::optional<Interpreter::Value> parseNumber(std::string_view s) {
  if (s.empty()) {
    return std::nullopt;
  }
  size_t i = 0;
  bool neg = false;
  if (s[0] == '-') {
    neg = true;
    i = 1;
  }
  if (i >= s.size()) {
    return std::nullopt;
  }
  const size_t dot = s.find('.', i);
  if (dot != std::string_view::npos) {
    const auto intPart = parseInteger(s.substr(i, dot - i));
    const auto fracPart = parseInteger(s.substr(dot + 1));
    if (!intPart || !fracPart) {
      return std::nullopt;
    }
    double whole = static_cast<double>(*intPart);
    double frac = static_cast<double>(*fracPart);
    size_t fracLen = s.size() - dot - 1;
    for (size_t p = 0; p < fracLen; ++p) {
      frac /= 10.0;
    }
    double v = whole + frac;
    return neg ? Interpreter::Value{-v} : Interpreter::Value{v};
  }
  if (auto n = parseInteger(s.substr(i))) {
    return neg ? Interpreter::Value{-*n} : Interpreter::Value{*n};
  }
  return std::nullopt;
}

bool isIdentContinue(unsigned char c) {
  return std::isalnum(c) || c == '_';
}

bool matchKeywordAt(std::string_view script, size_t cpos, std::string_view kw) {
  if (cpos + kw.size() > script.size()) {
    return false;
  }
  if (script.substr(cpos, kw.size()) != kw) {
    return false;
  }
  if (cpos + kw.size() < script.size() &&
      isIdentContinue(static_cast<unsigned char>(script[cpos + kw.size()]))) {
    return false;
  }
  return true;
}

Interpreter::Value parseLiteralText(std::string_view s) {
  return Interpreter::valueFromLiteral(s);
}

Interpreter::Value parseQuotedLiteral(std::string s) {
  return s;
}

Interpreter::Value valueOfValueExpr(const Interpreter::Value& result,
    const std::vector<Interpreter::Value>& params) {
  if (!params.empty()) {
    if (const auto* name = std::get_if<std::string>(&params[0])) {
      if (!name->empty()) {
        return *name;
      }
    }
  }
  return result;
}

const std::string& paramKey(const std::vector<Interpreter::Value>& params) {
  static const std::string kEmpty;
  if (params.empty()) {
    return kEmpty;
  }
  if (const auto* s = std::get_if<std::string>(&params[0])) {
    return *s;
  }
  return kEmpty;
}

size_t paramIndex(const std::vector<Interpreter::Value>& params) {
  if (params.empty()) {
    return kNoIndex;
  }
  if (const auto* i = std::get_if<int64_t>(&params[0])) {
    return static_cast<size_t>(*i);
  }
  if (const auto* s = std::get_if<std::string>(&params[0])) {
    if (auto n = parseInteger(*s)) {
      return static_cast<size_t>(*n);
    }
  }
  return kNoIndex;
}

bool isBreakValue(const Interpreter::Value& v) {
  const auto* cf = std::get_if<Interpreter::ControlFlow>(&v);
  return cf && *cf == Interpreter::ControlFlow::Break;
}

bool isContinueValue(const Interpreter::Value& v) {
  const auto* cf = std::get_if<Interpreter::ControlFlow>(&v);
  return cf && *cf == Interpreter::ControlFlow::Continue;
}

} // namespace detail
} // namespace ir

// --- value helpers ---

std::string Interpreter::valueToString(const Interpreter::Value& v) {
  return std::visit([](auto&& arg) -> std::string {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, bool>) {
      return arg ? "1" : "0";
    }
    else if constexpr (std::is_same_v<T, int64_t>) {
      return std::to_string(arg);
    }
    else if constexpr (std::is_same_v<T, double>) {
      std::ostringstream os;
      os << std::fixed << std::setprecision(10) << arg;
      std::string s = os.str();
      while (s.size() > 1 && s.back() == '0') {
        s.pop_back();
      }
      if (s.size() > 1 && s.back() == '.') {
        s.pop_back();
      }
      return s;
    }
    else if constexpr (std::is_same_v<T, Interpreter::ControlFlow>) {
      return arg == Interpreter::ControlFlow::Break ? "break" : "continue";
    }
    else {
      return arg;
    }
  }, v);
}

Interpreter::Value Interpreter::valueFromLiteral(std::string_view s) {
  if (s.empty()) {
    return std::string{};
  }
  if (s == "true") {
    return true;
  }
  if (s == "false") {
    return false;
  }
  if (auto n = ir::detail::parseNumber(s)) {
    return *n;
  }
  return std::string(s);
}

bool Interpreter::valueIsTruthy(const Interpreter::Value& v) {
  return std::visit([](auto&& arg) -> bool {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, bool>) {
      return arg;
    }
    else if constexpr (std::is_same_v<T, int64_t>) {
      return arg != 0;
    }
    else if constexpr (std::is_same_v<T, double>) {
      return arg != 0.0;
    }
    else if constexpr (std::is_same_v<T, Interpreter::ControlFlow>) {
      return true;
    }
    else {
      return !arg.empty();
    }
  }, v);
}

std::string Interpreter::valueAsString(const Interpreter::Value& v) {
  if (const auto* s = std::get_if<std::string>(&v)) {
    return *s;
  }
  return Interpreter::valueToString(v);
}

std::string Interpreter::valueAsInitBody(const Interpreter::Value& v) {
  if (const auto* s = std::get_if<std::string>(&v)) {
    return *s;
  }
  return {};
}

std::string Interpreter::operandName(Interpreter& ir, const Interpreter::Value& opd) {
  const size_t idx = ir.currentEntity().beginIndex;
  if (idx > 0) {
    const std::string name = ir.getEntityByIndex(idx - 1).name;
    if (!name.empty()) {
      return name;
    }
  }
  return Interpreter::valueAsString(opd);
}

bool Interpreter::valueIsInteger(const Interpreter::Value& v) {
  return std::holds_alternative<int64_t>(v);
}

bool Interpreter::valueIsNumeric(const Interpreter::Value& v) {
  return std::holds_alternative<int64_t>(v) || std::holds_alternative<double>(v);
}

int64_t Interpreter::valueAsInt64(const Interpreter::Value& v) {
  if (const auto* i = std::get_if<int64_t>(&v)) {
    return *i;
  }
  if (const auto* b = std::get_if<bool>(&v)) {
    return *b ? 1 : 0;
  }
  if (const auto* d = std::get_if<double>(&v)) {
    return static_cast<int64_t>(*d);
  }
  return 0;
}

std::vector<Interpreter::Value> Interpreter::makeParam(const std::string& s) {
  return Interpreter::makeParam(std::string_view{s});
}

std::vector<Interpreter::Value> Interpreter::makeParam(std::string_view s) {
  if (s.empty()) {
    return {};
  }
  return {std::string(s)};
}

std::string Interpreter::paramAsString(const std::vector<Interpreter::Value>& params) {
  return params.empty() ? std::string{} : Interpreter::valueAsString(params[0]);
}

Interpreter::Value Interpreter::valueFromParams(const std::vector<Interpreter::Value>& params,
    const Interpreter::Value& fallback) {
  if (params.empty()) {
    return fallback;
  }
  if (const auto* i = std::get_if<int64_t>(&params[0])) {
    return *i;
  }
  return fallback;
}

std::string Interpreter::cmdResultToString(const Interpreter::CmdResult& r) {
  if (r.second) {
    return r.second.message;
  }
  return Interpreter::valueToString(r.first);
}

bool Interpreter::hasError(const Interpreter::Error& err) {
  return static_cast<bool>(err);
}

bool Interpreter::isParseError(const Interpreter::Error& err) {
  return err && err.kind == Interpreter::Error::Kind::Parse;
}

std::string_view Interpreter::valueAsStringView(const Interpreter::Value& v) {
  if (const auto* s = std::get_if<std::string>(&v)) {
    return *s;
  }
  return {};
}

namespace InterpreterCompareDetail {

std::optional<double> valueAsDouble(const Interpreter::Value& v) {
  if (const auto* i = std::get_if<int64_t>(&v)) {
    return static_cast<double>(*i);
  }
  if (const auto* d = std::get_if<double>(&v)) {
    return *d;
  }
  return std::nullopt;
}

} // namespace InterpreterCompareDetail

bool Interpreter::valueEquals(const Interpreter::Value& a, const Interpreter::Value& b) {
  if (a.index() != b.index()) {
    return Interpreter::valueAsString(a) == Interpreter::valueAsString(b);
  }
  return std::visit([&b](const auto& lhs) -> bool {
    using T = std::decay_t<decltype(lhs)>;
    return lhs == std::get<T>(b);
  }, a);
}

int Interpreter::valueCompare(const Interpreter::Value& a, const Interpreter::Value& b) {
  if (Interpreter::valueIsNumeric(a) && Interpreter::valueIsNumeric(b)) {
    const auto da = InterpreterCompareDetail::valueAsDouble(a);
    const auto db = InterpreterCompareDetail::valueAsDouble(b);
    if (da && db) {
      if (*da < *db) {
        return -1;
      }
      if (*da > *db) {
        return 1;
      }
      return 0;
    }
  }
  const std::string sa = Interpreter::valueAsString(a);
  const std::string sb = Interpreter::valueAsString(b);
  if (sa.size() < sb.size()) {
    return -1;
  }
  if (sa.size() > sb.size()) {
    return 1;
  }
  return 0;
}
