CREATE TABLE pie_host(
        hst_id INTEGER NOT NULL, 
        hst_name VARCHAR(64) NOT NULL,
        hst_fqdn VARCHAR(64) NOT NULL,
        PRIMARY KEY(hst_id),
        CONSTRAINT uc_hst_name UNIQUE(hst_name));
   
        
