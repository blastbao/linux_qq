/*
* File Socket class
*/

#ifndef __FSOCKET_H__
#define __FSOCKET_H__

#include	"../util.h"
#include	"../const.h"

#include	<sys/fcntl.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<dirent.h>

#include	<string.h>

#include	<stdio.h>

/*
*A File Socket is determined by 
* <user name of other side, file name that transfered> pair
*/

/*
*File Socket of Sender
*/
class Fsocket_s
{

public:

	/*
	*build a half connection for file transfering
	*@param user_name_r in
		user name of receiver
	*@param path in
		the path of file that will be transfered
	*@return 
		true  if success;
		false if the path of file is wrong,
			  or the size of path exceeds the limitation.
	*/
	bool fsock_connect(const char* user_name_r, const char* path);

	/*
	*read data from the file that will be transfered to buff
	*@param buff out
		the buffer used to hold data, 
		the size of buff must greater than the size of message data field,
		it must be end with '\0'.
		
		the data in buff contains two parts:
		first part is flag, 
			which is used to represent whether data is a dir or file
		second part is data, 
			if flag represents a dir, the data will be the dir name,
			if flag represents a file begining, the data will be the file name,
			if flag represents a file middle, the data will be the file data,
			if flag represents a file ending, the data will be invalid.
	*@return
		1 if success
		0 if transfering finish
		-1 if system error occur
	*/
	int fsock_read(char* buff);
	
	void fsock_close();

	/* attribute */
	char* get_othside_name() {return _user_name_r;}
	char* get_file_name() {return _files[0];}
	void  set_trans() {_is_trans = true;}
	bool  get_trans() {return _is_trans;}

private:

	char _user_name_r[CONST::USER_NAME_SIZE+1]; /* user name of receiver */
	
	char _dir[CONST::MAX_DIR_LEN+1];						/* dir path */
	
	char _files[CONST::MAX_FILE_NUM][CONST::MAX_DIR_LEN+1];	/* file names that will be transfered */
	int  _n_file;											/* file number */
	int  _p_file;											/* the pointer that points to the next file will be transfered */
	int  _fd;												/* if the file pointed by _p_file is a original file, 
																_fd >= 0 represents this file is opening, 
																_fd < 0 represents this file has not opened 
															   if the file pointed by _p_file is a dir,
															   	_fd is invalid */

	bool _is_trans;	/* whether this class is transfering file */

	/*
	*split dir path and file name from path
	*@param dir_path out
		it must end with '/'
	*@param file_name out
		it must not begin with '/'
	*@return
		true  if success
		false if path does not exist, or file name is '.' or '..'
	*/
	bool split_dir_file(const char* path, char* dir_path, char* file_name);
	/*
	*write files to _files
	*/
	void scan(const char* dir, const char* file);

	bool is_exist(const char* path);
	bool is_dir(const char* path);
};

/*
*File Socket of receiver
*/
class Fsocket_r
{

public:

	/*
	*assuming this function always succeed
	*/
	void fsock_accept(const char* user_name_s, const char* file);

	/*
	*write buff to the approprite file
	*@param buff in
		it contains two parts,
		please look at fsock_read() function to probe two parts
	*@return 
		1  if success
		0  if file transfering finish
		-1 if system error occur
	*/
	int fsock_write(const char* buff);
	
	bool fsock_is_exist(const char* user_name_s, const char* file);

	void fsock_close();

	/* attribute */
	void  set_trans() {_is_trans = true;}
	bool  get_trans() {return _is_trans;}

private:

	char _user_name_s[CONST::USER_NAME_SIZE+1]; /* user name of sender */

	char _dir[CONST::MAX_DIR_LEN+1];			/* dir path */
	char _file[CONST::MAX_DIR_LEN+1];			/* file name that will be received */

	int  _fd;									/* if a original file will be received,
													_fd >= 0 represents this file is opening,
													_fd < 0 represents this file has not be created 
												   if a dir will be received,
												   	_fd is invalid */
	
	bool _is_trans;	/* whether this class is transfering file */
};

#endif
