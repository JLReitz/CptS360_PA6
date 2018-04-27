#ifndef PA6_H
#define PA6_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/stat.h>

#define BLKSIZE 1024

// Globals ****************************************************************************************

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs 

int _fd, _ninodes;
char _buf[BLKSIZE];
char * _diskname = "diskimage", * _pathname = "/";

u32 _iblk;

GD    * _gp;
SUPER * _sp;
INODE * _ip;

// Prototypes *************************************************************************************
int get_block(int fd, int blk, char buf[]);

u32 init_GD0();
u32 search_dir(char * pathname, INODE * inode);
u32 search_fs(char * pathname, INODE * inode);

INODE * get_inode(int dev, u32 ino);

void init_super();
void init_root(u32 inode_start_block);
void print_dir(INODE * inode);
void print_file(INODE * inode);

// Functions **************************************************************************************

int get_block(int fd, int blk, char buf[])
{
  lseek(_fd, (long)blk*BLKSIZE, 0);
  read(_fd, buf, BLKSIZE);
}

u32 init_GD0()
{
	//Read GD0 block
  get_block(_fd, 2, _buf);  
  _gp = (GD *)_buf;
  
  if(!_gp)
  {
  	printf("Group Descirptor Block could not be read.\n");
  	exit(1);
  }
	
	//Print/Store pertinent information
  printf("************************* GD0 Block *************************\n");
  printf("bg_block_bitmap = %d\n", _gp->bg_block_bitmap);
  printf("bg_inode_bitmap = %d\n", _gp->bg_inode_bitmap);
  printf("bg_inode_table = %d\n", _gp->bg_inode_table);
  printf("bg_free_blocks_count = %d\n", _gp->bg_free_blocks_count);
  printf("bg_free_inodes_count= %d\n", _gp->bg_free_inodes_count);
  printf("bg_used_dirs_count = %d\n", _gp->bg_used_dirs_count);
  
  //return first INODE number
  _iblk = _gp->bg_inode_table;
  return _gp->bg_inode_table;
}

u32 search_dir(char * pathname, INODE * inode)
{
	int i = 0;

	//Check for if the supplied inode is a directory
	if(inode->i_mode == 16877)
	{
		printf("Looking for \"%s\":\n", pathname);		
		print_dir(inode);
		
		//Access every data block that this directory contains
		for(i; i<12; i++)
		{
			//Use this INODE to find the data block number
			u32 iblock = inode->i_block[i];
			
			if(iblock) //If the block number is 0, that block does not exist
			{
				//Read in the block and print the directory listing
				get_block(_fd, iblock, _buf);
				
				char * cp = _buf; //Character pointer to hold the current position within the block
				DIR * dp = (DIR *)_buf;
				
				while(cp < &_buf[BLKSIZE])
				{
					//If the pathname has been found
					if(!strcmp(dp->name, pathname))
					{
						//Print INODE information
						printf("Found %s: inode %d\n\n", dp->name, dp->inode);
				
						return dp->inode; //Return the INODE of the corresponding file or subdirectory
					}
					
					//Increment cp and set dp to the next file in the directory
					cp += dp->rec_len;
					dp = (DIR *)cp;
				}
			}
		}
	}
	
	return 0; //Return 0 if nothing is found
}

u32 search_fs(char * pathname, INODE * inode)
{
	int i = 0, pathname_location = 0, pathname_length = 0, success = 0;
	char * pathname_tokens[128] = {0};	
	u32 ino;
	INODE * ip = inode;
	
	printf("************************* Searching *************************\n");
	
	//Tokenize the pathname
	pathname_tokens[i++] = (char *)strtok(pathname, "/");
	pathname_length++;
	
	while(pathname_tokens[i++] = (char *)strtok(0, "/"))
	{
		pathname_length++;
	}
	
	//Begin the search
	while(pathname_location <= pathname_length) //Go until we've run out of a path to follow
	{		
		if(ino = search_dir(pathname_tokens[pathname_location++], ip))
		{
			//Check to see if this is the file we are ultimately looking for
			if(pathname_location == pathname_length)
				return ino; //Return the inode number of the file
			else
				ip = get_inode(_fd, ino); //Load in the next INODE and continue to search
		}
		else
			break;
	}
	
	return 0;
}

INODE * get_inode(int dev, u32 ino)
{
	char ibuf[BLKSIZE];
	INODE *ip;
	
	//Mailman's algorithm
	int blk = _iblk + (ino-1)/_ninodes;
	get_block(dev, blk, ibuf);
	ip = (INODE *)ibuf + (ino-1)%_ninodes;
	
	return ip;
}

