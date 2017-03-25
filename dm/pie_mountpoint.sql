CREATE TABLE pie_mountpoint(
        mnt_hst_id BIGINT NOT NULL,
        mnt_stg_id BIGINT NOT NULL,
        mnt_path VARCHAR(128) NOT NULL,
        PRIMARY KEY(mnt_hst_id, mnt_stg_id),
        CONSTRAINT fk_mnt_hst FOREIGN KEY (mnt_hst_id)
                REFERENCES pie_host(hst_id)
                ON UPDATE CASCADE
                ON DELETE RESTRICT,
        CONSTRAINT fk_mnt_stg FOREIGN KEY (mnt_stg_id)
                REFERENCES pie_storage(stg_id)
                ON UPDATE CASCADE
                ON DELETE RESTRICT);
