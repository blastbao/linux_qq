#ifndef __SERVICE_H__
#define __SERVICE_H__

/*
*This file contains service interface for the message of client requesting
*/

#include	"../util.h"
#include	"../const.h"
#include	"../msg_util.h"

#include	"./db.h"

#include	<map>
#include	<string>
#include	<fstream>
#include	<cstdio>
using namespace std;

class Service
{

public:

	/*
	*@param request_sockfd in
	*@request_fields in 
		socket and message fields from client
	*@param echo_sockfd out
	*@param echo_msg out
		socket and message that server will echo
	*@param db in, out
		database
	*@return 
		0 if success
		-1 if user name already in regist table
	*/
	static int regist(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db);
	
	/*
	* login is a broadcast service,
	* it will notification all friends if login succeed,
	*@return
		0 if success
		-1 if user name not in regist table or user has already logined
	*/
	static int login(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db);

	/*
	* logout is a broadcast service,
	* it will notification all friends.
	*@return
		this interface always success
	*/
	static void logout(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db);

	/*
	*get the friend list of client
	*friend list is like "friend1friend1friend2friend1", which '1' represents this friend logined, otherwise is '2'
	*@return
		this interface always success
	*/
	static void get_friendlist(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db);

	static void add_friend(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db);

	/*
	*if to_user is online, transponder to he,
		else transponder to sender
	*@return
		this interface always success
	*/
	static void transponder(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db);

private:
	
	/* 
	*save two string into specified file
	*@return 
		0 if success
		-1 if open file fail
	*/
	static int _save2file(const char* fname, const string & s1, const string & s2);

	/*
	*get the friend list of user_name
	*@return
		this function is always success
	*/
	static void _get_friendlist(const string & user_name, const DB & db, char* friend_list);

	/*
	*broadcast msg_data to from_name's friends
	@return 
		this function is always success
	*/
	static void _broadcast_msg(const string & from_name, DB & db, const char* msg_type, const char* msg_data, 
							vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields);

};
#endif
