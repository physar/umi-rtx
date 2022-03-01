/*
 * $Id$
 *
 * Definition of commands and responses of RTX
 * (From: Using intelligent peripheral commuications.)
 */

#ifndef _rtxcmds_h
#define _rtxcmds_h

#define IPDONTCARE		-1
#define IP0			0
#define IP1			1

#define CMD_MASK		0xc0
#define CMD_SV			0x00
#define CMD_PA			0x40
#define CMD_DI			0x80
#define CMD_IP			0xc0

/* SV: Simple commands */
#define CMD_GO			0x00
#define CMD_IDENTIFY		0x01
#define CMD_INIT		0x08
#define CMD_START_SOAK		0x09
#define CMD_INIT_START_SOAK	0x0a
#define CMD_STOP_SOAK		0x0b

/* SV: Status */
#define CMD_STAT_BASE	0x10

#define CMD_STAT_CTL_0		0x10
#define CMD_STAT_CTL_1		0x11
#define CMD_STAT_CTL_2		0x12
#define CMD_STAT_CTL_3		0x13
#define CMD_STAT_CTL_4		0x14

#define GENERAL			0x07
#define CMD_STAT_GENERAL	0x17

/* Mode settings: parameter = controller number. */
#define CMD_MODE_SET	0x18

#define CMD_MODE_SET_POS	0x18
#define CMD_MODE_SET_FRC	0x19
#define CMD_MODE_SET_ABS	0x1a
#define CMD_MODE_SET_REL	0x1b
#define CMD_MODE_SET_UIOI	0x1c
#define CMD_MODE_SET_UIOO	0x1d

/* IP control */
#define CMD_IP_RESET		0x20
#define CMD_IP_DEF_HOME		0x21
#define CMD_IP_DEFAULT		0x22

#define CMD_XXXX_STOP		0x24
#define CMD_DEAD_STOP		0x24
#define CMD_RAMP_STOP		0x25
#define CMD_FREE_STOP		0x26
#define CMD_FREE_OFF		0x27

#define CMD_TOGGLE_OFF		0x28
#define CMD_TOGGLE_ONCE		0x29
#define CMD_TOGGLE_ON		0x2a

/* #define CMD_SET_BAUD	0x30  */

#define CMD_VERSION		0x38

/*
 * Parameter commands
 */

#define CMD_IM_READ_ERR		0x40	/* + ctl */
#define CMD_IM_READ_CP		0x48	/* + ctl */

#define CMD_IM_WRITE_CP		0x58	/* + ctl */

#define CMD_DEF_READ1		0x60	/* Param = #of ctl param */
#define CMD_DEF_READ2		0x68	/* + ctl */

#define CMD_DEF_WRITE1		0x70	/* Param = #of ctl param */
#define CMD_DEF_WRITE2		0x78	/* + ctl */ 
/*
 * Directive commands
 */

#define CMD_DIR_MAN		0x80

#define CMD_DIR_NUM		0xa0
/*
 * Interpolation commands.
 */

#define CMD_INTERP		0xe0

/*
 * SV responses (1 byte)
 */

#define RESP_TYPE_MASK		0xf0
#define RESP_TYPE_SV_SIMPLE	0x00
#define RESP_TYPE_SV_STAT	0x10
#define RESP_TYPE_SV_ID		0x20
#define RESP_TYPE_IM_READ	0x80
#define RESP_TYPE_IM_WRITE	0xa0
#define RESP_TYPE_DEF_R1	0xc0
#define RESP_TYPE_DEF_R2	0xd0
#define RESP_TYPE_DEF_W1	0xe0
#define RESP_TYPE_DEF_W2	0xf0

#define RESP_ACK		0x00
#define RESP_PROGRESS		0x01
#define RESP_STORED		0x02
#define RESP_AXIS_BUSY		0x03
#define RESP_IP_BUSY		0x04
#define RESP_PARAM_OOR		0x05
#define RESP_READ_ONLY		0x06
#define RESP_READ_ONLY1		0x07
#define RESP_SELECT_OOR		0x08
#define RESP_CMD_OOR		0x09
#define RESP_CMD_NOT_SUP	0x0a
#define RESP_FRAME_TIMEO	0x0b
#define RESP_FRAME_OVERRUN	0x0c
#define RESP_PARITY_ERROR	0x0d
#define RESP_IP_RESTART1	0x0e
#define RESP_IP_RESTART		0x0f

/*
 * Status (3 bytes)
 * Byte one = status
 * Byte 2 = IP
 */
#define RESP_STAT_BASE		0x10
#define RESP_STAT_CTL_0		0x10
#define RESP_STAT_CTL_1		0x11
#define RESP_STAT_CTL_2		0x12
#define RESP_STAT_CTL_3		0x13
#define RESP_STAT_CTL_4		0x14

#define RESP_STAT_GENERAL	0x17

#define RESP_MASK_TASKS		0x01
#define RESP_MASK_AXIS		0x02
#define RESP_MASK_ERR_LIMITS	0x04
#define RESP_MASK_AXIS_RESET	0x08
#define RESP_MASK_USER_IO	0x10
#define RESP_MASK_MOTOR_REL	0x10
#define RESP_MASK_USER_IO_CONF	0x20
#define RESP_MASK_MOTOR_FRC	0x20

#define RESP_IDENTIFY_MASK	0x07
#define RESP_IDENTIFY_0		0x20
#define RESP_IDENTIFY_1		0x21

#define RESP_RW_MASK		0xf0
#define RESP_IM_READ		0x80	/* + cksum */
#define RESP_IM_WRITE		0xa0	/* + cksum */

#define RESP_DEF_READ1		0xc0
#define RESP_DEF_READ2		0xd0	/* + cksum */
#define RESP_DEF_WRITE1		0xe0
#define RESP_DEF_WRITE2		0xf0	/* + cksum */

int rtx_raw_command(int ip, int cmdlen, int b1, int b2, int b3);
int rtx_raw_response(int ip, int *len, unsigned char *resp);
int rtx_version(int vers[]);
int rtx_define_origin();
int rtx_reload_pids();
int rtx_set_mode(int motor, int mode);
int rtx_stop(int mode);
int rtx_go(int mode, int bits);
int rtx_motor_status(int motor, int *status);
int rtx_general_status(int *status);
int rtx_read(int motor, int code, int *result);
int rtx_write(int motor, int code, int input);
int rtx_interpolate(int *data);
int rtx_soak (int s);

char *rtxstrcommand(int what);
#endif /* _rtxcmds_h */
