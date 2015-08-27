#include	"db.h"

int DB::load_rigested_user_table(const char* fname)
{
	ifstream fi(fname);
	if (fi.is_open())
	{
		string str1, str2;
		fi >> str1 >> str2;
		
		while (!fi.eof())
		{
			map_regist[str1] = str2;
			fi >> str1 >> str2;
		}

		fi.close();
		return 0;
	}
	else
		return -1;
}

int DB::load_friendship_table(const char* fname)
{
	ifstream fi(fname);
	if (fi.is_open())
	{
		string str1, str2;
		fi >> str1 >> str2;
		
		while (!fi.eof())
		{
			pair<string, string> p = make_pair(str1, str2);
			v_friendship.push_back(p);
			fi >> str1 >> str2;
		}

		fi.close();
		return 0;
	}
	else
		return -1;
}
