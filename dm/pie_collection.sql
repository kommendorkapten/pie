CREATE TABLE pie_collection(
       col_id INTEGER NOT NULL,
       col_path VARCHAR(255) NOT NULL,
       col_usr_id INTEGER NOT NULL,
       col_grp_id INTEGER NOT NULL,
       col_acl INTEGER NOT NULL,
       PRIMARY KEY(col_id),
       CONSTRAINT uc_col_path UNIQUE(col_path));
CREATE INDEX ic_col_path on pie_collection(col_path);
