/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	$Id: bothdefs.h,v 1.7 2006/01/04 03:49:02 disconn3ct Exp $
*/

// defs common to client and server

#ifndef _BOTHDEFS
#define _BOTHDEFS

#define	MSG_BUF_SIZE	8192

#define	MINIMUM_MEMORY	0x550000


#define	MAX_QPATH		64		// max length of a quake game pathname
#define	MAX_OSPATH		128		// max length of a filesystem pathname

#define	MAX_MSGLEN		1450		// max length of a reliable message
#define	MAX_DATAGRAM		1450		// max length of unreliable message
#define	FILE_TRANSFER_BUF_SIZE	(MAX_MSGLEN - 100)
//
// per-level limits
//
#define	MAX_EDICTS		512		// FIXME: ouch! ouch! ouch! - trying to fix...
#define	MAX_LIGHTSTYLES		64
#define	MAX_MODELS		256		// these are sent over the net as bytes
#define	MAX_SOUNDS		256		// so they cannot be blindly increased

#define	MAX_STYLESTRING		64

//
// stats are integers communicated to the client by the server
//
#define	MAX_CL_STATS		32
#define	STAT_HEALTH		0
//define	STAT_FRAGS		1
#define	STAT_WEAPON		2
#define	STAT_AMMO		3
#define	STAT_ARMOR		4
//define	STAT_WEAPONFRAME	5
#define	STAT_SHELLS		6
#define	STAT_NAILS		7
#define	STAT_ROCKETS		8
#define	STAT_CELLS		9
#define	STAT_ACTIVEWEAPON	10
#define	STAT_TOTALSECRETS	11
#define	STAT_TOTALMONSTERS	12
#define	STAT_SECRETS		13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14		// bumped by svc_killedmonster
#define	STAT_ITEMS		15
#define	STAT_VIEWHEIGHT		16		// Z_EXT_VIEWHEIGHT extension
#define STAT_TIME		17		// Z_EXT_TIME extension


//
// item flags
//
#define	IT_SHOTGUN		1
#define	IT_SUPER_SHOTGUN	2
#define	IT_NAILGUN		4
#define	IT_SUPER_NAILGUN	8

#define	IT_GRENADE_LAUNCHER	16
#define	IT_ROCKET_LAUNCHER	32
#define	IT_LIGHTNING		64
#define	IT_SUPER_LIGHTNING	128

#define	IT_SHELLS		256
#define	IT_NAILS		512
#define	IT_ROCKETS		1024
#define	IT_CELLS		2048

#define	IT_AXE			4096

#define	IT_ARMOR1		8192
#define	IT_ARMOR2		16384
#define	IT_ARMOR3		32768

#define	IT_SUPERHEALTH		65536

#define	IT_KEY1			131072
#define	IT_KEY2			262144

#define	IT_INVISIBILITY		524288

#define	IT_INVULNERABILITY	1048576
#define	IT_SUIT			2097152
#define	IT_QUAD			4194304

#define	IT_SIGIL1		(1<<28)

#define	IT_SIGIL2		(1<<29)
#define	IT_SIGIL3		(1<<30)
#define	IT_SIGIL4		(1<<31)

//
// print flags
//
#define	PRINT_LOW		0		// pickup messages
#define	PRINT_MEDIUM		1		// death messages
#define	PRINT_HIGH		2		// critical messages
#define	PRINT_CHAT		3		// chat messages


#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#endif //_BOTHDEFS
