#include "file_system.h"

extern struct HDISK_STRUCT hdisk_struct[5 * MAX_DRIVES];

void init_dev0() {
	init_main_boot_sector();
	init_boot_sector(1);
	struct SUPER_BLOCK *super_block;
	init_super_block(super_block, 1);
	init_imap(super_block);
	init_bitmap(super_block);
	init_dir_dev0(super_block);
	super_block->root_inode = 0;
	write_super_block(super_block);
	return;
}

unsigned int chs2lba(unsigned int cyl, unsigned int head, unsigned int sector) {
	return (head * (hd_info[0].head - 1) + head) * hd_info[0].sector + sector - 1;
}

void init_main_boot_sector(int drive) {
	struct HDISK_REQUEST main_boot_sector_request;
	main_boot_sector_request.dev = drive;
	main_boot_sector_request.cmd = WRITE;
	main_boot_sector_request.error = 0;
	main_boot_sector_request.sector = 0;
	main_boot_sector_request.nr_sector = 1;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	main_boot_sector_request.buffer = (char *) memman_alloc_4k(memman, SECTOR_SIZE);
	main_boot_sector_request.buffer[0x1BE] = 0;
	main_boot_sector_request.buffer[0x1BF] = 0;
	main_boot_sector_request.buffer[0x1A0] = 1;
	main_boot_sector_request.buffer[0x1A1] = 0;
	main_boot_sector_request.buffer[0x1A2] = 1;
	main_boot_sector_request.buffer[0x1A3] = hd_info[0].sector | (hd_info[0].cyl & 0x0300);
	main_boot_sector_request.buffer[0x1A4] = hd_info[0].cyl & 0x00FF;
	unsigned int lba = chs2lba(0, 0, 1);
	main_boot_sector_request.buffer[0x1A5] = lba & 0x000000FF;
	main_boot_sector_request.buffer[0x1A6] = lba & 0x0000FF00;
	main_boot_sector_request.buffer[0x1A7] = lba & 0x00FF0000;
	main_boot_sector_request.buffer[0x1A8] = lba & 0xFF000000;
	int nr_sectors = hd_info[0].cyl * hd_info[0].head * hd_info[0].sector - 1;
	main_boot_sector_request.buffer[0x1A9] = nr_sectors & 0x000000FF;
	main_boot_sector_request.buffer[0x1AA] = nr_sectors & 0x0000FF00;
	main_boot_sector_request.buffer[0x1AB] = nr_sectors & 0x00FF0000;
	main_boot_sector_request.buffer[0x1AC] = nr_sectors & 0xFF000000;
	
	add_hdisk_request(&main_boot_sector_request);
	do_hdisk_request();
	memman_free_4k(memman, (int) main_boot_sector_request.buffer, SECTOR_SIZE);
}

struct HDISK_REQUEST *read_main_boot_sector(int drive) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct HDISK_REQUEST *main_boot_sector_r_request = (struct HDISK_REQUEST *) memman_alloc_4k(memman, sizeof(struct HDISK_REQUEST));
	main_boot_sector_r_request->dev = drive;
	main_boot_sector_r_request->cmd = READ;
	main_boot_sector_r_request->error = 0;
	main_boot_sector_r_request->sector = 1;
	main_boot_sector_r_request->nr_sector = 1;
	
	
	return main_boot_sector_r_request;
}

void init_boot_sector(unsigned int sector) {
	struct HDISK_REQUEST init_boot_sector_request;
	
}

void init_super_block(struct SUPER_BLOCK *super_block, int dev) {
	super_block->sb_dev = dev;
	
	super_block->magic = MAGIC_V1;
	super_block->nr_inodes = NR_INODES;
	super_block->nr_sectors = hdisk_struct[super_block->sb_dev].nr_sectors;
	super_block->first_imap_sector = 2;
	super_block->inode_size = SECTOR_SIZE;
	return;
}

