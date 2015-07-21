#ifndef __DB_H__
#define __DB_H__

/* 
* This is a simple simulation of a database
* It contains three tables
*/

#include	<map>
#include	<string>
using namespace std;

struct DB
{
	/* regist table, it contains all registed [user and password] pairs,
	   an entry of this table is <user_name : user_password> */
	map<string, string> map_regist;

	/* login table_1, it contains all logined [user and his socket] pairs,
	   an entry of this table is <user_name : socket> */
	map<string, int> map_login_name_sock;

	/* login table_1, it contains all logined [socket and its user name] pairs,
	   an entry of this table is <user_name : socket> */
	map<int, string> map_login_sock_name;
};

#endif
