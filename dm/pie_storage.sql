CREATE TABLE pie_storage(
        stg_id INTEGER NOT NULL,
        stg_name VARCHAR(64) NOT NULL,
        stg_type INTEGER NOT NULL,
        stg_hst_id INTEGER NOT NULL,
        PRIMARY KEY(stg_id),
        CONSTRAINT uc_stg_name UNIQUE(stg_name),
        CONSTRAINT fk_stg_hst FOREIGN KEY (stg_hst_id)
                REFERENCES pie_host(hst_id)
                ON UPDATE CASCADE
                ON DELETE RESTRICT);        
