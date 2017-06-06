CREATE TABLE pie_mob (
       mob_id BIGINT NOT NULL,
       mob_parent_mob_id BIGINT,
       mob_name VARCHAR(64),
       mob_capture_ts_millis BIGINT,
       mob_added_ts_millis BIGINT,
       mob_format SMALLINT,
       mob_color TINYINT,
       mob_rating TINYINT,
       PRIMARY KEY(mob_id)
)

CREATE INDEX ic_mob_cap on pie_collection(mob_capture_ts_millis);
CREATE INDEX ic_mob_add on pie_collection(mob_added_ts_millis);
CREATE INDEX ic_mob_par on pie_collection(mob_parent_mob_id);

