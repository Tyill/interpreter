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

#ifndef INTERPRETER_C_API_H_
#define INTERPRETER_C_API_H_

#ifdef _WIN32
#ifdef INTERPRETERDLL_EXPORTS
#define INTERPRETER_API __declspec(dllexport)
#else
#define INTERPRETER_API __declspec(dllimport)
#endif
#else
#define INTERPRETER_API
#endif

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef enum BOOL{ FALSE = 0, TRUE = 1}BOOL;

typedef char*(*irUserFunction)(char** args, size_t count);
typedef char*(*irUserOperator)(char** ioLeftOperand, char** ioRightOperand);

typedef void* HIntr;


INTERPRETER_API HIntr irCreateIntr();

/// Add function
/// @param name
/// @param ufunc function
/// return true - ok
INTERPRETER_API BOOL irAddFunction(HIntr, char* name, irUserFunction ufunc);

/// Add operator
/// @param name
/// @param uopr operator
/// @param priority
/// return true - ok
INTERPRETER_API BOOL irAddOperator(HIntr, char* name, irUserOperator uopr, uint32_t priority);
   
/// Execute script (== parseScript + runScript)
/// @param script
/// @return result or error
INTERPRETER_API char* irCmd(HIntr, char* script);

/// Parse script
/// @param script
/// return true - ok
INTERPRETER_API BOOL irParseScript(HIntr, char* script, char* outErr /*sz 256*/);

/// Run script
/// return result
INTERPRETER_API char* irRunScript(HIntr);

/// Value of variable
/// @param vname
/// @return value
INTERPRETER_API char* irVariable(HIntr, char* vname);

/// Set value of variable
/// @param vname
/// @param value
/// @return true - ok
INTERPRETER_API BOOL irSetVariable(HIntr, char* vname, char* value);

/// Set macro
/// @param mname
/// @param script
/// @return true - ok
INTERPRETER_API BOOL irSetMacro(HIntr, char* mname, char* script);

/// Go-to on label
/// @param lname label name
/// @return true - ok
INTERPRETER_API BOOL irGotoOnLabel(HIntr, char* lname);
    
/// Exit from script
INTERPRETER_API void irExitFromScript(HIntr);
  
 
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* INTERPRETER_C_API_H_ */