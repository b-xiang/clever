/**
 * Clever programming language
 * Copyright (c) Clever Team
 *
 * This file is distributed under the MIT license. See LICENSE for details.
 */

#include "core/cexception.h"
#include "modules/db/sqlite3/sqlite3.h"
#include "modules/std/core/map.h"
#include "modules/std/core/str.h"

namespace clever { namespace modules { namespace db {

static Value* _sqlite_to_value(sqlite3_stmt* stmt, int column)
{
	Value* value = new Value;

	switch (sqlite3_column_type(stmt, column)) {
		case SQLITE_INTEGER:
			value->setInt(sqlite3_column_int64(stmt, column));
			break;
		case SQLITE_FLOAT:
			value->setDouble(sqlite3_column_double(stmt, column));
			break;
		case SQLITE_NULL:
			break;
		case SQLITE_TEXT:
			{
				const char* str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
				value->setStr(new StrObject(str));
			}
			break;
		case SQLITE_BLOB:
			{
				const char* buf = static_cast<const char*>(sqlite3_column_blob(stmt, column));
				value->setStr(new StrObject(buf));
			}
			break;
	}

	return value;
}

TypeObject* SQLite3Type::allocData(CLEVER_TYPE_CTOR_ARGS) const
{
	return new SQLite3Conn(*(*args)[0]->getStr());
}

// SQLite3 constructor
CLEVER_METHOD(SQLite3Type::ctor)
{
	if (!clever_check_args("s")) {
		return;
	}

	SQLite3Conn* conn = static_cast<SQLite3Conn*>(allocData(&args));

	if (sqlite3_open(args[0]->getStr()->c_str(), &conn->handle) != SQLITE_OK) {
		clever_throw("An error occurred when opening database: %s",
			sqlite3_errmsg(conn->handle));

		clever_delref(conn);
		return;
	}

	result->setObj(this, conn);
}

// SQLite3::exec(String pattern)
CLEVER_METHOD(SQLite3Type::exec)
{
	if (!clever_check_args("s")) {
		return;
	}

	SQLite3Conn* conn = CLEVER_GET_OBJECT(SQLite3Conn*, CLEVER_THIS());
	char* errmsg = NULL;

	sqlite3_exec(conn->handle,
		args[0]->getStr()->c_str(),
		NULL,
		NULL,
		&errmsg);

	if (errmsg) {
		clever_throw("An error occurred when executing a statement: %s",
			errmsg);

		sqlite3_free(errmsg);
		return;
	}

	result->setBool(true);
}

// SQLite3::query(String query)
CLEVER_METHOD(SQLite3Type::query)
{
	if (!clever_check_args("s")) {
		return;
	}

	SQLite3Conn* conn = CLEVER_GET_OBJECT(SQLite3Conn*, CLEVER_THIS());
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(conn->handle,
		args[0]->getStr()->c_str(),
		args[0]->getStr()->size(),
		&stmt,
		NULL) != SQLITE_OK) {
		clever_throw("An error occurred when preparing query: %s",
			sqlite3_errmsg(conn->handle));
		return;
	}

	switch (sqlite3_step(stmt)) {
		case SQLITE_DONE:
		case SQLITE_ROW:
			sqlite3_reset(stmt);
			break;
		case SQLITE_ERROR:
			clever_throw("%s", sqlite3_errmsg(conn->handle));
			return;
	}


	SQLite3Result* res = new SQLite3Result(stmt);

	conn->results.push_back(res);
	clever_addref(res);

	result->setObj(m_module->getResultType(), res);
}

// SQLite::getLastId()
CLEVER_METHOD(SQLite3Type::getLastId)
{
	if (!clever_check_no_args()) {
		return;
	}

	SQLite3Conn* conn = CLEVER_GET_OBJECT(SQLite3Conn*, CLEVER_THIS());

	result->setInt(sqlite3_last_insert_rowid(conn->handle));
}

// SQLite::close()
CLEVER_METHOD(SQLite3Type::close)
{
	if (!clever_check_no_args()) {
		return;
	}

	SQLite3Conn* conn = CLEVER_GET_OBJECT(SQLite3Conn*, CLEVER_THIS());

	result->setBool(conn->close() == SQLITE_OK);
}

// SQLite3Result::fetch()
CLEVER_METHOD(SQLite3TypeResult::fetch)
{
	if (!clever_check_no_args()) {
		return;
	}

	SQLite3Result* res = CLEVER_GET_OBJECT(SQLite3Result*, CLEVER_THIS());
	int ret_code = sqlite3_step(res->stmt);

	if (ret_code != SQLITE_ROW) {
		result->setBool(false);
		return;
	}

	MapObject* map = new MapObject;

	for (int i = 0, n = sqlite3_data_count(res->stmt); i < n; ++i) {
		map->getData().insert(MapObjectPair(
			sqlite3_column_name(res->stmt, i), _sqlite_to_value(res->stmt, i)));
	}

	result->setObj(CLEVER_MAP_TYPE, map);
}

// SQLite3Result::finalize()
CLEVER_METHOD(SQLite3TypeResult::finalize)
{
	if (!clever_check_no_args()) {
		return;
	}

	SQLite3Result* res = CLEVER_GET_OBJECT(SQLite3Result*, CLEVER_THIS());

	sqlite3_reset(res->stmt);
}

// SQLite3 type initialization
CLEVER_TYPE_INIT(SQLite3Type::init)
{
	setConstructor((MethodPtr)&SQLite3Type::ctor);

	addMethod(new Function("exec",      (MethodPtr)&SQLite3Type::exec));
	addMethod(new Function("query",     (MethodPtr)&SQLite3Type::query));
	addMethod(new Function("getLastId", (MethodPtr)&SQLite3Type::getLastId));
	addMethod(new Function("close",     (MethodPtr)&SQLite3Type::close));
}

// SQLite3Stmt type initialization
CLEVER_TYPE_INIT(SQLite3TypeStmt::init)
{
}

// SQLite3Stmt type initialization
CLEVER_TYPE_INIT(SQLite3TypeResult::init)
{
	addMethod(new Function("fetch",    (MethodPtr)&SQLite3TypeResult::fetch));
	addMethod(new Function("finalize", (MethodPtr)&SQLite3TypeResult::finalize));
}

}}} // clever::modules::std