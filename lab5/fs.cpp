/* CS 361 Spring 2019
*  FUSE Project Template by Dr. Stephen Marz
*  Author: Casey Lemon
*  Date: 12 April 2019
*/

#ifndef __cplusplus
#error "You must compile this using C++"
#endif
#include <fuse.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fs.h>

using namespace std;

/* Use debugf() and NOT printf() for your messages.
* Uncomment #define DEBUG in block.h if you want messages to show
*/

/* Here is a list of error codes you can return for the fs_xxx() functions
*  EPERM          1       Operation not permitted 
*  ENOENT         2       No such file or directory 
*  ESRCH          3       No such process 
*  EINTR          4       Interrupted system call 
*  EIO            5       I/O error 
*  ENXIO          6       No such device or address 
*  ENOMEM        12       Out of memory 
*  EACCES        13       Permission denied 
*  EFAULT        14       Bad address 
*  EBUSY         16       Device or resource busy 
*  EEXIST        17       File exists 
*  ENOTDIR       20       Not a directory 
*  EISDIR        21       Is a directory 
*  EINVAL        22       Invalid argument 
*  ENFILE        23       File table overflow 
*  EMFILE        24       Too many open files 
*  EFBIG         27       File too large 
*  ENOSPC        28       No space left on device 
*  ESPIPE        29       Illegal seek 
*  EROFS         30       Read-only file system 
*  EMLINK        31       Too many links 
*  EPIPE         32       Broken pipe 
*  ENOTEMPTY     36       Directory not empty 
*  ENAMETOOLONG  40      The name given is too long
*/

/* Use debugf and NOT printf() to make your
* debug outputs. Do not modify this function.
*/

#if defined(DEBUG)
int debugf(const char *fmt, ...)
{
	int bytes = 0;
	va_list args;
	va_start(args, fmt);
	bytes = vfprintf(stderr, fmt, args);
	va_end(args);
	return bytes;
}
#else
int debugf(const char *fmt, ...)
{
	return 0;
}
#endif

/*
* START HERE W/ fs_drive()
*/
#include <map>
#include <vector>

/* Custom Functions */
static void print_node(NODE *inode);

/* Globals */
map <string, NODE *> inodes_map;
map <string, NODE *>::iterator node_it;
map<uint64_t, BLOCK *> block_map;
map<uint64_t, BLOCK *>::iterator block_it;
BLOCK_HEADER *block_header;

/*
* Read the hard drive file specified by dname
* into memory. You may have to use globals to store
* the nodes and / or blocks.
* Return 0 if you read the hard drive successfully (good MAGIC, etc).
* If anything fails, return the proper error code (-EWHATEVER)
* Right now this returns -EIO, so you'll get an Input/Output error
* if you try to run this program without programming fs_drive.
*/
int fs_drive(const char *dname){
	debugf("fs_drive: %s\n", dname);

    /* Open hard_drive, which is a file */
    FILE *harddrive = fopen(dname,"r");
    if(harddrive == NULL) return -EPERM;

    /* Read in block header info */
    block_header = (BLOCK_HEADER *) malloc(sizeof(BLOCK_HEADER));
    if(fread(block_header, sizeof(BLOCK_HEADER), 1, harddrive) != 1) return -EPERM;
    if(strncmp(block_header->magic, MAGIC, sizeof(MAGIC)) != 0) return -EINVAL;

    debugf("Blocksize = %u, Nodes = %u, Blocks = %u\n", block_header->block_size, block_header->nodes, block_header->blocks);

    /* Read in each node */
    uint64_t block_num;
    for(unsigned int i = 0; i < block_header->nodes; i++){
        /* Create a new node and read it in */
        NODE *inode = (NODE *) malloc(sizeof(NODE));
        if(fread(inode, ONDISK_NODE_SIZE, 1, harddrive) != 1) return -EPERM;
        inode->blocks = NULL;

        /* If reading in a file, need to read in it's data order so read in it's list number */
        if((inode->mode & ~(0xfff)) == S_IFREG){
            block_num = (inode->size / block_header->block_size) + 1;
			inode->blocks = (uint64_t *) malloc(block_num * sizeof(uint64_t));
            if(fread(inode->blocks, 1, sizeof(uint64_t) * block_num, harddrive) != sizeof(uint64_t) * block_num) return -EPERM;
        }

        debugf("NODE: %s [%u]\n", inode->name, inode->id);

        /* Insert into inodes_map */
        inodes_map.insert(std::pair<string, NODE *>(inode->name, inode));
    }

    for(unsigned int i = 0; i < block_header->blocks; i++){
        /* Create new block and data, read it in */
        BLOCK *block = (BLOCK *) malloc(sizeof(BLOCK));
        block->data = (char *) malloc(sizeof(char) * block_header->block_size);
        if(fread(block->data, sizeof(char), block_header->block_size, harddrive) != block_header->block_size) return -EPERM;
        
        /* Insert into block_map */
        block_map.insert(std::pair<uint64_t, BLOCK *>(i, block));
    }

	return 0;
}

