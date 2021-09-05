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
