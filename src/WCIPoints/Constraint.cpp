#include "pch.h"
#include "afxwinappex.h"

#include "mysql_connection.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>

#include "Constraint.h"


// Add a constraint without specialization, returns id
int Constraint::add(sql::Connection* con, int type, CString name, CString desc, bool award) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO cnst (name, description, type, is_award) VALUES (?, ?, ?, ?)"));
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;
	std::string std_name = CT2A(name);
	std::string std_desc = CT2A(desc);

	pstmt->setString(1, std_name);
	pstmt->setString(2, std_desc);
	pstmt->setInt(3, type);
	pstmt->setBoolean(4, award);

	pstmt->execute();

	res.reset(stmt->executeQuery("SELECT LAST_INSERT_ID() id"));

	if (res->next()) // Should always return true, next() must be called for getInt() to be valid
		return res->getInt("id");

	return -1; // Something went wrong
}

// Add additional info for basic constraint
void Constraint::add_basic(sql::Connection* con, int id, int type, int action_id, int action_type, int mx, int x, int y) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	
	if (type == 2) { // The sum of action points, with at most mx points per year, should be in an interval x to y
		pstmt.reset(con->prepareStatement("INSERT INTO basic_cnst (cnst_id, actn_type, mx, x, y) VALUES (?, ?, ?, ?, ?)"));

		pstmt->setInt(1, id);
		pstmt->setInt(2, action_type);
		pstmt->setInt(3, mx);
		pstmt->setInt(4, x);
		pstmt->setInt(5, y);
	}
	else if (type == 3) { // The difference between student grad year and start year should be in an interval x to y
		pstmt.reset(con->prepareStatement("INSERT INTO basic_cnst (cnst_id, x, y) VALUES (?, ?, ?)"));

		pstmt->setInt(1, id);
		pstmt->setInt(2, x);
		pstmt->setInt(3, y);
	}
	else if (type == 4) {
		pstmt.reset(con->prepareStatement("INSERT INTO basic_cnst (cnst_id, actn_id, actn_type, x, y) VALUES (?, ?, ?, ?, ?)"));

		pstmt->setInt(1, id);

		if (action_id > 0 && action_type == -1) { // The frequency of a specific action should be in an interval x to y
			pstmt->setInt(2, action_id);
			pstmt->setNull(3, sql::DataType::INTEGER); // sqlType doesn't actually do anything in the source code, but setting it here just for clarity
		}
		else if (action_id > 0 && action_type >= 0) { // The frequency of any specific action in a specific action type should be in an interval x to y (action_id is ignored)
			pstmt->setInt(2, action_id);
			pstmt->setInt(3, action_type);
		}
		else if (action_id == -1 && action_type >= 0) { // The frequency of all actions in a specific action type should be in an interval x to y
			pstmt->setNull(2, sql::DataType::INTEGER);
			pstmt->setInt(3, action_type);
		}
		else return;
		
		pstmt->setInt(4, x);
		pstmt->setInt(5, y);
	}
	else if (type == 5) {
		pstmt.reset(con->prepareStatement("INSERT INTO basic_cnst (cnst_id, actn_id, actn_type, x, y) VALUES (?, ?, ?, ?, ?)"));

		pstmt->setInt(1, id);

		if (action_id > 0 && action_type == -1) { // The maximum consecutive years of a specific action should be in an interval x to y
			pstmt->setInt(2, action_id);
			pstmt->setNull(3, sql::DataType::INTEGER);
		}
		else if (action_id > 0 && action_type >= 0) { // The maximum consecutive years of any specific action in a specific action type should be in an interval x to y (action_id is ignored)
			pstmt->setInt(2, action_id);
			pstmt->setInt(3, action_type);
		}
		else if (action_id == -1 && action_type >= 0) { // The maximum consecutive years of all actions in a specific action type should be in an interval x to y
			pstmt->setNull(2, sql::DataType::INTEGER);
			pstmt->setInt(3, action_type);
		}
		else return;

		pstmt->setInt(4, x);
		pstmt->setInt(5, y);
	}

	pstmt->execute();
}

// Add additional sub-constraint for a compound constraint
// Return true on success, or false on failure (if adding the constraint would create a cycle)
bool Constraint::add_compound(sql::Connection* con, int id, int sub_id) {
	if (check_cycle(con, id, sub_id))
		return false;

	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO compound_cnst (super_cnst, sub_cnst) VALUES (?, ?)"));

	pstmt->setInt(1, id);
	pstmt->setInt(2, sub_id);

	pstmt->execute();
	return true;
}


