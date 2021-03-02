/*
 *
 * Disk information utility functions
 *
 */

#include <dos.h>
#include "diskinfo.h"
#include "int13.h"
#include "crc32.h"
#include "error.h"

char *part_type_to_str(unsigned char type)
{
    switch(type) {
    case PART_EMPTY:
        return "Empty";
    case PART_FAT12:
        return "FAT12";
    case PART_FAT16_32M:
        return "FAT16 (32MB limit)";
    case PART_EXT:
        return "Extended Container";
    case PART_FAT16:
        return "FAT16B";
    case PART_NTFS:
        return "HPFS/NTFS";
    case PART_FAT32:
        return "FAT32";
    case PART_FAT32_LBA:
        return "FAT32 LBA";
    case PART_FAT16_LBA:
        return "FAT16 LBA";
    case PART_EXT_LBA:
        return "Extended Container LBA";
    case PART_HIDDEN_FLAG | PART_FAT12:
        return "Hidden FAT12";
    case PART_HIDDEN_FLAG | PART_FAT16_32M:
        return "Hidden FAT16 (32MB limit)";
    case PART_HIDDEN_FLAG | PART_EXT:
        return "Hidden Extended Container";
    case PART_HIDDEN_FLAG | PART_FAT16:
        return "Hidden FAT16B";
    case PART_HIDDEN_FLAG | PART_NTFS:
        return "Hidden HPFS/NTFS";
    case PART_HIDDEN_FLAG | PART_FAT32:
        return "Hidden FAT32";
    case PART_HIDDEN_FLAG | PART_FAT32_LBA:
        return "Hidden FAT32 LBA";
    case PART_HIDDEN_FLAG | PART_FAT16_LBA:
        return "Hidden FAT16 LBA";
    case PART_HIDDEN_FLAG | PART_EXT_LBA:
        return "Hidden Extended Container LBA";
    case PART_DYNAMIC_EXT:
        return "Dynamic Disk Extended Container";
    case PART_LINUX_SWAP:
        return "Linux Swap";
    case PART_LINUX:
        return "Linux";
    case PART_EXT_LINUX:
        return "Linux Extended Container";
    case PART_LINUX_LVM:
        return "Linux LVM";
    case PART_LINUX_RAID:
        return "Linux RAID";
    case PART_LINUX_LVM_OLD:
        return "Linux LVM Old";
    default:
        return "Unknown";
    }
}

int part_type_uses_lba(unsigned char type)
{
    switch (type) {
    case PART_FAT32_LBA:
    case PART_FAT16_LBA:
    case PART_EXT_LBA:
    case PART_HIDDEN_FLAG | PART_FAT32_LBA:
    case PART_HIDDEN_FLAG | PART_FAT16_LBA:
    case PART_HIDDEN_FLAG | PART_EXT_LBA:
        return 1;
    default:
        return 0;
    }
}

int part_type_is_hidden(unsigned char type)
{
    switch(type) {
    case PART_HIDDEN_FLAG | PART_FAT12:
    case PART_HIDDEN_FLAG | PART_FAT16_32M:
    case PART_HIDDEN_FLAG | PART_EXT:
    case PART_HIDDEN_FLAG | PART_FAT16:
    case PART_HIDDEN_FLAG | PART_NTFS:
    case PART_HIDDEN_FLAG | PART_FAT32:
    case PART_HIDDEN_FLAG | PART_FAT32_LBA:
    case PART_HIDDEN_FLAG | PART_FAT16_LBA:
    case PART_HIDDEN_FLAG | PART_EXT_LBA:
        return 1;
    default:
        return 0;
    }
}

int read_mbr(unsigned char disk, struct MBR *mbr)
{
    unsigned char count;
    int readres;
    count = 1;
    readres = read_sectors_chs(disk, 0, 0, 1, &count, mbr);
    if (count != 1)
        return MAKE_ERROR(ERR_MAJOR_INCOMPLETE, count);
    else
        return readres;
}

