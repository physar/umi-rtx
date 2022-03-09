/*
 * $Id$
 *
 * compute a checksum over nbytes starting at addr.
 */
#include "checksum.h"

int
checksum(addr,len)
char *addr;
unsigned long len;
{
    unsigned long i;
    unsigned int sum = 0;

    for (i = 0; i < len; i++,addr++)
	sum += *addr & 0xff;

    return sum % 0xffff;
}
