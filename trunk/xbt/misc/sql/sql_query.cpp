#include "stdafx.h"
#include "xbt/sql_query.h"

#include <cstdio>
#include <vector>
#include <xbt/find_ptr.h>
#include "xbt/database.h"

Csql_query::Csql_query(Cdatabase& database, const std::string& v):
	m_database(database)
{
	m_in = v;
}

Csql_result Csql_query::execute() const
{
	return m_database.query(read());
}

std::string Csql_query::read() const
{
	return m_out + m_in;
}

void Csql_query::operator=(const std::string& v)
{
	m_in = v;
	m_out.clear();
}

void Csql_query::operator+=(const std::string& v)
{
	m_in += v;
}

Csql_query& Csql_query::p_name(const std::string& v0)
{
	const std::string& v = m_database.name(v0);
	std::vector<char> r(2 * v.size() + 2);
	r.resize(mysql_real_escape_string(m_database.handle(), &r.front() + 1, v.data(), v.size()) + 2);
	r.front() = '`';
	r.back() = '`';
	p_raw(r);
	return *this;
}

Csql_query& Csql_query::p_raw(const_memory_range v)
{
	size_t i = m_in.find('?');
	assert(i != std::string::npos);
	if (i == std::string::npos)
		return *this;
	m_out.append(m_in.data(), i);
	m_in.erase(0, i + 1);
	m_out.append(v.begin, v.end);
	return *this;
}

Csql_query& Csql_query::p(long long v)
{
	char b[21];
#ifdef WIN32
	sprintf(b, "%I64d", v);
#else
	sprintf(b, "%lld", v);
#endif
	p_raw(const_memory_range(b));
	return *this;
}

Csql_query& Csql_query::p(const_memory_range v)
{
	std::vector<char> r(2 * v.size() + 2);
	r.resize(mysql_real_escape_string(m_database.handle(), &r.front() + 1, reinterpret_cast<const char*>(v.begin), v.size()) + 2);
	r.front() = '\'';
	r.back() = '\'';
	p_raw(r);
	return *this;
}
