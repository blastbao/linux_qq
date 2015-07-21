#include "service.h"

int Service::regist(const int request_sockfd, const Msg_field & request_fields, int & echo_sockfd, Msg_field & echo_fields, DB & db)
{
	/* split to get user name and password */
	char ch_name[CONST::USER_NAME_SIZE];
	char ch_psw[CONST::USER_PSW_SIZE];
	str_split(request_fields.msg_data, CONST::CHAR_GLUE_NAME_PSW, ch_name, ch_psw);

	string name(ch_name);
	string psw(ch_psw);

	/* regist */
	echo_sockfd = request_sockfd;
	
	if (db.map_regist.count(name) == 0)
	{
		/* regist */
		if ( save_registed_user(CONST::REGISTED_FILE, name, psw) < 0)
		{
			/* set echo message */
			echo_fields.set_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"注册失败");
			return -1;
		}
		db.map_regist[name] = psw;
		
		/* login */
		db.map_login_name_sock[name] = request_sockfd;
		db.map_login_sock_name[request_sockfd] = name;
		
		/* set echo message */
		echo_fields.set_fields(CONST::MSG_TYPE_RET_SUCC, (char*)"", (char*)"", (char*)"注册成功");
		return 0;
	}
	else
	{
		/* set echo message */
		echo_fields.set_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"注册失败");
		return -1;
	}
}

int Service::login(const int request_sockfd, const Msg_field & request_fields, int & echo_sockfd, Msg_field & echo_fields, DB & db)
{
	/* split to get user name and password */
	char ch_name[CONST::USER_NAME_SIZE];
	char ch_psw[CONST::USER_PSW_SIZE];
	str_split(request_fields.msg_data, CONST::CHAR_GLUE_NAME_PSW, ch_name, ch_psw);

	string name(ch_name);
	string psw(ch_psw);

	/* login */
	echo_sockfd = request_sockfd;

	if (db.map_regist.count(name) > 0 && db.map_regist[name] == psw)
	{
		if (db.map_login_name_sock.count(name) == 0)
		{
			/* login */
			db.map_login_name_sock[name] = request_sockfd;
			db.map_login_sock_name[request_sockfd] = name;
			
			/* set echo message */
			echo_fields.set_fields(CONST::MSG_TYPE_RET_SUCC, (char*)"", (char*)"", (char*)"登录成功");
			return 0;
		}
		else
		{
			/* set echo message */
			echo_fields.set_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"登录失败,账号已登录,不得重复登录");
			return 0;
		}
	}
	else
	{
		/* set echo message */
		echo_fields.set_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"登录失败,账号或密码错误");
		return -1;
	}
}

void Service::transponder(const int request_sockfd, const Msg_field & request_fields, int & echo_sockfd, Msg_field & echo_fields, DB & db)
{
	string to_name(request_fields.msg_to);

	if (db.map_login_name_sock.count(to_name) > 0)
	{
		echo_sockfd = db.map_login_name_sock[to_name];
		echo_fields.set_fields(request_fields.msg_type,
							   request_fields.msg_from,
							   request_fields.msg_to,
							   request_fields.msg_data);
	}
	else
	{
		echo_sockfd = request_sockfd;
		echo_fields.set_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"对方不在线");
	}
}

/*
* private
*/

void Service::str_split(const char* old_str, const char delimiter, char* s1, char* s2)
{
	int i;
	for (i = 0; old_str[i] != delimiter; ++ i)
		s1[i] = old_str[i];
	s1[i++] = '\0';

	int j = 0;
	for (; old_str[i] != '\0'; ++ i)
		s2[j++] = old_str[i];
	s2[j] = '\0';
}

int Service::save_registed_user(char* fname, const string & name, const string & psw)
{
	ofstream fo(fname, ios_base::app);
	if (fo.is_open())
	{
		fo << name << "\t" << psw << "\n";	 
		fo.close();
		return 0;
	}
	else
		return -1;
}
