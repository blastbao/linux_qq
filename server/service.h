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
	static int regist(const int request_sockfd, const Msg_field & request_fields, int & echo_sockfd, Msg_field & echo_fields, DB & db);
	/*
	*@params
		same as regist interface
	*@return
		0 if success
		-1 if user name not in regist table or user has already logined
	*/
	static int login(const int request_sockfd, const Msg_field & request_fields, int & echo_sockfd, Msg_field & echo_fields, DB & db);


	/*
	*if to_user is online, transponder to he,
		else transponder to sender
	*@return
		this interface always success
	*/
	static void transponder(const int request_sockfd, const Msg_field & request_fields, int & echo_sockfd, Msg_field & echo_fields, DB & db);

private:
	
	

	/* 
	*save one user to registed file 
	*@return 
		0 if success
		-1 if open file fail
	*/
	static int save_registed_user(char* fname, const string & name, const string & psw);

};
#endif
