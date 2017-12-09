CREATE TABLE pie_min (
       min_id BIGINT NOT NULL,
       min_mob_id BIGINT NOT NULL,
       min_added_ts_millis BIGINT,
       min_stg_id INTEGER NOT NULL,
       min_size BIGINT NOT NULL,
       min_path VARCHAR(257),
       min_sha1_hash VARCHAR(129),
       PRIMARY KEY (min_id)
);

CREATE INDEX im_min_mob on pie_min(min_mob_id);
CREATE INDEX im_min_stg on pie_min(min_stg_id);
CREATE INDEX im_min_hsh on pie_min(min_sha1_hash);