/*
* Open a file <path>. This really doesn't have to do anything
* except see if the file exists. If the file does exist, return 0,
* otherwise return -ENOENT
*/
int fs_open(const char *path, struct fuse_file_info *fi){
	debugf("fs_open: %s\n", path);

	if((node_it = inodes_map.find((char *) path)) == inodes_map.end()) return -ENOENT;

    if((node_it->second->mode & ~(0xfff)) != S_IFREG) return -EINVAL;
    debugf("    %s exists and is a regular file\n", path);

	return 0;
}

/*
* Read a file <path>. You will be reading from the block and
* writing into <buf>, this buffer has a size of <size>. You will
* need to start the reading at the offset given by <offset> and
* write the data into *buf up to size bytes.
*/
int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    debugf("fs_read: %s\n", path);
    debugf("    Size = %d, offset = %d\n", size);

    if((node_it = inodes_map.find((char *) path)) == inodes_map.end()) return -ENOENT;

    NODE *inode = node_it->second;
    uint64_t *block;
    uint64_t start_block = 0, start_byte = 0, total_bytes = 0, current_bytes = 0;

    if(size > inode->size) size = inode->size;

    if(offset != 0){
        start_block = offset / block_header->block_size;
		start_byte = offset % block_header->block_size;
    }

    block = inode->blocks + start_block;

    for(uint64_t i = 0; total_bytes < size; i++){
        if(start_byte != 0 && i == 0) {
            if((size - total_bytes) > (block_header->block_size - start_byte)){
                current_bytes = (block_header->block_size - start_byte);
            } else {
                current_bytes = (size - total_bytes);
            }

            memcpy(buf+total_bytes, block_map[*block]->data+start_byte, current_bytes);
            total_bytes += current_bytes;
	    } else {
            if(size-total_bytes > block_header->block_size) {
			    current_bytes = block_header->block_size;
            } else {
                current_bytes = (size - total_bytes);
            }
            memcpy(buf+total_bytes, block_map[*(block + i)]->data, current_bytes);
            total_bytes += current_bytes;
        }
    }

	return total_bytes;
}

/*
* Write a file <path>. If the file doesn't exist, it is first
* created with fs_create. You need to write the data given by
* <data> and size <size> into this file block. You will also need
* to write data starting at <offset> in your file. If there is not
* enough space, return -ENOSPC. Finally, if we're a read only file
* system (fi->flags & O_RDONLY), then return -EROFS
* If all works, return the number of bytes written.
*/
int fs_write(const char *path, const char *data, size_t size, off_t offset, struct fuse_file_info *fi){
	debugf("fs_write: %s\n", path);
	return -EIO;
}