void write_super_block(struct SUPER_BLOCK *super_block) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct HDISK_REQUEST request;
	request.dev = super_block->sb_dev;
	request.cmd = WRITE;
	request.error = 0;
	request.sector = hdisk_struct[super_block->sb_dev].start_sector + 1;
	request.nr_sector = 1;
	request.buffer = (char *) memman_alloc_4k(memman, SECTOR_SIZE);
	
	unsigned int x = 0;
	store_int(request.buffer, super_block->magic, &x);
	store_int(request.buffer, super_block->nr_inodes, &x);
	store_int(request.buffer, super_block->nr_sectors, &x);
	store_int(request.buffer, super_block->first_imap_sector, &x);
	store_int(request.buffer, super_block->nr_imap_sectors, &x);
	store_int(request.buffer, super_block->first_bitmap_sector, &x);
	store_int(request.buffer, super_block->nr_bitmap_sectors, &x);
	store_int(request.buffer, super_block->nr_inode_sectors, &x);
	store_int(request.buffer, super_block->root_inode, &x);
	store_int(request.buffer, super_block->inode_size, &x);
	add_hdisk_request(&request);
	do_hdisk_request();
	return;
}

void init_imap(struct SUPER_BLOCK *super_block) {
	unsigned int inodes_per_sector = SECTOR_SIZE / I_NODE_SIZE;
	super_block->nr_inode_sectors = super_block->nr_inodes / inodes_per_sector;	/*inodes所占扇区数*/
	if (super_block->nr_inodes % inodes_per_sector != 0) {
		++super_block->nr_inode_sectors;
	}
	super_block->nr_imap_sectors = super_block->nr_inodes / (SECTOR_SIZE << 3);	/*imap扇区数*/
	unsigned int left_bit = super_block->nr_inodes % (SECTOR_SIZE << 3);
	unsigned char buf[SECTOR_SIZE];
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i = 2;
	int j;
	for (j = 0; j < SECTOR_SIZE; ++j) {
		buf[j] = 0xFF;
	}
	if (super_block->nr_imap_sectors != 0) {
		for ( ; i < 2 + super_block->nr_imap_sectors; ++i) {
			struct HDISK_REQUEST request;
			request.dev = super_block->sb_dev;
			request.cmd = WRITE;
			request.error = 0;
			request.sector = i + hdisk_struct[super_block->sb_dev].start_sector;
			request.nr_sector = 1;
			request.buffer = (char *)memman_alloc_4k(memman, SECTOR_SIZE);
			for (j = 0; j < SECTOR_SIZE; ++j) {
				request.buffer[j] = buf[j];
			}
			add_hdisk_request(&request);
			do_hdisk_request();
			memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE); 
		}
	}
	if (left_bit != 0) {
		++super_block->nr_imap_sectors;
		unsigned short num_char = left_bit >> 3;
		unsigned char temp = 0;
		for (j = 0; j < left_bit % 8; ++j) {
			temp |= 1 << (7 - j);
		}
		buf[num_char] = temp;
		struct HDISK_REQUEST request;
		request.dev = super_block->sb_dev;
		request.cmd = WRITE;
		request.error = 0;
		request.sector = i + hdisk_struct[super_block->sb_dev].start_sector;
		request.nr_sector = 1;
		request.buffer = (char *)memman_alloc_4k(memman, SECTOR_SIZE);
		for (j = 0; j < num_char; ++j) {
			request.buffer[j] = 0xFF;
		}
		for (j = num_char; j < SECTOR_SIZE; ++j) {
			request.buffer[j] = buf[j];
		}
		add_hdisk_request(&request);
		do_hdisk_request();
		memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
	}
	return;
}

