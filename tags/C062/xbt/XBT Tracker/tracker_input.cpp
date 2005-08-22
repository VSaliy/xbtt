// tracker_input.cpp: implementation of the Ctracker_input class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tracker_input.h"

#include "bt_misc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Ctracker_input::Ctracker_input()
{
	m_downloaded = 0;
	m_event = e_none;
	m_ipa = 0;
	m_left = 0;
	m_port = 0;
	m_uploaded = 0;
	m_num_want = -1;
	m_compact = false;
	m_no_peer_id = false;
}

void Ctracker_input::set(const string& name, const string& value)
{
	if (name.empty())
		return;
	switch (name[0])
	{
	case 'c':
		if (name == "compact")
			m_compact = atoi(value.c_str());
		break;
	case 'd':
		if (name == "downloaded")
			m_downloaded = atoll(value.c_str());
		break;
	case 'e':
		if (name == "event")
		{
			if (value == "completed")
				m_event = e_completed;
			else if (value == "started")
				m_event = e_started;
			else if (value == "stopped")
				m_event = e_stopped;
			else
				m_event = e_none;
		}
		break;
	case 'i':
		if (name == "info_hash" && value.length() == 20)
			m_info_hash = value;
		else if (name == "ip")
			m_ipa = inet_addr(value.c_str());
		break;
	case 'k':
		if (name == "key")
			m_key = value.substr(0, 8);
		break;
	case 'l':
		if (name == "left")
			m_left = atoll(value.c_str());
		break;
	case 'n':
		if (name == "no_peer_id")
			m_no_peer_id = atoi(value.c_str());
		else if (name == "numwant")
			m_num_want = atoi(value.c_str());
		break;
	case 'p':
		if (name == "peer_id" && value.length() == 20)
			m_peer_id = value;
		else if (name == "port")
			m_port = htons(atoi(value.c_str()));
		break;
	case 'u':
		if (name == "uploaded")
			m_uploaded = atoll(value.c_str());
		break;
	}
}

bool Ctracker_input::valid() const
{
	return m_downloaded >= 0
		&& (m_event != e_completed || !m_left)
		&& m_info_hash.length() == 20
		&& m_left >= -1
		&& m_peer_id.length() == 20
		&& m_port >= 0
		&& m_uploaded >= 0;
}
