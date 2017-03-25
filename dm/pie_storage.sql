CREATE TABLE pie_storage(
        stg_id BIGINT NOT NULL,
        stg_name VARCHAR(64) NOT NULL,
        stg_type INTEGER NOT NULL,
        PRIMARY KEY(stg_id),
        CONSTRAINT uc_stg_name UNIQUE(stg_id));
