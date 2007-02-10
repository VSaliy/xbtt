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
) engine = myisam;

CREATE TABLE xbt_config
(
  name varchar(255) NOT NULL,
  value varchar(255) NOT NULL
);

CREATE TABLE xbt_deny_from_hosts
(
  begin int NOT NULL,
  end int NOT NULL
);

CREATE TABLE xbt_files
(
  fid int NOT NULL auto_increment,
  info_hash blob NOT NULL,
  leechers int NOT NULL default 0,
  seeders int NOT NULL default 0,
  completed int NOT NULL default 0,
  flags int NOT NULL default 0,
  mtime int NOT NULL,
  ctime int NOT NULL,
  PRIMARY KEY (fid),
  UNIQUE KEY (info_hash(20))
);

CREATE TABLE xbt_files_users
(
  fid int NOT NULL,
  uid int NOT NULL,
  active tinyint NOT NULL,
  announced int NOT NULL,
  completed int NOT NULL,
  downloaded bigint(20) NOT NULL,
  `left` bigint(20) NOT NULL,
  uploaded bigint(20) NOT NULL,
  mtime int NOT NULL,
  UNIQUE KEY (fid, uid),
  KEY (uid)
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
  can_leech tinyint NOT NULL default 1,
  wait_time int NOT NULL,
  peers_limit int NOT NULL,
  torrents_limit int NOT NULL,
  torrent_pass char(32) NOT NULL,
  torrent_pass_secret bigint NOT NULL,
  downloaded bigint NOT NULL,
  uploaded bigint NOT NULL,
  PRIMARY KEY (uid)
) engine = myisam;