void init_bitmap(struct SUPER_BLOCK *super_block) {
	super_block->first_bitmap_sector = 2 + super_block->nr_imap_sectors + super_block->nr_inode_sectors;
	super_block->nr_bitmap_sectors = super_block->nr_sectors / (SECTOR_SIZE << 3);
	unsigned int left_bit = super_block->nr_sectors % (SECTOR_SIZE << 3);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned char buf[SECTOR_SIZE];
	unsigned short num_char = super_block->first_bitmap_sector >> 3;
	int j;
	for (j = 0; j < num_char; ++j) {
		buf[j] = 0x00;
	}
	unsigned char temp = 0;
	for (j = num_char; j < super_block->first_bitmap_sector % 8; ++j) {
		temp |= 1 << (7 - j);
	}
	buf[num_char] = temp;
	for (j = num_char + 1; j < SECTOR_SIZE; ++j) {
		buf[j] = 0xFF;
	}
	int i = super_block->first_bitmap_sector;
	if (super_block->nr_bitmap_sectors != 0) {
		struct HDISK_REQUEST request;
		request.dev = super_block->sb_dev;
		request.cmd = WRITE;
		request.error = 0;
		request.sector = i + hdisk_struct[super_block->sb_dev].start_sector;
		request.nr_sector = 1;
		request.buffer = (char *) memman_alloc_4k(memman, SECTOR_SIZE);
		for (j = 0; j < SECTOR_SIZE; ++j) {
			request.buffer[j] = buf[j];
		}
		add_hdisk_request(&request);
		do_hdisk_request();
		memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);	
		
		for (j = 0; j < num_char + 1; ++j) {
			buf[j] = 0xFF;
		}
		++i;
	}
	for ( ; i < super_block->first_bitmap_sector + super_block->nr_bitmap_sectors; ++i) {
		struct HDISK_REQUEST request;
		request.dev = super_block->sb_dev;
		request.cmd = WRITE;
		request.error = 0;
		request.sector = i + hdisk_struct[super_block->sb_dev].start_sector;
		request.nr_sector = 1;
		request.buffer = (char *)memman_alloc_4k(memman, SECTOR_SIZE);
		for (i = 0; i < SECTOR_SIZE; ++i) {
			request.buffer[i] = buf[i];
		}
		add_hdisk_request(&request);
		do_hdisk_request();
		memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
	}
	if (left_bit != 0) {
		++super_block->nr_bitmap_sectors;
		num_char = left_bit >> 3;
		temp = 0;
		for (i = 0; i < left_bit % 8; ++i) {
			temp |= 1 << (7 - i);
		}
		buf[num_char] = temp;
		struct HDISK_REQUEST request;
		request.dev = super_block->sb_dev;
		request.cmd = WRITE;
		request.error = 0;
		request.sector = super_block->first_bitmap_sector + super_block->nr_bitmap_sectors + hdisk_struct[super_block->sb_dev].start_sector - 1;
		request.nr_sector = 1;
		request.buffer = (char *)memman_alloc_4k(memman, SECTOR_SIZE);
		for (i = 0; i < num_char; ++i) {
			request.buffer[i] = 1;
		}
		request.buffer[num_char] = buf[num_char];
		for (i = num_char + 1; i < SECTOR_SIZE; ++i) {
			request.buffer[i] = 0;
		}
		add_hdisk_request(&request);
		do_hdisk_request();
		memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
	}
	return;
}

void init_dir_dev0(struct SUPER_BLOCK *super_block) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct I_NODE *inode = create_dir(super_block, "dev0", 0);
	memman_free_4k(memman, (int) inode, sizeof(struct I_NODE));
	return;
}

unsigned int get_unused_inode(struct SUPER_BLOCK *super_block) {
	int first = super_block->first_imap_sector + hdisk_struct[super_block->sb_dev].start_sector;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i;
	unsigned int x = 0;
	for(i = 0; i < super_block->nr_imap_sectors; ++i) {
		struct HDISK_REQUEST request;
		request.dev = super_block->sb_dev;
		request.cmd = READ;
		request.error = 0;
		request.sector = first + i;
		request.nr_sector = 1;
		request.buffer = (char *)memman_alloc_4k(memman, SECTOR_SIZE);
		add_hdisk_request(&request);
		do_hdisk_request();
		
		int j;
		for (j = 0; j < SECTOR_SIZE; ++j) {
			if (request.buffer[j] == (char)0xFF) {
				x += 8;
				continue;
			}
			else {
				if (request.buffer[j] & 0x80) { }
				else if (request.buffer[j] & 0x40)
					++x;
				else if (request.buffer[j] & 0x20)
					x += 2;
				else if (request.buffer[j] & 0x10)
					x += 3;
				else if (request.buffer[j] & 0x08)
					x += 4;
				else if (request.buffer[j] & 0x04)
					x += 5;
				else if (request.buffer[j] & 0x02)
					x += 6;
				else
					x += 7;
				memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
				return x;
			}
		}
		memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
		/*返回第x个inode*/
	}
	return x;
}

