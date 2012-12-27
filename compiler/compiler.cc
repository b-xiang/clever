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

#include "compiler/cstring.h"
#include "compiler/compiler.h"
#include "compiler/scope.h"
#include "compiler/value.h"
#include "interpreter/location.hh"
#include "types/native_types.h"

namespace clever {

/**
 * Declaration of namespace global type ptrs for Clever native types
 */
DECLARE_CLEVER_NATIVE_TYPES();

/**
 * Compiler initialization phase
 */
void Compiler::init()
{
	m_scope = new Scope;

	m_ir.reserve(20);
	m_scope_pool.reserve(10);
	m_value_pool.reserve(30);
	m_type_pool.reserve(15);

	m_scope_pool[m_scope_id++] = m_scope;
	m_value_pool[m_value_id++] = NULL;

	/**
	 * Native type allocation
	 */
	m_type_pool[m_type_id++] = CLEVER_INT_TYPE    = new IntType;
	m_type_pool[m_type_id++] = CLEVER_DOUBLE_TYPE = new DoubleType;
}

/**
 * Frees all resource used by the compiler
 */
void Compiler::shutdown()
{
	CLEVER_SAFE_DELETE(g_cstring_tbl);
	CLEVER_SAFE_DELETE(m_scope);

	ValuePool::const_iterator it = m_value_pool.begin(),
		end = m_value_pool.end();

	while (it != end) {
		delete *it;
		++it;
	}

	TypePool::const_iterator it2 = m_type_pool.begin(),
		end2 = m_type_pool.end();

	while (it2 != end2) {
		delete *it2;
		++it2;
	}
}

/**
 * Compiler termination phase
 */
void Compiler::end()
{
	m_ir.push_back(IR(OP_RETURN));
}

/**
 * Displays an error message and exits
 */
void Compiler::error(const char* msg) const
{
	std::cerr << "Compile error: " << msg << std::endl;
	CLEVER_EXIT_FATAL();
}

/**
 * Displays an error message
 */
void Compiler::error(const std::string& message, const location& loc) const
{
	if (loc.begin.filename) {
		std::cerr << "Compile error: " << message << " on "
			<< *loc.begin.filename << " line " << loc.begin.line << "\n";
	} else {
		std::cerr << "Compile error: " << message << " on line "
			<< loc.begin.line << "\n";
	}
	CLEVER_EXIT_FATAL();
}

/**
 * Displays a formatted error message and abort the compilation
 */
void Compiler::errorf(const location& loc, const char* format, ...) const
{
	std::ostringstream out;
	va_list args;

	va_start(args, format);

	vsprintf(out, format, args);

	va_end(args);

	error(out.str(), loc);
}

/**
 * Compiles a variable declaration
 */
void Compiler::varDeclaration(const CString* var, Value* node)
{
	/**
	 * A NULL value is created for uninitialized declaration
	 */
	if (!node) {
		node = new Value();
	}

	m_scope->push(var, m_value_id);

	m_ir.push_back(
		IR(OP_ASSIGN, FETCH_VAL, m_value_id, FETCH_VAL, m_value_id+1));

	/**
	 * Creating the symbol related value
	 */
	m_value_pool[m_value_id++] = new Value();
	m_value_pool[m_value_id++] = node;
}

/**
 * Compiles a variable assignment
 */
void Compiler::assignment(const CString* var, Value* value, const location& loc)
{
	Symbol* sym = m_scope->getSymbol(var);

	if (!sym) {
		errorf(loc, "Variable `%S' cannot be found!", var);
	}

	m_ir.push_back(
		IR(OP_ASSIGN, FETCH_VAL, sym->getValueId(), FETCH_VAL, m_value_id));

	m_value_pool[m_value_id++] = value;
}

/**
 * Compiles a set of binary operation
 */
Value* Compiler::binOp(Opcode op, Value* lhs, Value* rhs)
{
	Value* result = new Value();

	m_ir.push_back(
		IR(OP_PLUS, FETCH_VAL, m_value_id, FETCH_VAL, m_value_id+1, result));

	m_value_pool[m_value_id++] = lhs;
	m_value_pool[m_value_id++] = rhs;

	return result;
}

/**
 * Temporary print statement compilation
 */
void Compiler::print(const CString* var, const location& loc)
{
	Symbol* sym = m_scope->getSymbol(var);

	if (!sym) {
		errorf(loc, "Variable `%S' cannot be found!", var);
	}
	m_ir.push_back(IR(OP_PRINT, FETCH_SYM, sym->getValueId()));
}

/**
 * Creates a new lexical scope
 */
void Compiler::newScope()
{
	m_scope = m_scope->newLexicalScope();
	m_scope->setId(m_scope_id);

	m_ir.push_back(IR(OP_SCOPE, FETCH_SCOPE, m_scope_id));

	m_scope_pool[m_scope_id++] = m_scope;
}

/**
 * Scope end marker to switch to parent scope
 */
void Compiler::endScope()
{
	m_scope = m_scope->getParent();

	m_ir.push_back(IR(OP_SCOPE, FETCH_SCOPE, m_scope->getId()));
}

} // clever