/*
* Create a file <path>. Create a new file and give it the mode
* given by <mode> OR'd with S_IFREG (regular file). If the name
* given by <path> is too long, return -ENAMETOOLONG. As with
* fs_write, if we're a read only file system
* (fi->flags & O_RDONLY), then return -EROFS.
* Otherwise, return 0 if all succeeds.
*/
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
	debugf("fs_create: %s\n", path);

    /* Check basic conditions before writing to block */
    if(fi->flags & O_RDONLY) return -EROFS;
    if(strlen(path) > NAME_SIZE) return -ENAMETOOLONG;

    /* Check if node with path already exists, error if it does */
    if((node_it = inodes_map.find(path)) != inodes_map.end()) return -EACCES;

    NODE *new_node = (NODE *) malloc(sizeof(NODE));
    strcpy(new_node->name, path);
    new_node->size = 0;
    new_node->uid = getuid();
    new_node->gid = getgid();
    new_node->mode = mode | S_IFREG;
    new_node->ctime = time(NULL);
    new_node->mtime = time(NULL);
    new_node->atime = time(NULL);
    new_node->blocks = NULL;

    inodes_map.insert(std::pair<string, NODE *>(path, new_node));
    block_header->nodes++;
    debugf("    Created new file node for %s\n", path);
    debugf("    New NODE: %s [%u]\n", new_node->name, new_node->id);

	return 0;
}

/*
* Get the attributes of file <path>. A static structure is passed
* to <s>, so you just have to fill the individual elements of s:
* s->st_mode = node->mode
* s->st_atime = node->atime
* s->st_uid = node->uid
* s->st_gid = node->gid
*  ...
* Most of the names match 1-to-1, except the stat structure
* prefixes all fields with an st_*
* Please see stat for more information on the structure. Not
* all fields will be filled by your filesystem.
*/
int fs_getattr(const char *path, struct stat *s)
{
	debugf("fs_getattr: %s\n", path);

    NODE *inode;
    if((node_it = inodes_map.find(path)) == inodes_map.end()) return -ENOENT;

    inode = node_it->second;

    /* Set stat to inode's properties */
	s->st_mode = inode->mode;
    s->st_nlink = 1;
	s->st_uid = inode->uid;
	s->st_gid = inode->gid;
	s->st_size = inode->size;
	s->st_ctime = inode->ctime;
	s->st_atime = inode->atime;
	s->st_mtime = inode->mtime;
    debugf("    Returning attributes for \"%s\"\n", node_it->first.c_str());

	return 0;
}

/*
* Read a directory <path>. This uses the function <filler> to
* write what directories and/or files are presented during an ls
* (list files).
* 
* filler(buf, "somefile", 0, 0);
* 
* You will see somefile when you do an ls
* (assuming it passes fs_getattr)
*/
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	debugf("fs_readdir: %s\n", path);

    if((node_it = inodes_map.find(path)) == inodes_map.end()) return -ENOENT;

    NODE *inode = node_it->second;
    string node_pathname(inode->name), current_path, path_substr;
    uint64_t str_index;

    if((inode->mode & ~(0xfff)) != S_IFDIR) return -ENOTDIR;
    
    for(node_it = inodes_map.begin(); node_it != inodes_map.end(); node_it++) {
		current_path = node_it->first;

		if(current_path.size() < node_pathname.size() || current_path == path) continue;

		str_index = current_path.rfind("/");

		if(node_pathname == "/") path_substr = current_path.substr(0, str_index+1);
		else path_substr = current_path.substr(0, str_index);


		if(node_pathname == path_substr) {
		    debugf("    %s %s\n", node_pathname.c_str(), current_path.c_str());
			filler(buf, current_path.substr(str_index+1).c_str(), 0, 0);
		}
	}

	filler(buf, ".", 0, 0);
	filler(buf, "..", 0, 0);

	/*You MUST make sure that there is no front slashes in the name (second parameter to filler)
	* Otherwise, this will FAIL.
    */

	return 0;
}