int read_part_bootsect(unsigned char disk, unsigned char part, void *buf)
{
    unsigned char count;
    int readres;
    struct MBR mbr;
    struct INT13_EXT_INFO info;
    struct LBA_PACKET pkt;
    int cyl, head, sect;

    readres = read_mbr(disk, &mbr);
    if (FAILED(readres))
        return readres;

    if (part_type_uses_lba(mbr.entries[part].type)) {
        if (supports_int13_ext(disk, &info))
            return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_NO_LBA_EXT);
        if (!(info.subset & INT13_EXT_SUBSET_ENHANCED))
            return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_NO_LBA_EXT);
        pkt.size = sizeof(pkt);
        pkt.reserved = 0;
        pkt.count = 1;
        pkt.bufofs = FP_OFF(buf);
        pkt.bufseg = FP_SEG(buf);
        pkt.lbalow = mbr.entries[part].lba_first;
        readres = read_sectors_lba(disk, &pkt);
        if (pkt.count != 1)
            return MAKE_ERROR(ERR_MAJOR_INCOMPLETE, pkt.count);
        return readres;
    }
    else {
        cyl = mbr.entries[part].sc_first.cylinder_high << 8;
        cyl |= mbr.entries[part].cylinder_low_first;
        head = mbr.entries[part].head_first;
        sect = mbr.entries[part].sc_first.sector;
        count = 1;
        readres = read_sectors_chs(disk, cyl, head, sect, &count, buf);
        if (count != 1)
            return MAKE_ERROR(ERR_MAJOR_INCOMPLETE, count);
        else
            return readres;
    }
}

int write_part_bootsect(unsigned char disk, unsigned char part, void *buf)
{
    unsigned char count;
    int writeres;
    struct MBR mbr;
    struct INT13_EXT_INFO info;
    struct LBA_PACKET pkt;
    int cyl, head, sect;

    writeres = read_mbr(disk, &mbr);
    if (FAILED(writeres))
        return writeres;

    if (part_type_uses_lba(mbr.entries[part].type)) {
        if (supports_int13_ext(disk, &info))
            return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_NO_LBA_EXT);
        if (!(info.subset & INT13_EXT_SUBSET_ENHANCED))
            return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_NO_LBA_EXT);
        pkt.size = sizeof(pkt);
        pkt.reserved = 0;
        pkt.count = 1;
        pkt.bufofs = FP_OFF(buf);
        pkt.bufseg = FP_SEG(buf);
        pkt.lbalow = mbr.entries[part].lba_first;
        writeres = write_sectors_lba(disk, &pkt);
        if (pkt.count != 1)
            return MAKE_ERROR(ERR_MAJOR_INCOMPLETE, pkt.count);
        return writeres;
    }
    else {
        cyl = mbr.entries[part].sc_first.cylinder_high << 8;
        cyl |= mbr.entries[part].cylinder_low_first;
        head = mbr.entries[part].head_first;
        sect = mbr.entries[part].sc_first.sector;
        count = 1;
        writeres = write_sectors_chs(disk, cyl, head, sect, &count, buf);
        if (count != 1)
            return MAKE_ERROR(ERR_MAJOR_INCOMPLETE, count);
        else
            return writeres;
    }
}

int part_bootsect_crc32(unsigned char disk, unsigned char part,
                        unsigned long *crc32)
{
    struct MBR mbr;
    int readres;
    unsigned char bsbuf[512];
    struct FAT_BOOTSECT *bsfat;
    struct NTFS_BOOTSECT *bsntfs;

    if ((disk & 0x7F) >= *(unsigned char far *)0x00400075)
        return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_DISK_NOT_PRES);

    if (part >= 4)
        return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_PART_OOB);

    readres = read_mbr(disk, &mbr);
    if (FAILED(readres))
        return readres;

    readres = read_part_bootsect(disk, part, bsbuf);
    if (FAILED(readres))
        return readres;

    switch (mbr.entries[part].type) {
    case PART_FAT12:
    case PART_HIDDEN_FLAG | PART_FAT12:
    case PART_FAT16_32M:
    case PART_FAT16:
    case PART_FAT16_LBA:
    case PART_HIDDEN_FLAG | PART_FAT16_32M:
    case PART_HIDDEN_FLAG | PART_FAT16:
    case PART_HIDDEN_FLAG | PART_FAT16_LBA:
        bsfat = (struct FAT_BOOTSECT *)bsbuf;
        *crc32 = crc(bsfat->version_specific.fat12_or_fat16.bootcode,
            sizeof(bsfat->version_specific.fat12_or_fat16.bootcode));
        break;
    case PART_FAT32:
    case PART_FAT32_LBA:
    case PART_HIDDEN_FLAG | PART_FAT32:
    case PART_HIDDEN_FLAG | PART_FAT32_LBA:
        bsfat = (struct FAT_BOOTSECT *)bsbuf;
        *crc32 = crc(bsfat->version_specific.fat32.bootcode,
            sizeof(bsfat->version_specific.fat32.bootcode));
        break;
    case PART_NTFS:
    case PART_HIDDEN_FLAG | PART_NTFS:
        bsntfs = (struct NTFS_BOOTSECT *)bsbuf;
        *crc32 = crc(bsntfs->bootcode, sizeof(bsntfs->bootcode));
        break;
    default:
        return MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_PART_FMT_UNK);
    }

    return ERR_SUCCESS;
}