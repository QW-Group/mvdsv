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

	$Id: model.h,v 1.10 2006/03/27 22:54:38 disconn3ct Exp $
*/

#ifndef __MODEL__
#define __MODEL__

#include "bspfile.h"

/*
==============================================================================
BRUSH MODELS
==============================================================================
*/

// in memory representation
typedef struct
{
	vec3_t		position;
} mvertex_t;

// plane_t structure
typedef struct mplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

typedef struct
{
	vec3_t	normal;
	float	dist;
} plane_t;

typedef struct
{
	qbool		allsolid;	// if true, plane is not valid
	qbool		startsolid;	// if true, the initial point was in a solid area
	qbool		inopen, inwater;
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	plane_t		plane;		// surface normal at impact
	union {				// entity the surface is on
		int		entnum;	// for pmove
		struct edict_s	*ent;	// for sv_world
	} e;
} trace_t;

typedef struct texture_s
{
	char		name[16];
	unsigned	width, height;
	int		anim_total;		// total tenths in sequence ( 0 = no)
	int		anim_min, anim_max;	// time for this frame min <=time< max
	struct texture_s *anim_next;		// in the animation sequence
	struct texture_s *alternate_anims;	// bmodels in frmae 1 use these
	unsigned	offsets[MIPLEVELS];	// four mip maps stored
} texture_t;


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40

typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct
{
	float		vecs[2][4];
	float		mipadjust;
	texture_t	*texture;
	int		flags;
} mtexinfo_t;

typedef struct msurface_s
{
	int		visframe;	// should be drawn when node is crossed

	int		dlightframe;
	int		dlightbits;

	mplane_t	*plane;
	int		flags;

	int		firstedge;	// look up in model->surfedges[], negative numbers
	int		numedges;	// are backwards edges

// surface generation data
	struct surfcache_s	*cachespots[MIPLEVELS];

	short		texturemins[2];
	short		extents[2];

	mtexinfo_t	*texinfo;

// lighting info
	byte		styles[MAXLIGHTMAPS];
	byte		*samples;		// [numstyles*surfsize]
} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int		contents;		// 0, to differentiate from leafs
	int		visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];

	unsigned short	firstsurface;
	unsigned short	numsurfaces;
} mnode_t;

typedef struct mleaf_s
{
// common with node
	int		contents;		// wil be a negative contents number
	int		visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
	struct efrag_s	*efrags;

	msurface_t	**firstmarksurface;
	int		nummarksurfaces;
	int		key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

typedef struct
{
	dclipnode_t	*clipnodes;
	mplane_t	*planes;
	int		firstclipnode;
	int		lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;

/*
==============================================================================
ALIAS MODELS
Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

typedef enum {ST_SYNC=0, ST_RAND } synctype_t;
typedef enum {mod_brush, mod_sprite, mod_alias} modtype_t;

// some models are special
typedef enum {MOD_NORMAL, MOD_PLAYER, MOD_EYES, MOD_FLAME, MOD_THUNDERBOLT} modhint_t;

typedef struct model_s
{
	char		name[MAX_QPATH];
	qbool		needload; // bmodels and sprites don't cache normally

	modhint_t	modhint;

	modtype_t	type;
	int		numframes;
	synctype_t	synctype;

	int		flags;

//
// volume occupied by the model graphics
//
	vec3_t		mins, maxs;
	float		radius;

//
// solid volume for clipping (sent from server)
//
	qbool		clipbox;
	vec3_t		clipmins, clipmaxs;

//
// brush model
//
	int		firstmodelsurface, nummodelsurfaces;

	int		numsubmodels;
	dmodel_t	*submodels;

	int		numplanes;
	mplane_t	*planes;

	int		numleafs; // number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int		numvertexes;
	mvertex_t	*vertexes;

	int		numedges;
	medge_t		*edges;

	int		numnodes;
	mnode_t		*nodes;

	int		numtexinfo;
	mtexinfo_t	*texinfo;

	int		numsurfaces;
	msurface_t	*surfaces;

	int		numsurfedges;
	int		*surfedges;

	int		numclipnodes;
	dclipnode_t	*clipnodes;

	int		nummarksurfaces;
	msurface_t	**marksurfaces;

	hull_t		hulls[MAX_MAP_HULLS];

	int		numtextures;
	texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;

	int		bspversion;
	unsigned	checksum; // for world models only
	unsigned	checksum2; // for world models only
	
	// additional model data
	cache_user_t	cache;
} qmodel_t;

typedef struct {

	int		numplanes;
	mplane_t	*planes;

	int		numleafs; // number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int		numnodes;
	mnode_t		*nodes;

	byte		*visdata;
} spec_worldmodel_t;

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
qmodel_t *Mod_ForName (char *name, qbool crash);
mleaf_t *Mod_PointInLeaf (float *p, qmodel_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, qmodel_t *model, qbool);

#endif	// __MODEL__
