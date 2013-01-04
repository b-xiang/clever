/**
 * Clever programming language
 * Copyright (c) Clever Team
 *
 * This file is distributed under the MIT license. See LICENSE for details.
 */

#include <string>
#include <sstream>
#include "types/str.h"

namespace clever {

CLEVER_TYPE_OPERATOR(StrType::add)
{
	if (EXPECTED(rhs->getType() == this)) {
		// TODO(Felipe): Do not require CString everywhere (because it stores the
		// data in an string table)
		result->setStr(CSTRING(*lhs->getStr() + *rhs->getStr()));
	}
}

CLEVER_TYPE_OPERATOR(StrType::sub)
{	
}

CLEVER_TYPE_OPERATOR(StrType::mul)
{
	if (rhs->getType() == CLEVER_INT_TYPE) {
		std::ostringstream os;

		for (long i = 0, j = rhs->getInt(); i < j; ++i) {
			os << *lhs->getStr();
		}

		result->setStr(CSTRING(os.str()));
	}
}

CLEVER_TYPE_OPERATOR(StrType::div)
{
}

CLEVER_TYPE_OPERATOR(StrType::mod)
{	
}

// String.subString(string str, [int start, [int count]])
// String.subString(int start, [int count])
// Returns a substring of the argument or string object referenced using bounds provided
CLEVER_METHOD(StrType::subString) 
{
	const CString *of = NULL;
	int bounds[2];

	bounds[0]=-1;
	bounds[1]=-1;

	if (CLEVER_THIS()) {
		of = (const CString*) CLEVER_THIS()->getStr();
		switch(CLEVER_ARG_COUNT()) {
			case 2:
				bounds[0]=CLEVER_ARG_INT(0);
				bounds[1]=CLEVER_ARG_INT(1);
			break;
			case 1: 
				bounds[0]=CLEVER_ARG_INT(0);
			break;
			default: std::cerr << "String.subString expected at least one argument";
		}
	} else if (CLEVER_ARG_COUNT()) {
		of = (const CString*) CLEVER_ARG_CSTR(0);
		switch(CLEVER_ARG_COUNT()) {
			case 3:
				bounds[0]=CLEVER_ARG_INT(1);
				bounds[1]=CLEVER_ARG_INT(2);
			break;
			case 2: bounds[0]=CLEVER_ARG_INT(1); break;
			
			default: std::cerr << "String.subString expected at least two arguments";
		}
	}

	if (of && bounds[0]>-1) {
		if (bounds[1]>-1) {
			result->setStr(CSTRING(of->substr(bounds[0], bounds[1])));
		} else result->setStr(CSTRING(of->substr(bounds[0])));
	}
}

// String.findString(string haystack, string needle, [int position, [int count]])
// String.findString(string needle, [int position, [int count]])
// Finds a string in a string returning the position
CLEVER_METHOD(StrType::find) 
{
	const char *needle;
	const CString *haystack;
	int bounds[2];

	bounds[0]=-1;
	bounds[1]=-1;
	
	if (CLEVER_THIS()) {
		haystack = (const CString*) CLEVER_THIS()->getStr();
		switch (CLEVER_ARG_COUNT()) {
			case 1:
				needle = CLEVER_ARG_PSTR(0);
			break;
			case 2:
				needle = CLEVER_ARG_PSTR(0);
				bounds[0] = CLEVER_ARG_INT(1);
			break;
			case 3:
				needle = CLEVER_ARG_PSTR(0);
				bounds[0] = CLEVER_ARG_INT(1);		
				bounds[1] = CLEVER_ARG_INT(2);
			break;
			default: std::cerr << "String.find expected a maximum of 2 arguments";
		}
	} else if(CLEVER_ARG_COUNT()) {
		haystack = (const CString*) CLEVER_ARG_CSTR(0);
		switch (CLEVER_ARG_COUNT()) {
			case 4:
				needle = CLEVER_ARG_PSTR(1);
				bounds[0] = CLEVER_ARG_INT(2);
				bounds[1] = CLEVER_ARG_INT(3);
			break;
			case 3:
				needle = CLEVER_ARG_PSTR(1);
				bounds[0] = CLEVER_ARG_INT(2);
			break;
			case 2:
				needle = CLEVER_ARG_PSTR(1);
			break;
			default: std::cerr << "String.find expected at least two arguments";
		}
	}	

	if (needle && haystack) {
		if (bounds[0]>-1) {
			if (bounds[1]>-1) {
				result->setInt(haystack->find((const char*)needle, bounds[0], bounds[1]));
			} else result->setInt(haystack->find((const char*)needle, bounds[0]));
		} else result->setInt(haystack->find((const char*)needle));
	}
}

CLEVER_TYPE_INIT(StrType::init) 
{
	addMethod(CSTRING("subString"),  	(MethodPtr) &StrType::subString);
	addMethod(CSTRING("find"), 			(MethodPtr) &StrType::find);
}

} // clever
