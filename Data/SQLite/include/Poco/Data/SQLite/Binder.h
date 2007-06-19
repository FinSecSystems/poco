//
// Binder.h
//
// $Id: //poco/Main/Data/SQLite/include/Poco/Data/SQLite/Binder.h#4 $
//
// Library: SQLite
// Package: SQLite
// Module:  Binder
//
// Definition of the Binder class.
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


#ifndef DataConnectors_SQLite_Binder_INCLUDED
#define DataConnectors_SQLite_Binder_INCLUDED


#include "Poco/Data/SQLite/SQLite.h"
#include "Poco/Data/AbstractBinder.h"
#include "Poco/Any.h"


struct sqlite3_stmt;


namespace Poco {
namespace Data {
namespace SQLite {


class SQLite_API Binder: public Poco::Data::AbstractBinder
	/// Binds placeholders in the sql query to the provided values. Performs data types mapping.
{
public:
	Binder(sqlite3_stmt* pStmt);
		/// Creates the Binder.

	~Binder();
		/// Destroys the Binder.

	void bind(std::size_t pos, const Poco::Int8 &val, Direction dir = PD_IN);
		/// Binds an Int8.

	void bind(std::size_t pos, const Poco::UInt8 &val, Direction dir = PD_IN);
		/// Binds an UInt8.

	void bind(std::size_t pos, const Poco::Int16 &val, Direction dir = PD_IN);
		/// Binds an Int16.

	void bind(std::size_t pos, const Poco::UInt16 &val, Direction dir = PD_IN);
		/// Binds an UInt16.

	void bind(std::size_t pos, const Poco::Int32 &val, Direction dir = PD_IN);
		/// Binds an Int32.

	void bind(std::size_t pos, const Poco::UInt32 &val, Direction dir = PD_IN);
		/// Binds an UInt32.

	void bind(std::size_t pos, const Poco::Int64 &val, Direction dir = PD_IN);
		/// Binds an Int64.

	void bind(std::size_t pos, const Poco::UInt64 &val, Direction dir = PD_IN);
		/// Binds an UInt64.

	void bind(std::size_t pos, const bool &val, Direction dir = PD_IN);
		/// Binds a boolean.

	void bind(std::size_t pos, const float &val, Direction dir = PD_IN);
		/// Binds a float.

	void bind(std::size_t pos, const double &val, Direction dir = PD_IN);
		/// Binds a double.

	void bind(std::size_t pos, const char &val, Direction dir = PD_IN);
		/// Binds a single character.

	void bind(std::size_t pos, const char* const &pVal, Direction dir = PD_IN);
		/// Binds a const char ptr.

	void bind(std::size_t pos, const std::string& val, Direction dir = PD_IN);
		/// Binds a string.

	void bind(std::size_t pos, const Poco::Data::BLOB& val, Direction dir = PD_IN);
		/// Binds a BLOB.

	void bind(std::size_t pos, const DateTime& val, Direction dir = PD_IN);
		/// Binds a DateTime.

	void bind(std::size_t pos, const NullData& val, Direction dir = PD_IN);
		/// Binds a null.

private:
	void checkReturn(int rc);
		/// Checks the SQLite return code and throws an appropriate exception
		/// if error has occurred.

	sqlite3_stmt* _pStmt;
};


//
// inlines
//
inline void Binder::bind(std::size_t pos, const Poco::Int8 &val, Direction dir)
{
	Poco::Int32 tmp = val;
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt8 &val, Direction dir)
{
	Poco::Int32 tmp = val;
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::Int16 &val, Direction dir)
{
	Poco::Int32 tmp = val;
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt16 &val, Direction dir)
{
	Poco::Int32 tmp = val;
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt32 &val, Direction dir)
{
	Poco::Int32 tmp = static_cast<Poco::Int32>(val);
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt64 &val, Direction dir)
{
	Poco::Int64 tmp = static_cast<Poco::Int64>(val);
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const bool &val, Direction dir)
{
	Poco::Int32 tmp = (val ? 1 : 0);
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const float &val, Direction dir)
{
	double tmp = val;
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const char &val, Direction dir)
{
	Poco::Int32 tmp = val;
	bind(pos, tmp, dir);
}


inline void Binder::bind(std::size_t pos, const char* const &pVal, Direction dir)
{
	std::string val(pVal);
	bind(pos, val, dir);
}


} } } // namespace Poco::Data::SQLite


#endif // DataConnectors_SQLite_Binder_INCLUDED
