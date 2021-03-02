/*
 *
 * Various disk structure definitions
 *
 */

#ifndef __DISKINFO_H__
#define __DISKINFO_H__

/* Partition Types */

#define PART_EMPTY            0x00
#define PART_FAT12            0x01
#define PART_FAT16_32M        0x04
#define PART_EXT              0x05
#define PART_FAT16            0x06
#define PART_HPFS             0x07
#define PART_NTFS             0x07
#define PART_FAT32            0x0b
#define PART_FAT32_LBA        0x0c
#define PART_FAT16_LBA        0x0e
#define PART_EXT_LBA          0x0f

#define PART_HIDDEN_FLAG      0x10 /* Usable on FAT, HPFS, NTFS & extendeds */

#define PART_DYNAMIC_EXT      0x42
#define PART_LINUX_SWAP       0x82
#define PART_LINUX            0x83
#define PART_EXT_LINUX        0x85
#define PART_LINUX_LVM        0x8e
#define PART_LINUX_RAID       0xfd
#define PART_LINUX_LVM_OLD    0xfe

#pragma option -a- /* structures below must be packed */

/* Partition Structures */

struct PARTITION_ENTRY {
    unsigned char status;
    unsigned char head_first;
    struct {
        unsigned char sector        : 6;
        unsigned char cylinder_high : 2;
    } sc_first;
    unsigned char cylinder_low_first;
    unsigned char type;
    unsigned char head_last;
    struct {
        unsigned char sector        : 6;
        unsigned char cylinder_high : 2;
    } sc_last;
    unsigned char cylinder_low_last;
    unsigned long lba_first;
    unsigned long lba_length;
};

struct MBR {
    unsigned char bootcode1[0xDA];
    union {
        struct {
            unsigned int zero;
            unsigned char orig_disk;
            unsigned char second;
            unsigned char minute;
            unsigned char hour;
        } ts;
        unsigned char sig[6];
    } ts_sig;
    unsigned char bootcode2[0xd8];
    union {
        struct {
            unsigned long sig;
            unsigned int copyprot;
        } sig;
        unsigned char bootcode3[6];
    } bc_sig;
    struct PARTITION_ENTRY entries[4];
    unsigned int bootsig;
};

/* Obtained from GRUB project, grub_fat_bpb in include/grub/fat.h */
struct FAT_BOOTSECT {
    unsigned char jmp_boot[3];                /* 0x0000 */
    unsigned char oem_name[8];                /* 0x0003 */
    unsigned int bytes_per_sector;            /* 0x000b */
    unsigned char sectors_per_cluster;        /* 0x000d */
    unsigned int num_reserved_sectors;        /* 0x000e */
    unsigned char num_fats;                   /* 0x0010 */
    unsigned int num_root_entries;            /* 0x0011 */
    unsigned int num_total_sectors_16;        /* 0x0013 */
    unsigned char media;                      /* 0x0015 */
    unsigned int sectors_per_fat_16;          /* 0x0016 */
    unsigned int sectors_per_track;           /* 0x0018 */
    unsigned int num_heads;                   /* 0x001a */
    unsigned long num_hidden_sectors;         /* 0x001c */
    unsigned long num_total_sectors_32;       /* 0x0020 */
    union {
        struct {
            unsigned char num_ph_drive;       /* 0x0024 */
            unsigned char reserved;           /* 0x0025 */
            unsigned char boot_sig;           /* 0x0026 */
            unsigned long num_serial;         /* 0x0027 */
            unsigned char label[11];          /* 0x002b */
            unsigned char fstype[8];          /* 0x0036 */
            unsigned char bootcode[0x1c2];    /* 0x003e */
        } fat12_or_fat16;
        struct {
            unsigned long sectors_per_fat_32; /* 0x0024 */
            unsigned int extended_flags;      /* 0x0028 */
            unsigned int fs_version;          /* 0x002a */
            unsigned long root_cluster;       /* 0x002c */
            unsigned int fs_info;             /* 0x0030 */
            unsigned int backup_boot_sector;  /* 0x0032 */
            unsigned char reserved[12];       /* 0x0034 */
            unsigned char num_ph_drive;       /* 0x0040 */
            unsigned char reserved1;          /* 0x0041 */
            unsigned char boot_sig;           /* 0x0042 */
            unsigned long num_serial;         /* 0x0043 */
            unsigned char label[11];          /* 0x0047 */
            unsigned char fstype[8];          /* 0x0052 */
            unsigned char bootcode[0x1a6];    /* 0x005a */
        } fat32;
    } version_specific;
};

/* Obtained from GRUB project, grub_ntfs_bpb in include/grub/ntfs.h */
struct NTFS_BOOTSECT {
    unsigned char jmp_boot[3];            /* 0x0000 */
    unsigned char oem_name[8];            /* 0x0003 */
    unsigned int bytes_per_sector;        /* 0x000b */
    unsigned char sectors_per_cluster;    /* 0x000d */
    unsigned char reserved_1[7];          /* 0x000e */
    unsigned char media;                  /* 0x0015 */
    unsigned int reserved_2;              /* 0x0016 */
    unsigned int sectors_per_track;       /* 0x0018 */
    unsigned int num_heads;               /* 0x001a */
    unsigned long num_hidden_sectors;     /* 0x001c */
    unsigned long reserved_3;             /* 0x0020 */
    unsigned char bios_drive;             /* 0x0024 */
    unsigned char head_num;               /* 0x0025 */
    unsigned char reserved_4[2];          /* 0x0026 */
    unsigned long num_total_sectors_low;  /* 0x0028 */
    unsigned long num_total_sectors_high; /* 0x002c */
    unsigned long mft_lcn_low;            /* 0x0030 */
    unsigned long mft_lcn_high;           /* 0x0034 */
    unsigned long mft_mirr_lcn_low;       /* 0x0038 */
    unsigned long mft_mirr_lcn_high;      /* 0x003c */
    unsigned char clusters_per_mft;       /* 0x0040 */
    unsigned char reserved_5[3];          /* 0x0041 */
    unsigned char clusters_per_index;     /* 0x0044 */
    unsigned char reserved_6[3];          /* 0x0045 */
    unsigned long num_serial_low;         /* 0x0048 */
    unsigned long num_serial_high;        /* 0x004c */
    unsigned long checksum;               /* 0x0050 */
    unsigned char bootcode[0x1aC];        /* 0x0054 */
};

#pragma option -a. /* ensure packing returned to default */

int read_mbr(unsigned char disk, struct MBR *mbr);
int read_part_bootsect(unsigned char disk, unsigned char part, void *buf);
int write_part_bootsect(unsigned char disk, unsigned char part, void *buf);
int part_bootsect_crc32(unsigned char disk, unsigned char part,
                        unsigned long *crc32);
char *part_type_to_str(unsigned char type);

#endif /* __DISKINFO_H__ */