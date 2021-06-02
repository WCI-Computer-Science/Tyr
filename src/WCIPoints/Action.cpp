#include "pch.h"
#include "afxwinappex.h"

#include "mysql_connection.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>

#include "Action.h"


// Add an action
void Action::add(sql::Connection* con, int type, CString name, int points) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	pstmt.reset(con->prepareStatement("INSERT INTO action (type, name, points) VALUES (?, ?, ?)"));
	std::string std_name = CT2A(name);

	pstmt->setInt(1, type);
	pstmt->setString(2, std_name);
	pstmt->setInt(3, points);

	pstmt->execute();
}


// Remove an action
// Fails if student_action link exists due to foreign key reference
// Action::archive should be used if references to action exists
void Action::remove(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	pstmt.reset(con->prepareStatement("DELETE FROM action WHERE actn_id=?"));

	pstmt->setInt(1, id);

	pstmt->execute();
}


// Archive an action
void Action::archive(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	pstmt.reset(con->prepareStatement("INSERT INTO action_archive (actn_id) VALUES (?)"));

	pstmt->setInt(1, id);

	pstmt->execute();
}


// Edit various attributes of an action
void Action::edit_type(sql::Connection* con, int id, int type) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	pstmt.reset(con->prepareStatement("UPDATE action SET type=? WHERE actn_id=?"));

	pstmt->setInt(1, type);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Action::edit_name(sql::Connection* con, int id, CString name) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	pstmt.reset(con->prepareStatement("UPDATE action SET name=? WHERE actn_id=?"));
	std::string std_name = CT2A(name);

	pstmt->setString(1, std_name);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Action::edit_points(sql::Connection* con, int id, int points) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	pstmt.reset(con->prepareStatement("UPDATE action SET points=? WHERE actn_id=?"));

	pstmt->setInt(1, points);
	pstmt->setInt(2, id);

	pstmt->execute();
}


// Get actions
std::auto_ptr<sql::ResultSet> Action::get(sql::Connection* con) {
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;

	res.reset(stmt->executeQuery(
		"SELECT a.actn_id id, a.type type, a.name name, a.points points FROM action a LEFT JOIN action_archive a_a ON a_a.actn_id=a.actn_id WHERE a_a.actn_id IS NULL"
	));
	return res;
}

std::auto_ptr<sql::ResultSet> Action::get(sql::Connection* con, int type) {
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;

	res.reset(stmt->executeQuery(
		"SELECT a.actn_id id, a.name name, a.points points FROM action a LEFT JOIN action_archive a_a ON a_a.actn_id=a.actn_id WHERE a_a.actn_id IS NULL AND a.type=" + std::to_string(type)
	));
	return res;
}