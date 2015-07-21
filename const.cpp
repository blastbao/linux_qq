#ifndef __CONST_H__
#define __CONST_H__

/* all const variables of client and server are here */

namespace CONST
{
	/* message types */
	char MSG_TYPE_REGIST[] = "regist";
	char MSG_TYPE_LOGIN[]  = "login";
	char MSG_TYPE_MSG[]    = "msg";
	char MSG_TYPE_RET_SUCC[] = "succ";
	char MSG_TYPE_RET_FAIL[] = "fail";

	/* registed user file */
	char REGISTED_FILE[] = "registed";
};

#endif
