/* For detailed explantion of schematic, consult DOCUMENTATION.md */

DROP TABLE IF EXISTS compound_cnst;
DROP TABLE IF EXISTS basic_cnst;
DROP TABLE IF EXISTS cnst;
DROP TABLE IF EXISTS student_action;
DROP TABLE IF EXISTS action_archive;
DROP TABLE IF EXISTS action;
DROP TABLE IF EXISTS student;

DROP PROCEDURE IF EXISTS ACTN_SUM;
DROP PROCEDURE IF EXISTS YEAR_CNT;
DROP PROCEDURE IF EXISTS ACTN_FREQ;
DROP PROCEDURE IF EXISTS ACTN_CSTV;




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



/* Actions */
CREATE TABLE action (
	actn_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	type TINYINT(1) NOT NULL, /* 0 for Athletic, 1 for Academic, 2 for Activity */
	name VARCHAR(64) NOT NULL,
	points TINYINT UNSIGNED NOT NULL DEFAULT 0, /* Point value of action */

	PRIMARY KEY (actn_id),
	UNIQUE (name)
);

/* Archived actions */
CREATE TABLE action_archive (
	actn_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,

	PRIMARY KEY (actn_id),
	FOREIGN KEY (actn_id) REFERENCES action(actn_id)
);

/* Student participating in an action for a specific year */
CREATE TABLE student_action (
	stdt_id MEDIUMINT UNSIGNED NOT NULL,
	actn_id SMALLINT UNSIGNED NOT NULL,
	start_year YEAR NOT NULL, /* Start year of current school year. If the year is 2020-2021, put 2020 here */

	FOREIGN KEY (stdt_id) REFERENCES student(stdt_id),
	FOREIGN KEY (actn_id) REFERENCES action(actn_id),
	UNIQUE (stdt_id, actn_id, start_year)
);



