#pragma once

// Error handling should be done by caller
namespace Action {
	
	void add(sql::Connection* con, int type, CString name, int points);

	bool autoremove(sql::Connection* con, int id);

	void remove(sql::Connection* con, int id);

	void archive(sql::Connection* con, int id);
	void unarchive(sql::Connection* con, int id);

	void edit_type(sql::Connection* con, int id, int type);
	void edit_name(sql::Connection* con, int id, CString name);
	void edit_points(sql::Connection* con, int id, int points);
	
	std::auto_ptr<sql::ResultSet> get(sql::Connection* con); // Get all actions
	std::auto_ptr<sql::ResultSet> get(sql::Connection* con, int type); // Get actions of a specific type
}