/*
* Open a directory <path>. This is analagous to fs_open in that
* it just checks to see if the directory exists. If it does,
* return 0, otherwise return -ENOENT
*/
int fs_opendir(const char *path, struct fuse_file_info *fi)
{
	debugf("fs_opendir: %s\n", path);
		
	if((node_it = inodes_map.find((char *) path)) != inodes_map.end()){
        debugf("    %s exists\n", path);
		if((node_it->second->mode ^ (S_IFDIR | node_it->second->mode)) == 0) return 0;
	}

	return -ENOENT;
}

/*
* Change the mode (permissions) of <path> to <mode>
*/
int fs_chmod(const char *path, mode_t mode)
{
    debugf("fs_chmod: %s\n", path);

    if((node_it = inodes_map.find((char *) path)) == inodes_map.end()) return -ENOENT;

    node_it->second->mode = mode;
    debugf("    Changed mode for [%u] \"%s\"\n", node_it->second->id, node_it->second->name);
	return 0;
}

/*
* Change the ownership of <path> to user id <uid> and group id <gid>
*/
int fs_chown(const char *path, uid_t uid, gid_t gid)
{
	debugf("fs_chown: %s\n", path);

    if((node_it = inodes_map.find((char *) path)) == inodes_map.end()) return -ENOENT;

    node_it->second->uid = uid;
    node_it->second->gid = gid;
    debugf("    Changed ownership for [%u] \"%s\"\n", node_it->second->id, node_it->second->name);
	return -0;
}

/*
* Unlink a file <path>. This function should return -EISDIR if a
* directory is given to <path> (do not unlink directories).
* Furthermore, you will not need to check O_RDONLY as this will
* be handled by the operating system.
* Otherwise, delete the file <path> and return 0.
*/
int fs_unlink(const char *path)
{
	debugf("fs_unlink: %s\n", path);

    /* Check if path exists */
    if((node_it = inodes_map.find((char*) path)) == inodes_map.end()) return -EACCES;

    /* Check if inode is a directory, if it is return error */
    NODE *inode = node_it->second;
    if((inode->mode & ~(0xfff)) == S_IFDIR) return -EISDIR;

    /* Delete inode and remove from maps/update block header */
    delete inode;
    inodes_map.erase(node_it);
    block_header->nodes--;
    debugf("    Removed %s\n", path);

	return 0;
}

/*
* Make a directory <path> with the given permissions <mode>. If
* the directory already exists, return -EEXIST. If this function
* succeeds, return 0.
*/
int fs_mkdir(const char *path, mode_t mode)
{
	debugf("fs_mkdir: %s\n", path);

    if(strlen(path) > NAME_SIZE) return -ENAMETOOLONG;
    if((node_it = inodes_map.find((char* ) path)) != inodes_map.end()) return -EEXIST;

    NODE *new_node = (NODE *) malloc(sizeof(NODE));
    strcpy(new_node->name, path);
    new_node->id = block_header->nodes + 1;
    new_node->size = 0;
    new_node->mode = S_IFDIR | mode;
    new_node->uid = getuid();
    new_node->gid = getgid();
    new_node->ctime = time(NULL);
    new_node->atime = time(NULL);
    new_node->mtime = time(NULL);

    inodes_map.insert(std::pair<string, NODE *>(path, new_node));
    block_header->nodes++;
    debugf("    Created new directory node for %s\n", path);
    debugf("    New NODE: %s [%u]\n", new_node->name, new_node->id);

	return 0;
}

/*
* Remove a directory. You have to check to see if it is
* empty first. If it isn't, return -ENOTEMPTY, otherwise
* remove the directory and return 0.
*/
int fs_rmdir(const char *path)
{
	debugf("fs_rmdir: %s\n", path);
	return -EIO;
}

/*
* Rename the file given by <path> to <new_name>
* Both <path> and <new_name> contain the full path. If
* the new_name's path doesn't exist return -ENOENT. If
* you were able to rename the node, then return 0.
*/
int fs_rename(const char *path, const char *new_name)
{
	debugf("fs_rename: %s -> %s\n", path, new_name);
	return -EIO;
}

