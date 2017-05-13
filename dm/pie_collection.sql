CREATE TABLE pie_collection(
       col_id INTEGER NOT NULL,
       col_name VARCHAR(64),
       col_path VARCHAR(255) NOT NULL,
       hst_usr_id INTEGER NOT NULL,
       hst_grp_id INTEGER NOT NULL,
       hst_acl INTEGER NOT NULL,
       PRIMARY KEY(col_id),
       CONSTRAINT uc_col_name UNIQUE(col_name),
       CONSTRAINT uc_col_path UNIQUE(col_path));
CREATE INDEX ic_col_path on pie_collection(col_path);
