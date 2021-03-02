/*
 * A simple tool to check NTFS volumes for proper BIOS boot drive parameters
 * and code that can boot from secondary drives. This is handy when utilizing
 * boot loaders that can load from secondary drives.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "crc32.h"
#include "diskinfo.h"
#include "error.h"
#include "fixall.h"

enum COMMAND {
    MODE_HELP,
    MODE_INFO,
    MODE_FIX,
    MODE_SAVE,
    MODE_RESTORE
};

enum COMMAND command = MODE_INFO;
int disknum = -1;
int partnum = -1;
int lenient_fix = 0;
char *filename = "bootsect.bin";

int usage(int error, char *errmsg, char **argv)
{
    FILE *output;
    char *cmdname;

    output = error ? stderr : stdout;
    cmdname = strrchr(argv[0], '\\');
    if (!*cmdname)
        cmdname = argv[0];
    else
        cmdname++;
    if (error)
        fprintf(output, "%s\n\n", errmsg);
    fprintf(output, "usage: %s <command> [<args>]\n\n", cmdname);
    fprintf(output, "These are the available commands:\n");
    fprintf(output, "   help\n");
    fprintf(output, "      Show this help\n");
    fprintf(output, "   info [<disknum>] [<partnum>]\n");
    fprintf(output, "      Show partition information, optionally limited to disk <disknum> or\n");
    fprintf(output, "      partition <partnum> (both indexed from 0).\n");
    fprintf(output, "   fix <disknum> <partnum> [<filename>] [/lenient]\n");
    fprintf(output, "      Fix a boot sector to properly boot from a secondary drive. Before\n");
    fprintf(output, "      applying a fix, a CRC-32 checksum will be calculated against the current\n");
    fprintf(output, "      boot code in addition to checking the regions to be patched. Specify\n");
    fprintf(output, "      /lenient to skip the CRC check and only check the patched regions. A\n");
    fprintf(output, "      backup of the boot sector will be written to <filename> (or bootsect.bin\n");
    fprintf(output, "      if not specified).\n");
    fprintf(output, "   save <disknum> <partnum> [<filename>]\n");
    fprintf(output, "      Save a copy of the boot sector from disk disknum and partition partnum to\n");
    fprintf(output, "      <filename> (or bootsect.bin if not specified).\n");
    fprintf(output, "   restore <disknum> <partnum> [<filename>]\n");
    fprintf(output, "      Restore a boot sector from <filename> (or bootsect.bin if unspecified) to\n");
    fprintf(output, "      partition <partnum> on disk <disknum>.\n");

    return error;
}


int parse_cmdline(int argc, char **argv)
{
    int i;
    char *numend;

    if (argc == 1) {
        command = MODE_INFO;
        return ERR_SUCCESS;
    }

    if (stricmp(argv[1], "help") == 0)
        command = MODE_HELP;
    else if (stricmp(argv[1], "info") == 0)
        command = MODE_INFO;
    else if (stricmp(argv[1], "fix") == 0)
        command = MODE_FIX;
    else if (stricmp(argv[1], "save") == 0)
        command = MODE_SAVE;
    else if (stricmp(argv[1], "restore") == 0)
        command = MODE_RESTORE;
    else
        return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_INVALID_ARGS);

    i = 2;

    switch (command) {
    case MODE_INFO:
    case MODE_FIX:
    case MODE_SAVE:
    case MODE_RESTORE:
        if (i < argc) {
            disknum = strtoul(argv[i], &numend, 0);
            if ((disknum == 0) && (numend == argv[i]))
                return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_INVALID_DISK_NUM);
            i++;
        }
        if (i < argc) {
            partnum = strtoul(argv[i], &numend, 0);
            if ((partnum == 0) && (numend == argv[i]))
                return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_INVALID_PART_NUM);
            i++;
        }
        break;
    }

    switch (command) {
    case MODE_FIX:
    case MODE_SAVE:
    case MODE_RESTORE:
        if (disknum < 0)
            return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_INVALID_DISK_NUM);
        else if (partnum < 0)
            return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_INVALID_PART_NUM);
        break;
    }

    while (i < argc) {
        switch (command) {
        case MODE_FIX:
            if (i < argc) {
                if (argv[i][0] == '/') {
                    if (stricmp(argv[i], "/lenient") == 0)
                        lenient_fix = 1;
                    else
                        return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_INVALID_ARGS);
                }
                else
                    filename = argv[i];
            }
            break;
        case MODE_SAVE:
        case MODE_RESTORE:
            filename = argv[i];
            break;
        }
        i++;
    }

    return ERR_SUCCESS;
}

int showinfopart(int disk, int part)
{
    struct MBR mbr;
    int readres;
    int cyl, head, sect;
    unsigned long bscrc;

    readres = read_mbr(0x80 + disk, &mbr);
    if (FAILED(readres))
        return readres;
    else {
        printf("Fixed disk %i partition entry %i:\n", disk, part);
        printf("----------------------------------------------------------------------\n");
        printf("Status:\t\t\t%s\n",
            (mbr.entries[part].status & 0x80) ? "Active" : "Inactive");
        printf("Type:\t\t\t%s (0x%02x)\n",
            part_type_to_str(mbr.entries[part].type), mbr.entries[part].type);
        cyl = mbr.entries[part].sc_first.cylinder_high << 8;
        cyl |= mbr.entries[part].cylinder_low_first;
        head = mbr.entries[part].head_first;
        sect = mbr.entries[part].sc_first.sector;
        printf("First CHS:\t\t%u,%u,%u\n", cyl, head, sect);
        cyl = mbr.entries[part].sc_last.cylinder_high << 8;
        cyl |= mbr.entries[part].cylinder_low_last;
        head = mbr.entries[part].head_last;
        sect = mbr.entries[part].sc_last.sector;
        printf("Last CHS:\t\t%u,%u,%u\n", cyl, head, sect);
        printf("First LBA:\t\t%lu\n", mbr.entries[part].lba_first);
        printf("LBA length:\t\t%lu\n", mbr.entries[part].lba_length);
        readres = part_bootsect_crc32(0x80 + disk, part, &bscrc);
        if (SUCCEEDED(readres))
            printf("Boot code CRC-32:\t0x%08lx\n\n", bscrc);
        else
            printf("\n");
    }
    return ERR_SUCCESS;
}

int showinfodisk(int disk)
{
    struct MBR mbr;
    int readres;
    int i;

    readres = read_mbr(0x80 + disk, &mbr);
    if (FAILED(readres))
        return readres;
    else {
        printf("Fixed disk %i partition table:\n", disk);
        printf("======================================================================\n");
        printf("Original disk:\t\t\t%u\n", mbr.ts_sig.ts.orig_disk);
        printf("Timestamp (Win 95/98/ME):\t%02u:%02u:%02u\n",
               mbr.ts_sig.ts.hour, mbr.ts_sig.ts.minute,
               mbr.ts_sig.ts.second);
        printf("Signature:\t\t\t0x%08x\n", mbr.bc_sig.sig.sig);
        printf("Copy prot:\t\t\t0x%04x\n\n", mbr.bc_sig.sig.copyprot);
        for (i = 0; i < 4; i++) {
            readres = showinfopart(disk, i);
            if (FAILED(readres))
                return readres;
        }
    }
    return ERR_SUCCESS;
}

int showinfo(void)
{
    int i;
    unsigned char diskcount;
    int res;

    diskcount = *(unsigned char far *)0x00400075;
    printf("Number of fixed disks: %u\n\n", diskcount);

    for (i = 0; i < diskcount; i++) {
        res = showinfodisk(i);
        if (FAILED(res))
            return res;
    }
    return ERR_SUCCESS;
}

int save_bs(void)
{
    unsigned char bs[512];
    int outfile;
    int res;
    int bytes;

    outfile = open(filename, O_WRONLY | O_CREAT | O_BINARY);
    if (outfile < 0) {
        fprintf(stderr, "Couldn't open %s.", filename);
        return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_COULDNT_OPEN_FILE);
    }

    res = read_part_bootsect(0x80 + disknum, partnum, bs);
    if (FAILED(res)) {
        fprintf(stderr, "Couldn't read boot sector: %s (0x%04x)", errstr(res),
            res);
        return res;
    }

    bytes = write(outfile, bs, sizeof(bs));
    if (bytes != sizeof(bs)) {
        fprintf(stderr, "Couldn't write to %s.", filename);
        return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_COULDNT_WRITE_FILE);
    }

    close(outfile);
    return ERR_SUCCESS;
}

int restore_bs(void)
{
    unsigned char bs[512];
    int infile;
    int res;
    int bytes;

    infile = open(filename, O_RDONLY | O_BINARY);
    if (infile < 0) {
        fprintf(stderr, "Couldn't open %s.", filename);
        return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_COULDNT_OPEN_FILE);
    }

    bytes = read(infile, bs, sizeof(bs));
    if (bytes != sizeof(bs)) {
        fprintf(stderr, "Couldn't read from %s.", filename);
        return MAKE_ERROR(ERR_MAJOR_APP, ERR_APP_COULDNT_READ_FILE);
    }

    res = write_part_bootsect(0x80 + disknum, partnum, bs);
    if (FAILED(res)) {
        fprintf(stderr, "Couldn't write boot sector: %s (0x%04x)", errstr(res),
            res);
        return res;
    }

    close(infile);
    return ERR_SUCCESS;
}

int main(int argc, char **argv)
{
    int res;

    res = parse_cmdline(argc, argv);
    if (FAILED(res))
        return usage(1, errstr (res), argv);

    switch (command) {
    case MODE_HELP:
        return usage(0, NULL, argv);
    case MODE_INFO:
        if (partnum >= 0)
            res = showinfopart(disknum, partnum);
        else if (disknum >= 0)
            res = showinfodisk(disknum);
        else
            res = showinfo();
        if (FAILED(res)) {
            fprintf(stderr, "Error while getting disk info: %s (0x%04x)\n",
                errstr(res), res);
            return res;
        }
        return ERR_SUCCESS;
    case MODE_FIX:
        res = save_bs();    /* save boot sector copy before modification */
        if (FAILED(res))
            return res;
        res = fix_boot(0x80 + disknum, partnum, !lenient_fix);
        if (FAILED(res)) {
            fprintf(stderr, "Error while fixing partition boot sector: %s (0x%04x)\n",
                errstr(res), res);
            return res;
        }
        return ERR_SUCCESS;
    case MODE_SAVE:
        res = save_bs();
        if (FAILED(res))
            return res;
        return ERR_SUCCESS;
    case MODE_RESTORE:
        res = restore_bs();
        if (FAILED(res))
            return res;
        return ERR_SUCCESS;
    }

    return ERR_SUCCESS;
}