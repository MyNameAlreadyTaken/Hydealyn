#include "head.h"

#define NR_PART_PER_DRIVE	4					/*分区/磁盘*/
#define NR_SUB_PER_PART	16					/*逻辑分区/分区*/
#define NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)	/*最多逻辑分区*/
#define NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)
#define MAX_PRIM		(MAX_DRIVE * NR_PRIM_PER_DRIVE - 1)
#define MAX_SUBPARTITIONS	(NR_SUB_PER_DRIVE * MAX_DRIVES)

#define HDISK_INTR	do_hdisk
void (*HDISK_INTR)(void) = (void *)0;

#define DEV_CLOSE 	0
#define DEV_OPEN 		1
#define DEV_IDENTIFY	3

#define PRI_DATA 		0x1F0
#define PRI_ERROR		0x1F1
#define PRI_FEATURES 	PRI_ERROR
#define PRI_SECTORCOUNT	0x1F2		/* nr of sectors to read/write*/
#define PRI_LBA_L		0x1F3
#define PRI_LBA_M		0x1F4
#define PRI_LBA_H		0x1F5
#define PRI_DEVICE		0x1F6
#define PRI_STATUS	0x1F7
#define PRI_CMD		PRI_STATUS

#define CTL_BLK_REG	0x3F6		/*控制寄存器端口*/
#define ALT_STATUS	CTL_BLK_REG
#define DEV_CTL		CTL_BLK_REG
#define REG_DRV_ADDR	0x3F7

/*bits for STATUS*/
#define	STATUS_BSY	0x80		/*控制器忙碌*/
#define	STATUS_READY	0x40		/*驱动器就绪*/
#define	STATUS_DFSE	0x20		/*驱动器故障*/
#define	STATUS_DSC	0x10		/*寻道结束*/
#define	STATUS_DRQ	0x08		/*请求服务*/
#define	STATUS_CORR	0x04		/*ECC校验错*/
#define	STATUS_IDX	0x02		/*收到索引*/
#define	STATUS_ERR	0x01		/*命令执行错误*/

#define PRI_TO_SEC		0x90

#define MAKE_DEVICE_REG(lba, drv, lba_highest) ((lba << 6) | (drv << 4) | (lba_highest & 0xF) | 0xA0)

/*values for PRI_CMD*/
#define ATA_IDENTIFY	0xEC
#define ATA_RESTORE	0x10
#define ATA_READ		0x20
#define ATA_WRITE		0x30
#define ATA_VERIFY	0x40;
#define ATA_FORMAT	0x50;
#define ATA_INIT		0x60
#define ATA_SEEK		0x70
#define ATA_DIAGNOSE	0x90
#define ATA_SPECIFY	0x91

/*bits for PRI_ERROR*/
#define MARK_ERR		0x01
#define TRKO_ERR		0x02
#define ABRT_ERR		0x04
#define ID_ERR		0x10
#define ECC_ERR		0x40
#define BBD_ERR		0x80

#define MAX_ERROR	7

typedef unsigned char DEV_REG;
typedef unsigned char STATUS_REG;
typedef unsigned char DEV_CTL_REG;

struct CMD_BLK_REG {
	unsigned char data, features, count, lba_low, lba_mid, lba_high, command;
	DEV_REG device;
};

struct DRIVE_INFO { char dummy[32]; };

#define DRIVE_INFO_ADDR (*(struct DRIVE_INFO *)0x8080)

#define PARTITION_OFFSET	0x1BE

#define DRV_OF_DEV(dev) (dev <= MAX_PRIM ? dev / NR_PRIM_PER_DRIVE : (dev - 16) / NR_SUB_PER_DRIVE)

struct PROCESS *create_hdprocess();

void hdisk_sendCmd(struct CMD_BLK_REG *cmd_blk_reg);
int controller_ready(void);
int winchester_result(void);
int drive_busy(void);
void reset_controller(void);
void reset_hdisk(int nr);
void unexpected_hdisk_interrupt(void);
void bad_rw_intr(void);
void read_intr(void);
void recal_intr(void);
