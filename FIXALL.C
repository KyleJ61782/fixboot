/*
 *
 * Boot fix routine dispatcher
 * 
 */

#include "diskinfo.h"
#include "fixntfs.h"
#include "error.h"

int fix_boot(unsigned char disk, unsigned char part, int strict)
{
    struct MBR mbr;
    struct NTFS_BOOTSECT ntfsbs;
    int res;
    unsigned long crc32;
    int fixup_idx;

    /* Verify disk is valid */
    if ((disk & 0x7F) >= *(unsigned char far *)0x00400075)
        return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_DISK_NOT_PRES);

    /* Verify partition is in range */
    if (part >= 4)
        return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_PART_OOB);

    /* Identify partition type */
    res = read_mbr(disk, &mbr);
    if (FAILED(res))
        return res;
    
    switch (mbr.entries[part].type) {
    case PART_NTFS:
        return fix_ntfs_boot(disk, part, strict);
    }
    return MAKE_ERROR(ERR_MAJOR_FIXALL, ERR_FIXALL_UNSUPPORTED);
}