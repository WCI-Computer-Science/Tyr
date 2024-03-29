#pragma once

#include "mysql_connection.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <map>


// Logic and database interfacing for constraints and awards
// Error handling should be done by caller
namespace Constraint
{
	int add(sql::Connection* con, int type, CString name, CString desc, bool award=false); // Add a constraint without specialization, return id
	void add_basic(sql::Connection* con, int id, int type, int action_id, int action_type, int mx, int x, int y); // Add additonal info for a basic constraint
	bool add_compound(sql::Connection* con, int id, int sub_id); // Add additional sub-constraint for a compound constraint, return true on success

	bool autoremove(sql::Connection* con, int id);

	void remove(sql::Connection* con, int id);

	void archive(sql::Connection* con, int id);
	void unarchive(sql::Connection* con, int id);

	void add_student_award(sql::Connection* con, int id, int student_id);
	void remove_student_award(sql::Connection* con, int id, int student_id);
	
	// Edit general constraint info
	void edit_name(sql::Connection* con, int id, CString name);
	void edit_description(sql::Connection* con, int id, CString desc);
	void edit_award(sql::Connection* con, int id, bool award);

	// Edit basic constraints
	void edit_action_id(sql::Connection* con, int id, int action_id);
	void edit_action_type(sql::Connection* con, int id, int action_type);
	void edit_mx(sql::Connection* con, int id, int mx);
	void edit_x(sql::Connection* con, int id, int x);
	void edit_y(sql::Connection* con, int id, int y);
	// Edit compound constraints
	void remove_compound(sql::Connection* con, int id, int sub_id); // Remove a sub-constraint for a compound constraint

	// Graph operations
	// Class for a constraint vertex
	struct C {
		int id, type; // ID from database, type of constraint (from 0 to 5)
		bool is_award; // Whether the constraint is an award
	};
	void load(sql::Connection* con, std::map<int, std::vector<C>>& g, std::vector<C>& v); // Load graph from MySQL into C++
	bool check_cycle(sql::Connection* con, int super_id, int sub_id); // Check if adding an edge will create a cycle
	bool evaluate(sql::Connection* con, int id, int type, int student_id, const std::set<int>& valid); // Evaluate a constraint for a certain user, given set of true constraints
	bool assign_awards(sql::Connection* con, int student_id); // Add all awards a user qualifies for to the DB, return true if user qualifies for at least 1, false otherwise

	// Get constraints, and see archived if archive is set to true
	std::auto_ptr<sql::ResultSet> get(sql::Connection* con, bool award=false, bool archive=false); // Get all constraints, and see only awards if award is set to true
	std::auto_ptr<sql::ResultSet> get_basic(sql::Connection* con, int id); // Get the info on a basic constraint
	std::auto_ptr<sql::ResultSet> get_compound(sql::Connection* con, int id); // Get all sub-constraints of a compound constraint, and each one's info
	std::auto_ptr<sql::ResultSet> get_compound_except(sql::Connection* con, int id); // Get constraints that are not sub-constraints of a compound constraint, and each one's info
}