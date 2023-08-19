#include "head.h"

#define MAGIC_V1 0x1023
#define SUPER_BLOCK_MAGIC_V1 0x1023
#define SUPER_BLOCK_SIZE 56
#define NR_INODES 4096

extern struct hdisk_info hd_info[MAX_DRIVES];

struct SUPER_BLOCK {
	unsigned int magic;			/* magic number*/
	unsigned int nr_inodes;		/* inode number*/
	unsigned int nr_sectors;		/* sector number*/
	unsigned int first_imap_sector;
	unsigned int nr_imap_sectors;		/* inode-map number*/
	unsigned int first_bitmap_sector;
	unsigned int nr_bitmap_sectors;		/* sector-map number*/
	/*unsigned int n_lst_sector;		 lst data sector number*/
	unsigned int nr_inode_sectors;		/* inode sector number*/
	unsigned int root_inode;		/* inode nr of root directory*/
	unsigned int inode_size;		/* size of inode*/
	/*unsigned int inode_isize_offset;	 offset of `struct inode::i_size`
	unsigned int inode_start_offset;	 offset of `struct inode::i_start_sect`
	unsigned int dir_ent_size;		 size of dir_entry
	unsigned int dir_ent_inode_offset;	 offset of `struct dir_entry::inode_nr`
	unsigned int dir_ent_fname_offset;	 offset of `struct dir_entry::name`*/
	
	int sb_dev;			/* the super block's home device*/
	int sb_dirty;
};

struct BUFFER_HEAD {
	char *data;
	unsigned long nr_block;
	unsigned short dev;
	unsigned char uptodate;
	unsigned char dirt;
	unsigned char count;
	unsigned char lock;
	
	struct BUFFER_HEAD *prev;
	struct BUFFER_HEAD *next;
	struct BUFFER_HEAD *prev_free;
	struct BUFFER_HEAD *next_free;
};

#define I_NODE_SIZE 92
#define FS_NAME_LEN 18
#define FS_SUFFIX_LEN 6

struct I_NODE {
	unsigned int i_num;
	unsigned int i_mode;		/* access mode*/
	unsigned char name[FS_NAME_LEN];
	unsigned char suffix[FS_SUFFIX_LEN];
	unsigned int i_uid;
	unsigned int i_size;			/* size of file / */
	unsigned int i_time;
	unsigned int i_parent_inode;		/**/
	unsigned int i_start_sector;		/* the first sector of data*/
	unsigned int i_nr_sectors;		/* how many sectors the file occupies*/
	unsigned int i_zone[9];		/* */
	
	unsigned char i_dev;
	unsigned char i_dirty;
	unsigned int i_count;
};

struct FILE_DESC {
	int fd_mode;			/* R or W*/
	int fd_pos;			/* current position for R/W*/
	struct I_NODE *fd_inode;		/* ptr to the i-node*/
};

#define MAX_FILENAME_LEN 12

struct DIR_ENTRY {
	int inode_nr;
	char name[MAX_FILENAME_LEN];	/* filename*/
	struct I_NODE *dir_inode;
};

#define CHS2no(cyl, head, sector) ((cyl + 1) * (head + 1) * sector)
#define CHS2LBA(cyl, head, sector) ((head * (hd_info[0].head - 1) + head) * hd_info[0].sector + sector - 1)

void init_dev0();
unsigned int chs2lba(unsigned int cyl, unsigned int head, unsigned int sector);
void init_main_boot_sector();
struct HDISK_REQUEST* read_main_boot_sector(int dev);
void init_boot_sector(unsigned int sector);
void init_super_block(struct SUPER_BLOCK *super_block, int dev);
void write_super_block(struct SUPER_BLOCK *super_block);
void init_imap(struct SUPER_BLOCK *super_block);
void init_bitmap(struct SUPER_BLOCK *super_block);
void init_dir_dev0(struct SUPER_BLOCK *super_block);
unsigned int get_unused_inode(struct SUPER_BLOCK *super_block);
unsigned int get_unused_sector(struct SUPER_BLOCK *super_block);
void imap2inode(int dev, unsigned int x);
void bitmap2sector(int dev, unsigned int offset);
struct HDISK_REQUEST* read_sector(struct SUPER_BLOCK *super_block, int sector, int nr_sector);
void update_imap(struct SUPER_BLOCK *super_block, int offset, unsigned char temp);
void store_int(char *buffer, unsigned int content, unsigned int *offset);
void read_int(char *buffer, unsigned int *content, unsigned int *offset);
void write_inode(struct SUPER_BLOCK *super_block, struct I_NODE *inode, unsigned int offset);
struct I_NODE *create_dir(struct SUPER_BLOCK *super_block, unsigned char name[18], struct I_NODE *p_inode);
struct I_NODE *open_dir(struct SUPER_BLOCK *super_block, unsigned int i_num);
void close_dir(struct I_NODE *inode);
struct FILE_DESC *create_file(struct SUPER_BLOCK *super_block, unsigned char name[FS_NAME_LEN], unsigned char suffix[FS_SUFFIX_LEN], struct I_NODE *p_inode);
struct FILE_DESC *open_file(struct SUPER_BLOCK *super_block, unsigned int i_num);
void close_file(struct FILE_DESC *file);
