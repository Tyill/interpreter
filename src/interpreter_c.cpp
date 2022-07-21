
#include "../include/interpreter_c.h"
#include "../include/interpreter.h"

#include <cstring>

HIntr irCreateIntr(){
    return new Interpreter();
}

BOOL irAddFunction(HIntr h, char* name, irUserFunction ufunc){
    if (!h || !name || !ufunc) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    ir->addFunction(name, [ufunc](const std::vector<std::string>& args)->std::string{
        
        char** cargs = new char*[args.size()];
        for(size_t i = 0; i < args.size(); ++i){
        cargs[i] = (char*)args[i].c_str();
        }
        auto res = ufunc(cargs, args.size());

        free(cargs);

        return res;
    });
    return TRUE;
}

BOOL irAddOperator(HIntr h, char* name, irUserOperator uopr, uint32_t priority){
    if (!h || !name || !uopr) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    ir->addOperator(name, [uopr](std::string& left, std::string& right)->std::string{
      
      char* cleft = new char[left.size() + 1]{};
      strcpy(cleft, left.c_str());

      char* cright = new char[right.size() + 1]{};
      strcpy(cright, right.c_str());

      auto res = uopr(&cleft, &cright);

      left = cleft;
      right = cright;

      free(cleft);
      free(cright);

      return res;
    }, priority);

    return TRUE;
}
   
char* irCmd(HIntr h, char* script){
    if (!h || !script) return NULL;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    auto res = ir->cmd(script);

    char* cres = new char[res.size() + 1]{};
    strcpy(cres, res.c_str());

    return cres;
}

BOOL irParseScript(HIntr h, char* script, char* outErr /*sz 256*/){
    if (!h || !script) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    std::string err;
    bool res = ir->parseScript(script, err);

    if (outErr){
        strncpy(outErr, err.c_str(), 255);
    }
    return res ? TRUE : FALSE;
}

char* irRunScript(HIntr h){
    if (!h) return NULL;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    auto res = ir->runScript();

    char* cres = new char[res.size() + 1]{};
    strcpy(cres, res.c_str());

    return cres;
}

char* irVariable(HIntr h, char* vname){
    if (!h || !vname) return NULL;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    auto res = ir->variable(vname);

    char* cres = new char[res.size() + 1]{};
    strcpy(cres, res.c_str());

    return cres;
}

BOOL irSetVariable(HIntr h, char* vname, char* value){
    if (!h || !vname || !value) return FALSE;

    auto ir = reinterpret_cast<Interpreter*>(h); 

    auto res = ir->setVariable(vname, value);

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