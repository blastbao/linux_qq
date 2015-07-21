#include	"user_input_util.h"

bool User_input_util::extract(const char* user_input, const int input_len, User_input_field* p_input_fields)
{
	if ( !(user_input[0] == 't' &&
		   user_input[1] == 'o' && 
		   user_input[2] == ':') )
		return false;

	int i_colon = 3;
	for (; i_colon < input_len && user_input[i_colon] != ':'; ++ i_colon)
		;
	
	if (i_colon == input_len)
		return false;
	
	p_input_fields->set_toname(user_input+3, i_colon-3);
	p_input_fields->set_data(user_input+i_colon+1, input_len-(i_colon+1));
	return true;
}

void User_input_field::set_toname(const char* src, const int n_byte)
{
	memcpy(this->to_name, src, n_byte);
	this->to_name[n_byte] = '\0';
}

void User_input_field::set_data(const char* src, const int n_byte)
{
	memcpy(this->data, src, n_byte);
	this->data[n_byte] = '\0';
}
