CREATE TABLE IF NOT EXISTS t_user (
    f_id bigint(20) NOT NULL AUTO_INCREMENT,
    f_user_id bigint(20) NOT NULL,
    f_username varchar(64) NOT NULL,
    f_nickname varchar(64) NOT NULL,
    f_password varchar(64) DEFAULT NULL,
    f_facetype int(10) DEFAULT 0,
    f_customavatar varchar(32) DEFAULT NULL,
    f_customavatarfmt varchar(6) DEFAULT NULL,
    f_gender int(2) DEFAULT 0,
    f_birthday bigint(20) DEFAULT 20000101,
    f_signature varchar(256) DEFAULT NULL,
    f_address varchar(256) DEFAULT NULL,
    f_phonenumber varchar(64) DEFAULT NULL,
    f_mail varchar(256) DEFAULT NULL,
    f_register_time datetime NOT NULL,
    f_owner_id bigint(20) DEFAULT 0,
    f_remark varchar(64) DEFAULT NULL,
    f_update_time datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    f_teaminfo blob DEFAULT NULL,       
    PRIMARY KEY (f_user_id),
    INDEX f_user_id (f_user_id),
    KEY f_id (f_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS t_user_relationship (
    f_id bigint(20) NOT NULL AUTO_INCREMENT,
    f_user_id1 bigint(20) NOT NULL,
    f_user_id2 bigint(20) NOT NULL,
    f_user1_teamname varchar(32) NOT NULL DEFAULT 'My Friends',
    f_user1_markname varchar(32),
    f_user2_teamname varchar(32) NOT NULL DEFAULT 'My Friends',
    f_user2_markname varchar(32),
    f_update_time datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    f_remark varchar(64) DEFAULT NULL,
    PRIMARY KEY (f_id),
    KEY f_id (f_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS t_chatmsg (
    f_id bigint(20) NOT NULL AUTO_INCREMENT,
    f_senderid bigint(20) NOT NULL,
    f_targetid bigint(20) NOT NULL,
    f_msgcontent blob NOT NULL,
    f_create_time datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    f_remark varchar(64) DEFAULT NULL,
    PRIMARY KEY (f_id),
    KEY f_id (f_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
