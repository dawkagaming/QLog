ALTER TABLE rig_profiles ADD COLUMN share_rigctld INTEGER DEFAULT 0;
ALTER TABLE rig_profiles ADD COLUMN rigctld_port INTEGER DEFAULT 4532;
ALTER TABLE rig_profiles ADD COLUMN rigctld_path TEXT;
ALTER TABLE rig_profiles ADD COLUMN rigctld_args TEXT;
ALTER TABLE contacts_qsl_cards ADD COLUMN favorite INTEGER DEFAULT 0;

ALTER TABLE cwkey_profiles ADD paddle_only_sidetone INTEGER DEFAULT 0;
ALTER TABLE cwkey_profiles ADD sidetone_frequency INTEGER DEFAULT 800;

INSERT INTO qso_filter_operators VALUES(7, "regexp");
