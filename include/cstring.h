/*
 * Clever language
 * Copyright (c) 2010 Clever Team
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
 *
 * $Id$
 */

#ifndef CLEVER_CSTRING_H
#define CLEVER_CSTRING_H

#include <string>
#include <map>
#include <locale>

#define CSTRING(xstring) (clever::CString(xstring).intern())

namespace clever {

class CStringTable;

class CString : public std::string {
public:
	CString(const CString& str, long id)
		: std::string(str), m_id(id) { }

	//explicit CString(const CString& str) : std::string(str), m_id(-1) { store(); }
	explicit CString(std::string str) : std::string(str), m_id(-1) { store(); }
	//explicit CString(const char* str) : std::string(str), m_id(-1) { store(); }

	CString* intern();

	inline bool hasSameId(CString* cstring) const {
		return get_id() == cstring->get_id();
	}

	inline long get_id() const {
		return m_id;
	}

	inline void set_id(long id) {
		if (m_id == -1)  {
			m_id = id;
		}
	}

	bool operator==(CString* cstring) {
		return hasSameId(cstring);
	}

	bool operator==(const std::string& string) {
		return compare(string) == 0;
	}

private:
	static CStringTable table;
	long m_id;

	void store();
};

typedef std::map<long, CString*> CStringTableBase;

class CStringTable : public CStringTableBase {
public:
	CStringTable() : CStringTableBase() {
		coll = &std::use_facet<std::collate<char> >(loc);
	}
	~CStringTable();

	bool contains(CString* cstring) const {
		int id = cstring->get_id();
		return id != -1 && id < (signed)size() && (*find(id)).second->hasSameId(cstring);
	}

	CString* getCString(long id) {
		return (*find(id)).second;
	}

	long insert(CString* cstring) {
		long id = hash(*cstring);

		cstring->set_id(id);

		if (CStringTableBase::find(id) == end()) {
			CString* new_string = new CString(*cstring, id);

			/* assume conflicts are not possible. */
			CStringTableBase::insert(std::pair<long, CString*>(id, new_string));
		}

		return id;
	}
private:
	std::locale loc;
	const std::collate<char>* coll;

	inline long hash(CString& cstring) const {
		return coll->hash(cstring.data(), cstring.data()+cstring.length());
	}
};

} // clever

#endif /* CLEVER_CSTRING_H */

