// CPSC 3500: File Sys
// Implements the file system commands that are available to the shell.
#include <cmath>
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

void FileSys::mkdir(const char *name) {
  if (file_exists(name))
    return; // File Exists error
  if (!size_check(name))
    return; // File name exceeds limit error
  if (check_dir())
    return; // Directory is Full error
  struct dirblock_t root = return_root();
  struct dirblock_t block;
  block.magic = DIR_MAGIC_NUM;
  block.num_entries = 1; // Default 1 for "home" in new directory
  block.dir_entries[0] = root.dir_entries[0]; // Copy "home" from parent
  block.dir_entries[0].block_num = bfs.get_free_block();
  if (block.dir_entries[0].block_num == 0)
    return; // Disk Full error
  bfs.write_block(block.dir_entries[0].block_num, (void *)&block);
  root.num_entries++;
  int i = 0;
  int set = 0;
  do {
    if (i >= MAX_DIR_ENTRIES) {
      bfs.reclaim_block(block.dir_entries[0].block_num);
      return; // Directory Full error
    }
    if (root.dir_entries[i].block_num == 0) {
      root.dir_entries[i].block_num = block.dir_entries[0].block_num;
      copy_name(root.dir_entries[i].name, name); // Set name at current index
      set = 1;
    }
    i++;
  } while (set == 0);
  bfs.write_block(curr_dir, (void *)&root);
}
// switch to a directory
void FileSys::cd(const char *name) {
  // search root directory for filename
  struct dirblock_t root = return_root();
  struct dirblock_t child; // assume filename will be a directory initially
  int i = 0;
  int set = 0;
  do {
    if (compare_name(name,
                     root.dir_entries[i].name)) { // Fixed: missing parentheses
      bfs.read_block(root.dir_entries[i].block_num, (void *)&child);
      if (child.magic != DIR_MAGIC_NUM) // Fixed: pointer dereference
        return;                         // Not a directory error
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
  curr_dir = 1; // root.dir_entries[0].block_num;
  return;       // assume always successful
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
      if (!is_dir(root.dir_entries[i].block_num)) {
				cout << name <<" is not a directory\n";
        return; // File is not a directory error
			}
      struct dirblock_t child;
      bfs.read_block(root.dir_entries[i].block_num, (void *)&child);
      // check if directory is empty
      if (child.num_entries > 1)
        return; // Directory is not empty error
                // free memory first, then delete entry from parent
      bfs.reclaim_block(root.dir_entries[i].block_num);
      root.dir_entries[i].block_num = 0;
      root.num_entries--;
      // overwrite name to avoid corruption/stale names
      int k = 0;
      while (root.dir_entries[i].name[k] != '\0') {
        root.dir_entries[i].name[k] = '\0';
        k++;
      }
      bfs.write_block(curr_dir, (void *)&root);
      return; // return successful
    }
  } while (i++ < root.num_entries); // Fixed: do-while syntax
  return; // FDNE error (logically impossible with prior check)
}

// list the contents of current directory
void FileSys::ls() {
  struct dirblock_t root = return_root(); // Fixed: missing variable name
                                          // do not print "home" directory
	cout << "Directories:\n"; 
  int dir_count = 0;
  if (root.num_entries <= 0) {
    cout << "empty directory\n";
  }
  for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
    if (root.dir_entries[i].block_num != 0 && dir_count < root.num_entries) {
      cout << root.dir_entries[i].name << '\n';
      dir_count++;
    }
  }
	cout << endl;
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

