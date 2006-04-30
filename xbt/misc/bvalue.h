#if !defined(AFX_BVALUE_H__AE7DA755_2638_4709_8C54_495AA3840EFB__INCLUDED_)
#define AFX_BVALUE_H__AE7DA755_2638_4709_8C54_495AA3840EFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <string>
#include <vector>
#include "virtual_binary.h"

class Cbvalue
{
public:
	enum t_value_type
	{
		vt_int,
		vt_string,
		vt_list,
		vt_dictionary,
	};

	typedef std::map<std::string, Cbvalue> t_map;
	typedef std::vector<Cbvalue> t_list;

	void clear();
	const t_map& d() const;
	const Cbvalue& d(const std::string&) const;
	const t_list& l() const;
	long long i() const;
	const std::string& s() const;
	bool d_has(const std::string&) const;
	Cbvalue& d(const std::string& v, const Cbvalue& w);
	Cbvalue& l(const Cbvalue& v);
	int pre_read() const;
	int read(char* d) const;
	int read(void* d) const;
	Cvirtual_binary read() const;
	int write(const char* s, int cb_s);
	int write(const void* s, int cb_s);
	int write(const Cvirtual_binary&);
	Cbvalue(long long v = 0);
	Cbvalue(t_value_type t);
	Cbvalue(const std::string& v);
	Cbvalue(const Cbvalue&);
	Cbvalue(const void* s, int cb_s);
	Cbvalue(const Cvirtual_binary&);
	const Cbvalue& operator=(const Cbvalue&);
	~Cbvalue();
private:
	t_value_type m_value_type;

	union
	{
		long long m_int;
		std::string* m_string;
		t_list* m_list;
		t_map* m_map;
	};

	int write(const char*& s, const char* s_end);
};

#endif // !defined(AFX_BVALUE_H__AE7DA755_2638_4709_8C54_495AA3840EFB__INCLUDED_)
