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
	Msg_field(char* type, char* from, char* to, char* data) {
		strcpy(msg_type, type);
		strcpy(msg_from, from);
		strcpy(msg_to, to);
		strcpy(msg_data, data);
	}

	Msg_field() {}
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
	static int packing(Msg_field* p_msg_field, char* msg)
	{
		int msg_len = 0;
		strcpy(msg+msg_len, p_msg_field->msg_type);
	
		msg_len += strlen(p_msg_field->msg_type) + 1;
		strcpy(msg+msg_len, p_msg_field->msg_from);
	
		msg_len += strlen(p_msg_field->msg_from) + 1;
		strcpy(msg+msg_len, p_msg_field->msg_to);
	
		msg_len += strlen(p_msg_field->msg_to) + 1;
		strcpy(msg+msg_len, p_msg_field->msg_data);
	
		return msg_len + strlen(p_msg_field->msg_data) + 1;
	}
	
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
	static bool unpacking(char* msg, int msg_len, Msg_field* p_msg_field)
	{
		/* check whether msg has four fields */
		int i, c;
		for (i = 0, c = 0; i < msg_len; ++ i)
			if (msg[i] == '\0')
			{
				++ c;
				if (c == 4)
					break;
			}
		if (i+1 != msg_len)
			return false;
	
		/* unpacking */
		i = 0;
		strcpy(p_msg_field->msg_type, msg+i);
	
		i += strlen(msg+i) + 1;
		strcpy(p_msg_field->msg_from, msg+i);
	
		i += strlen(msg+i) + 1;
		strcpy(p_msg_field->msg_to, msg+i);
	
		i += strlen(msg+i) + 1;
		strcpy(p_msg_field->msg_data, msg+i);
	
		return true;
	}

};
#endif