unsigned int get_unused_sector(struct SUPER_BLOCK *super_block) {
	int first = super_block->first_bitmap_sector + hdisk_struct[super_block->sb_dev].start_sector;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int x = 0;
	int i;
	for (i = 0; i < super_block->nr_bitmap_sectors; ++i) {
		struct HDISK_REQUEST request;
		request.dev = super_block->sb_dev;
		request.cmd = READ;
		request.error = 0;
		request.sector = first + i;
		request.nr_sector = 1;
		request.buffer = (char *)memman_alloc_4k(memman, SECTOR_SIZE);
		add_hdisk_request(&request);
		do_hdisk_request();
		
		int j;
		for (j = 0; j < SECTOR_SIZE; ++j) {
			if (request.buffer[j] == (char) 0xFF) {
				x += 8;
				continue;
			}
			else {
				if (request.buffer[j] & 0x80) { }
				else if (request.buffer[j] & 0x40)
					++x;
				else if (request.buffer[j] & 0x20)
					x += 2;
				else if (request.buffer[j] & 0x10)
					x += 3;
				else if (request.buffer[j] & 0x08)
					x += 4;
				else if (request.buffer[j] & 0x04)
					x += 5;
				else if (request.buffer[j] & 0x02)
					x += 6;
				else
					x += 7;
				memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
				return x;
			}
		}
		memman_free_4k(memman, (int) request.buffer, SECTOR_SIZE);
		/*返回第x个sector*/
	}
	return x;
}

void imap2inode(int dev, unsigned int x) {
	unsigned int inode_per_sector = SECTOR_SIZE / I_NODE_SIZE;
	unsigned int no_sector = x / inode_per_sector + hdisk_struct[dev].start_sector;
	unsigned int offset = x % inode_per_sector - 1;
	
}

void bitmap2sector(int dev, unsigned int offset) {
	unsigned int block_no = hdisk_struct[dev].start_sector + offset;
	unsigned int cyl = block_no / (hd_info[dev].head * hd_info[dev].sector);
	block_no %= hd_info[dev].head * hd_info[dev].sector;
	unsigned int head = block_no / hd_info[dev].sector;
	unsigned int sector = block_no % hd_info[dev].sector + 1;
	
}

struct HDISK_REQUEST* read_sector(struct SUPER_BLOCK *super_block, int sector, int nr_sector) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct HDISK_REQUEST *request = (struct HDISK_REQUEST *) memman_alloc_4k(memman, sizeof(struct HDISK_REQUEST));
	request->dev = super_block->sb_dev;
	request->cmd = READ;
	request->error = 0;
	request->sector = sector;
	request->nr_sector = nr_sector;
	
	add_hdisk_request(request);
	do_hdisk_request();
	
	return request;
}

void update_imap(struct SUPER_BLOCK *super_block, int offset, unsigned char temp) {
	unsigned int imap_sector_offset = offset / (SECTOR_SIZE << 3);
	offset %= SECTOR_SIZE << 3;
	
	struct HDISK_REQUEST *request = read_sector(super_block, super_block->first_imap_sector + imap_sector_offset, 1);
	request->cmd = WRITE;
	if (temp == 0) {
		request->buffer[offset] &= temp << (offset >> 3);
	}
	else {
		request->buffer[offset] |= temp << (offset >> 3);
	}
	add_hdisk_request(request);
	do_hdisk_request();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	memman_free_4k(memman, (int) request->buffer, SECTOR_SIZE);
	memman_free_4k(memman, (int) request, sizeof(struct HDISK_REQUEST));
	return;
}

void store_int(char *buffer, unsigned int content, unsigned int *offset) {
	buffer[*offset++] = content & 0x000000FF;
	buffer[*offset++] = (content & 0x0000FF00) >> 8;
	buffer[*offset++] = (content & 0x00FF0000) >> 16;
	buffer[*offset++] = (content & 0xFF000000) >> 24;
	return;
}

