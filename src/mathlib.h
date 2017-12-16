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
// mathlib.h
#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#define	PITCH	0 // up / down
#define	YAW	1 // left / right
#define	ROLL	2 // fall over

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI	3.14159265358979323846	// matches value in gcc v2 math.h
#endif

struct mplane_s;

extern vec3_t vec3_origin;
extern float _mathlib_temp_float1, _mathlib_temp_float2, _mathlib_temp_float3;
extern int _mathlib_temp_int1, _mathlib_temp_int2, _mathlib_temp_int3;

#define DEG2RAD(a) (((a) * M_PI) / 180.0F)

#define NANMASK (255<<23)
#define IS_NAN(x) (((*(int *)&x)&NANMASK)==NANMASK)

#ifndef Q_rint
#define Q_rint(x) ((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#endif

#define FloatInterpolate(a, bness, b, c) (c) = (a)*(1-(bness)) + (b)*(bness)

#define VectorInterpolate(a, bness, b, c) FloatInterpolate((a)[0], bness, (b)[0], (c)[0]),FloatInterpolate((a)[1], bness, (b)[1], (c)[1]),FloatInterpolate((a)[2], bness, (b)[2], (c)[2])

#define DotProduct(x,y)		(x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c)	(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])
#define VectorAdd(a,b,c)	(c[0]=a[0]+b[0],c[1]=a[1]+b[1],c[2]=a[2]+b[2])
#define VectorCopy(a,b)		(b[0]=a[0],b[1]=a[1],b[2]=a[2])
#define VectorClear(a)		(a[0]=a[1]=a[2]=0)
#define VectorNegate(a,b)	(b[0]=-a[0],b[1]=-a[1],b[2]=-a[2])
#define VectorSet(v, x, y, z)	(v[0]=(x),v[1]=(y),v[2]=(z))

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

vec_t VectorLength (vec3_t v);
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
float VectorNormalize (vec3_t v);		// returns vector length
#define VectorScale(in, _scale, out)		\
do {										\
	float scale = (_scale);					\
	(out)[0] = (in)[0] * (scale); (out)[1] = (in)[1] * (scale); (out)[2] = (in)[2] * (scale);	\
} while (0);


void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
#ifdef __cplusplus
extern "C" {
#endif
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
#ifdef __cplusplus
}  /* extern "C" */
#endif
float anglemod(float a);

#define PlaneDiff(point, plane) (																			\
	(((plane)->type < 3) ? (point)[(plane)->type] - (plane)->dist: DotProduct((point), (plane)->normal) - (plane)->dist) 	\
)

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

#define PlaneDiff(point, plane) (																			\
	(((plane)->type < 3) ? (point)[(plane)->type] - (plane)->dist: DotProduct((point), (plane)->normal) - (plane)->dist) 	\
)

void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
void VectorVectors(vec3_t forward, vec3_t right, vec3_t up);

#endif /* !__MATHLIB_H__ */
