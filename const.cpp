#include	"const.h"

namespace CONST
{
	/* message types */
	char MSG_TYPE_REGIST[]  = "regist";
	char MSG_TYPE_LOGIN[]   = "login";
	char MSG_TYPE_LOGOUT[]  = "logout";
	char MSG_TYPE_MSG[]    = "msg";
	char MSG_TYPE_RET_SUCC[] = "succ";
	char MSG_TYPE_RET_FAIL[] = "fail";
	char MSG_TYPE_NOTI_LOGIN[]  = "noti_login";
	char MSG_TYPE_NOTI_LOGOUT[] = "noti_logout";
	char MSG_TYPE_FB[] = "fb";
	char MSG_TYPE_FI[] = "fi";
	char MSG_TYPE_FE[] = "fe";
	char MSG_TYPE_FOK[] = "fok";
	char MSG_TYPE_FNO[] = "fno";
	char MSG_TYPE_FRIENDLIST[] = "friendlist";
	char MSG_TYPE_ADDFRIEND[]  = "add";
	char MSG_TYPE_ADDOK[]	   = "addok";
	char MSG_TYPE_ADDNO[]      = "addno";

	/* user request types */
	char USER_REQUEST_TYPE_MSG[] = "to";
	char USER_REQUEST_TYPE_FTO[] = "fto";
	char USER_REQUEST_TYPE_FOK[] = "fok";
	char USER_REQUEST_TYPE_FNO[] = "fno";
	char USER_REQUEST_TYPE_FRIENDLIST[] = "list";
	char USER_REQUEST_TYPE_ADDFRIEND[]  = "add";
	char USER_REQUEST_TYPE_ADDOK[]      = "addok";
	char USER_REQUEST_TYPE_ADDNO[]      = "addno";

	/* file transfer type */
	char FILE_TYPE_DIR[] = "dir";
	char FILE_TYPE_FB[]  = "file_b";
	char FILE_TYPE_FI[]  = "file_i";
	char FILE_TYPE_FE[]  = "file_e";

	/* receiving dir for file transfering */
	char RECEIVE_DIR[] = "./data/";

	/* registed user file */
	char REGISTED_FILE[] = "registed";
	char FRIENDSHIP_FILE[] = "friendship";
};
