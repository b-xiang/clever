/**
 * Clever programming language
 * Copyright (c) 2011-2012 Clever Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CLEVER_VM_H
#define CLEVER_VM_H

#include <vector>
#include <stack>
#include "cthread.h"
#include "clever.h"
#include "compiler.h"
#include "ir.h"
#include "opcode.h"

namespace clever {

// Helper macros to be used to change the VM program counter
// The compiler optimizer is supposed to use tail call optimization here
#define VM_CONT()
//(this->*m_handlers[m_inst[m_pc].opcode])(m_inst[m_pc])
#define VM_NEXT() ++m_pc; VM_CONT()
#define VM_GOTO(n) m_pc = n; VM_CONT(); return

// Helper macro for opcode handler declaration
#define VM_HANDLER_ARG IR& op
#define VM_HANDLER(name) CLEVER_FASTCALL void VM::name(VM_HANDLER_ARG)
#define VM_HANDLER_D(name) void name(VM_HANDLER_ARG)

class Scope;
class Value;

typedef std::vector<std::pair<size_t, Value*> > FuncVars;

/// Stackframe representation
struct StackFrame {
    size_t ret_addr;     // Return address
    Value* ret_val;      // Return value
    Scope* arg_vars;     // Function arguments
    Scope* local_vars;   // Local variables
    FuncVars vars;       // Arg and local vars storage

    StackFrame()
        : ret_addr(0), ret_val(NULL), arg_vars(NULL), local_vars(NULL), vars() {}
};

class VM;

struct Thread {
    VM* vm_handler;
    pthread_t t_handler;
    pthread_mutex_t m_handler;
};

/// VM representation
class VM {
public:
    typedef void (VM::*OpHandler)(VM_HANDLER_ARG);

    typedef std::vector<Thread*> ThreadPool;

    VM(IRVector& inst)
        : m_pc(0), m_is_main_thread(true), m_inst(inst), m_scope_pool(NULL) {}
    ~VM() {}

    /// Sets the symbol table to used by the VM to fetch the symbol names
    void setSymbolTable(ScopePool* scope) { m_scope_pool = scope; }

    void copy(VM*);

    void setChild() { m_is_main_thread = false; }
    bool isChild() const { return !m_is_main_thread; }
    bool isMain() const { return !m_is_main_thread; }
    size_t getPC() { return m_pc; }
    IRVector& getInst() { return m_inst; }

    /// Helper to retrive a Value* from ValuePool
    Value* getValue(size_t, size_t) const;

    /// Save function variables on recursion
    void saveVars();
    void restoreVars() const;

    /// Start the VM execution
    void run();

    /// Wait threads
    void wait();

    /// Methods for dumping opcodes
#ifdef CLEVER_DEBUG
    void dumpOpcodes() const;
#endif

    // VM opcode handlers
    VM_HANDLER_D(var_decl);
    VM_HANDLER_D(switch_scope);
    VM_HANDLER_D(ret);
    VM_HANDLER_D(assignment);
    VM_HANDLER_D(add);
    VM_HANDLER_D(sub);
    VM_HANDLER_D(mul);
    VM_HANDLER_D(div);
    VM_HANDLER_D(mod);
    VM_HANDLER_D(jmp);
    VM_HANDLER_D(fcall);
    VM_HANDLER_D(threadcall);
    VM_HANDLER_D(endthread);
    VM_HANDLER_D(leave);
    VM_HANDLER_D(send_val);
    VM_HANDLER_D(jmpz);
    VM_HANDLER_D(inc);
    VM_HANDLER_D(dec);
private:
    /// Initialization phase
    void init();

    /// VM program counter
    size_t m_pc;

    bool m_is_main_thread;

    /// Vector of instruction
    IRVector& m_inst;

    /// Scope pool
    ScopePool* m_scope_pool;

    /// VM opcode handlers
    OpHandler m_handlers[NUM_OPCODES];

    /// Stack frame
    std::stack<StackFrame> m_call_stack;

    /// Call arguments
    std::vector<Value*> m_call_args;

    ThreadPool m_thread_pool;

    DISALLOW_COPY_AND_ASSIGN(VM);
};

} // clever

#endif // CLEVER_VM_H