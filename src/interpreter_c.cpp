
#include "../include/interpreter_c.h"
#include "../include/interpreter.h"

#include <cstring>

HIntr irCreateIntr(){
    return new Interpreter();
}

BOOL irAddFunction(HIntr h, char* name, irUserFunction ufunc){
    if (!h || !name || !ufunc) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h);

    ir->addFunction(name, [ufunc](const std::vector<Interpreter::Value>& args)->Interpreter::Value{

        char** cargs = new char*[args.size()];
        for(size_t i = 0; i < args.size(); ++i){
        const auto s = Interpreter::valueToString(args[i]);
        cargs[i] = new char[s.size() + 1]{};
        strcpy(cargs[i], s.c_str());
        }
        auto res = ufunc(cargs, args.size());

        for(size_t i = 0; i < args.size(); ++i){
          delete[] cargs[i];
        }
        delete[] cargs;

        return Interpreter::valueFromLiteral(res ? res : "");
    });
    return TRUE;
}

BOOL irAddOperator(HIntr h, char* name, irUserOperator uopr, uint32_t priority){
    if (!h || !name || !uopr) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h);

    ir->addOperator(name, [uopr](Interpreter::Value& left, Interpreter::Value& right)->Interpreter::Value{

      auto leftStr = Interpreter::valueToString(left);
      auto rightStr = Interpreter::valueToString(right);

      char* cleft = new char[leftStr.size() + 1]{};
      strcpy(cleft, leftStr.c_str());

      char* cright = new char[rightStr.size() + 1]{};
      strcpy(cright, rightStr.c_str());

      auto res = uopr(&cleft, &cright);

      left = Interpreter::valueFromLiteral(cleft);
      right = Interpreter::valueFromLiteral(cright);

      delete[] cleft;
      delete[] cright;

      return Interpreter::valueFromLiteral(res ? res : "");
    }, priority);

    return TRUE;
}

char* irCmd(HIntr h, char* script){
    if (!h || !script) return NULL;

    auto ir = reinterpret_cast<Interpreter*>(h);

    const auto res = Interpreter::cmdResultToString(ir->cmd(script));

    char* cres = new char[res.size() + 1]{};
    strcpy(cres, res.c_str());

    return cres;
}

BOOL irParseScript(HIntr h, char* script, char* outErr, size_t outErrSize, size_t* outPos) {
    if (!h || !script) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h);

    Interpreter::Error err;
    const bool ok = ir->parseScript(script, err);

    if (outErr && outErrSize > 0) {
        strncpy(outErr, err.message.c_str(), outErrSize - 1);
        outErr[outErrSize - 1] = '\0';
    }
    if (outPos) {
        *outPos = err.position;
    }
    return ok ? TRUE : FALSE;
}

char* irRunScript(HIntr h){
    if (!h) return NULL;

    auto ir = reinterpret_cast<Interpreter*>(h);

    const auto res = Interpreter::valueToString(ir->runScript().first);

    char* cres = new char[res.size() + 1]{};
    strcpy(cres, res.c_str());

    return cres;
}

char* irVariable(HIntr h, char* vname){
    if (!h || !vname) return NULL;

    auto ir = reinterpret_cast<Interpreter*>(h);

    const auto res = Interpreter::valueToString(ir->variable(vname));

    char* cres = new char[res.size() + 1]{};
    strcpy(cres, res.c_str());

    return cres;
}

BOOL irSetVariable(HIntr h, char* vname, char* value){
    if (!h || !vname || !value) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h);

    auto res = ir->setVariable(vname, Interpreter::valueFromLiteral(value));

    return res ? TRUE : FALSE;
}

BOOL irSetMacro(HIntr h, char* mname, char* script){
    if (!h || !mname || !script) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h);

    auto res = ir->setMacro(mname, script);

    return res ? TRUE : FALSE;
}

BOOL irGotoOnLabel(HIntr h, char* lname){
    if (!h || !lname) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h);

    auto res = ir->gotoOnLabel(lname);

    return res ? TRUE : FALSE;
}

void irExitFromScript(HIntr h){
    if (!h) return;

    auto ir = reinterpret_cast<Interpreter*>(h);

    ir->exitFromScript();
}

void irFree(char* p) {
    delete[] p;
}
