// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#ifndef FILESYS_H
#define FILESYS_H

#include "BasicFileSys.h"
#include "Blocks.h"

class FileSys {

public:
  // mounts the file system
  void mount(int sock);

  // unmounts the file system
  void unmount();

  // make a directory
  void mkdir(const char *name);

  // switch to a directory
  void cd(const char *name);

  // switch to home directory
  void home();

  // remove a directory
  void rmdir(const char *name);

  // list the contents of current directory
  void ls();

  // create an empty data file
  void create(const char *name);

  // append data to a data file
  void append(const char *name, const char *data);

  // display the contents of a data file
  void cat(const char *name);

  // display the first N bytes of the file
  void head(const char *name, unsigned int n);

  // delete a data file
  void rm(const char *name);

  // display stats about file or directory
  void stat(const char *name);

private:
  BasicFileSys bfs; // basic file system
  short curr_dir;   // current directory

  int fs_sock; // file server socket

  // Additional private variables and Helper functions - if desired
  struct dirblock_t return_root();

  // Additional private variables and Helper functions - if desired
  struct dirblock_t* return_root_ptr();

  int blockspace();

  void copy_name(char output[MAX_FNAME_SIZE + 1], const char *input);

  bool is_dir(short block_num);

  bool file_exists(const char *name);

  bool size_check(const char *name);

  bool compare_name(const char* name1, const char *name2);

  bool check_dir();

	int get_size(const char* data);   
};

#endif
