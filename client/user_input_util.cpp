#include	"user_input_util.h"

bool User_input_util::extract(const char* user_input, const int input_len, User_input_field & input_fields)
{
	int  state;
	char tmp_buff[CONST::MSG_SIZE];
	int  tmp_len;

	state   = 0;
	tmp_len = 0;
	for (int i = 0; i < input_len; ++ i)
	{
		switch (user_input[i])
		{
		case 't':
			if (state == 0 || state == 5)
			{ 
				tmp_buff[tmp_len++] = user_input[i]; 
				state = 4; 
			}
			else if (state == 2 || state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else 
				return false;
			break;
		case 'o':
			if (state == 4 || state == 5 || state == 7) 
			{ 
				tmp_buff[tmp_len++] = user_input[i]; 
				if (state == 4 || state == 7) state = 1; 
				else state = 6; 
			}
			else if (state == 2 || state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else
				return false;
			break;
		case 'f':
			if (state == 0) 
			{
				tmp_buff[tmp_len++] = user_input[i];
				state = 5;
			}
			else if (state == 2 || state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else
				return false;	
			break;
		case 'k':
			if (state == 6) 
			{
				tmp_buff[tmp_len++] = user_input[i];
				state = 1;
			}
			else if (state == 2 || state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else
				return false;
			break;
		case 'n':
			if (state == 5) 
			{
				tmp_buff[tmp_len++] = user_input[i];
				state = 7;
			}
			else if (state == 2 || state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else
				return false;
			break;
		case ':':
			if (state == 1)
			{
				input_fields.set_request_type(tmp_buff, tmp_len);
				tmp_len = 0;
				state = 2;
			}
			else if (state == 2)
			{
				input_fields.set_toname(tmp_buff, tmp_len);
				tmp_len = 0;
				state = 3;
			}
			else if (state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else
				return false;
			break;
		default:
			if (state == 2 || state == 3)
				tmp_buff[tmp_len++] = user_input[i];
			else
				return false;
			break;
		}
	}

	if (state == 3)
	{
		input_fields.set_data(tmp_buff, tmp_len);
		return true;
	}
	else
		return false;
}

void User_input_field::set_request_type(const char* src, const int n_byte)
{
	memcpy(this->request_type, src, n_byte);
	this->request_type[n_byte] = '\0';
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
