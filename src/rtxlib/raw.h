/*
 * $Id: raw.h,v 1.0 2022/03/09 11:44:22 arnoud Exp $
 *
 * The functions defined in raw.c and used in externally.
 */

#ifndef _raw_h
#define _raw_h

int rtx_init_comms(int tmode,int debug);
int rtx_raw(int ip, int cmdlen, unsigned char b1, unsigned char b2, unsigned char b3, int *resplen, unsigned char *resp);
int rtx_resync(int err);
int rtx_set_toggle_mode(int tmode);
int rtx_restart();

#endif /* _raw_h */
