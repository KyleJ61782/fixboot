/*
 *
 * Source file containing NTFS boot fix routines
 *
 */

#include <mem.h>

#include "fixntfs.h"
#include "diskinfo.h"
#include "error.h"
#include "misc.h"

struct CODE_RANGE {
    const unsigned int offset;
    const unsigned int len;
    const unsigned char *const orig_code, *const fixed_code;
};

enum PARAM_CHANGE_KIND {
    pckBIOSDrive
    };

struct PARAM_CHANGE {
    const enum PARAM_CHANGE_KIND param_change;
    const unsigned int offset;
};

struct FIXUP {
    const unsigned long orig_crc;
    const int range_count, param_change_count;
    const struct CODE_RANGE *const ranges;
    const struct PARAM_CHANGE *const param_changes;
};

/* The NT 4.0 boot sector is broken in two ways:
   1) The NTFS_BOOTSECT::bios_drive value is always 0x80, namely fixed disk 0.
   2) The boot code assumes fixed disk 0 as well as we can see from the
      disassembly below. All we need to do to make it boot successfully from a
      secondary drive is update the bios_drive value and change the code to
      the fixup code below.
*/
const unsigned char nt40_orig_code[] = {
    0x8a, 0x36, 0x25, 0x00,     /* mov dh, ds:[NTFS_BOOTSECT::head_num] */
    0xb2, 0x80                  /* mov dl, 0x80 */
    };
const unsigned char nt40_fixed_code[] = {
    /* The following line works because dl ends up getting the fixed bios_drive
       and dh gets head_num for the immediately following int 13h call. */
    0x8b, 0x16, 0x24, 0x00,     /* mov dx, ds:[NTFS_BOOTSECT::bios_drive] */
    0x90,                       /* nop */
    0x90                        /* nop */
    };
const struct CODE_RANGE nt40_code_ranges[] = {
    {0xe4, 6, nt40_orig_code, nt40_fixed_code}
};
const struct PARAM_CHANGE nt40_param_changes[] = {
    {pckBIOSDrive, OFFSETOF(struct NTFS_BOOTSECT, bios_drive)}
};

#define FIXUP_COUNT 1
const struct FIXUP all_fixups[FIXUP_COUNT] = {
    {0x4af911e5, 1, 1, nt40_code_ranges, nt40_param_changes}
};

int bootcode_matches(const struct NTFS_BOOTSECT *ntfsbs, 
    const struct FIXUP *fixup)
{
    int i;
    const struct CODE_RANGE *range;

    for (i = 0; i < fixup->range_count; i++) {
        range = &fixup->ranges[i];
        if (memcmp(&((unsigned char *)ntfsbs)[range->offset],
                   range->orig_code, range->len) != 0)
            return 0;
    }
    return 1;
}

int identify_applicable_fixup(int strict, unsigned char disk,
                              unsigned char part, int *matched) {
    int i, j;
    struct NTFS_BOOTSECT ntfsbs;
    unsigned long crc;
    int readres;

    if (strict) {
        readres = part_bootsect_crc32(disk, part, &crc);
        if (FAILED(readres))
            return readres;
    }

    readres = read_part_bootsect(disk, part, &ntfsbs);
    if (FAILED(readres))
        return readres;

    for (i = 0; i < FIXUP_COUNT; i++) {
        if (strict) {
            if (all_fixups[i].orig_crc == crc) {
                if (bootcode_matches(&ntfsbs, &all_fixups[i])) {
                    *matched = i;
                    return ERR_SUCCESS;
                }
            }
        }
        else {
            if (bootcode_matches(&ntfsbs, &all_fixups[i])) {
                *matched = i;
                return ERR_SUCCESS;
            }
        }
    }
    return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_NO_APPL_FIXUP);
}

int fix_ntfs_boot(unsigned char disk, unsigned char part, int strict)
{
    struct MBR mbr;
    struct NTFS_BOOTSECT ntfsbs;
    int res;
    unsigned long crc32;
    int fixup_idx;
    struct FIXUP *fixup;
    struct CODE_RANGE *range;
    int i;

    /* Verify disk is valid */
    if ((disk & 0x7F) >= *(unsigned char far *)0x00400075)
        return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_DISK_NOT_PRES);

    /* Verify partition is in range */
    if (part >= 4)
        return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_PART_OOB);

    /* Verify partition is NTFS */
    res = read_mbr(disk, &mbr);
    if (FAILED(res))
        return res;
    if (mbr.entries[part].type != PART_NTFS)
        return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_NOT_NTFS_PART);
    res = read_part_bootsect(disk, part, &ntfsbs);
    if (FAILED(res))
        return res;
    if (memcmp(ntfsbs.oem_name, "NTFS", 4) != 0)
        return MAKE_ERROR(ERR_MAJOR_FIXNTFS, ERR_FIXNTFS_NOT_NTFS_PART);

    /* Identify matching fixup */
    res = identify_applicable_fixup(strict, disk, part, &fixup_idx);
    if (FAILED(res))
        return res;

    /* Fixup boot code */
    fixup = &all_fixups[fixup_idx];
    for (i = 0; i < fixup->range_count; i++) {
        range = &fixup->ranges[i];
        memcpy(&((unsigned char *)&ntfsbs)[range->offset], range->fixed_code, range->len);
    }

    /* Fixup boot sector data as instructed */
    for (i = 0; i < fixup->param_change_count; i++) {
        switch (fixup->param_changes[i].param_change) {
        case pckBIOSDrive:
            ((unsigned char *)&ntfsbs)[fixup->param_changes[i].offset] = disk;
            break;
        }
    }

    /* Write boot sector out */
    return write_part_bootsect(disk, part, &ntfsbs);
}