void FileSys::append(const char *name, const char *data) {
  if (!file_exists(name)) {
    cout << "file name: " << name
         << " doesn't exist, can't enter this: " << data << '\n';
    return;
  }
  struct dirblock_t root = return_root();
  int i = 0;
  while (i < root.num_entries && i < MAX_DIR_ENTRIES) {
    if (compare_name(root.dir_entries[i].name, name)) {
      if (is_dir(root.dir_entries[i].block_num)) {
        cout << "Error: " << name << " is a directory\n";
        return;
      }
      struct inode_t inode;
      bfs.read_block(root.dir_entries[i].block_num, (void *)&inode);
      int maxsize = 7680 - inode.size;
      int remaining = strlen(data);
      if (remaining > maxsize) {
        cout << "Error: Append exceeds max size for " << name << '\n';
        return;
      }
      int blocksavailable = blockspace();
      if (blocksavailable <= 0) {
        cout << "Error: No disk space available\n";
        return;
      }
      int fill = 128 - (inode.size % 128);
      int blockstoadd = (remaining - fill + 127) / 128;
      if (blockstoadd > blocksavailable) {
        cout << "Error: Insufficient blocks available\n";
        return;
      }
      int offset = 0;
      int initial_size = inode.size; // Track initial size for block index
      if (fill != 0 && inode.size > 0) {
        struct datablock_t lastblock;
        int last_block_idx = (inode.size - 1) / 128; // Correct index
        cout << "Last block index: " << last_block_idx
             << ", block_num: " << inode.blocks[last_block_idx] << endl;
        bfs.read_block(inode.blocks[last_block_idx], (void *)&lastblock);
        for (int j = fill; j < 128 && remaining > 0; j++) {
          lastblock.data[j] = data[offset++];
          remaining--;
          inode.size++;
        }
        bfs.write_block(inode.blocks[last_block_idx], (void *)&lastblock);
      }
      int current_block_idx = inode.size / 128; // Index for new blocks
      while (remaining > 0) {
        struct datablock_t newblock = {0};
        short block_num = bfs.get_free_block();
        if (block_num == 0) {
          cout << "Error: No free block available\n";
          return;
        }
        for (int j = 0; j < 128 && remaining > 0; j++) {
          newblock.data[j] = data[offset++];
          remaining--;
          inode.size++;
        }
        bfs.write_block(block_num, (void *)&newblock);
        if (current_block_idx >= MAX_DATA_BLOCKS) {
          cout << "Error: Exceeded max blocks for " << name << '\n';
          return;
        }
        inode.blocks[current_block_idx] = block_num;
        current_block_idx++;
      }
      bfs.write_block(root.dir_entries[i].block_num, (void *)&inode);
      return;
    }
    i++;
  }
  cout << "File " << name << " not found in directory\n";
}

void FileSys::cat(const char *name) {
  if (!file_exists(name)) {
    cout << "file name doesn't exist\n";
    return; // FDNE error
  }
  struct dirblock_t root = return_root();
  int i = 0; // Start at 0 to include all entries, adjust based on "home"
  bool found = false;
  do {
    if (compare_name(root.dir_entries[i].name, name)) {
      found = true;
      if (is_dir(root.dir_entries[i].block_num)) {
        cout << "Error: " << name << " is a directory\n";
        return;
      }
      struct inode_t inode;
      bfs.read_block(root.dir_entries[i].block_num, (void *)&inode);
      if (inode.size == 0) {
        cout << "File " << name << " is empty\n";
        return;
      }
      // Read and print data from blocks
      for (int j = 0; j < inode.size / 128 + (inode.size % 128 ? 1 : 0); j++) {
        if (j >= MAX_DATA_BLOCKS) { // Prevent out-of-bounds
          cout << "Error: Exceeded max blocks for " << name << endl;
          return;
        }
        struct datablock_t block;
        bfs.read_block(inode.blocks[j], (void *)&block);
        for (int k = 0; k < 128 && (j * 128 + k) < inode.size; k++) {
          cout << block.data[k];
        }
      }
      cout << endl;
      return;
    }
    i++;
  } while (i < root.num_entries && i < MAX_DIR_ENTRIES);
  if (!found) {
    cout << "File " << name
         << " not found in directory\n"; // Shouldn't reach here due to
                                         // file_exists
  }
}

