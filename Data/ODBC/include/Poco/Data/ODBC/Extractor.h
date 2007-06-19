//
// Extractor.h
//
// $Id: //poco/Main/Data/ODBC/include/Poco/Data/ODBC/Extractor.h#5 $
//
// Library: ODBC
// Package: ODBC
// Module:  Extractor
//
// Definition of the Extractor class.
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


#ifndef DataConnectors_ODBC_Extractor_INCLUDED
#define DataConnectors_ODBC_Extractor_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include "Poco/Data/AbstractExtractor.h"
#include "Poco/Data/ODBC/Preparation.h"
#include "Poco/Data/ODBC/ODBCColumn.h"
#include "Poco/Data/ODBC/Error.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/DateTime.h"
#include "Poco/Any.h"
#include "Poco/Exception.h"
#include <map>
#ifdef POCO_OS_FAMILY_WINDOWS
#include <windows.h>
#endif
#include <sqltypes.h>


namespace Poco {
namespace Data {
namespace ODBC {


class ODBC_API Extractor: public Poco::Data::AbstractExtractor
	/// Extracts and converts data values from the result row returned by ODBC.
	/// If NULL is received, the incoming val value is not changed and false is returned
{
public:
	Extractor(const StatementHandle& rStmt, 
		Preparation& rPreparation);
		/// Creates the Extractor.

	~Extractor();
		/// Destroys the Extractor.

	bool extract(std::size_t pos, Poco::Int8& val);
		/// Extracts an Int8.

	bool extract(std::size_t pos, Poco::UInt8& val);
		/// Extracts an UInt8.

	bool extract(std::size_t pos, Poco::Int16& val);
		/// Extracts an Int16.

	bool extract(std::size_t pos, Poco::UInt16& val);
		/// Extracts an UInt16.

	bool extract(std::size_t pos, Poco::Int32& val);
		/// Extracts an Int32.

	bool extract(std::size_t pos, Poco::UInt32& val);
		/// Extracts an UInt32.

	bool extract(std::size_t pos, Poco::Int64& val);
		/// Extracts an Int64.

	bool extract(std::size_t pos, Poco::UInt64& val);
		/// Extracts an UInt64.

	bool extract(std::size_t pos, bool& val);
		/// Extracts a boolean.

	bool extract(std::size_t pos, float& val);
		/// Extracts a float.

	bool extract(std::size_t pos, double& val);
		/// Extracts a double.

	bool extract(std::size_t pos, char& val);
		/// Extracts a single character.

	bool extract(std::size_t pos, std::string& val);
		/// Extracts a string.

	bool extract(std::size_t pos, Poco::Data::BLOB& val);
		/// Extracts a BLOB.

	bool extract(std::size_t pos, Poco::DateTime& val);
		/// Extracts a DateTime.
	
	bool extract(std::size_t pos, Poco::Any& val);
		/// Extracts an Any.

	void setDataExtraction(Preparation::DataExtraction ext);
		/// Set data extraction mode.

	Preparation::DataExtraction getDataExtraction() const;
		/// Returns data extraction mode.

	bool isNull(std::size_t pos);
		/// Returns true if the current row value at pos column is null.

private:
	static const int CHUNK_SIZE = 1024;
		/// Amount of data retrieved in one SQLGetData() request when doing manual extract.

	static const std::string FLD_SIZE_EXCEEDED_FMT;
		/// String format for the exception message when the field size is exceeded.

	void checkDataSize(std::size_t size);
		/// This check is only performed for bound data
		/// retrieval from variable length columns.
		/// The reason for this check is to ensure we can
		/// accept the value ODBC driver is supplying
		/// (i.e. the bound buffer is large enough to receive
		/// the returned value)

	template<typename T>
	bool extractBoundImpl(std::size_t pos, T& val)
	{
		poco_assert (typeid(T) == _rPreparation[pos].type());
		val = *AnyCast<T>(&_rPreparation[pos]); 
		return true;
	}

	template<typename T>
	bool extractManualImpl(std::size_t pos, T& val, SQLSMALLINT cType)
	{
		SQLRETURN rc = 0;
		SQLLEN len = 0;
		T value = (T) 0;
		
		rc = SQLGetData(_rStmt, 
			(SQLUSMALLINT) pos + 1, 
			cType,  //C data type
			&value, //returned value
			0,      //buffer length (ignored)
			&len);  //length indicator

		//for fixed-length data, buffer must be large enough
		//otherwise, driver may write past the end
		poco_assert_dbg (len <= sizeof(T));

		if (Utility::isError(rc))
			throw StatementException(_rStmt, "SQLGetData()");
		
		if (SQL_NULL_DATA == len) val = (T) 0;
		else val = value;

		return true;
	}

	const StatementHandle& _rStmt;
	Preparation& _rPreparation;
	Preparation::DataExtraction _dataExtraction;
};


///
/// inlines
///


inline void Extractor::setDataExtraction(Preparation::DataExtraction ext)
{
	_rPreparation.setDataExtraction(_dataExtraction = ext);
}


inline Preparation::DataExtraction Extractor::getDataExtraction() const
{
	return _dataExtraction;
}


inline bool Extractor::isNull(std::size_t pos)
{
	throw NotImplementedException("TODO");
}


} } } // namespace Poco::Data::ODBC


#endif // DataConnectors_ODBC_Extractor_INCLUDED
