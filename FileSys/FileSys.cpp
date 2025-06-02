// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

#include "BasicFileSys.h"
#include "Blocks.h"
#include "FileSys.h"

// mounts the file system
void FileSys::mount(int sock) {
  bfs.mount();
  curr_dir =
      1; // by default current directory is home directory, in disk block #1
  fs_sock = sock; // use this socket to receive file system operations from the
                  // client and send back response messages
}

// unmounts the file system
void FileSys::unmount() {
  bfs.unmount();
  close(fs_sock);
}

// make a directory
void FileSys::mkdir(const char *name) {
  // add code for error checking
  if (file_exists(name))
    return; // File Exists error

  if (!size_check(name))
    return; // File name exceeds limit error

  if (check_dir())
    return; // Directory is Full error

  // format new directory block
  struct dirblock_t root = return_root();
  struct dirblock_t block;
  block.magic = DIR_MAGIC_NUM;
  block.num_entries = 1; // default 1 for "home" directory
  block.dir_entries[0] =
      root.dir_entries[0]; // copy "home" directory from parent

  // acquire free block on disk (add error checking if disk allocation fails)
  block.dir_entries[0].block_num = bfs.get_free_block();
  if (block.dir_entries[0].block_num == 0)
    return; // Disk Full error

  // write information -- 1 do this first as we can rewrite info if next step
  // fails
  bfs.write_block(block.dir_entries[0].block_num, (void *)&block);

  // update parent directory -- 2 do this second/last, as if 1 & 2 does not
  // occur atomically will prevent corrupted files
  root.num_entries++; // is this a reference?
  int i = 0;
  int set = 0;
  do {
    if (root.dir_entries[i].block_num == 0) { // Fixed: assignment to comparison
      root.dir_entries[i].block_num =
          block.dir_entries[0]
              .block_num; // Fixed: incorrect reference to block_num
      set = 1;
    }
    i++; // Fixed: missing semicolon
  } while (set == 0); // Fixed: assignment to comparison
  copy_name(root.dir_entries[i - 1].name,
            name); // copy name of new directory to parent
  bfs.write_block(curr_dir, (void *)&root);
}

// switch to a directory
void FileSys::cd(const char *name) {
  // search root directory for filename
  struct dirblock_t root = return_root();
  struct dirblock_t *child; // assume filename will be a directory initially
  int i = 0;
  int set = 0;
  do {
    if (compare_name(name,
                     root.dir_entries[i].name)) { // Fixed: missing parentheses
      bfs.read_block(root.dir_entries[i].block_num, (void *)child);
      if (child->magic != DIR_MAGIC_NUM) // Fixed: pointer dereference
        return;                          // Not a directory error
      // assume child is a directory and change curr_dir to match
      curr_dir = root.dir_entries[i].block_num;
      set = 1;
      return; // successful
    }
    i++;
  } while (set == 0 && i < MAX_DIR_ENTRIES);
  return; // File Does not Exist error
}

// switch to home directory
void FileSys::home() {
  // go to num block 1
  struct dirblock_t root = return_root();
  curr_dir = root.dir_entries[0].block_num;
  return; // assume always successful
}

// remove a directory
void FileSys::rmdir(const char *name) {
  // error checking
  if (!file_exists(name))
    return; // FDNE error may be redundant on performance considering check at
            // bottom

  struct dirblock_t root = return_root();
  int i = 0; // do not let user delete "home" directory or when i=0
  do {
    if (compare_name(root.dir_entries[i].name,
                     name)) { // Fixed: missing parentheses
      // if names match, confirm file is directory
      if (!is_dir(root.dir_entries[i].block_num))
        return; // File is not a directory error
      struct dirblock_t *child;
      bfs.read_block(root.dir_entries[i].block_num, (void *)child);
      // check if directory is empty
      if (child->num_entries > 1)
        return; // Directory is not empty error
      // free memory first, then delete entry from parent
      bfs.reclaim_block(root.dir_entries[i].block_num);
      root.dir_entries[i].block_num = 0;
      // overwrite name to avoid corruption/stale names
      int k = 0;
      while (root.dir_entries[i].name[k] != '\0') {
        root.dir_entries[i].name[k] = '\0';
        k++;
      }
      return; // return successful
    }
  } while (i++ < root.num_entries); // Fixed: do-while syntax
  return; // FDNE error (logically impossible with prior check)
}

// list the contents of current directory
void FileSys::ls() {
  struct dirblock_t root = return_root(); // Fixed: missing variable name
  // do not print "home" directory
	cout<<"before now";
  for (int i = 0; i <= root.num_entries; i++) {
    printf("%s ", root.dir_entries[i].name);
    cout << root.dir_entries[i].name << " ";
  }
}

