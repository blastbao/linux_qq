#include	"msg_util.h"

/*
* Msg_field define
*/

Msg_field::Msg_field(const char* type, const char* from, const char* to, const char* data) {
	this->set_fields(type, from, to, data);	
}

Msg_field::Msg_field() {}

void Msg_field::set_fields(const char* type, const char* from, const char* to, const char* data)
{
	strcpy(msg_type, type);
	strcpy(msg_from, from);
	strcpy(msg_to, to);
	strcpy(msg_data, data);
}

/*
* Msg_util define
*/

int Msg_util::packing(const Msg_field* p_msg_field, char* msg)
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

bool Msg_util::unpacking(const char* msg, const int msg_len, Msg_field* p_msg_field)
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

int Msg_util::err_packing(const char* err_info, char* msg)
{
	Msg_field fields((char*) "ERR", (char*) "", (char*) "", err_info);
	return packing(&fields, msg);
}
