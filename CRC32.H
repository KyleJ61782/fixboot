/*
 *
 * Simple CRC-32 calculator obtained from
 * https://www.w3.org/TR/PNG/#D-CRCAppendix.
 *
 */

#ifndef __CRC32_H__
#define __CRC32_H__

unsigned long crc(unsigned char *buf, int len);

#endif /* __CRC32_H__ */