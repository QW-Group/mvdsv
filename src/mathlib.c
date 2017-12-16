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
 

*/
// mathlib.c -- math primitives

#ifdef SERVERONLY
#include "qwsvdef.h"
#else
#include "common.h"
#endif

vec3_t vec3_origin = {0,0,0};
float _mathlib_temp_float1, _mathlib_temp_float2, _mathlib_temp_float3;
int _mathlib_temp_int1, _mathlib_temp_int2, _mathlib_temp_int3;

/*-----------------------------------------------------------------*/

float anglemod(float a)
{
#if 0
	if (a >= 0)
		a -= 360*(int)(a/360);
	else
		a += 360*( 1 + (int)(-a/360) );
#endif
	a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}

#define VectorNormalizeFast(_v)		\
do {								\
	_mathlib_temp_float1 = DotProduct((_v), (_v));						\
	if (_mathlib_temp_float1) {											\
		_mathlib_temp_float2 = 0.5f * _mathlib_temp_float1;				\
		_mathlib_temp_int1 = *((int *) &_mathlib_temp_float1);			\
		_mathlib_temp_int1 = 0x5f375a86 - (_mathlib_temp_int1 >> 1);	\
		_mathlib_temp_float1 = *((float *) &_mathlib_temp_int1);		\
		_mathlib_temp_float1 = _mathlib_temp_float1 * (1.5f - _mathlib_temp_float2 * _mathlib_temp_float1 * _mathlib_temp_float1);	\
		VectorScale((_v), _mathlib_temp_float1, (_v))					\
	}																	\
} while (0);

void PerpendicularVector(vec3_t dst, const vec3_t src) {
	if (!src[0]) {
		VectorSet(dst, 1, 0, 0);
	}
	else if (!src[1]) {
		VectorSet(dst, 0, 1, 0);
	}
	else if (!src[2]) {
		VectorSet(dst, 0, 0, 1);
	}
	else {
		VectorSet(dst, -src[1], src[0], 0);
		VectorNormalizeFast(dst);
	}
}

void VectorVectors(vec3_t forward, vec3_t right, vec3_t up) {
	PerpendicularVector(right, forward);
	CrossProduct(right, forward, up);
}

void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees) {
	float t0, t1, angle, c, s;
	vec3_t vr, vu, vf;

	angle = DEG2RAD(degrees);
	c = cos(angle);
	s = sin(angle);
	VectorCopy(dir, vf);
	VectorVectors(vf, vr, vu);

	t0 = vr[0] *  c + vu[0] * -s;
	t1 = vr[0] *  s + vu[0] *  c;
	dst[0] = (t0 * vr[0] + t1 * vu[0] + vf[0] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[0] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[0] * vf[2]) * point[2];

	t0 = vr[1] *  c + vu[1] * -s;
	t1 = vr[1] *  s + vu[1] *  c;
	dst[1] = (t0 * vr[0] + t1 * vu[0] + vf[1] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[1] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[1] * vf[2]) * point[2];

	t0 = vr[2] *  c + vu[2] * -s;
	t1 = vr[2] *  s + vu[2] *  c;
	dst[2] = (t0 * vr[0] + t1 * vu[0] + vf[2] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[2] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[2] * vf[2]) * point[2];
}

/*
==================
BOPS_Error

Split out like this for ASM to call.
==================
*/
#ifdef __cplusplus
extern "C" {
#endif
void BOPS_Error (void)
{
	Sys_Error ("BoxOnPlaneSide:  Bad signbits");
}
#ifdef __cplusplus
}  /* extern "C" */
#endif
#ifndef id386

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
#ifdef __cplusplus
extern "C" {
#endif
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, mplane_t *p)
{
	float dist1, dist2;
	int sides;

#if 0	// this is done by the BOX_ON_PLANE_SIDE macro before calling this
	// function
	// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}
#endif

	// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		BOPS_Error ();
		break;
	}

#if 0
	int		i;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (plane->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist = DotProduct (plane->normal, corners[0]) - plane->dist;
	dist2 = DotProduct (plane->normal, corners[1]) - plane->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

#endif

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

#ifdef PARANOID
	if (sides == 0)
		Sys_Error ("BoxOnPlaneSide: sides==0");
#endif

	return sides;
}
#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* id386 */


void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;
	
	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}

	if (right || up)
	{
		angle = angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		if (right)
		{
			right[0] = (-1*sr*sp*cy+-1*cr*-sy);
			right[1] = (-1*sr*sp*sy+-1*cr*cy);
			right[2] = -1*sr*cp;
		}

		if (up)
		{
			up[0] = (cr*sp*cy+-sr*-sy);
			up[1] = (cr*sp*sy+-sr*cy);
			up[2] = cr*cp;
		}
	}
}

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}

void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

vec_t VectorLength (vec3_t v)
{
	return sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

float VectorNormalize (vec3_t v)
{
	float length, ilength;

	length = sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (length)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}
