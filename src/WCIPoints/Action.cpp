#include "pch.h"
#include "afxwinappex.h"

#include "Action.h"


// Add an action
void Action::add(sql::Connection* con, int type, CString name, int points) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO action (type, name, points) VALUES (?, ?, ?)"));
	std::string std_name = CT2A(name);

	pstmt->setInt(1, type);
	pstmt->setString(2, std_name);
	pstmt->setInt(3, points);

	pstmt->execute();
}


// Remove an action if no links exist, otherwise archive it
// Returns true if action is removed, false if it is or remains archived
bool Action::autoremove(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement("SELECT COUNT(*) c FROM student_action WHERE actn_id=?"));
	pstmt->setInt(1, id);
	res.reset(pstmt->executeQuery());
	int ref = -1;
	if (res->next()) // Should always return true, next() must be called for getInt() to be valid
		ref = res->getInt("c");

	pstmt.reset(con->prepareStatement("SELECT COUNT(*) c FROM action_archive WHERE actn_id=?"));
	pstmt->setInt(1, id);
	res.reset(pstmt->executeQuery());
	int arch = -1;
	if (res->next()) // Should always return true, next() must be called for getInt() to be valid
		arch = res->getInt("c");

	if (ref == 0) {
		if (arch > 0)
			unarchive(con, id); // Unarchive to remove foreign key reference (make it possible to delete)
		remove(con, id);
		return true;
	}
	else {
		if (arch == 0)
			archive(con, id);
		return false;
	}
}

// Remove an action
// Fails if student_action link exists due to foreign key reference
// Action::archive should be used if references to action exists
void Action::remove(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("DELETE FROM action WHERE actn_id=?"));

	pstmt->setInt(1, id);

	pstmt->execute();
}


// Archive an action
void Action::archive(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO action_archive (actn_id) VALUES (?)"));

	pstmt->setInt(1, id);

	pstmt->execute();
}

// Unarchive an action
void Action::unarchive(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("DELETE FROM action_archive WHERE actn_id=?"));

	pstmt->setInt(1, id);

	pstmt->execute();
}


// Edit various attributes of an action
void Action::edit_type(sql::Connection* con, int id, int type) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE action SET type=? WHERE actn_id=?"));

	pstmt->setInt(1, type);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Action::edit_name(sql::Connection* con, int id, CString name) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE action SET name=? WHERE actn_id=?"));
	std::string std_name = CT2A(name);

	pstmt->setString(1, std_name);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Action::edit_points(sql::Connection* con, int id, int points) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE action SET points=? WHERE actn_id=?"));

	pstmt->setInt(1, points);
	pstmt->setInt(2, id);

	pstmt->execute();
}


// Get actions
std::auto_ptr<sql::ResultSet> Action::get(sql::Connection* con, bool archive) {
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;

	if (!archive)
		res.reset(stmt->executeQuery(
			"SELECT a.actn_id id, a.type type, a.name name, a.points points FROM action a LEFT JOIN action_archive a_a ON a_a.actn_id=a.actn_id WHERE a_a.actn_id IS NULL ORDER BY name"
		));
	else
		res.reset(stmt->executeQuery(
			"SELECT a.actn_id id, a.type type, a.name name, a.points points FROM action a INNER JOIN action_archive a_a ON a_a.actn_id=a.actn_id ORDER BY name"
		));

	return res;
}

std::auto_ptr<sql::ResultSet> Action::get(sql::Connection* con, int type, bool archive) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	if (!archive)
		pstmt.reset(con->prepareStatement(
			"SELECT a.actn_id id, a.name name, a.points points FROM action a LEFT JOIN action_archive a_a ON a_a.actn_id=a.actn_id WHERE a_a.actn_id IS NULL AND a.type=? ORDER BY name"
		));
	else
		pstmt.reset(con->prepareStatement(
			"SELECT a.actn_id id, a.name name, a.points points FROM action a INNER JOIN action_archive a_a ON a_a.actn_id=a.actn_id WHERE a.type=? ORDER BY name"
		));

	pstmt->setInt(1, type);
	
	res.reset(pstmt->executeQuery());
	return res;
}


// Get info
std::auto_ptr<sql::ResultSet> Action::info(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT type, name, points FROM action WHERE actn_id=?"
	));

	pstmt->setInt(1, id);

	res.reset(pstmt->executeQuery());
	return res;
}