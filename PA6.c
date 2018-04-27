#include "PA6.h"

#define DEBUG_SEARCH

extern int _fd;
extern char _buf[BLKSIZE];
extern char * _diskname, * _pathname;

extern GD    * _gp;
extern SUPER * _sp;
extern INODE * _ip;

int main(int argc, char * argv[], char * env[])
{
	INODE iroot;

	if(argc == 3)
	{
		_diskname = argv[1];
		_pathname = argv[2];
	}
	else
	{
		printf("Please specify both a disk name and a pathname to search for within the disk\n");
		printf("Using default values \"mydisk\" and \"/\" for the disk name and pathname\n");
	}
	
	_fd = open(_diskname, O_RDONLY);

	//Print out SUPER Block info
	init_Super();
	
	//Print out GD0 Block info
	u32 inode_start = init_GD0();
	
	//Print out Root Block info
	init_root(inode_start);
	iroot = *_ip;
	
	printf("\n - Press enter to continue - ");
	getchar();
	printf("\n");
	
	print_dir(_ip);
	
	printf("\n - Press enter to continue - ");
	getchar();
	printf("\n");
	
#ifdef DEBUG_SEARCH
	_pathname = "Z/hugefile";
#endif
	
	//Search for the file specified in pathname
	u32 ino = search_fs(_pathname, &iroot);
	printf("%d\n", ino);
	
	/*if(ino)
		print_file(inode);
	else
		printf("The file specified could not be found\n");*/
	
	exit(0);
}
