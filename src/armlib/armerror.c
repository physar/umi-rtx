/**************************************************************************/
/*                                                              /\/\      */
/*                                                              \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser, Joris van Dam, Casper Dik          _  \/\/  _   */
/*         University of Amsterdam                          | |      | |  */
/*         Dept. of Computer Systems                        | | /\/\ | |  */
/*         Kruislaan 403, NL 1098 SJ Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*                                                          | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robot course       \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/*                                                              \/\/      */
/* This file is copyright protected.                                      */
/* Copyright (c) 1991-2003 Universiteit van Amsterdam                     */
/* This software or any part thereof may only be used for non-commercial  */
/* research or purposes, as long as the author and University are         */
/* mentioned. Commercial use without explicit prior written consent by    */
/* the Universiteit van Amsterdam is strictly prohibited. This copyright  */
/* notice must be included with any copy of this software or any part     */
/* thereof.                                                               */
/*                                                                        */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/**************************************************************************/
/*
 * $Id$
 *
 * Translation of armerrno to an error message.
 */

#include <rtx.h>
#include <stdio.h>

char*
armstrerror(err)
int err;
{
    static char mess[100];

    switch(err) {
 /* communications faults */
    case COMMS_FAULT:
    case COMMS_NOT_READY:
	return "Communication failure";
    case COMMS_NOT_INITIALISED:
	return "Communication not initialised";

 /* response errors */
    case NO_RESPONSE:
	return "No response";
    case RESPONSE_OVERRUN:
	return "Too many bytes in response";
    case RESPONSE_INCOMPLETE:
	return "Not enough bytes in response";
    case RESPONSE_UNKNOWN:
	return "Unexpected response";

 /* arm warnings */
    case ARM_IN_PROGRESS:
    case ARM_STORED:
	return "Reserved for future ROM releases";
    case ARM_AXIS_BUSY:
	return "Axis busy, wait until axis is stopped";
    case ARM_DECODER_BUSY:
	return "IP command decoder busy, try again later";
    case ARM_PARAMETER_OOR:
	return "Parameter out of range";

    case READ_ONLY:
    case ARM_READ_ONLY:
	return "Read only variable";
    case SELECTION_OOR:
    case ARM_SELECTION_OOR:
	return "Selection out of range";
    case ARM_COMMAND_OOR:
    case ARM_NOT_SUPPORTED:
	return "Illegal command";
    case ARM_FRAME_TIMEOUT:
	return "Not enough bytes in command";
    case ARM_FRAME_OVERRUN:
	return "Too many bytes in command";
    case ARM_PARITY:
	return "Parity error";
    case ARM_RESTARTED:
	return "The IP was restarted. restart";

    case CHECKSUM:
	return "Checksum error";
    case PARAMETER_OOR:
	return "Parameter out of range";
    case TOGGLE_MODE_OFF:
	return "Toggle mode off (must be on for arm_interpolate)";
    case PRIVILEGE_VIOLATION:
	return "Privileged command";
    case ARM_IN_USE:
	return "RTX in use";
    case COMMAND_OOR:
	return "Command not supported";
    case NO_CONNECTION:
	return "Cannot connect to daemon";
    case LOST_CONNECTION:
	return "Lost connection to RTX daemon";
    case CONNECTION_READ_ONLY:
	return "Connection read-only";
    default:
	sprintf(mess, "Unknown error code 0x%x", err);
	return mess;
    }
}

int
armperror(s)
char *s;
{
    if (s && *s)
	return fprintf(stderr,"%s: %s\n",s,armstrerror(armerrno));
    else
	return fprintf(stderr,"%s\n",armstrerror(armerrno));
}
