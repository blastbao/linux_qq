#include	"fsocket.h"

bool Fsocket_s::fsock_connect(const char* user_name_r, const char* path)
{
	if ( strlen(path) > CONST::MAX_DIR_LEN)
		return false;

	/* get dir */
	char file[CONST::MAX_DIR_LEN+1];
	if ( !split_dir_file(path, _dir, file))
		return false;

	/* get all files */
	_n_file = 0;
	_p_file = 0;
	_fd     = -1;
	scan(_dir, file);
	
	/* get user name */
	strcpy(_user_name_r, user_name_r);

	_is_trans = false;

	printf("user name: %s\n", _user_name_r);
	printf("dir : %s\n", _dir);
	printf("file # : %d\n", _n_file);
	for (int i = 0; i < _n_file; ++ i)
		printf("%s\n", _files[i]);

	return true;
}

int Fsocket_s::fsock_read(char* buff)
{
	if (_p_file >= _n_file)
		return 0;

	char path[CONST::MAX_DIR_LEN+1];
	strcpy(path, _dir);
	strcat(path, _files[_p_file]);

	char file_type[CONST::FILE_TYPE_SIZE+1];
	char data[CONST::FILE_DATA_BLOCK_SIZE+1];

	if ( is_dir(path))
	{
		strcpy(file_type, CONST::FILE_TYPE_DIR);
		strcpy(data, _files[_p_file ++]);
	}
	else
	{
		if (_fd < 0)
		{
			if ( (_fd = open(path, O_RDONLY)) < 0)
				return -1;

			strcpy(file_type, CONST::FILE_TYPE_FB);
			strcpy(data, _files[_p_file]);
		}
		else
		{
			int n;
			if ( (n = read(_fd, data, CONST::FILE_DATA_BLOCK_SIZE)) < 0)
				return -1;

			if (n > 0)
			{
				strcpy(file_type, CONST::FILE_TYPE_FI);
				data[n] = '\0';
			}
			else
			{
				strcpy(file_type, CONST::FILE_TYPE_FE);
				data[0] = '\0';
				
				++ _p_file;
				close(_fd);
				_fd = -1;
			}
		}
	}

	Util::str_glue(file_type, data, CONST::CHAR_GLUE_NAME_PSW, buff);
	return 1;
}

void Fsocket_s::fsock_close()
{
	_user_name_r[0] = '\0';
	_files[0][0] = '\0';
	_n_file = 0;
	_p_file = 0;
	_fd     = -1;
	
	_is_trans = false;
}

void Fsocket_r::fsock_accept(const char* user_name_s, const char* file)
{
	strcpy(_user_name_s, user_name_s);
	strcpy(_dir, CONST::RECEIVE_DIR);
	strcpy(_file, file);
	_fd = -1;
	_is_trans = false;

	printf("from:%s\n", _user_name_s);
	printf("dir:%s\n", _dir);
	printf("file:%s\n", _file);
}

int Fsocket_r::fsock_write(const char* buff)
{
	char file_type[CONST::FILE_TYPE_SIZE+1];
	char data[CONST::FILE_DATA_BLOCK_SIZE+1];

	Util::str_split(buff, CONST::CHAR_GLUE_NAME_PSW, file_type, data);
	
	if ( strcmp(file_type, CONST::FILE_TYPE_DIR) == 0)
	{
		char path[CONST::MAX_DIR_LEN+1];
		strcpy(path, _dir);
		strcat(path, data);
	
		if ( mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
			return -1;
	}
	else
	{
		if ( strcmp(file_type, CONST::FILE_TYPE_FB) == 0)
		{
			if (_fd >= 0)
				return -1;
			
			char path[CONST::MAX_DIR_LEN+1];
			strcpy(path, _dir);
			strcat(path, data);

			if ( (_fd = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IROTH)) < 0)
				return -1;
		}
		else if ( strcmp(file_type, CONST::FILE_TYPE_FI) == 0)
		{
			if (_fd < 0)
				return -1;

			int n_data;
			n_data = strlen(data);
			
			int n_write;
			if ( (n_write = write(_fd, data, n_data)) < 0)
				return -1;
		}
		else if ( strcmp(file_type, CONST::FILE_TYPE_FE) == 0)
		{
			if (_fd < 0)
				return -1;
			
			close(_fd);
			_fd = -1;
		}
		else if ( strcmp(file_type, CONST::MSG_TYPE_FE) == 0)
			return 0;
		else
			return -1;
	}

	return 1;
}

bool Fsocket_r::fsock_is_exist(const char* user_name_s, const char* file)
{
	if ( strcmp(_user_name_s, user_name_s) == 0 && strcmp(_file, file) == 0)
		return true;
	return false;
}

void Fsocket_r::fsock_close()
{
	_user_name_s[0] = '\0';
	_file[0] = '\0';
	_fd = -1;
	_is_trans = false;
}

/*
* private
*/

bool Fsocket_s::is_exist(const char* path)
{
	struct stat stat_buff;
	
	if ( lstat(path, &stat_buff) < 0)
		return false;

	if ( S_ISREG(stat_buff.st_mode) || S_ISDIR(stat_buff.st_mode))
		return true;

	return false;
}

bool Fsocket_s::is_dir(const char* path)
{
	struct stat stat_buff;
	
	if ( lstat(path, &stat_buff) < 0)
		return false;

	if ( S_ISDIR(stat_buff.st_mode))
		return true;

	return false;
}

bool Fsocket_s::split_dir_file(const char* path, char* dir_path, char* file_name)
{
	if ( !is_exist(path))
		return false;

	int slen;
	int pos, i, l;
	
	slen = strlen(path);
	if (path[slen-1] == '/')
		-- slen;

	for (pos = slen - 1; pos >= 0; -- pos)
		if (path[pos] == '/')
			break;
		else
			continue;

	for (i = pos + 1; i < slen; ++ i)
		file_name[i-pos-1] = path[i];
	file_name[i-pos-1] = '\0';

	for (i = 0; i <= pos; ++ i)
		dir_path[i] = path[i];
	dir_path[i] = '\0';

	if (strcmp(file_name, (char*) ".") == 0 || 
		strcmp(file_name, (char*) "..") == 0)
		return false;

	return true;
}

void Fsocket_s::scan(const char* dir, const char* file)
{
	strcpy(_files[_n_file], file);
	++ _n_file;

	char path[CONST::MAX_DIR_LEN+1];
	strcpy(path, dir);
	strcat(path, file);

	if ( is_dir(path))
	{
		DIR* p_dir;
		struct dirent* p_dirent;

		p_dir = opendir(path);
		while ( (p_dirent = readdir(p_dir)) != NULL)
		{
			if ( strcmp(p_dirent->d_name, (char*)".") == 0 || 
				 strcmp(p_dirent->d_name, (char*)"..") == 0)
				continue;

			char child_file[CONST::MAX_DIR_LEN+1];
			strcpy(child_file, file);
			strcat(child_file, (char*) "/");
			strcat(child_file, p_dirent->d_name);

			scan(dir, child_file);
		}
		closedir(p_dir);
	}
}
