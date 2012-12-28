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

#ifndef CLEVER_IR_H
#define CLEVER_IR_H

#include <vector>
#include "vm/opcode.h"

namespace clever {

class Value;

enum OperandType {
	UNUSED,      // Operand is not used
	FETCH_VAL,   // For Value* fetchs
	FETCH_SCOPE, // For scope switching
	JMP_ADDR     // For instr addr
};

/// Intermediate representation
struct IR {
	IR(Opcode _op)
		: opcode(_op),
			op1_type(UNUSED), op2_type(UNUSED),
			op1(0), op2(0),
			result(NULL) {}

	IR(Opcode _op, OperandType _op1_type, size_t _op1)
		: opcode(_op),
			op1_type(_op1_type), op2_type(UNUSED),
			op1(_op1), op2(0),
			result(NULL) {}

	IR(Opcode _op, OperandType _op1_type, size_t _op1,
		OperandType _op2_type, size_t _op2, Value* res = NULL)
		: opcode(_op),
			op1_type(_op1_type), op2_type(_op2_type),
			op1(_op1), op2(_op2),
			result(res) {}

	Opcode opcode;
	OperandType op1_type, op2_type;
	size_t op1, op2;
	Value* result;
};

// Vector of VM instructions
typedef std::vector<IR> IRVector;

} // clever

#endif // CLEVER_IR_H