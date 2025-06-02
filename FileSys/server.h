#ifndef SERVER
#define SERVER
#include <cstddef>
#include <string>
#include <sys/types.h>
#include "FileSys.h"

ssize_t read_in(int sock, char* buf, size_t buf_size);

struct Command
{
	char* name;		// name of command
	char* file_name;		// name of file
	char* append_data;	// append data (append only)
};

struct Command get_cmd(char* buf);
bool execute_cmd(struct Command cmd, FileSys &fs);

#endif
