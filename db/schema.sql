/* Students */
CREATE TABLE student (
	stdt_id MEDIUMINT UNSIGNED NOT NULL AUTO_INCREMENT,
	oen VARCHAR(9) NOT NULL, /* Ontario education number */
	last_name VARCHAR(64) NOT NULL,
	first_name VARCHAR(64) NOT NULL,
	pref_name VARCHAR(64), /* Preferred name (if applicable) */
	start_year YEAR NOT NULL, /* Start of highschool career. If they start in 2019-2020 school year, put 2019. */
	grad_year YEAR NOT NULL, /* End of highschool career. If they graduate in 2022-2023 school year, put 2023. */

	PRIMARY KEY (stdt_id)
);


/* Athletics ("Red W") */
CREATE TABLE athletic ( 
	athl_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL, /* Point value of athletic */

	PRIMARY KEY (athl_id)
);

/* Academics ("White W")*/
CREATE TABLE academic (
	acdm_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL, /* Point value of academic */

	PRIMARY KEY (acdm_id)
);

/* Activities ("Blue W")*/
CREATE TABLE activity (
	actv_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL, /* Point value of activity */

	PRIMARY KEY (actv_id)
);


/* Student taking athletics for a specific year */
CREATE TABLE student_athletic (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	athl_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */

	UNIQUE (stdt_id, athl_id, start_year)
);

/* Student achieving academics for a specific year */
CREATE TABLE student_academic (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	acdm_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */

	UNIQUE (stdt_id, acdm_id, start_year)
);

/* Student partaking in activities for a specific year */
CREATE TABLE student_activity (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	actv_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */

	UNIQUE (stdt_id, actv_id, start_year)
);