#ifndef SERVER
#define SERVER
#include <cstddef>
#include <string>

void read_in(int sock, char* buf, size_t buf_size);

struct Command
{
	std::string name;		// name of command
	std::string file_name;		// name of file
	std::string append_data;	// append data (append only)
};

struct Command get_cmd();
void execute_cmd(struct Command cmd);

#endif