// create an empty data file
void FileSys::create(const char *name) {
  if (file_exists(name))
    return; // File exists error
  if (!size_check(name))
    return; // name too long error
  if (check_dir())
    return; // Directory full error

  // get free block for inode
  short block_num = bfs.get_free_block();
  if (block_num == 0)
    return; // Disk full error

  // format inode block
  short blocks[MAX_DATA_BLOCKS];
  struct inode_t inode;
  inode.magic = INODE_MAGIC_NUM;
  inode.size = 0; // 0 for new file, indicates ready for data

  // write inode to disk
  bfs.write_block(block_num, (void *)&inode); // Fixed: pointer usage

  // update parent directory
  struct dirblock_t root = return_root();

  int index = 0;
  root.num_entries++;
  int i = 0;
  int set = 0;
  do {
    if (root.dir_entries[i].block_num == 0) { // Fixed: assignment to comparison
      root.dir_entries[i].block_num =
          block_num; // set the first available directory entry to inode block
                     // number
      set = 1;
    }
    i++; // Fixed: missing semicolon
  } while (set == 0); // Fixed: assignment to comparison
  copy_name(root.dir_entries[i - 1].name,
            name); // copy name of new file to parent directory
  bfs.write_block(curr_dir, (void *)&root);
}

// append data to a data file
void FileSys::append(const char *name, const char *data) {
  if (!file_exists(name))
    return; // FDNE error
  struct dirblock_t root = return_root();
  int i = 1; // skip "home" directory
  do {
    if (compare_name(root.dir_entries[i].name,
                     name)) { // Fixed: missing parentheses
      // if names match, confirm file is an inode
      if (is_dir(root.dir_entries[i].block_num))
        return; // File is a directory error
      // read inode info
      struct inode_t inode;
      bfs.read_block(root.dir_entries[i].block_num,
                     (void *)&inode); // Fixed: pointer usage
      // check current inode for size allowed (in bytes) (((128 - 8)/2) max
      // blocks * 128 bytes/block = 7680 bytes/inode MAX
      int maxsize = 7680 - inode.size;
      int remaining = get_size(data);
      if (remaining > maxsize)
        return; // Appended exceeds max size error
      // find available # of bytes in last datablock
      int fill = inode.size % 128;
      // append to last datablock if possible
      // get new data block and fill (loop while remaining)
    }
  } while (i++ < root.num_entries); // Fixed: do-while syntax
}

// display the contents of a data file
void FileSys::cat(const char *name) {}

// display the first N bytes of the file
void FileSys::head(const char *name, unsigned int n) {}

// delete a data file
void FileSys::rm(const char *name) {}

// display stats about file or directory
void FileSys::stat(const char *name) {}

// HELPER FUNCTIONS (optional)

// returns the dirblock_t structure corresponding to the current parent
// directory, copying the format of the first directory
struct dirblock_t FileSys::return_root() {
  struct dirblock_t root;
  bfs.read_block(curr_dir, (void *)&root); // Fixed: pointer usage
  return root;
}

// copies the input pointer into the output array, assumes the input is null
// terminated and null terminates the output
void FileSys::copy_name(char output[MAX_FNAME_SIZE + 1], const char *input) {
  int i = 0;
  while (input[i] != '\0') {
    output[i] = input[i];
    i++;
  }
  output[i] = '\0';
}

// returns the size of the data buffer in bytes (assumes 1 byte/char) (assume
// data is null terminated)
int FileSys::get_size(const char *data) { // Fixed: missing scope resolution
  int size = 0;
  while (data[size] != '\0') {
    size++;
  }
  return size;
}

// checks the given block to see if it is a directory or not (assume block_num
// given is valid)
bool FileSys::is_dir(short block_num) {
  struct dirblock_t child;
  bfs.read_block(block_num, (void *)&child); // Fixed: pointer usage
  if (child.magic == DIR_MAGIC_NUM)          // Fixed: assignment to comparison
    return true;
  return false;
}

// checks to see if the name of the created file/directory already exists
// inside this directory returns 1 if the current file/directory exists, 0
// otherwise
bool FileSys::file_exists(const char *name) {
  struct dirblock_t root = return_root();
  // search each directory within the root
  for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
    // get the name of each directory entry
    if (root.dir_entries[i].block_num == 0)
      return false; // no more entries to check
    if (compare_name(root.dir_entries[i].name, name))
      return true; // matching name found, Fixed: missing parentheses
  }
  return false; // checked all entries without match
}

// checks the size of the char array used for the name of the file/directory
// returns 1 if the size is acceptable, 0 otherwise
bool FileSys::size_check(const char *name) {
  int size = 0;
  for (int i = 0; i < MAX_FNAME_SIZE + 1; i++) {
    if (name[i] == '\0') // Fixed: assignment to comparison
      break;             // exit loop on null terminator
    size++;
  }
  // assume at least 1 char and +1 for null terminator
  if (size > MAX_FNAME_SIZE + 1)
    return false;
  return true;
}

// compares two separate char* names assumes name1 and name2 are null terminated
// returns True if they match, false otherwise.
bool FileSys::compare_name(const char name1[MAX_FNAME_SIZE + 1],
                           const char *name2) { // Fixed: array syntax
  for (int i = 0; i < MAX_FNAME_SIZE + 1;
       i++) { // iterate through both names, assume names are null terminated
    if (name1[i] != name2[i])
      return false; // if chars do not match
    if (name1[i] == '\0')
      return true; // reached end of strings without failure
  }
  return true; // reached max FNAME size without failure
}

// check if the directory is full before write
// returns 1 if directory is full, 0 otherwise
bool FileSys::check_dir() {
  struct dirblock_t block; // Fixed: incorrect type blockdir_t
  bfs.read_block(curr_dir, (void *)&block);

  if (block.num_entries == MAX_DIR_ENTRIES) // Fixed: assignment to comparison
    return true;
  return false;
}
