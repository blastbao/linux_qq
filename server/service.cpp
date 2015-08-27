#include "service.h"

int Service::regist(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db)
{
	/* split to get user name and password */
	char ch_name[CONST::USER_NAME_SIZE+1];
	char ch_psw[CONST::USER_PSW_SIZE+1];
	Util::str_split(request_fields.msg_data, CONST::CHAR_GLUE_NAME_PSW, ch_name, ch_psw);

	string name(ch_name);
	string psw(ch_psw);

	/* regist */
	v_echo_sockfd.push_back(request_sockfd);
	
	if (db.map_regist.count(name) == 0)
	{
		/* regist */
		if ( _save2file(CONST::REGISTED_FILE, name, psw) < 0)
		{
			/* set echo message */
			Msg_field echo_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"注册失败");
			v_echo_fields.push_back(echo_fields);
			return -1;
		}
		db.map_regist[name] = psw;
		
		/* login */
		db.map_login_name_sock[name] = request_sockfd;
		db.map_login_sock_name[request_sockfd] = name;
		
		/* set echo message */
		Msg_field echo_fields(CONST::MSG_TYPE_RET_SUCC, (char*)"", (char*)"", (char*)"注册成功");
		v_echo_fields.push_back(echo_fields);
		return 0;
	}
	else
	{
		/* set echo message */
		Msg_field echo_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"注册失败");
		v_echo_fields.push_back(echo_fields);
		return -1;
	}
}

int Service::login(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db)
{
	/* split to get user name and password */
	char ch_name[CONST::USER_NAME_SIZE+1];
	char ch_psw[CONST::USER_PSW_SIZE+1];
	Util::str_split(request_fields.msg_data, CONST::CHAR_GLUE_NAME_PSW, ch_name, ch_psw);

	string name(ch_name);
	string psw(ch_psw);

	/* login */
	v_echo_sockfd.push_back(request_sockfd);

	if (db.map_regist.count(name) > 0 && db.map_regist[name] == psw)
	{
		if (db.map_login_name_sock.count(name) == 0)
		{
			/* login */
			db.map_login_name_sock[name] = request_sockfd;
			db.map_login_sock_name[request_sockfd] = name;
			
			/* set echo message */
			Msg_field echo_fields(CONST::MSG_TYPE_RET_SUCC, (char*)"", (char*)"", (char*)"登录成功");
			v_echo_fields.push_back(echo_fields);

			/* set echo broadcast socket and message */
			char tmp_msg_data[CONST::MSG_DATA_SIZE+1];
			snprintf(tmp_msg_data, sizeof(tmp_msg_data), "[%s]刚刚登录，和他打个招呼吧～～", name.c_str());
			
			_broadcast_msg(name, db, CONST::MSG_TYPE_NOTI_LOGIN, tmp_msg_data, v_echo_sockfd, v_echo_fields);
			return 0;
		}
		else
		{
			/* set echo message */
			Msg_field echo_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"登录失败,账号已登录,不得重复登录");
			v_echo_fields.push_back(echo_fields);
			return 0;
		}
	}
	else
	{
		/* set echo message */
		Msg_field echo_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"登录失败,账号或密码错误");
		v_echo_fields.push_back(echo_fields);
		return -1;
	}
}

void Service::logout(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db)
{
	string name = db.map_login_sock_name[request_sockfd];
	
	char tmp_msg_data[CONST::MSG_DATA_SIZE+1];
	snprintf(tmp_msg_data, sizeof(tmp_msg_data), "your friend [%s] logout~ ~", name.c_str());

	_broadcast_msg(name, db, CONST::MSG_TYPE_NOTI_LOGOUT, tmp_msg_data, v_echo_sockfd, v_echo_fields);
}

void Service::get_friendlist(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db)
{
	string from_name(request_fields.msg_from);

	/* get friend list of user 'from_name' */
	char friend_list[CONST::MSG_DATA_SIZE+1];
	_get_friendlist(from_name, db, friend_list);

	/* set echo message */
	v_echo_sockfd.push_back(request_sockfd);
	Msg_field echo_fields(CONST::MSG_TYPE_FRIENDLIST, (char*)"", (char*)"", friend_list);
	v_echo_fields.push_back(echo_fields);
}

void Service::add_friend(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db)
{
	transponder(request_sockfd, request_fields, v_echo_sockfd, v_echo_fields, db);
	
	/* save friendship to database */
	string user1(request_fields.msg_from);
	string user2(request_fields.msg_to);
	
	_save2file(CONST::FRIENDSHIP_FILE, user1, user2);
	_save2file(CONST::FRIENDSHIP_FILE, user2, user1);

	db.v_friendship.push_back(make_pair(user1, user2));
	db.v_friendship.push_back(make_pair(user2, user1));
}

void Service::transponder(const int request_sockfd, const Msg_field & request_fields, vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields, DB & db)
{
	string to_name(request_fields.msg_to);

	if (db.map_login_name_sock.count(to_name) > 0)
	{
		int echo_sockfd = db.map_login_name_sock[to_name];
		v_echo_sockfd.push_back(echo_sockfd);
		v_echo_fields.push_back(request_fields);
	}
	else
	{
		v_echo_sockfd.push_back(request_sockfd);
		Msg_field echo_fields(CONST::MSG_TYPE_RET_FAIL, (char*)"", (char*)"", (char*)"对方不在线");
		v_echo_fields.push_back(echo_fields);
	}
}

/*
* private
*/

int Service::_save2file(const char* fname, const string & s1, const string & s2)
{
	ofstream fo(fname, ios_base::app);
	if (fo.is_open())
	{
		fo << s1 << "\t" << s2 << "\n";	 
		fo.close();
		return 0;
	}
	else
		return -1;
}

void Service::_get_friendlist(const string & user_name, const DB & db, char* friend_list)
{
	int	 friend_list_len = 0;
	for (vector<pair<string, string> >::const_iterator iter=db.v_friendship.begin(); iter!=db.v_friendship.end(); ++ iter)
	{
		if (user_name == iter->first)
		{
			/* add a friend */
			strcpy(friend_list+friend_list_len, (iter->second).c_str());
			friend_list_len += strlen( (iter->second).c_str() ) + 1;

			/* add flag that represents whether this friend logined */
			if (db.map_login_name_sock.count(iter->second) > 0)
				friend_list[friend_list_len-1] = 1;	
			else
				friend_list[friend_list_len-1] = 2;
		}
	}
	friend_list[friend_list_len] = '\0';
}

void Service::_broadcast_msg(const string & from_name, DB & db, const char* msg_type, const char* msg_data, 
							vector<int> & v_echo_sockfd, vector<Msg_field> & v_echo_fields)
{
	/* broadcast to all friends */
	for (vector<pair<string, string> >::const_iterator iter=db.v_friendship.begin(); iter!=db.v_friendship.end(); ++ iter)
	{
		if (from_name == iter->first)
		{
			if (db.map_login_name_sock.count(iter->second) > 0) /* friend is online */
			{
				/* set echo message */
				Msg_field msg_fields(msg_type, from_name.c_str(), (iter->second).c_str(), msg_data);
				v_echo_fields.push_back(msg_fields);
				
				/* set echo socket */
				int friend_sockfd = db.map_login_name_sock[iter->second];
				v_echo_sockfd.push_back(friend_sockfd);
			}
		}
	}
}
