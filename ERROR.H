/*
 *
 * Error messages and handling
 *
 */

#ifndef __ERROR_H__
#define __ERROR_H__

#define SUCCEEDED(x) (x == 0)
#define FAILED(x) (x != 0)

#define MAKE_ERROR(major, minor) \
    ((ERR_MAJOR_MASK & ((major) << 8)) | (ERR_MINOR_MASK & (minor)))

#define ERR_MAJOR_MASK 0xff00
#define ERR_MINOR_MASK 0x00ff

#define ERR_MAJOR(x) (((x) & ERR_MAJOR_MASK) >> 8)
#define ERR_MINOR(x) ((x) & ERR_MINOR_MASK)

#define ERR_SUCCESS 0x0000

/* ============================== *
 * Major error groupings          *
 * ============================== */

/* Defined BIOS error */
#define ERR_MAJOR_BIOS       0x00
/* Undefined BIOS error */
#define ERR_MAJOR_BIOS_UNK   0x01
/* Incomplete read or write */
#define ERR_MAJOR_INCOMPLETE 0x02
/* Disk info error */
#define ERR_MAJOR_DISKINFO   0x03
/* Fix all error */
#define ERR_MAJOR_FIXALL     0x04
/* Fix NTFS error */
#define ERR_MAJOR_FIXNTFS    0x05
/* Application error */
#define ERR_MAJOR_APP        0xff

/* ============================== *
 * Minor error groupings          *
 * ============================== */

/* BIOS errors - Obtained from Ralf Brown's Interrupt List */
#define ERR_BIOS_SUCCESS               0x00
#define ERR_BIOS_INVALID_PARM          0x01
#define ERR_BIOS_ADDR_MARK_NOT_FOUND   0x02
#define ERR_BIOS_WRITE_PROT            0x03
#define ERR_BIOS_SECT_NOT_FOUND        0x04
#define ERR_BIOS_RESET_FAILED          0x05
#define ERR_BIOS_DISK_CHANGED          0x06
#define ERR_BIOS_DRIVE_PARM_ACT_FAILED 0x07
#define ERR_BIOS_DMA_OVERRUN           0x08
#define ERR_BIOS_DATA_BOUNDARY         0x09
#define ERR_BIOS_BAD_SECTOR            0x0a
#define ERR_BIOS_BAD_TRACK             0x0b
#define ERR_BIOS_INVALID_MEDIA         0x0c
#define ERR_BIOS_INVALID_NUM_SECTORS   0x0d
#define ERR_BIOS_CTRL_DATA_ADDR_DET    0x0e
#define ERR_BIOS_DMA_ARB_OOR           0x0f
#define ERR_BIOS_UNCORRECTABLE_CRC     0x10
#define ERR_BIOS_CORRECTED_ECC         0x11
#define ERR_BIOS_CONTROLLER_FAILURE    0x20
#define ERR_BIOS_NO_MEDIA              0x31
#define ERR_BIOS_BAD_DRIVE_TYPE        0x32
#define ERR_BIOS_SEEK_FAILED           0x40
#define ERR_BIOS_TIMEOUT               0x80
#define ERR_BIOS_NOT_READY             0xaa
#define ERR_BIOS_VOL_NOT_LOCKED        0xb0
#define ERR_BIOS_VOL_LOCKED            0xb1
#define ERR_BIOS_VOL_NOT_REMOVABLE     0xb2
#define ERR_BIOS_VOL_IN_USE            0xb3
#define ERR_BIOS_LOCK_COUNT_EXCEEDED   0xb4
#define ERR_BIOS_EJECT_FAILED          0xb5
#define ERR_BIOS_VOL_READ_PROTECTED    0xb6
#define ERR_BIOS_UNDEFINED             0xbb
#define ERR_BIOS_WRITE_FAULT           0xcc
#define ERR_BIOS_STATUS_REGISTER       0xe0
#define ERR_BIOS_SENSE_FAILED          0xff

/* Undefined BIOS error */
#define ERR_BIOS_UNK_UNKNOWN           0x00
#define ERR_BIOS_UNK_NO_INT13_EXT      0x01

/* Incomplete read or write error (no codes here, minor is actual count) */

/* Disk info error */
#define ERR_DISKINFO_DISK_NOT_PRES     0x00
#define ERR_DISKINFO_PART_OOB          0x01
#define ERR_DISKINFO_PART_FMT_UNK      0x02
#define ERR_DISKINFO_NO_LBA_EXT        0x03

/* Fix all boot error */
#define ERR_FIXALL_UNSUPPORTED         0x00

/* Fix NTFS boot error */
#define ERR_FIXNTFS_DISK_NOT_PRES      0x00
#define ERR_FIXNTFS_PART_OOB           0x01
#define ERR_FIXNTFS_NOT_NTFS_PART      0x02
#define ERR_FIXNTFS_NO_APPL_FIXUP      0x03 

/* Application errors */
#define ERR_APP_INVALID_ARGS           0x00
#define ERR_APP_INVALID_DISK_NUM       0x01
#define ERR_APP_INVALID_PART_NUM       0x02
#define ERR_APP_READ_FIXED_DISK        0x03
#define ERR_APP_OUT_OF_MEMORY          0x04
#define ERR_APP_COULDNT_OPEN_FILE      0x05
#define ERR_APP_COULDNT_WRITE_FILE     0x06
#define ERR_APP_COULDNT_READ_FILE      0x07

char *errstr(int errnum);

#endif /* __ERROR_H__ */