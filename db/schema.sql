/* For detailed explantion of schematic, consult DOCUMENTATION.md */

DROP TABLE IF EXISTS student;
DROP TABLE IF EXISTS athletic;
DROP TABLE IF EXISTS academic;
DROP TABLE IF EXISTS activity;
DROP TABLE IF EXISTS student_athletic;
DROP TABLE IF EXISTS student_academic;
DROP TABLE IF EXISTS student_activity;
DROP TABLE IF EXISTS cnst;
DROP TABLE IF EXISTS basic_cnst;
DROP TABLE IF EXISTS compound_cnst;



/*
 * Tables
 */



/* Students */
CREATE TABLE student (
	stdt_id MEDIUMINT UNSIGNED NOT NULL AUTO_INCREMENT,
	oen VARCHAR(9) NOT NULL, /* Ontario education number */
	last_name VARCHAR(64) NOT NULL,
	first_name VARCHAR(64) NOT NULL,
	pref_name VARCHAR(64), /* Preferred name (if applicable) */
	start_year YEAR NOT NULL, /* Start of highschool career. e.g. if they start in 2019-2020 school year, put 2019. */
	grad_year YEAR NOT NULL, /* End of highschool career. e.g. if they graduate in 2022-2023 school year, put 2023. */

	PRIMARY KEY (stdt_id)
);



/* Athletics ("Red W") */
CREATE TABLE athletic ( 
	athl_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL DEFAULT 0, /* Point value of athletic */

	PRIMARY KEY (athl_id)
);

/* Academics ("White W")*/
CREATE TABLE academic (
	acdm_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL DEFAULT 0, /* Point value of academic */

	PRIMARY KEY (acdm_id)
);

/* Activities ("Blue W")*/
CREATE TABLE activity (
	actv_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL DEFAULT 0, /* Point value of activity */

	PRIMARY KEY (actv_id)
);



/* Student taking athletics for a specific year */
CREATE TABLE student_athletic (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	athl_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */

	FOREIGN KEY (stdt_id) REFERENCES student(stdt_id),
	FOREIGN KEY (athl_id) REFERENCES athletic(athl_id),
	UNIQUE (stdt_id, athl_id, start_year)
);

/* Student achieving academics for a specific year */
CREATE TABLE student_academic (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	acdm_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */
	
	FOREIGN KEY (stdt_id) REFERENCES student(stdt_id),
	FOREIGN KEY (acdm_id) REFERENCES academic(acdm_id),
	UNIQUE (stdt_id, acdm_id, start_year)
);

/* Student partaking in activities for a specific year */
CREATE TABLE student_activity (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	actv_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */

	FOREIGN KEY (stdt_id) REFERENCES student(stdt_id),
	FOREIGN KEY (actv_id) REFERENCES activity(actv_id),
	UNIQUE (stdt_id, actv_id, start_year)
);



/* Constraint information */
CREATE TABLE cnst (
	cnst_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	depth SMALLINT UNSIGNED NOT NULL DEFAULT 0,
	name VARCHAR(64),
	description VARCHAR(64) NOT NULL,
	type TINYINT(1) NOT NULL,
	is_award BOOLEAN NOT NULL,

	PRIMARY KEY (cnst_id),
	CHECK (type IN (0, 1, 2, 3, 4, 5)),
	CHECK (is_award=FALSE OR name IS NOT NULL)
);

/* Basic constraint parameter info */
CREATE TABLE basic_cnst (
	cnst_id SMALLINT UNSIGNED NOT NULL,
	actn_id SMALLINT UNSIGNED,
	actn_type TINYINT(1),
	mx TINYINT UNSIGNED, /* Max points */
	x TINYINT UNSIGNED NOT NULL, /* Interval start */
	y TINYINT UNSIGNED NOT NULL, /* Interval end */

	PRIMARY KEY (cnst_id),
	FOREIGN KEY (cnst_id) REFERENCES cnst(cnst_id),
	CHECK (actn_type IN (0, 1, 2))
);

/* Compound constraint edge list */
CREATE TABLE compound_cnst (
	sub_cnst SMALLINT UNSIGNED NOT NULL,
	super_cnst SMALLINT UNSIGNED NOT NULL,

	FOREIGN KEY (sub_cnst) REFERENCES cnst(cnst_id),
	FOREIGN KEY (super_cnst) REFERENCES cnst(cnst_id),
	UNIQUE (sub_cnst, super_cnst)
);



/*
 * Procedures
 */



/* Calculation for basic constraint type 1 (summing action points) */
CREATE PROCEDURE actn_sum (
	IN student MEDIUMINT UNSIGNED,
	IN actn_type TINYINT(1),
	IN mx TINYINT UNSIGNED,
	OUT result TINYINT
)
BEGIN
	SELECT COALESCE(SUM(points), 0) INTO result FROM (
		SELECT GREATEST(points, COALESCE(mx, 0)) AS points
		FROM (
			CASE actn_type
				WHEN 0 THEN athletic,
				WHEN 1 THEN academic,
				WHEN 2 THEN activity
		)
		WHERE stdt_id=student
	)
END;