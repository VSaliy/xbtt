CREATE TABLE xbt_announce_log
(
  id int NOT NULL auto_increment,
  ipa int unsigned NOT NULL,
  port int NOT NULL,
  event int NOT NULL,
  info_hash blob NOT NULL,
  peer_id blob NOT NULL,
  downloaded bigint NOT NULL,
  left0 bigint NOT NULL,
  uploaded bigint NOT NULL,
  uid int NOT NULL,
  mtime int NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE xbt_config
(
  name varchar(255) NOT NULL,
  value varchar(255) NOT NULL
);

CREATE TABLE xbt_files
(
  fid int NOT NULL auto_increment,
  info_hash blob NOT NULL,
  leechers int NOT NULL,
  seeders int NOT NULL,
  announced_http int NOT NULL,
  announced_http_compact int NOT NULL,
  announced_http_no_peer_id int NOT NULL,
  announced_udp int NOT NULL,
  scraped_http int NOT NULL,
  scraped_udp int NOT NULL,
  completed int NOT NULL,
  started int NOT NULL,
  stopped int NOT NULL,
  flags int NOT NULL,
  mtime timestamp NOT NULL,
  ctime timestamp NOT NULL,
  PRIMARY KEY (fid),
  UNIQUE KEY (info_hash(20))
);

CREATE TABLE xbt_files_updates
(
  fid int NOT NULL,
  leechers int NOT NULL,
  seeders int NOT NULL,
  completed int NOT NULL,
  started int NOT NULL,
  stopped int NOT NULL,
  announced_http int NOT NULL,
  announced_http_compact int NOT NULL,
  announced_http_no_peer_id int NOT NULL,
  announced_udp int NOT NULL,
  scraped_http int NOT NULL,
  scraped_udp int NOT NULL
);

CREATE TABLE xbt_files_users
(
  info_hash blob NOT NULL,
  uid int NOT NULL,
  announced int NOT NULL,
  completed int NOT NULL,
  downloaded bigint(20) NOT NULL,
  `left` bigint(20) NOT NULL,
  uploaded bigint(20) NOT NULL,
  mtime timestamp NOT NULL,
  UNIQUE KEY (info_hash(20), uid),
  KEY (uid)
);

CREATE TABLE xbt_ipas
(
  ipa int NOT NULL,
  uid int NOT NULL,
  mtime timestamp NOT NULL,
  PRIMARY KEY (ipa)
);

CREATE TABLE xbt_scrape_log
(
  id int NOT NULL auto_increment,
  ipa int NOT NULL,
  info_hash blob,
  uid int NOT NULL,
  mtime int NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE xbt_users
(
  uid int NOT NULL auto_increment,
  name char(8) NOT NULL,
  pass blob NOT NULL,
  fid_end int NOT NULL,
  peers_limit int NOT NULL,
  torrents_limit int NOT NULL,
  torrent_pass char(32) NOT NULL,
  downloaded bigint NOT NULL,
  uploaded bigint NOT NULL,
  PRIMARY KEY (uid)
);