void read_int(char *buffer, unsigned int *content, unsigned int *offset) {
	*content = buffer[*offset++];
	*content |= buffer[*offset++] << 8;
	*content |= buffer[*offset++] << 16;
	*content |= buffer[*offset++] << 24;
	return;
}

void write_inode(struct SUPER_BLOCK *super_block, struct I_NODE *inode, unsigned int offset) {
	unsigned inode_sector_offset = offset / (SECTOR_SIZE / I_NODE_SIZE);
	offset %= SECTOR_SIZE / I_NODE_SIZE;
	struct HDISK_REQUEST *request = read_sector(super_block, super_block->first_imap_sector + super_block->nr_imap_sectors + inode_sector_offset, 1);
	request->cmd = WRITE;
	store_int(request->buffer, inode->i_num, &offset);
	store_int(request->buffer, inode->i_mode, &offset);
	int i;
	for (i = 0; i < FS_NAME_LEN; ++i) {
		request->buffer[offset++] = inode->name[i];
	}
	for (i = 0; i < FS_SUFFIX_LEN; ++i) {
		request->buffer[offset++] = inode->suffix[i];
	}
	store_int(request->buffer, inode->i_uid, &offset);
	store_int(request->buffer, inode->i_size, &offset);
	store_int(request->buffer, inode->i_time, &offset);
	store_int(request->buffer, inode->i_parent_inode, &offset);
	store_int(request->buffer, inode->i_start_sector, &offset);
	store_int(request->buffer, inode->i_nr_sectors, &offset);
	for (i = 0; i < 9; ++i) {
		store_int(request->buffer, inode->i_zone[i], &offset);
	}
	add_hdisk_request(request);
	do_hdisk_request();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	memman_free_4k(memman, (int) request->buffer, SECTOR_SIZE);
	memman_free_4k(memman, (int) request, sizeof(struct HDISK_REQUEST));
	return;
}

struct I_NODE *create_dir(struct SUPER_BLOCK *super_block, unsigned char name[FS_NAME_LEN], struct I_NODE *p_inode) {
	/* offset : 第 offset 个inode*/
	unsigned int offset = get_unused_inode(super_block);
	update_imap(super_block, offset, 1);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct I_NODE *inode = (struct I_NODE *) memman_alloc_4k(memman, sizeof(struct I_NODE));
	inode->i_num = offset;
	inode->i_mode = 0x000041FF;
	int i;
	for (i = 0; i < FS_NAME_LEN; ++i) {
		inode->name[i] = name[i];
	}
	for (i = 0; i < FS_SUFFIX_LEN; ++i) {
		inode->suffix[i] = 0;
	}
	inode->i_size = 0;
	inode->i_time = 0;
	if (p_inode != 0) {
		inode->i_parent_inode = p_inode->i_num;
		inode->i_uid = p_inode->i_uid;
	}
	else {
		inode->i_parent_inode = NR_INODES;
		inode->i_uid = 0;
	}
	inode->i_start_sector = 0;
	inode->i_nr_sectors = 0;
	for (i = 0; i < 9; ++i) {
		inode->i_zone[i] = 0;
	}
	inode->i_dev = super_block->sb_dev;
	inode->i_dirty = 0;
	inode->i_count = 0;
	if (inode->i_parent_inode < NR_INODES) {
		/*update parent inode*/
		if (p_inode->i_zone[0] == 0) {
			p_inode->i_zone[0] = inode->i_num;
		}
		else if (p_inode->i_zone[1] == 0) {
			p_inode->i_zone[1] = inode->i_num;
		}
		else if (p_inode->i_zone[2] == 0) {
			p_inode->i_zone[2] = inode->i_num;
		}
		else if (p_inode->i_zone[3] == 0) {
			p_inode->i_zone[3] = inode->i_num;
		}
		else if (p_inode->i_zone[4] == 0) {
			p_inode->i_zone[4] = inode->i_num;
		}
		else if (p_inode->i_zone[5] == 0) {
			p_inode->i_zone[5] = inode->i_num;
		}
		else if (p_inode->i_zone[6] == 0) {
			p_inode->i_zone[6] = inode->i_num;
		}
		else {
			return 0;
		}
	}
	write_inode(super_block, inode, offset);
	return inode;
}

