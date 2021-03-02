/*
 *
 * Error messages and handling
 *
 */

#include <string.h>
#include "error.h"

char errstrbuf[81];

char *errstr(int errnum)
{
    if (SUCCEEDED(errnum))
        return strcpy(errstrbuf, "Succeeded");

    switch (ERR_MAJOR(errnum)) {
    case ERR_MAJOR_BIOS:
        switch (ERR_MINOR(errnum)) {
        case ERR_BIOS_INVALID_PARM:
            strcpy(errstrbuf, "Invalid parameter");
            break;
        case ERR_BIOS_ADDR_MARK_NOT_FOUND:
            strcpy(errstrbuf, "Address mark not found");
            break;
        case ERR_BIOS_WRITE_PROT:
            strcpy(errstrbuf, "Disk write protected");
            break;
        case ERR_BIOS_SECT_NOT_FOUND:
            strcpy(errstrbuf, "Sector not found");
            break;
        case ERR_BIOS_RESET_FAILED:
            strcpy(errstrbuf, "Reset failed");
            break;
        case ERR_BIOS_DISK_CHANGED:
            strcpy(errstrbuf, "Disk changed");
            break;
        case ERR_BIOS_DRIVE_PARM_ACT_FAILED:
            strcpy(errstrbuf, "Drive parameter activity failed");
            break;
        case ERR_BIOS_DMA_OVERRUN:
            strcpy(errstrbuf, "DMA buffer overrun");
            break;
        case ERR_BIOS_DATA_BOUNDARY:
            strcpy(errstrbuf, "Data boundary error");
            break;
        case ERR_BIOS_BAD_SECTOR:
            strcpy(errstrbuf, "Bad sector detected");
            break;
        case ERR_BIOS_BAD_TRACK:
            strcpy(errstrbuf, "Bad track detected");
            break;
        case ERR_BIOS_INVALID_MEDIA:
            strcpy(errstrbuf, "Unsupported track or media");
            break;
        case ERR_BIOS_INVALID_NUM_SECTORS:
            strcpy(errstrbuf, "Invalid number of sectors on format");
            break;
        case ERR_BIOS_CTRL_DATA_ADDR_DET:
            strcpy(errstrbuf, "Control data address mark detected");
            break;
        case ERR_BIOS_DMA_ARB_OOR:
            strcpy(errstrbuf, "DMA arbitration level out of range");
            break;
        case ERR_BIOS_UNCORRECTABLE_CRC:
            strcpy(errstrbuf, "Uncorrectable CRC or ECC error");
            break;
        case ERR_BIOS_CORRECTED_ECC:
            strcpy(errstrbuf, "Data ECC corrected");
            break;
        case ERR_BIOS_CONTROLLER_FAILURE:
            strcpy(errstrbuf, "Controller failure");
            break;
        case ERR_BIOS_NO_MEDIA:
            strcpy(errstrbuf, "No media in drive");
            break;
        case ERR_BIOS_BAD_DRIVE_TYPE:
            strcpy(errstrbuf, "Incorrected drive type stored in CMOS");
            break;
        case ERR_BIOS_SEEK_FAILED:
            strcpy(errstrbuf, "Seek failed");
            break;
        case ERR_BIOS_TIMEOUT:
            strcpy(errstrbuf, "Timeout (not ready)");
            break;
        case ERR_BIOS_NOT_READY:
            strcpy(errstrbuf, "Drive not ready");
            break;
        case ERR_BIOS_VOL_NOT_LOCKED:
            strcpy(errstrbuf, "Volume not locked in drive");
            break;
        case ERR_BIOS_VOL_LOCKED:
            strcpy(errstrbuf, "Volume locked in drive");
            break;
        case ERR_BIOS_VOL_NOT_REMOVABLE:
            strcpy(errstrbuf, "Volume not removable");
            break;
        case ERR_BIOS_VOL_IN_USE:
            strcpy(errstrbuf, "Volume in use");
            break;
        case ERR_BIOS_LOCK_COUNT_EXCEEDED:
            strcpy(errstrbuf, "Lock count exceeded");
            break;
        case ERR_BIOS_EJECT_FAILED:
            strcpy(errstrbuf, "Eject failed");
            break;
        case ERR_BIOS_VOL_READ_PROTECTED:
            strcpy(errstrbuf, "Volume read protected");
            break;
        case ERR_BIOS_UNDEFINED:
            strcpy(errstrbuf, "Undefined error");
            break;
        case ERR_BIOS_WRITE_FAULT:
            strcpy(errstrbuf, "Write fault");
            break;
        case ERR_BIOS_STATUS_REGISTER:
            strcpy(errstrbuf, "Status register error");
            break;
        case ERR_BIOS_SENSE_FAILED:
            strcpy(errstrbuf, "Sense operation failed");
            break;
        default:
            strcpy(errstrbuf, "Unknown int 13h error");
        }
        break;
    case ERR_MAJOR_BIOS_UNK:
        switch (ERR_MINOR(errnum)) {
        case ERR_BIOS_UNK_NO_INT13_EXT:
            strcpy(errstrbuf, "No int 13h extensions");
            break;
        default:
            strcpy(errstrbuf, "Int 13h operation failed without error code");
        }
        break;
    case ERR_MAJOR_INCOMPLETE:
        strcpy(errstrbuf, "Int 13h operation did not complete.");
        break;
    case ERR_MAJOR_DISKINFO:
        switch (ERR_MINOR(errnum)) {
        case ERR_DISKINFO_DISK_NOT_PRES:
            strcpy(errstrbuf, "Disk not present in system");
            break;
        case ERR_DISKINFO_PART_OOB:
            strcpy(errstrbuf, "Partition index out of bounds");
            break;
        case ERR_DISKINFO_PART_FMT_UNK:
            strcpy(errstrbuf, "Partition format unreadable");
            break;
        case ERR_DISKINFO_NO_LBA_EXT:
            strcpy(errstrbuf, "Int 13h extensions not available");
            break;
        default:
            strcpy(errstrbuf, "Unknown diskinfo error");
        }
        break;
    case ERR_MAJOR_FIXALL:
        switch (ERR_MINOR(errnum)) {
        case ERR_FIXALL_UNSUPPORTED:
            strcpy(errstrbuf, "Unsupported partition type");
            break;
        default:
            strcpy(errstrbuf, "Unknown fixall error");
        }
        break;
    case ERR_MAJOR_FIXNTFS:
        switch (ERR_MINOR(errnum)) {
        case ERR_FIXNTFS_DISK_NOT_PRES:
            return errstr(
                MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_DISK_NOT_PRES));
        case ERR_FIXNTFS_PART_OOB:
            return errstr(
                MAKE_ERROR(ERR_MAJOR_DISKINFO, ERR_DISKINFO_PART_OOB));
        case ERR_FIXNTFS_NOT_NTFS_PART:
            strcpy(errstrbuf, "Not an NTFS partition");
            break;
        case ERR_FIXNTFS_NO_APPL_FIXUP:
            strcpy(errstrbuf, "No applicable boot sector fixup found");
            break;
        default:
            strcpy(errstrbuf, "Unknown fixntfs error");
        }
        break;
    case ERR_MAJOR_APP:
        switch (ERR_MINOR(errnum)) {
        case ERR_APP_INVALID_ARGS:
            strcpy(errstrbuf, "Invalid arguments supplied");
            break;
        case ERR_APP_INVALID_DISK_NUM:
            strcpy(errstrbuf, "Invalid disk number argument");
            break;
        case ERR_APP_INVALID_PART_NUM:
            strcpy(errstrbuf, "Invalid partition number argument");
            break;
        case ERR_APP_COULDNT_OPEN_FILE:
            strcpy(errstrbuf, "Couldn't open file");
            break;
        case ERR_APP_COULDNT_WRITE_FILE:
            strcpy(errstrbuf, "Couldn't write to file");
            break;
        case ERR_APP_COULDNT_READ_FILE:
            strcpy(errstrbuf, "Couldn't read from file");
            break;
        default:
            strcpy(errstrbuf, "Unknown application error");
        }
        break;
    default:
        strcpy(errstrbuf, "Unknown major error");
    }
    return errstrbuf;
}