// display the first N bytes of the file
void FileSys::head(const char *name, unsigned int n) {
  if (!file_exists(name))
    return; // FDNE error
  struct dirblock_t root = return_root();
  int i = 1; // skip "home" directory
  do {
    if (compare_name(root.dir_entries[i].name, name)) {
      // if names match, confirm file is an inode
      if (is_dir(root.dir_entries[i].block_num))
        return; // File is a directory error
      // read inode info
      struct inode_t inode;
      bfs.read_block(root.dir_entries[i].block_num, (void *)&inode);
      // set bytes to write to n or inode size (if n>size)
      int remaining = n;
      if (n > inode.size)
        remaining = inode.size;
      // read content from each data block attached to inode (remaining to break
      // loop)
      while (remaining > 0) {
        // loop through each block inside inode
        for (int j = 0; j < inode.size / 128; j++) {
          struct datablock_t block;
          bfs.read_block(inode.blocks[j], (void *)&block);
          // loop through each element inside block and print
          for (int k = 0; k < 128; k++) {
            if (remaining <= 0)
              break;
            cout << block.data[k];
            remaining--;
          }
        }
      }
    }
  } while (i++ < root.num_entries);
}

// delete a data file
void FileSys::rm(const char *name) {
  if (!file_exists(name))
    return; // FDNE error
  struct dirblock_t root = return_root();
  int i = 0; // skip "home" directory
  do {
    if (compare_name(root.dir_entries[i].name, name)) {
      // if names match, confirm file is an inode
      if (is_dir(root.dir_entries[i].block_num))
        return; // File is a directory error
      // read inode info
      struct inode_t inode;
      bfs.read_block(root.dir_entries[i].block_num, (void *)&inode);
      // calculate #datablocks in inode
      int blocks = inode.size / 128;
      // reclaim datablocks
      for (int i = 0; i <= blocks; i++) {
        bfs.reclaim_block(inode.blocks[i]);
      }
      // reclaim inode
      bfs.reclaim_block(root.dir_entries[i].block_num);
      // update root directory
      root.dir_entries[i].block_num = 0;
      // overwrite name to avoid corruption/stale names
      int k = 0;
      while (root.dir_entries[i].name[k] != '\0') {
        root.dir_entries[i].name[k] = '\0';
        k++;
      }
      // write updated root to memory
      bfs.write_block(curr_dir, (void *)&root);
      return; // return successful
    }
  } while (i++ < root.num_entries);
}

// display stats about file or directory
void FileSys::stat(const char *name) {
  if (!file_exists(name))
    return; // FDNE error
  struct dirblock_t root = return_root();
  int i = 0; // skip "home" directory
  do {
    if (compare_name(root.dir_entries[i].name, name)) {
      // if names match determine whether it is a directory or inode
      // assume inode init
      struct inode_t inode;
      bfs.read_block(root.dir_entries[i].block_num, (void *)&inode);
      if (inode.magic == INODE_MAGIC_NUM) { // if inode
        printf("Inode block: %d \n", root.dir_entries[i].block_num);
        printf("Bytes in file: %d \n", inode.size);

        printf("Number of blocks: %ld\n",
               (long)ceil(inode.size / 128.0)); // if inode.size is long or int
        printf("First block: %d\n", inode.blocks[0]); // if blocks[0] is an int

      } else { // if directory
        struct dirblock_t block;
        bfs.read_block(root.dir_entries[i].block_num, (void *)&block);
        printf("Directory name: %s \n", root.dir_entries[i].name);
        printf("Directory block: %d \n", root.dir_entries[i].block_num);
      }
    }
  } while (i++ < root.num_entries);
}
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

int FileSys::blockspace() {
  int output = 0;
  // get superblock
  struct superblock_t super_block;
  bfs.read_block(0, (void *)&super_block);

  // look for available blocks
  for (int byte = 0; byte < BLOCK_SIZE; byte++) {

    // check to see if byte has available slot
    if (super_block.bitmap[byte] != 0xFF) {

      // loop to check each bit
      for (int bit = 0; bit < 8; bit++) {
        int mask = 1 << bit;
        if (mask & ~super_block.bitmap[byte]) {
          // Available block is found: increment available block space
          output++;
        }
      }
    }
  }
  return output;
}
