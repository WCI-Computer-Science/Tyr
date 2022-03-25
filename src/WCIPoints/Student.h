#pragma once

#include "mysql_connection.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


// Logic and database interfacing for constraints and awards
// Error handling should be done by caller
namespace Student {
	void add(sql::Connection* con, CString OEN, CString last_name, CString first_name, CString pref_name, int start_year, int grad_year); // Add a student into the database
	
	void remove(sql::Connection* con, int id); // Students can always be instantly deleted, simply by removing all references first
	// Make sure that in the UI, it is very difficult to delete students!

	void assign_action(sql::Connection* con, int id, int action_id, int year); // Assign an action to a student
	void remove_action(sql::Connection* con, int id, int action_id, int year); // Remove an action from a student

	void edit_OEN(sql::Connection* con, int id, CString OEN);
	void edit_last_name(sql::Connection* con, int id, CString last_name);
	void edit_first_name(sql::Connection* con, int id, CString first_name);
	void edit_pref_name(sql::Connection* con, int id, CString pref_name);
	void edit_start_year(sql::Connection* con, int id, int start_year);
	void edit_grad_year(sql::Connection* con, int id, int grad_year);

	std::auto_ptr<sql::ResultSet> get(sql::Connection* con); // Get all students
	std::auto_ptr<sql::ResultSet> get(sql::Connection* con, int year); // Get all students that are in school for a given year (start <= year <= grad)
	std::auto_ptr<sql::ResultSet> get_start(sql::Connection* con, int start_year); // Get all students that start at a given year
	std::auto_ptr<sql::ResultSet> get_grad(sql::Connection* con, int grad_year); // Get all students that graduate at a given year

	std::auto_ptr<sql::ResultSet> get_actions(sql::Connection* con, int id); // Get all actions of a certain student
	std::auto_ptr<sql::ResultSet> get_awards(sql::Connection* con, int id); // Get all awards of a certain student
	std::auto_ptr<sql::ResultSet> get_awards_grad(sql::Connection* con, int grad_year); // Get all awards given to all students that graduate a certain year
}