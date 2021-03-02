/*
 * Provides C prototypes and structures
 */

#ifndef __INT13_H__
#define __INT13_H__

int read_sectors_chs(unsigned char disk, unsigned int cyl, unsigned char head,
                     unsigned char sec, unsigned char *count, void *buf);
int write_sectors_chs(unsigned char disk, unsigned int cyl, unsigned char head,
                      unsigned char sec, unsigned char *count, void *buf);

#define INT13_EXT_SUBSET_FIXED_DISKS 0x01
#define INT13_EXT_SUBSET_REMOVABLE   0x02
#define INT13_EXT_SUBSET_ENHANCED    0x04

struct INT13_EXT_INFO {
    unsigned int sig;       /* should always be 0xAA55 */
    unsigned char ver;      /* 0x01 = 1.x
                               0x20 = 2.0 / EDD-1.0
                               0x21 = 2.1 / EDD-1.1
                               0x30 = 3.0 / EDD-3.0 */
    unsigned int subset;    /* API subset supported */
};

int supports_int13_ext(unsigned char disk, struct INT13_EXT_INFO *info);

struct LBA_PACKET {
    unsigned char size;     /* should be 0x10 */
    unsigned char reserved; /* should be 0 */
    unsigned int count;
    unsigned int bufofs;
    unsigned int bufseg;
    unsigned long lbalow;
    unsigned long lbahigh;
};

int read_sectors_lba(unsigned char disk, struct LBA_PACKET *pkt);
int write_sectors_lba(unsigned char disk, struct LBA_PACKET *pkt);

#endif /* __INT13_H__ */