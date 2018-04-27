/*
struct ext2_group_desc
{
  u32  bg_block_bitmap;          // Bmap block number
  u32  bg_inode_bitmap;          // Imap block number
  u32  bg_inode_table;           // Inodes begin block number
  u16  bg_free_blocks_count;     // THESE are OBVIOUS
  u16  bg_free_inodes_count;
  u16  bg_used_dirs_count;        

  u16  bg_pad;                   // ignore these 
  u32  bg_reserved[3];
};
*/

/********* gd.c code ***************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

#define BLKSIZE 1024

// define shorter TYPES, save typing efforts
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs 

int _fd;
char _buf[BLKSIZE];
char * _disk = "mydisk";

GD    * _gp;
SUPER * _sp;
INODE * _ip;
DIR   * _dp;

// Functions **************************************************************************************

int get_block(int fd, int blk, char buf[])
{
  lseek(_fd, (long)blk*BLKSIZE, 0);
  read(_fd, buf, BLKSIZE);
}

void gd()
{
  // read GD0 block
  get_block(_fd, 2, _buf);  
  _gp = (GD *)_buf;
  
  if(!_gp)
  {
  	printf("Group Descirptor Block could not be read.\n");
  	exit(1);
  }
	
	printf("Group Descirptor Block read successfully\n\n");
  printf("Printing GD0 Information:\n");
  printf("bg_block_bitmap = %d\n", _gp->bg_block_bitmap);
  printf("bg_inode_bitmap = %d\n", _gp->bg_inode_bitmap);
  printf("bg_inode_table = %d\n", _gp->bg_inode_table);
  printf("bg_free_blocks_count = %d\n", _gp->bg_free_blocks_count);
  printf("bg_free_inodes_count= %d\n", _gp->bg_free_inodes_count);
  printf("bg_used_dirs_count = %d\n", _gp->bg_used_dirs_count);
}

// Main *******************************************************************************************

main(int argc, char *argv[ ])

{ 
  if (argc > 1)
    _disk = argv[1];
    
  _fd = open(_disk, O_RDONLY);
  
  if (_fd < 0)
  {
    printf("open failed\n");
    exit(1);
  }

  gd();
}
