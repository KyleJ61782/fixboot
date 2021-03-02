/*
 *
 * Header for NTFS partition fix routines
 *
 */

#ifndef __FIXNTFS_H__
#define __FIXNTFS_H__

int fix_ntfs_boot(unsigned char disk, unsigned char part, int strict);

#endif /* __FIXNTFS_H__ */