/*
* Reduce the size of the file <path> down to size. This will
* potentially remove blocks from the file. Make sure you properly
* handle reducing the size of a file and properly updating
* the node with the number of blocks it will take. Return 0 
* if successful
*/
int fs_truncate(const char *path, off_t size)
{
	debugf("fs_truncate: %s to size %d\n", path, size);
	return -EIO;
}

/*
* fs_destroy is called when the mountpoint is unmounted
* this should save the hard drive back into <filename>
*/
void fs_destroy(void *ptr)
{
	const char *filename = (const char *)ptr;
	debugf("fs_destroy: %s\n", filename);

	/* Save the internal data to the hard drive specified by <filename> */
    FILE *harddrive = fopen(filename, "w");
    if(harddrive == NULL) return;

    /* Write the block_header to the block file */
    if(fwrite(block_header, sizeof(BLOCK_HEADER), 1, harddrive) != 1) return;
    debugf("    Wrote block_header to \"%s\"\n", filename);

    /* Write nodes to the block file */
    NODE *cur_node;
    uint64_t block_num;
    for(node_it = inodes_map.begin(); node_it != inodes_map.end(); node_it++){
        cur_node = node_it->second;
        /* Write node */
        if(fwrite(cur_node, ONDISK_NODE_SIZE, 1, harddrive) != 1) return;

        /* Write block number for regular files */
        if((cur_node->mode & ~(0xfff)) == S_IFREG){
            block_num = (cur_node->size / block_header->block_size) + 1;
            if(fwrite(cur_node->blocks, sizeof(uint64_t), block_num, harddrive) != block_num) return;
        }

        debugf("    Wrote node [%u] \"%s\" to \"%s\"\n", cur_node->id, cur_node->name, filename);
    }

    /* Write the blocks to the block file */
    BLOCK *cur_block;
    uint64_t i = 0;
    for(block_it = block_map.begin(); block_it != block_map.end(); block_it++){
        cur_block = block_it->second;
        fwrite(cur_block->data, sizeof(char), block_header->block_size, harddrive);
        debugf("    Wrote block %u to \"%s\"\n", i, filename);
        i++;
    }
}

/*
* int main()
* DO NOT MODIFY THIS FUNCTION
*/
int main(int argc, char *argv[])
{
	fuse_operations *fops;
	char *evars[] = { "./fs", "-f", "mnt", NULL };
	int ret;

	if ((ret = fs_drive(HARD_DRIVE)) != 0) {
		debugf("Error reading hard drive: %s\n", strerror(-ret));
		return ret;
	}
	//FUSE operations
	fops = (struct fuse_operations *) calloc(1, sizeof(struct fuse_operations));
	fops->getattr = fs_getattr;
	fops->readdir = fs_readdir;
	fops->opendir = fs_opendir;
	fops->open = fs_open;
	fops->read = fs_read;
	fops->write = fs_write;
	fops->create = fs_create;
	fops->chmod = fs_chmod;
	fops->chown = fs_chown;
	fops->unlink = fs_unlink;
	fops->mkdir = fs_mkdir;
	fops->rmdir = fs_rmdir;
	fops->rename = fs_rename;
	fops->truncate = fs_truncate;
	fops->destroy = fs_destroy;

	debugf("Press CONTROL-C to quit\n\n");

	return fuse_main(sizeof(evars) / sizeof(evars[0]) - 1, evars, fops,
			 (void *)HARD_DRIVE);
}

static void print_node(NODE *inode){
    //debugf("NODE: %s [%u]\n", inode->name, inode->id);
    debugf("%s, %u, %u, %u, %u, %u, %u, %u, %u, 0x%x\n", inode->name, inode->id, inode->mode,
        inode->ctime, inode->atime, inode->mtime, inode->uid, inode->gid, inode->size, inode->blocks);
}