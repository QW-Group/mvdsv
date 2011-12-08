/*
	bothtools.s

	x86 assembly language math routines

	Copyright (C) 2006 VVD (vvd0@sorceforge.net).

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#include "asm_i386.h"
#include "quakeasm.h"


#ifdef id386

	.text

#define	val	4

.globl C(ShortSwap)
C(ShortSwap):

/*	movzwl	val(%esp),%eax*/
	movw	val(%esp),%ax
	xchgb	%al,%ah
	ret


.globl C(LongSwap)
C(LongSwap):

.globl C(FloatSwap)
C(FloatSwap):

	movl	val(%esp),%eax
	bswap	%eax
	ret


#endif	/* id386 */