struct I_NODE *open_dir(struct SUPER_BLOCK *super_block, unsigned int i_num) {
	unsigned int inode_sector_offset = i_num / (SECTOR_SIZE / I_NODE_SIZE);
	unsigned short offset = i_num % (SECTOR_SIZE / I_NODE_SIZE);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct HDISK_REQUEST request;
	request.dev = super_block->sb_dev;
	request.cmd = READ;
	request.error = 0;
	request.sector = hdisk_struct[super_block->sb_dev].start_sector + inode_sector_offset;
	request.nr_sector = 1;
	request.buffer = (char *) memman_alloc_4k(memman, SECTOR_SIZE);
	add_hdisk_request(&request);
	do_hdisk_request();
	struct I_NODE *inode = (struct I_NODE *) memman_alloc_4k(memman, sizeof(struct I_NODE));
	unsigned int x = offset * I_NODE_SIZE;
	read_int(request.buffer, &inode->i_num, &x);
	read_int(request.buffer, &inode->i_mode, &x);
	int i;
	for (i = 0; i < FS_NAME_LEN; ++i) {
		inode->name[i] = request.buffer[x++];
	}
	for (i = 0; i < FS_SUFFIX_LEN; ++i) {
		inode->suffix[i] = request.buffer[x++];
	}
	read_int(request.buffer, &inode->i_uid, &x);
	read_int(request.buffer, &inode->i_size, &x);
	read_int(request.buffer, &inode->i_time, &x);
	read_int(request.buffer, &inode->i_parent_inode, &x);
	read_int(request.buffer, &inode->i_start_sector, &x);
	read_int(request.buffer, &inode->i_nr_sectors, &x);
	for (i = 0; i < 9; ++i) {
		read_int(request.buffer, &inode->i_zone[i], &x);
	}
	inode->i_dev = super_block->sb_dev;
	inode->i_dirty = 0;
	inode->i_count = 0;
	return inode;
}

void close_dir(struct I_NODE *inode) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	memman_free_4k(memman, (int) inode, sizeof(struct I_NODE));
	return;
}

void delete_dir(struct SUPER_BLOCK *super_block, struct I_NODE *inode) {
	
}

struct FILE_DESC *create_file(struct SUPER_BLOCK *super_block, unsigned char name[FS_NAME_LEN], unsigned char suffix[FS_SUFFIX_LEN], struct I_NODE *p_inode) {
	unsigned int offset = get_unused_inode(super_block);
	update_imap(super_block, offset, 1);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct I_NODE *inode = (struct I_NODE *) memman_alloc_4k(memman, sizeof(struct I_NODE));
	inode->i_num = offset;
	inode->i_mode = 0x0000A1FF;
	int i;
	for (i = 0; i < FS_NAME_LEN; ++i) {
		inode->name[i] = name[i];
	}
	for (i = 0; i < FS_SUFFIX_LEN; ++i) {
		inode->suffix[i] = suffix[i];
	}
	if (p_inode != 0) {
		inode->i_parent_inode = p_inode->i_num;
		inode->i_uid = p_inode->i_uid;
	}
	else {
		inode->i_parent_inode = NR_INODES;
		inode->i_uid = 0;
	}
	inode->i_size = 0;
	inode->i_time = 0;
	inode->i_start_sector = 0;
	inode->i_nr_sectors = 0;
	for (i = 0; i < 9; ++i) {
		inode->i_zone[i] = 0;
	}
	inode->i_dev = super_block->sb_dev;
	inode->i_dirty = 0;
	inode->i_count = 0;
	if (inode->i_parent_inode < NR_INODES) {
		/*update parent inode*/
		if (p_inode->i_zone[0] == 0) {
			p_inode->i_zone[0] = inode->i_num;
		}
		else if (p_inode->i_zone[1] == 0) {
			p_inode->i_zone[1] = inode->i_num;
		}
		else if (p_inode->i_zone[2] == 0) {
			p_inode->i_zone[2] = inode->i_num;
		}
		else if (p_inode->i_zone[3] == 0) {
			p_inode->i_zone[3] = inode->i_num;
		}
		else if (p_inode->i_zone[4] == 0) {
			p_inode->i_zone[4] = inode->i_num;
		}
		else if (p_inode->i_zone[5] == 0) {
			p_inode->i_zone[5] = inode->i_num;
		}
		else if (p_inode->i_zone[6] == 0) {
			p_inode->i_zone[6] = inode->i_num;
		}
		else {
			return 0;
		}
	}
	write_inode(super_block, inode, offset);
	struct FILE_DESC *file = (struct FILE_DESC *)memman_alloc_4k(memman, sizeof(struct FILE_DESC));
	file->fd_mode = 0;
	file->fd_pos = 0;
	file->fd_inode = inode;
	return file;
}