// Remove a constraint if it isn't an award or if it is but hasn't ever been assigned
// Returns true if action is removed, false if it becomes or remains archived
bool Constraint::autoremove(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement("SELECT COUNT(*) c FROM student_award WHERE cnst_id=?"));
	pstmt->setInt(1, id);
	res.reset(pstmt->executeQuery());
	int ref = -1;
	if (res->next()) // Should always return true, next() must be called for getInt() to be valid
		ref = res->getInt("c");

	pstmt.reset(con->prepareStatement("SELECT COUNT(*) c FROM cnst_archive WHERE cnst_id=?"));
	pstmt->setInt(1, id);
	res.reset(pstmt->executeQuery());
	int arch = -1;
	if (res->next()) // Should always return true, next() must be called for getInt() to be valid
		arch = res->getInt("c");

	if (ref == 0) {
		if (arch > 0)
			unarchive(con, id);
		remove(con, id);
		return true;
	}
	else {
		if (arch == 0)
			archive(con, id);
		return false;
	}
}


// Remove a constraint and all its associated edges
void Constraint::remove(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	
	// If the constraint is basic, delete its info
	pstmt.reset(con->prepareStatement("DELETE FROM basic_cnst WHERE cnst_id=?"));
	pstmt->setInt(1, id);

	pstmt->execute();

	// Delete all edges
	pstmt.reset(con->prepareStatement("DELETE FROM compound_cnst WHERE sub_cnst=? OR super_cnst=?"));
	pstmt->setInt(1, id);
	pstmt->setInt(2, id);

	pstmt->execute();

	// Delete constraint
	pstmt.reset(con->prepareStatement("DELETE FROM cnst WHERE cnst_id=?"));
	pstmt->setInt(1, id);

	pstmt->execute();
}


// Archive an award that has already been handed out to a student
void Constraint::archive(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;

	pstmt.reset(con->prepareStatement("INSERT INTO cnst_archive (cnst_id) VALUES (?)"));
	pstmt->setInt(1, id);

	pstmt->execute();

	// Delete all edges
	pstmt.reset(con->prepareStatement("DELETE FROM compound_cnst WHERE sub_cnst=? OR super_cnst=?"));
	pstmt->setInt(1, id);
	pstmt->setInt(2, id);

	pstmt->execute();
}

// Unarchive an award
// Note that this doesn't recover its conditions if it was a compound constraint
void Constraint::unarchive(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("DELETE FROM cnst_archive WHERE cnst_id=?"));

	pstmt->setInt(1, id);

	pstmt->execute();
}


// Edit general constraint info
void Constraint::edit_name(sql::Connection* con, int id, CString name) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE cnst SET name=? WHERE cnst_id=?"));
	std::string std_name = CT2A(name);

	pstmt->setString(1, std_name);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Constraint::edit_description(sql::Connection* con, int id, CString desc) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE cnst SET description=? WHERE cnst_id=?"));
	std::string std_desc = CT2A(desc);

	pstmt->setString(1, std_desc);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Constraint::edit_award(sql::Connection* con, int id, bool award) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE cnst SET is_award=? WHERE cnst_id=?"));

	pstmt->setBoolean(1, award);
	pstmt->setInt(2, id);

	pstmt->execute();
}


// Edit basic constraints
void Constraint::edit_action_id(sql::Connection* con, int id, int action_id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE basic_cnst SET actn_id=? WHERE cnst_id=?"));

	pstmt->setInt(1, action_id);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Constraint::edit_action_type(sql::Connection* con, int id, int action_type) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE basic_cnst SET actn_type=? WHERE cnst_id=?"));

	pstmt->setInt(1, action_type);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Constraint::edit_mx(sql::Connection* con, int id, int mx) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE basic_cnst SET mx=? WHERE cnst_id=?"));

	pstmt->setInt(1, mx);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Constraint::edit_x(sql::Connection* con, int id, int x) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE basic_cnst SET x=? WHERE cnst_id=?"));

	pstmt->setInt(1, x);
	pstmt->setInt(2, id);

	pstmt->execute();
}

void Constraint::edit_y(sql::Connection* con, int id, int y) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE basic_cnst SET y=? WHERE cnst_id=?"));

	pstmt->setInt(1, y);
	pstmt->setInt(2, id);

	pstmt->execute();
}


