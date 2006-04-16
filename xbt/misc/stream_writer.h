#if !defined(AFX_STREAM_WRITER_H__283B8C8E_68DD_4E16_9122_42ADD010E634__INCLUDED_)
#define AFX_STREAM_WRITER_H__283B8C8E_68DD_4E16_9122_42ADD010E634__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stream_int.h"
#include "virtual_binary.h"

class Cstream_writer
{
public:
	byte* w() const
	{
		return m_w;
	}

	byte* write(int size)
	{
		m_w += size;
		return m_w - size;
	}

	void write_int(int cb, long long v)
	{
		m_w = ::write_int(cb, m_w, v);
	}

	void write_data(const Cvirtual_binary& v)
	{
		write_int(4, v.size());
		v.read(write(v.size()));
	}

	void write_string(const string& v)
	{
		write_int(4, v.length());
		memcpy(write(v.length()), v.c_str(), v.length());
	}

	Cstream_writer()
	{
	}

	Cstream_writer(byte* w)
	{
		m_w = w;
	}
private:
	byte* m_w;
};

#endif // !defined(AFX_STREAM_WRITER_H__283B8C8E_68DD_4E16_9122_42ADD010E634__INCLUDED_)
