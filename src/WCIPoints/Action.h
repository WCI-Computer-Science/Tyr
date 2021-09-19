#pragma once

// Logic and database interfacing for actions
// Error handling should be done by caller
namespace Action
{
	void add(sql::Connection* con, int type, CString name, int points);

	bool autoremove(sql::Connection* con, int id);

	void remove(sql::Connection* con, int id);

	void archive(sql::Connection* con, int id);
	void unarchive(sql::Connection* con, int id);

	void edit_type(sql::Connection* con, int id, int type);
	void edit_name(sql::Connection* con, int id, CString name);
	void edit_points(sql::Connection* con, int id, int points);
	
	std::auto_ptr<sql::ResultSet> get(sql::Connection* con, bool archive=false); // Get all actions, and see archived if archive is set to true
	std::auto_ptr<sql::ResultSet> get(sql::Connection* con, int type, bool archive=false); // Get actions of a specific type, and see archived if archive is set to true
}