// Edit compound constraints
void Constraint::remove_compound(sql::Connection* con, int id, int sub_id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("DELETE FROM compound_cnst WHERE super_cnst=? AND sub_cnst=?"));
	pstmt->setInt(1, id);
	pstmt->setInt(2, sub_id);

	pstmt->execute();
}


// Load graph from MySQL into C++
// Each sub-constraint is a node pointing to a super-constraint (this is in reverse to a parent/child relationship in a tree)
// Graph is loaded as an adjacency list, vertex list is loaded as a vector
void Constraint::load(sql::Connection* con, std::map<int, std::vector<int>>& g, std::vector<int>& v) {
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;

	res.reset(stmt->executeQuery("SELECT sub_cnst a, super_cnst b FROM compound_cnst"));

	while (res->next()) {
		int a = res->getInt("a"), b = res->getInt("b");
		g[a].push_back(b);
	}

	res.reset(stmt->executeQuery("SELECT cnst_id id FROM cnst"));

	while (res->next()) {
		int id = res->getInt("id");
		v.push_back(id);
	}
}

// Check if adding a certain edge will create a cycle
bool Constraint::check_cycle(sql::Connection* con, int super_id, int sub_id) {
	std::map<int, std::vector<int>> g;
	std::vector<int> v;
	load(con, g, v);

	g[sub_id].push_back(super_id);

	// Non-recursive DFS to check for cycle (cycle exists iff there is a backedge)

	std::set<int> vis, stk; // Check if vertex is visited or on the call stack, respectively
	std::stack<int> s; // Call stack
	
	for (int i : v) if (!vis.count(i)) {
		s.push(i);

		while (!s.empty()) {
			int t = s.top();
			
			if (!vis.count(t)) { // First time reaching vertex, push onto stack
				stk.insert(t);
				vis.insert(t);
			}
			else { // Second time reaching vertex, pop off stack
				s.pop();
				stk.erase(t);
				continue;
			}
			
			for (int j : g[t]) {
				if (stk.count(j))
					return true;
				if (!vis.count(j))
					s.push(j);
			}
		}
	}

	return false;
}


// Get constraints
std::auto_ptr<sql::ResultSet> Constraint::get(sql::Connection* con, bool award, bool archive) {
	std::auto_ptr<sql::Statement> stmt(con->createStatement());
	std::auto_ptr<sql::ResultSet> res;

	if (!archive) { // See constraints that aren't archived
		if (!award) // See all constraints
			res.reset(stmt->executeQuery(
				"SELECT c.cnst_id id, c.name name, c.description description, c.type type, c.is_award award "
				"FROM cnst c LEFT JOIN cnst_archive c_a ON c_a.cnst_id=c.cnst_id "
				"WHERE c_a.cnst_id IS NULL"
			));
		else // See only awards
			res.reset(stmt->executeQuery(
				"SELECT c.cnst_id id, c.name name, c.description description, c.type type "
				"FROM cnst c LEFT JOIN cnst_archive c_a ON c_a.cnst_id=c.cnst_id "
				"WHERE c_a.cnst_id IS NULL AND c.is_award=true"
			));
	}
	else // See all archived awards (note that non-award constraints are deleted, never archived; everything in the archive is an award)
		res.reset(stmt->executeQuery(
			"SELECT c.cnst_id id, c.name name, c.description description, c.type type "
			"FROM cnst c INNER JOIN cnst_archive c_a ON c_a.cnst_id=c.cnst_id"
		));

	return res;
}

std::auto_ptr<sql::ResultSet> Constraint::get_basic(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT actn_id, actn_type, mx, x, y FROM basic_cnst WHERE cnst_id=?"));
	std::auto_ptr<sql::ResultSet> res;

	pstmt->setInt(1, id);

	res.reset(pstmt->executeQuery());
	return res;
}

std::auto_ptr<sql::ResultSet> Constraint::get_compound(sql::Connection* con, int id) {
	std::auto_ptr<sql::PreparedStatement> pstmt;
	std::auto_ptr<sql::ResultSet> res;

	pstmt.reset(con->prepareStatement(
		"SELECT c.cnst_id id, c.name name, c.description description, c.type type, c.is_award award "
		"FROM cnst c INNER JOIN compound_cnst c_c ON c.cnst_id=c_c.sub_cnst AND c_c.super_cnst=?"
	));

	pstmt->setInt(1, id);

	res.reset(pstmt->executeQuery());
	return res;
}