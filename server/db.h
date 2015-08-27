#ifndef __DB_H__
#define __DB_H__

/* 
* This is a simple simulation of a database
* It contains four tables
*/

#include	<map>
#include	<vector>
#include	<string>
#include	<utility>
#include	<fstream>
using namespace std;

struct DB
{
public:
	/* regist table, it contains all registed [user and password] pairs,
	   an entry of this table is map<user_name : user_password> */
	map<string, string> map_regist;

	/* login table_1, it contains all logined [user name and his socket] pairs,
	   an entry of this table is map<user_name : socket> */
	map<string, int> map_login_name_sock;

	/* login table_2, it contains all logined [socket and its user name] pairs,
	   an entry of this table is map<socket : user_name> */
	map<int, string> map_login_sock_name;

	/* friendship table, it contains all friendship pair 
	   an entry of this table is pair<user_name1 : user_name2>
	   if <user_name1 : user_name2> is an entry, 
	   then <user_name2 : user_name1> have to be an entry */
	vector<pair<string, string> > v_friendship;

	/*
	* return 
		0  if success
		-1 if open file failed
	*/
	int load_rigested_user_table(const char* fname);
	int load_friendship_table(const char* fname);
};

#endif
