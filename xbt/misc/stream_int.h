#if !defined(AFX_STREAM_INT_H__57F9BAC0_D02F_4067_9891_5C484F35B67F__INCLUDED_)
#define AFX_STREAM_INT_H__57F9BAC0_D02F_4067_9891_5C484F35B67F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template <class T>
long long read_int(int cb, const T r0)
{
	const unsigned char* r = reinterpret_cast<const unsigned char*>(r0);
	long long v = 0;
	while (cb--)
		v = v << 8 | *r++;
	return v;
}

template <class T>
long long read_int(int cb, const T r, const T s_end)
{
	return read_int(cb, r);
}

template <class T>
T write_int(int cb, T w0, long long v)
{
	unsigned char* w = reinterpret_cast<unsigned char*>(w0);
	w += cb;
	for (int i = 0; i < cb; i++)
	{
		*--w = v & 0xff;
		v >>= 8;
	}
	return reinterpret_cast<T>(w + cb);
}

template <class T>
long long read_int_le(int cb, const T r0)
{
	const unsigned char* r = reinterpret_cast<const unsigned char*>(r0);
	r += cb;
	long long v = 0;
	while (cb--)
		v = v << 8 | *--r;
	return v;
}

template <class T>
long long read_int_le(int cb, const T r, const T s_end)
{
	return read_int_le(cb, r);
}

template <class T>
T write_int_le(int cb, T w0, long long v)
{
	unsigned char* w = reinterpret_cast<unsigned char*>(w0);
	for (int i = 0; i < cb; i++)
	{
		*w++ = v & 0xff;
		v >>= 8;
	}
	return reinterpret_cast<T>(w);
}

#endif // !defined(AFX_STREAM_INT_H__57F9BAC0_D02F_4067_9891_5C484F35B67F__INCLUDED_)
