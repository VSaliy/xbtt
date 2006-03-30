#include "stdafx.h"
#include "config.h"

Cconfig::Cconfig()
{
	m_announce_interval = 1800;
	m_anonymous_connect = true;
	m_anonymous_announce = true;
	m_anonymous_scrape = true;
	m_auto_register = true;
	m_clean_up_interval = 60;
	m_daemon = true;
	m_debug = false;
	m_gzip_announce = true;
	m_gzip_debug = true;
	m_gzip_scrape = true;
	m_listen_check = false;
	m_listen_ipas.clear();
	m_listen_ports.clear();
	m_log_access = false;
	m_log_announce = false;
	m_log_scrape = false;
	m_pid_file = "xbt_tracker.pid";
	m_read_config_interval = 60;
	m_read_db_interval = 60;
	m_redirect_url.erase();
	m_scrape_interval = 0;
	m_write_db_interval = 60;
}

void Cconfig::set(const string& name, const string& value)
{
	struct t_attribute
	{
		const char* key;
		string* value;
	} attributes[] =
	{
		"column_files_completed", &m_column_files_completed,
		"column_files_fid", &m_column_files_fid, 
		"column_files_leechers", &m_column_files_leechers, 
		"column_files_seeders", &m_column_files_seeders, 
		"column_users_uid", &m_column_users_uid, 
		"pid_file", &m_pid_file,
		"redirect_url", &m_redirect_url,
		"offline_message", &m_offline_message,
		"table_announce_log", &m_table_announce_log,
		"table_deny_from_hosts", &m_table_deny_from_hosts,
		"table_files", &m_table_files,
		"table_files_users", &m_table_files_users,
		"table_scrape_log", &m_table_scrape_log,
		"table_users", &m_table_users,
		NULL
	};
	t_attribute* i = attributes; 
	while (i->key && i->key != name)
		i++;
	if (i->key)
		*i->value = value;
	else if (name == "listen_ipa" && value != "*")
		m_listen_ipas.insert(inet_addr(value.c_str()));
	else
		set(name, atoi(value.c_str()));
}

void Cconfig::set(const string& name, int value)
{
	struct t_attribute
	{
		const char* key;
		int* value;
	} attributes[] =
	{
		"announce_interval", &m_announce_interval,
		"clean_up_interval", &m_clean_up_interval,
		"read_config_interval", &m_read_config_interval,
		"read_db_interval", &m_read_db_interval,
		"scrape_interval", &m_scrape_interval,
		"write_db_interval", &m_write_db_interval,
		NULL
	};
	t_attribute* i = attributes; 
	while (i->key && i->key != name)
		i++;
	if (i->key)
		*i->value = value;
	else if (name == "listen_port")
		m_listen_ports.insert(value);
	else
		set(name, static_cast<bool>(value));
}

void Cconfig::set(const string& name, bool value)
{
	struct t_attribute
	{
		const char* key;
		bool* value;
	} attributes[] =
	{
		"auto_register", &m_auto_register,
		"anonymous_connect", &m_anonymous_connect,
		"anonymous_announce", &m_anonymous_announce,
		"anonymous_scrape", &m_anonymous_scrape,
		"daemon", &m_daemon,
		"debug", &m_debug,
		"gzip_announce", &m_gzip_announce,
		"gzip_debug", &m_gzip_debug,
		"gzip_scrape", &m_gzip_scrape,
		"listen_check", &m_listen_check,
		"log_access", &m_log_access,
		"log_announce", &m_log_announce,
		"log_scrape", &m_log_scrape,
		NULL
	};
	t_attribute* i = attributes; 
	while (i->key && i->key != name)
		i++;
	if (i->key)
		*i->value = value;
}
