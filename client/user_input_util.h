#ifndef __USER_INPUT_UTIL_H__
#define __USER_INPUT_UTIL_H__

#include	"../const.h"

#include	<string.h>
#include	<stdio.h>

struct User_input_field
{
/* all of fields must be end with '\0' */
	char request_type[CONST::USER_REQUEST_TYPE_SIZE+1];
	char to_name[CONST::USER_NAME_SIZE+1];
	char data[CONST::MSG_DATA_SIZE+1];

	/*
	*@param src in
		the begin pos will be copyed
	*@param n_byte in
		the number of byte will be copyed
	*/
	void set_request_type(const char* src, const int n_byte);
	void set_toname(const char* src, const int n_byte);
	void set_data(const char* src, const int n_byte);
};

class User_input_util
{

public:

	/*
	*extract user input string, the formation of user input string is "to:name:data"
	*@param user_input in
		the string that user inputing
	*@param input_len in
		the length of user input string
	*@input_fields out
		a structure to hold user input fields, the fields contain {user_request_type, to_name, data}
	*@return
		true if user input formation is correct
		false if user input formation is wrong
	*/
	static bool extract(const char* user_input, const int input_len, User_input_field & input_fields);

};

#endif
