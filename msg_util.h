#ifndef __MSG_UTIL_H__
#define __MSG_UTIL_H__

#include	"const.h"

#include	<string.h>

struct Msg_field
{
/* all fields must use '\0' as the end!!! */
	char msg_type[CONST::MSG_TYPE_SIZE+1]; 	/* the type of msg is in {regist, login, succ, fail, msg} */
	char msg_from[CONST::MSG_FROM_SIZE+1]; 	/* the user name of sender */
	char msg_to[CONST::MSG_TO_SIZE+1];   	/* the user name of receiver */
	char msg_data[CONST::MSG_DATA_SIZE+1];	/* the data of msg */
	
	/*
	* all params must use '\0' as the end!!!
	*/
	Msg_field();
	Msg_field(const char* type, const char* from, const char* to, const char* data);
	
	/*
	* all params must use '\0' as the end!!!
	*/
	void set_fields(const char* type, const char* from, const char* to, const char* data);
};


class Msg_util
{

public:

	/*
	*packing [four message fields] into [a message content],
	*@notice
		msg must have enough space to hold message content
	*@param p_msg_field in
		the four message fields that need to be packed
	*@param msg out
		the buff to hold the message content, 
	@return
		> 0 the length of message content,
		there is no failed case.
	*/
	static int packing(const Msg_field* p_msg_field, char* msg);
		
	/*
	*unpacking [a message content] to [four message fields],
	*@param msg in
		the message content need to be unpacked
	*@param msg_len in
		the length of message content
	*@param p_msg_field out
		four message fields used to hold unpacking result
	*@return
		true if the message content format is correct,
		false if the message content format is wrong.
	*/
	static bool unpacking(const char* msg, const int msg_len, Msg_field* p_msg_field);

	/*
	*generate message with formation "ERR'\0''\0''\0'err_info'\0'"
	*@param err_info in
		error infomation
	*@param msg out
		error message
	*@return
		> 0 the length of message content,
		there is no failed case.
	*/
	static int err_packing(const char* err_info, char* msg);
	
};
#endif
