//
// Binder.cpp
//
// $Id: //poco/Main/Data/ODBC/src/Binder.cpp#4 $
//
// Library: ODBC
// Package: ODBC
// Module:  Binder
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Data/ODBC/Binder.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/Data/BLOB.h"
#include "Poco/Data/ODBC/ODBCException.h"
#include "Poco/DateTime.h"
#include "Poco/Exception.h"
#include <sql.h>


namespace Poco {
namespace Data {
namespace ODBC {


Binder::Binder(const StatementHandle& rStmt, 
	Binder::ParameterBinding dataBinding,
	TypeInfo* pDataTypes):
	_rStmt(rStmt),
	_paramBinding(dataBinding),
	_pTypeInfo(pDataTypes)
{
}


Binder::~Binder()
{
	LengthVec::iterator itLen = _lengthIndicator.begin();
	LengthVec::iterator itLenEnd = _lengthIndicator.end();
	for(; itLen != itLenEnd; ++itLen) delete *itLen;

	TimestampMap::iterator itTS = _timestamps.begin();
	TimestampMap::iterator itTSEnd = _timestamps.end();
	for(; itTS != itTSEnd; ++itTS) delete itTS->first;

	StringMap::iterator itStr = _strings.begin();
	StringMap::iterator itStrEnd = _strings.end();
	for(; itStr != itStrEnd; ++itStr) std::free(itStr->first);
}


void Binder::bind(std::size_t pos, const std::string& val, Direction dir)
{
	SQLPOINTER pVal = 0;
	SQLINTEGER size = (SQLINTEGER) val.size();

	if (isOutBound(dir))
	{
		try
		{
			Parameter p(_rStmt, pos);
			size = (SQLUINTEGER) p.columnSize();
		}
		catch (StatementException&)
		{
			size = DEFAULT_PARAM_SIZE;
//On Linux, PostgreSQL driver segfaults on SQLGetDescField, so this is disabled for now
#ifdef POCO_OS_FAMILY_WINDOWS
			SQLHDESC hIPD = 0;
			if (!Utility::isError(SQLGetStmtAttr(_rStmt, SQL_ATTR_IMP_PARAM_DESC, &hIPD, 0, 0)))
			{
				SQLUINTEGER sz = 0;
				if (!Utility::isError(SQLGetDescField(hIPD, (SQLSMALLINT) pos + 1, SQL_DESC_LENGTH, &sz, sizeof(sz), 0)) && 
					sz > 0)
				{
					size = sz;
				}
			}
#endif
		}

		char* pChar = (char*) std::calloc(size, sizeof(char));
		pVal = (SQLPOINTER) pChar;
		_outParams.insert(ParamMap::value_type(pVal, size));
		_strings.insert(StringMap::value_type(pChar, const_cast<std::string*>(&val)));
	}
	else if (isInBound(dir))
	{
		pVal = (SQLPOINTER) val.c_str();
		_inParams.insert(ParamMap::value_type(pVal, size));
	}
	else
		throw IllegalStateException("Parameter must be [in] OR [out] bound.");

	SQLLEN* pLenIn = new SQLLEN;
	*pLenIn = SQL_NTS;

	if (PB_AT_EXEC == _paramBinding)
		*pLenIn = SQL_LEN_DATA_AT_EXEC(size);

	_lengthIndicator.push_back(pLenIn);

	if (Utility::isError(SQLBindParameter(_rStmt, 
		(SQLUSMALLINT) pos + 1, 
		toODBCDirection(dir), 
		SQL_C_CHAR, 
		SQL_LONGVARCHAR, 
		(SQLUINTEGER) size,
		0,
		pVal, 
		(SQLINTEGER) size, 
		_lengthIndicator.back())))
	{
		throw StatementException(_rStmt, "SQLBindParameter(std::string)");
	}
}


void Binder::bind(std::size_t pos, const Poco::Data::BLOB& val, Direction dir)
{
	if (isOutBound(dir) || !isInBound(dir))
		throw NotImplementedException("BLOB parameter type can only be inbound.");

	SQLPOINTER pVal = (SQLPOINTER) val.rawContent();
	SQLINTEGER size = (SQLINTEGER) val.size();
		
	_inParams.insert(ParamMap::value_type(pVal, size));

	SQLLEN* pLenIn = new SQLLEN;
	*pLenIn  = size;

	if (PB_AT_EXEC == _paramBinding)
		*pLenIn  = SQL_LEN_DATA_AT_EXEC(size);

	_lengthIndicator.push_back(pLenIn);

	if (Utility::isError(SQLBindParameter(_rStmt, 
		(SQLUSMALLINT) pos + 1, 
		SQL_PARAM_INPUT, 
		SQL_C_BINARY, 
		SQL_LONGVARBINARY, 
		(SQLUINTEGER) size,
		0,
		pVal, 
		(SQLINTEGER) size, 
		_lengthIndicator.back())))
	{
		throw StatementException(_rStmt, "SQLBindParameter(BLOB)");
	}
}


void Binder::bind(std::size_t pos, const Poco::DateTime& val, Direction dir)
{
	SQLINTEGER size = (SQLINTEGER) sizeof(SQL_TIMESTAMP_STRUCT);
	SQLLEN* pLenIn = new SQLLEN;
	*pLenIn  = size;

	_lengthIndicator.push_back(pLenIn);

	SQL_TIMESTAMP_STRUCT* pTS = new SQL_TIMESTAMP_STRUCT;
	Utility::dateTimeSync(*pTS, val);
	
	_timestamps.insert(TimestampMap::value_type(pTS, const_cast<DateTime*>(&val)));

	SQLINTEGER colSize = 0;
	SQLSMALLINT decDigits = 0;

	if (_pTypeInfo)
	{
		try
		{
			colSize = _pTypeInfo->getInfo(SQL_TYPE_TIMESTAMP, "COLUMN_SIZE");
			decDigits = _pTypeInfo->getInfo(SQL_TYPE_TIMESTAMP, "MINIMUM_SCALE");
		}catch (NotFoundException&) { }
	}

	if (Utility::isError(SQLBindParameter(_rStmt, 
		(SQLUSMALLINT) pos + 1, 
		toODBCDirection(dir), 
		SQL_C_TIMESTAMP, 
		SQL_TIMESTAMP, 
		colSize,
		decDigits,
		(SQLPOINTER) pTS, 
		0, 
		_lengthIndicator.back())))
	{
		throw StatementException(_rStmt, "SQLBindParameter(BLOB)");
	}
}


void Binder::bindNull(std::size_t pos, SQLSMALLINT cDataType)
{
	_inParams.insert(ParamMap::value_type(0, 0));

	SQLLEN* pLenIn = new SQLLEN;
	*pLenIn  = SQL_NULL_DATA;

	if (PB_AT_EXEC == _paramBinding)
		*pLenIn  = SQL_LEN_DATA_AT_EXEC(SQL_NULL_DATA);

	_lengthIndicator.push_back(pLenIn);

	SQLINTEGER colSize = 0;
	SQLSMALLINT decDigits = 0;
	
	try
	{
		Parameter p(_rStmt, pos);
		colSize = (SQLINTEGER) p.columnSize();
		decDigits = (SQLSMALLINT) p.decimalDigits();
	}catch (StatementException&)
	{
		try
		{
			ODBCColumn c(_rStmt, pos);
			colSize = (SQLINTEGER) c.length();
			decDigits = (SQLSMALLINT) c.precision();
		}catch (StatementException&) { }
	}

	if (Utility::isError(SQLBindParameter(_rStmt, 
		(SQLUSMALLINT) pos + 1, 
		SQL_PARAM_INPUT, 
		cDataType, 
		Utility::sqlDataType(cDataType), 
		colSize,
		decDigits,
		0, 
		0, 
		_lengthIndicator.back())))
	{
		throw StatementException(_rStmt, "SQLBindParameter()");
	}
}


std::size_t Binder::parameterSize(SQLPOINTER pAddr) const
{
	ParamMap::const_iterator it = _inParams.find(pAddr);
	if (it != _inParams.end()) return it->second;

	it = _outParams.find(pAddr);
	if (it != _outParams.end()) return it->second;
	
	throw NotFoundException("Requested data size not found.");
}


void Binder::bind(std::size_t pos, const char* const &pVal, Direction dir)
{
	throw NotImplementedException("char* binding not implemented, Use std::string instead.");
}


SQLSMALLINT Binder::toODBCDirection(Direction dir) const
{
	bool in = isInBound(dir);
	bool out = isOutBound(dir);
	SQLSMALLINT ioType = SQL_PARAM_TYPE_UNKNOWN;
	if (in && out) ioType = SQL_PARAM_INPUT_OUTPUT; 
	else if(in)    ioType = SQL_PARAM_INPUT;
	else if(out)   ioType = SQL_PARAM_OUTPUT;
	else throw Poco::IllegalStateException("Binder not bound (must be [in] OR [out]).");

	return ioType;
}


void Binder::synchronize()
{
	TimestampMap::iterator itTS = _timestamps.begin();
	TimestampMap::iterator itTSEnd = _timestamps.end();
	for(; itTS != itTSEnd; ++itTS) 
		Utility::dateTimeSync(*itTS->second, *itTS->first);

	StringMap::iterator itStr = _strings.begin();
	StringMap::iterator itStrEnd = _strings.end();
	for(; itStr != itStrEnd; ++itStr)
		itStr->second->assign(itStr->first, strlen(itStr->first));
}


} } } // namespace Poco::Data::ODBC