struct FILE_DESC *open_file(struct SUPER_BLOCK *super_block, unsigned int i_num) {
	unsigned int inode_sector_offset = i_num / (SECTOR_SIZE / I_NODE_SIZE);
	unsigned short offset = i_num % (SECTOR_SIZE / I_NODE_SIZE);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct HDISK_REQUEST request;
	request.dev = super_block->sb_dev;
	request.cmd = READ;
	request.error = 0;
	request.sector = hdisk_struct[super_block->sb_dev].start_sector + inode_sector_offset;
	request.nr_sector = 1;
	request.buffer = (char *) memman_alloc_4k(memman, SECTOR_SIZE);
	add_hdisk_request(&request);
	do_hdisk_request();
	struct I_NODE *inode = (struct I_NODE *) memman_alloc_4k(memman, sizeof(struct I_NODE));
	unsigned int x = offset * I_NODE_SIZE;
	read_int(request.buffer, &inode->i_num, &x);
	read_int(request.buffer, &inode->i_mode, &x);
	int i;
	for (i = 0; i < FS_NAME_LEN; ++i) {
		inode->name[i] = request.buffer[x++];
	}
	for (i = 0; i < FS_SUFFIX_LEN; ++i) {
		inode->suffix[i] = request.buffer[x++];
	}
	read_int(request.buffer, &inode->i_uid, &x);
	read_int(request.buffer, &inode->i_size, &x);
	read_int(request.buffer, &inode->i_time, &x);
	read_int(request.buffer, &inode->i_parent_inode, &x);
	read_int(request.buffer, &inode->i_start_sector, &x);
	read_int(request.buffer, &inode->i_nr_sectors, &x);
	for (i = 0; i < 9; ++i) {
		read_int(request.buffer, &inode->i_zone[i], &x);
	}
	inode->i_dev = super_block->sb_dev;
	inode->i_dirty = 0;
	inode->i_count = 0;
	struct FILE_DESC *file = (struct FILE_DESC *)memman_alloc_4k(memman, sizeof(struct FILE_DESC));
	file->fd_mode = 0;
	file->fd_pos = 0;
	file->fd_inode = inode;
	return file;
}

void close_file(struct FILE_DESC *file) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	memman_free_4k(memman, (int) file->fd_inode, sizeof(struct I_NODE));
	memman_free_4k(memman, (int) file, sizeof(struct FILE_DESC));
	return;
}

char *read_file(struct SUPER_BLOCK *super_block, struct FILE_DESC *file) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char *buf = (char *) memman_alloc_4k(memman, 4096);
	int i;
	for (i = 0; i < 4096; ++i) {
		buf[i] = 0;
	}
	for (i = 0; i < 7; ++i) {
		if (file->fd_inode->i_zone[i] != 0) {
			struct HDISK_REQUEST *request = read_sector(super_block, file->fd_inode->i_zone[i], 1);
			int j;
			for (j = 0; j < SECTOR_SIZE; ++j) {
				buf[file->fd_pos++] = request->buffer[j];
			}
			memman_free_4k(memman, (int) request, sizeof(struct HDISK_REQUEST));
		}
	}
	file->fd_pos = 0;
	return buf;
}

void write_file() {
	
}

void delete_file() {
	
}
