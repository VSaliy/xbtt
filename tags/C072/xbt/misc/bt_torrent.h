#if !defined(AFX_BT_TORRENT_H__AF87B246_788C_42B3_BE1C_08679DFEFBA4__INCLUDED_)
#define AFX_BT_TORRENT_H__AF87B246_788C_42B3_BE1C_08679DFEFBA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bvalue.h"

class Cbt_torrent
{
public:
	class Cfile
	{
	public:
		const std::string& name() const
		{
			return m_name;
		}

		long long size() const
		{
			return m_size;
		}

		Cfile()
		{
		}

		Cfile(const std::string& name, long long size)
		{
			m_name = name;
			m_size = size;
		}
	private:
		std::string m_name;
		long long m_size;
	};

	typedef std::vector<std::string> t_announces;
	typedef std::vector<Cfile> t_files;

	long long size() const;
	bool valid() const;
	int write(const Cbvalue&);
	int write_info(const Cbvalue&);
	Cbt_torrent();
	Cbt_torrent(const Cbvalue&);

	const std::string& announce() const
	{
		return m_announce;
	}

	const t_announces& announces() const
	{
		return m_announces;
	}

	const t_files& files() const
	{
		return m_files;
	}

	const std::string& name() const
	{
		return m_name;
	}

	int piece_size() const
	{
		return m_piece_size;
	}
private:
	std::string m_announce;
	t_announces m_announces;
	t_files m_files;
	std::string m_name;
	int m_piece_size;
};

#endif // !defined(AFX_BT_TORRENT_H__AF87B246_788C_42B3_BE1C_08679DFEFBA4__INCLUDED_)
