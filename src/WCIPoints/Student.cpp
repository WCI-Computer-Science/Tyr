#include "pch.h"
#include "afxwinappex.h"

#include "Student.h"


// Add a student
void Student::add(sql::Connection* con, CString OEN, CString last_name, CString first_name, CString pref_name, int start_year, int grad_year) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
		"INSERT INTO student (oen, last_name, first_name, pref_name, start_year, grad_year) VALUES (?, ?, ?, ?, ?, ?)"
	));
	std::string std_OEN = CT2A(OEN), std_last_name = CT2A(last_name), std_first_name = CT2A(first_name), std_pref_name = CT2A(pref_name);

	pstmt->setString(1, std_OEN);
	pstmt->setString(2, std_last_name);
	pstmt->setString(3, std_first_name);
	if (pref_name.GetLength() > 0) // preferred name is optional
		pstmt->setString(4, std_pref_name);
	else
		pstmt->setNull(4, sql::DataType::VARCHAR); // sql::DataType technically does nothing in setNull source code, setting here anyway for clarity
	pstmt->setInt(5, start_year);
	pstmt->setInt(6, grad_year);

	pstmt->execute();
}

// Remove a student and all its associated awards and actions
void Student::remove(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;

	// Delete any actions
	pstmt.reset(con->prepareStatement("DELETE FROM student_action WHERE stdt_id=?"));
	pstmt->setInt(1, id);

	pstmt->execute();

	// Delete any awards
	pstmt.reset(con->prepareStatement("DELETE FROM student_award WHERE stdt_id=?"));
	pstmt->setInt(1, id);

	pstmt->execute();

	// Delete student
	pstmt.reset(con->prepareStatement("DELETE FROM student WHERE stdt_id=?"));
	pstmt->setInt(1, id);

	pstmt->execute();
}


// Assign an action
void Student::assign_action(sql::Connection* con, int id, int action_id, int year) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT IGNORE INTO student_action (stdt_id, actn_id, start_year) VALUES (?, ?, ?)")); // Insert ignore used since students may already be assigned action

	pstmt->setInt(1, id);
	pstmt->setInt(2, action_id);
	pstmt->setInt(3, year);

	pstmt->execute();
}

// Delete an action
void Student::remove_action(sql::Connection* con, int id, int action_id, int year) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("DELETE FROM student_action WHERE stdt_id=? AND actn_id=? AND start_year=?"));  

	pstmt->setInt(1, id);
	pstmt->setInt(2, action_id);
	pstmt->setInt(3, year);

	pstmt->execute();
}


// Edit various student attributes
void Student::edit_OEN(sql::Connection* con, int id, CString OEN) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE student SET oen=? WHERE stdt_id=?"));
	std::string std_OEN = CT2A(OEN);

	pstmt->setString(1, std_OEN);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Student::edit_last_name(sql::Connection* con, int id, CString last_name) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE student SET last_name=? WHERE stdt_id=?"));
	std::string std_last_name = CT2A(last_name);

	pstmt->setString(1, std_last_name);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Student::edit_first_name(sql::Connection* con, int id, CString first_name) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE student SET first_name=? WHERE stdt_id=?"));
	std::string std_first_name = CT2A(first_name);

	pstmt->setString(1, std_first_name);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Student::edit_pref_name(sql::Connection* con, int id, CString pref_name) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE student SET pref_name=? WHERE stdt_id=?"));
	std::string std_pref_name = CT2A(pref_name);

	if (pref_name.GetLength() > 0) // preferred name is optional
		pstmt->setString(1, std_pref_name);
	else
		pstmt->setNull(1, sql::DataType::VARCHAR); // sql::DataType technically does nothing in setNull source code, setting here anyway for clarity

	pstmt->setInt(2, id);

	pstmt->execute();
}

void Student::edit_start_year(sql::Connection* con, int id, int start_year) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE student SET start_year=? WHERE stdt_id=?"));

	pstmt->setInt(1, start_year);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Student::edit_grad_year(sql::Connection* con, int id, int grad_year) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE student SET grad_year=? WHERE stdt_id=?"));

	pstmt->setInt(1, grad_year);
	pstmt->setInt(2, id);

	pstmt->execute();
}


// Get all students
std::auto_ptr<sql::ResultSet> Student::get(sql::Connection* con) {
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;

	res.reset(stmt->executeQuery(
		"SELECT stdt_id id, oen, last_name, first_name, pref_name, start_year, grad_year FROM student ORDER BY grad_year, last_name"
	));

	return res;
}

// Get students that are in school for a certain year (start <= year <= grad)
std::auto_ptr<sql::ResultSet> Student::get(sql::Connection* con, int year) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT stdt_id id, oen, last_name, first_name, pref_name, start_year, grad_year FROM student WHERE start_year<=? AND grad_year>=? ORDER BY last_name"
	));

	pstmt->setInt(1, year);
	pstmt->setInt(2, year);

	res.reset(pstmt->executeQuery());
	return res;
}

// Get students that start at a certain year
std::auto_ptr<sql::ResultSet> Student::get_start(sql::Connection* con, int start_year) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT stdt_id id, oen, last_name, first_name, pref_name, start_year, grad_year FROM student WHERE start_year=? ORDER BY last_name"
	));

	pstmt->setInt(1, start_year);

	res.reset(pstmt->executeQuery());
	return res;
}

// Get students that graduate at a certain year
std::auto_ptr<sql::ResultSet> Student::get_grad(sql::Connection* con, int grad_year) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT stdt_id id, oen, last_name, first_name, pref_name, start_year, grad_year FROM student WHERE grad_year=? ORDER BY last_name"
	));

	pstmt->setInt(1, grad_year);

	res.reset(pstmt->executeQuery());
	return res;
}


// Get all actions of a student
std::auto_ptr<sql::ResultSet> Student::get_actions(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT a.actn_id actn_id, a.type type, a.name name, a.points points, s_a.start_year year FROM action a "
		"INNER JOIN student_action s_a ON s_a.actn_id = a.actn_id WHERE s_a.stdt_id = ? "
		"ORDER BY year, type, name"
	));

	pstmt->setInt(1, id);

	res.reset(pstmt->executeQuery());
	return res;
}

// Get all awards of a student
std::auto_ptr<sql::ResultSet> Student::get_awards(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT c.cnst_id cnst_id, c.name name, c.description description, s_a.confirmed FROM cnst c "
		"INNER JOIN student_award s_a ON s_a.cnst_id = c.cnst_id WHERE s_a.stdt_id = ?"
	));

	pstmt->setInt(1, id);

	res.reset(pstmt->executeQuery());
	return res;
}

// Get all awards given to all students that graduate a certain year
std::auto_ptr<sql::ResultSet> Student::get_awards_grad(sql::Connection* con, int grad_year) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT c.cnst_id cnst_id, s.stdt_id stdt_id, s.last_name last_name, s.first_name first_name, s.pref_name pref_name, c.name name, c.description description, s_a.confirmed "
		"FROM student_award s_a INNER JOIN student s ON s_a.stdt_id = s.stdt_id "
		"INNER JOIN cnst c ON s_a.cnst_id = c.cnst_id "
		"WHERE s.grad_year=?"
	));

	pstmt->setInt(1, grad_year);

	res.reset(pstmt->executeQuery());
	return res;
}