/* Constraint information */
CREATE TABLE cnst (
	cnst_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARCHAR(64),
	description VARCHAR(64) NOT NULL,
	type TINYINT(1) NOT NULL, /* See DOCUMENTATION.md */
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
	FOREIGN KEY (cnst_id) REFERENCES cnst(cnst_id)
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


 DELIMITER //

/* 
 * Calculation for basic constraint type 1 (summing action points)
 * p_stdt_id: student id, corresponds to stdt_id in student table
 * p_actn_type: action type, corresponds to type in action table
 * p_mx: max yearly points, corresponds to _m_ from DOCUMENTATION
 */
CREATE PROCEDURE ACTN_SUM (IN p_stdt_id MEDIUMINT UNSIGNED, IN p_actn_type TINYINT(1), IN p_mx TINYINT UNSIGNED, OUT result TINYINT UNSIGNED)
BEGIN
	SELECT COALESCE(SUM(res.points), 0) INTO result
	FROM (
		SELECT LEAST(a.points, p_mx) points, s_a.actn_id
		FROM student_action s_a
		INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
		WHERE s_a.stdt_id=p_stdt_id
	) res;
END;
//

/*
 * Calculation for basic constraint type 2 (years in high school)
 * p_stdt_id: student id
 */
CREATE PROCEDURE YEAR_CNT (IN p_stdt_id MEDIUMINT UNSIGNED, OUT result TINYINT UNSIGNED)
BEGIN
	SELECT grad_year-start_year INTO result
	FROM student WHERE stdt_id=p_stdt_id LIMIT 1;
END;
//

/*
 * Calculation for basic constraint type 3 (frequency)
 * p_stdt_id: student id
 * p_actn_type: action type
 * p_actn_id: action id, corresponds to actn_id in action table
 */
CREATE PROCEDURE ACTN_FREQ (IN p_stdt_id MEDIUMINT UNSIGNED, IN p_actn_type TINYINT(1), IN p_actn_id SMALLINT UNSIGNED, OUT result TINYINT UNSIGNED)
BEGIN
	/*
	 * a. If only action id is given, count frequency of that specific action
	 * b. If both are given, count maximum frequency of any specific action (note: the action id is ignored)
	 * c. If only action type is given, count frequency of all actions of that type
	 * If none of those are satisfied, raise an exception
	 */
	IF p_actn_type IS NULL AND p_actn_id IS NOT NULL THEN
		SELECT COUNT(*) INTO result FROM student_action s_a WHERE s_a.stdt_id=p_stdt_id AND actn_id=p_actn_id;
	ELSEIF p_actn_type IS NOT NULL AND p_actn_id IS NOT NULL THEN
		SELECT MAX(freq) INTO result
		FROM (
			SELECT s_a.actn_id, COUNT(*) freq
			FROM student_action s_a
			INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
			WHERE s_a.stdt_id=p_stdt_id
			GROUP BY s_a.actn_id
		) res;
	ELSEIF p_actn_type IS NOT NULL THEN
		SELECT COUNT(*) INTO result
		FROM (
			SELECT s_a.actn_id
			FROM student_action s_a
			INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
			WHERE s_a.stdt_id=p_stdt_id
		) res;
	ELSE
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'At least one of action id or action type must be not null';
	END IF;
	SELECT COALESCE(result, 0) INTO result;
END;
//

/*
 * Calculation for basic constraint type 4 (consecutive actions)
 * p_stdt_id: student id
 * p_actn_type: action type
 * p_actn_id: action id
 */
CREATE PROCEDURE ACTN_CSTV (IN p_stdt_id MEDIUMINT UNSIGNED, IN p_actn_type TINYINT(1), IN p_actn_id SMALLINT UNSIGNED, OUT result TINYINT UNSIGNED)
BEGIN
	SELECT @last_lst := 2, @last_actn_id := 0, @last_yr := 0;
	/*
	 * a. If only action id is given, count maximum consecutive occurrence of that specific action
	 * b. If both are given, count maximum consecutive occurrence of any specific action (note: the action id is ignored)
	 * c. If only action type is given, count maximum consecutive occurrence of all actions of that type
	 * If none of those are satisfied, raise an exception
	 */
	IF p_actn_type IS NULL AND p_actn_id IS NOT NULL THEN
		SELECT MAX(cstv) INTO result
		FROM (
			SELECT (lst-@last_lst>0) valid, yr-@last_yr cstv, @last_lst := lst, @last_yr := yr
			FROM (
				SELECT MIN(sequence_combined.lst) lst, yr
				FROM (
					SELECT 1 lst, start_year yr
					FROM student_action s_a
					WHERE s_a.stdt_id=p_stdt_id AND actn_id=p_actn_id
					UNION ALL
					SELECT 2 lst, start_year+1 yr
					FROM (
						SELECT s_a.start_year
						FROM student_action s_a
						WHERE s_a.stdt_id=p_stdt_id AND actn_id=p_actn_id
					) sequence_copy
				) sequence_combined
				GROUP BY yr
				HAVING COUNT(*)=1
				ORDER BY yr
			) bounds
		) res
		WHERE valid;
	ELSEIF p_actn_type IS NOT NULL AND p_actn_id IS NOT NULL THEN
		SELECT MAX(cstv) INTO result
		FROM (
			SELECT (lst-@last_lst>0 AND actn_id=@last_actn_id) valid, IF(yr>@last_yr, yr-@last_yr, 0) cstv, @last_lst := lst, @last_actn_id := actn_id, @last_yr := yr
			FROM (
				SELECT MIN(sequence_combined.lst) lst, actn_id, yr
				FROM (
					SELECT 1 lst, s_a.actn_id, s_a.start_year yr
					FROM student_action s_a
					INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
					WHERE s_a.stdt_id=p_stdt_id
					UNION ALL
					SELECT 2 lst, actn_id, start_year+1 yr
					FROM (
						SELECT s_a.actn_id, s_a.start_year
						FROM student_action s_a
						INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
						WHERE s_a.stdt_id=p_stdt_id
					) sequence_copy
				) sequence_combined
				GROUP BY actn_id, yr
				HAVING COUNT(*)=1
				ORDER BY actn_id, yr
			) bounds
		) res
		WHERE valid;
	ELSEIF p_actn_type IS NOT NULL THEN
		SELECT MAX(cstv) INTO result
		FROM (
			SELECT (lst-@last_lst>0) valid, yr-@last_yr cstv, @last_lst := lst, @last_yr := yr
			FROM (
				SELECT MIN(sequence_combined.lst) lst, yr
				FROM (
					SELECT 1 lst, s_a.start_year yr
					FROM student_action s_a
					INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
					WHERE s_a.stdt_id=p_stdt_id
					GROUP BY s_a.start_year
					UNION ALL
					SELECT 2 lst, start_year+1 yr
					FROM (
						SELECT s_a.start_year
						FROM student_action s_a
						INNER JOIN action a ON a.actn_id=s_a.actn_id AND a.type=p_actn_type
						WHERE s_a.stdt_id=p_stdt_id
						GROUP BY s_a.start_year
					) sequence_copy
				) sequence_combined
				GROUP BY yr
				HAVING COUNT(*)=1
				ORDER BY yr
			) bounds
		) res
		WHERE valid;
	ELSE
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'At least one of action id or action type must be not null';
	END IF;
	SELECT COALESCE(result, 0) INTO result;
END;
//

DELIMITER ;
