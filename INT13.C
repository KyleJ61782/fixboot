/*
 *
 * INT 13h wrappers
 *
 */


#include <dos.h>

#include "int13.h"
#include "error.h"

#define CF 1

void set_int13_regs_chs(struct REGPACK *regs, unsigned char disk,
                        unsigned int cyl, unsigned char head,
                        unsigned char sec, unsigned char count, void *buf)
{
    /* assign disk to DL */
    regs->r_dx = disk;
    /* assign low 8 bits of cylinder to CH */
    regs->r_cx = cyl << 8;
    /* assign high 2 bits of cylinder to high 2 bits of CL */
    regs->r_cx |= (cyl & 0x0300) >> 2;
    /* assign head to DH */
    regs->r_dx |= head << 8;
    /* assign sector to low 6 bits of CL */
    regs->r_cx |= sec & 0x3F;

    /* set the sector count to read */
    regs->r_ax = count;

    /* set the buffer */
    regs->r_bx = FP_OFF(buf);
    regs->r_es = FP_SEG(buf);
}

int read_sectors_chs(unsigned char disk, unsigned int cyl, unsigned char head,
                     unsigned char sec, unsigned char *count, void *buf)
{
    struct REGPACK regs;

    /* Set the disk and CHS parameters */
    set_int13_regs_chs(&regs, disk, cyl, head, sec, *count, buf);

    /* Set the function */
    regs.r_ax |= 0x02 << 8;

    intr(0x13, &regs);

    /* Presumably if CF is set AH has a value, but if not, return something
       beyond the range of AH. */
    if (regs.r_flags & CF) {
        *count = regs.r_ax;
        if (regs.r_ax & 0xff00)
            return regs.r_ax >> 8;
        else
            return MAKE_ERROR(ERR_MAJOR_BIOS_UNK, ERR_BIOS_UNK_UNKNOWN);
    }
    return ERR_SUCCESS;
}

int write_sectors_chs(unsigned char disk, unsigned int cyl, unsigned char head,
                      unsigned char sec, unsigned char *count, void *buf)
{
    struct REGPACK regs;

    /* Set the disk and CHS parameters */
    set_int13_regs_chs(&regs, disk, cyl, head, sec, *count, buf);

    /* set the function */
    regs.r_ax |= 0x03 << 8;

    intr(0x13, &regs);

    /* Presumably if CF is set AH has a value, but if not, return something
       beyond the range of AH. */
    if (regs.r_flags & CF) {
        *count = regs.r_ax;
        if (regs.r_ax & 0xff00)
            return regs.r_ax >> 8;
        else
            return MAKE_ERROR(ERR_MAJOR_BIOS_UNK, ERR_BIOS_UNK_UNKNOWN);
    }
    return ERR_SUCCESS;
}

int supports_int13_ext(unsigned char disk, struct INT13_EXT_INFO *info)
{
    struct REGPACK regs;

    /* Put disk number in DL */
    regs.r_dx = disk;

    /* Put magic number in BX */
    regs.r_bx = 0x55aa;

    /* Set function number in AH */
    regs.r_ax = 0x41 << 8;

    intr(0x13, &regs);

    /* Only set info if CF not set */
    if (regs.r_flags & CF)
        return MAKE_ERROR(ERR_MAJOR_BIOS_UNK, ERR_BIOS_UNK_NO_INT13_EXT);

    info->sig = regs.r_bx;
    info->ver = regs.r_ax >> 8;
    info->subset = regs.r_cx;
    return ERR_SUCCESS;
}

int read_sectors_lba(unsigned char disk, struct LBA_PACKET *pkt)
{
    struct REGPACK regs;

    /* Put disk number in DL */
    regs.r_dx = disk;

    /* Put packet address in DS:SI */
    regs.r_si = FP_OFF(pkt);
    regs.r_ds = FP_SEG(pkt);

    /* Set function number in AH */
    regs.r_ax = 0x42 << 8;

    intr(0x13, &regs);

    /* Presumably if CF is set AH has a value, but if not, return something
       beyond the range of AH. */
    if (regs.r_flags & CF) {
        if (regs.r_ax & 0xff00)
            return regs.r_ax >> 8;
        else
            return MAKE_ERROR(ERR_MAJOR_BIOS_UNK, ERR_BIOS_UNK_UNKNOWN);
    }
    return ERR_SUCCESS;
}

int write_sectors_lba(unsigned char disk, struct LBA_PACKET *pkt)
{
    struct REGPACK regs;

    /* Put disk number in DL */
    regs.r_dx = disk;

    /* Put packet address in DS:SI */
    regs.r_si = FP_OFF(pkt);
    regs.r_ds = FP_SEG(pkt);

    /* Set function number in AH */
    regs.r_ax = 0x43 << 8;

    /* Leave AL alone as we don't want verification */

    intr(0x13, &regs);

    /* Presumably if CF is set AH has a value, but if not, return something
       beyond the range of AH. */
    if (regs.r_flags & CF) {
        if (regs.r_ax & 0xff00)
            return regs.r_ax >> 8;
        else
            return MAKE_ERROR(ERR_MAJOR_BIOS_UNK, ERR_BIOS_UNK_UNKNOWN);
    }
    return ERR_SUCCESS;
}