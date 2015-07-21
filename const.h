#ifndef __CONST_H__
#define __CONST_H__

/* all const variables of client and server are here */

namespace CONST
{
/* port number */
	const int PORT_NO = 9999;

/* user name and password size */
	const int USER_NAME_SIZE = 30;
	const int USER_PSW_SIZE  = 10;

/* the char used to glue */
	const char CHAR_GLUE_MSG      = 0;
	const char CHAR_GLUE_NAME_PSW = 1;
	
/* client state */
	const int NON_LOGIN_REGIST = 0; /* the initial state of client */
	const int LOGIN_WAIT_USERNAME = 1; /* client is at login state, waiting for user name */
	const int LOGIN_WAIT_USERPSW  = 2; /* client is at login state, waiting for user password */
	const int REGIST_WAIT_USERNAME = 3; /* client is at regist state, waiting for user name */
	const int REGIST_WAIT_USERPSW  = 4; /* client is at regist state, waiting for user password */
	const int WAIT_SERVER_RET = 5; /* waiting for the return from server on login or regist */
	const int COMMUNICATE = 6; /* client is communicating with server */
	const int QUIT = -1; /* client quit */

/* message */
	/* message content size */
	const int MSG_SIZE = 1024;
	/* message field size */
	const int MSG_TYPE_SIZE = 10;
	const int MSG_FROM_SIZE = USER_NAME_SIZE;
	const int MSG_TO_SIZE   = USER_NAME_SIZE;
	const int MSG_DATA_SIZE = 950;
	/* message length size */
	const int MSG_N_BYTE_OF_LENGTH = 2;
	/* message buffer queue size */
	const int MSG_QUEUE_SIZE = 1024;
	/* message types */
	extern char MSG_TYPE_REGIST[];
	extern char MSG_TYPE_LOGIN[];
	extern char MSG_TYPE_MSG[];
	extern char MSG_TYPE_RET_SUCC[];
	extern char MSG_TYPE_RET_FAIL[];

/* server */
	/* registed user file */
	extern char REGISTED_FILE[];

/* epoll */
	const int MAX_EVENTS = 1024;
};

#endif