void init_Super()
{
	//Read SUPER Block in to buffer
  get_block(_fd, 1, _buf);  
  _sp = (SUPER *)_buf;

  //Check for EXT2 magic number:
  printf("s_magic = %x\n", _sp->s_magic);
  if (_sp->s_magic != 0xEF53)
  {
    printf("NOT an EXT2 FS\n");
    exit(1);
  }
  
  //Print/Store pertinent information
  printf("************************ SUPER Block ************************\n");
  printf("s_inodes_count = %d\n", _sp->s_inodes_count);
  printf("s_blocks_count = %d\n", _sp->s_blocks_count);
  printf("s_free_inodes_count = %d\n", _sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", _sp->s_free_blocks_count);
  printf("s_first_data_block = %d\n", _sp->s_first_data_block);
  printf("s_log_block_size = %d\n", _sp->s_log_block_size);
  printf("s_blocks_per_group = %d\n", _sp->s_blocks_per_group);
  printf("s_inodes_per_group = %d\n", _sp->s_inodes_per_group);
  printf("s_mnt_count = %d\n", _sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", _sp->s_max_mnt_count);
  printf("s_magic = %x\n", _sp->s_magic);
  
  _ninodes = _sp->s_inodes_count;
}

void init_root(u32 inode_start_block)
{
	//Read in the INODE start block
	get_block(_fd, inode_start_block, _buf);
	
	//Load in the second INODE (the first is not used by the filesystem)
	_ip = (INODE *)_buf + 1;
	
	//Use this INODE to print any pertinent INODE information
	
	printf("************************ Root Block *************************\n");
	printf("Block = %d\n", inode_start_block+1);
	printf("i_mode = 0x%x\n", _ip->i_mode);
	printf("i_uid = %d\n", _ip->i_uid);
	printf("i_size = %d\n", _ip->i_size);
	printf("i_links_count = %d\n", _ip->i_links_count);
}

void print_dir(INODE * inode)
{
	int i = 0;
	
	//Check for if the supplied inode is a directory
	if(inode->i_mode == 16877)
	{
		printf("-------------------------------------------------------------\n");
		printf("\t\tINODE\tRec_len\tName_len\tName\n");
		
		//Access every data block that this directory contains
		for(i; i<12; i++)
		{
			//Use this INODE to find the data block number
			u32 iblock = inode->i_block[i];
			
			if(iblock) //If the block number is 0, that block does not exist
			{
				//Read in the block and print the directory listing
				get_block(_fd, iblock, _buf);
				
				char * cp = _buf; //Character pointer to hold the current position within the block
				DIR * dp = (DIR *)_buf;
				
				printf("Block #: %d\n", iblock);
				
				while(cp < &_buf[BLKSIZE])
				{
					printf("\t\t%d\t%d\t%d\t\t%s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
					
					//Increment cp and set dp to the next file in the directory
					cp += dp->rec_len;
					dp = (DIR *)cp;
				}
			}
		}
	}
	else
	{
		printf("The file supplied was not a directory\n");
		exit(1);
	}
}

void print_file(INODE * inode)
{
	int i = 0;
	int * ip1, * ip2, *ip3;
	int ind_buf1[BLKSIZE/4], ind_buf2[BLKSIZE/4], ind_buf3[BLKSIZE/4];

	//Access every direct data block that this directory contains
	for(i; i<12; i++)
	{
		//Use this INODE to find the data block number
		u32 iblock = inode->i_block[i];

		//Read in the block and print the directory listing
		get_block(_fd, iblock, _buf);
		
		char * cp = _buf; //Character pointer to hold the current position within the block
		DIR * dp = (DIR *)_buf;
		
		printf("Direct Blocks:\n");
		
		while(cp < &_buf[BLKSIZE])
		{
			printf("%d ", dp->inode);
			
			//Increment cp and set dp to the next file in the directory
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
		
		printf("\n");
	}
	
	//Indirect data blocks
	get_block(_fd, inode->i_block[12], (char *)ind_buf1);
	
	ip1 = ind_buf1;
	
	printf("Indirect Blocks:\n");
	
	while(ip1 < &ind_buf1[BLKSIZE/4])
	{
		printf("%d:\n", *ip1);
		get_block(_fd, *ip1, (char *)ind_buf2);
		
		ip2 = ind_buf2;
		
		while(ip2 < &ind_buf2[BLKSIZE/4])
		{
			printf("%d ", *ip2);
			
			ip2++;
		}
		
		ip1++;
	}
	
	//Double indirect data blocks
	get_block(_fd, inode->i_block[13], (char *)ind_buf1);
	
	ip1 = ind_buf1;
	
	printf("Double Indirect Blocks:\n");
	
	while(ip1 < &ind_buf1[BLKSIZE/4])
	{
		printf("%d:\n", *ip1);
		get_block(_fd, *ip1, (char *)ind_buf2);
		
		ip2 = ind_buf2;
		
		while(ip2 < &ind_buf2[BLKSIZE/4])
		{
			printf("%d:\n", *ip2);
			
			get_block(_fd, *ip2, (char *)ind_buf3);
		
			ip3 = ind_buf3;
			
			while(ip3 < &ind_buf3[BLKSIZE/4])
			{
				printf("%d ", *ip3);
				
				ip3++;
			}
			
			ip2++;
		}
		
		ip1++;
	}
}


#endif
