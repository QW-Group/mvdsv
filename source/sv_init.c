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

#include "qwsvdef.h"
#include "version.h"

server_static_t	svs;				// persistent server info
server_t		sv;					// local server
demo_t			demo;				// server demo struct
entity_state_t	cl_entities[MAX_CLIENTS][UPDATE_BACKUP+1][MAX_PACKET_ENTITIES]; // client entities

char	localmodels[MAX_MODELS][5];	// inline model names for precache

char localinfo[MAX_LOCALINFO_STRING+1]; // local game info

extern spec_worldmodel_t specworld;

/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (char *name)
{
	int		i;
	
	if (!name || !name[0])
		return 0;

	for (i=0 ; i<MAX_MODELS && sv.model_precache[i] ; i++)
		if (!strcmp(sv.model_precache[i], name))
			return i;
	if (i==MAX_MODELS || !sv.model_precache[i])
		SV_Error ("SV_ModelIndex: model %s not precached", name);
	return i;
}

/*
================
SV_FlushSignon

Moves to the next signon buffer if needed
================
*/
void SV_FlushSignon (void)
{
	if (sv.signon.cursize < sv.signon.maxsize - 512)
		return;

	if (sv.num_signon_buffers == MAX_SIGNON_BUFFERS-1)
		SV_Error ("sv.num_signon_buffers == MAX_SIGNON_BUFFERS-1");

	sv.signon_buffer_size[sv.num_signon_buffers-1] = sv.signon.cursize;
	sv.signon.data = sv.signon_buffers[sv.num_signon_buffers];
	sv.num_signon_buffers++;
	sv.signon.cursize = 0;
}

/*
================
SV_CreateBaseline

Entity baselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline (void)
{
	int			i;
	edict_t			*svent;
	int				entnum;	
		
	for (entnum = 0; entnum < sv.num_edicts ; entnum++)
	{
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		// create baselines for all player slots,
		// and any other edict that has a visible model
		if (entnum > MAX_CLIENTS && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = svent->v.frame;
		svent->baseline.skinnum = svent->v.skin;
		if (entnum > 0 && entnum <= MAX_CLIENTS)
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = SV_ModelIndex("progs/player.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndex(PR_GetString(svent->v.model));
		}

		//
		// flush the signon message out to a seperate buffer if
		// nearly full
		//
		SV_FlushSignon ();

		//
		// add to the message
		//
		MSG_WriteByte (&sv.signon,svc_spawnbaseline);		
		MSG_WriteShort (&sv.signon,entnum);

		MSG_WriteByte (&sv.signon, svent->baseline.modelindex);
		MSG_WriteByte (&sv.signon, svent->baseline.frame);
		MSG_WriteByte (&sv.signon, svent->baseline.colormap);
		MSG_WriteByte (&sv.signon, svent->baseline.skinnum);
		for (i=0 ; i<3 ; i++)
		{
			MSG_WriteCoord(&sv.signon, svent->baseline.origin[i]);
			MSG_WriteAngle(&sv.signon, svent->baseline.angles[i]);
		}
	}
}


/*
================
SV_SaveSpawnparms

Grabs the current state of the progs serverinfo flags 
and each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	if (!sv.state)
		return;		// no progs loaded yet

	// serverflags is the only game related thing maintained
	svs.serverflags = pr_global_struct->serverflags;

	for (i=0, host_client = svs.clients ; i<MAX_CLIENTS ; i++, host_client++)
	{
		if (host_client->state != cs_spawned)
			continue;

		// needs to reconnect
		host_client->state = cs_connected;

		// call the progs to get default spawn parms for the new client
		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram (pr_global_struct->SetChangeParms);
		for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
			host_client->spawn_parms[j] = (&pr_global_struct->parm1)[j];
		
	}
}

/*
================
SV_CalcPHS

Expands the PVS and calculates the PHS
(Potentially Hearable Set)
================
*/
void SV_CalcPHS (void)
{
	int		rowbytes, rowwords;
	int		i, j, k, l, index, num;
	int		bitbyte;
	unsigned	*dest, *src;
	byte	*scan;
	int		count, vcount;

	Con_Printf ("Building PHS...\n");

	num = sv.worldmodel->numleafs;
	rowwords = (num+31)>>5;
	rowbytes = rowwords*4;

	sv.pvs = Hunk_Alloc (rowbytes*num);
	scan = sv.pvs;
	vcount = 0;
	for (i=0 ; i<num ; i++, scan+=rowbytes)
	{
		memcpy (scan, Mod_LeafPVS(sv.worldmodel->leafs+i, sv.worldmodel, true),
			rowbytes);
		if (i == 0)
			continue;
		for (j=0 ; j<num ; j++)
		{
			if ( scan[j>>3] & (1<<(j&7)) )
			{
				vcount++;
			}
		}
	}


	sv.phs = Hunk_Alloc (rowbytes*num);
	count = 0;
	scan = sv.pvs;
	dest = (unsigned *)sv.phs;
	for (i=0 ; i<num ; i++, dest += rowwords, scan += rowbytes)
	{
		memcpy (dest, scan, rowbytes);
		for (j=0 ; j<rowbytes ; j++)
		{
			bitbyte = scan[j];
			if (!bitbyte)
				continue;
			for (k=0 ; k<8 ; k++)
			{
				if (! (bitbyte & (1<<k)) )
					continue;
				// or this pvs row into the phs
				// +1 because pvs is 1 based
				index = ((j<<3)+k+1);
				if (index >= num)
					continue;
				src = (unsigned *)sv.pvs + index*rowwords;
				for (l=0 ; l<rowwords ; l++)
					dest[l] |= src[l];
			}
		}

		if (i == 0)
			continue;
		for (j=0 ; j<num ; j++)
			if ( ((byte *)dest)[j>>3] & (1<<(j&7)) )
				count++;
	}

	Con_Printf ("Average leafs visible / hearable / total: %i / %i / %i\n"
		, vcount/num, count/num, num);
}

unsigned SV_CheckModel(char *mdl)
{
	byte	stackbuf[1024];		// avoid dirtying the cache heap
	byte *buf;
	unsigned short crc;

	buf = (byte *)COM_LoadStackFile (mdl, stackbuf, sizeof(stackbuf));
	if (!buf)
		SV_Error ("SV_CheckModel: could not load %s\n", mdl);
	crc = CRC_Block(buf, com_filesize);

	return crc;
}

/*
================
Load_SpecVisData

Loads visibility data of the map for spectators
================
*/

typedef struct
{
	char		qvis[4];
	int			version;	
	int			visleafs;
	lump_t		lumps[4];
} visheader_t;

#define VIS_LUMP_VISIBILITY 0
#define VIS_LUMP_LEAFS		1
#define VIS_LUMP_PLANES		2
#define VIS_LUMP_NODES		3

void VIS_LoadPlanes(lump_t *l, byte *base, char *allocname)
{
	int			i, j;
	mplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Printf("VIS_LoadPlanes: lol?\n");
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*2*sizeof(*out), allocname);	
	
	specworld.planes = out;
	specworld.numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}

void VIS_LoadVisibility (lump_t *l, byte *base, char *allocname)
{
	if (!l->filelen)
	{
		specworld.visdata = NULL;
		Sys_Printf("VIS_LoadVisibility: lol?\n");
		return;
	}

	specworld.visdata = Hunk_AllocName ( l->filelen, allocname);	
	memcpy (specworld.visdata, base + l->fileofs, l->filelen);
}

void VIS_LoadLeafs (lump_t *l, byte *base, char *allocname)
{
	struct vdleaf_s {
		int contents;
		int visofs;
	} *in;
	mleaf_t 	*out;
	int			i, count, p;

	in = (void *)(base + l->fileofs);
	count = l->filelen / sizeof(*in);
	if (l->filelen % sizeof(*in))
		Sys_Printf("VIS_LoadLeafs: lol?\n");
	out = Hunk_AllocName ( count*sizeof(*out), allocname);	

	specworld.leafs = out;
	specworld.numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		p = LittleLong(in->contents);
		out->contents = p;
		p = LittleLong(in->visofs);
		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = specworld.visdata + p;
	}	
}

void Mod_SetParent (mnode_t *node, mnode_t *parent);
void VIS_LoadNodes (lump_t *l, byte *base, char *allocname)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (void *)(base + l->fileofs);
	count = l->filelen / sizeof(*in);
	if (l->filelen % sizeof(*in))
		Sys_Printf("VIS_LoadNodes: lol?\n");
	out = Hunk_AllocName ( count*sizeof(*out), allocname);	

	specworld.nodes = out;
	specworld.numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}
	
		p = LittleLong(in->planenum);
		out->plane = specworld.planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
		
		for (j=0 ; j<2 ; j++)
		{
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = specworld.nodes + p;
			else
				out->children[j] = (mnode_t *)(specworld.leafs + (-1 - p));
		}
	}
	
	Mod_SetParent (specworld.nodes, NULL);	// sets nodes and leafs
}


void Load_SpecVisData(char *visname)
{
	byte *base;
	visheader_t *header;
	int i;
	char loadname[32];

	memset(&specworld, 0, sizeof(specworld));

	if ((!atoi(Info_ValueForKey (svs.info, "watervis")) && !atoi(Info_ValueForKey (svs.info, "spec_watervis"))) || (base = COM_LoadHunkFile (visname)) == NULL) {
		specworld.visdata = sv.worldmodel->visdata;
		specworld.nodes = sv.worldmodel->nodes;
		specworld.planes = sv.worldmodel->planes;
		specworld.leafs = sv.worldmodel->leafs;

		specworld.numnodes = sv.worldmodel->numnodes;
		specworld.numplanes = sv.worldmodel->numplanes;
		specworld.numleafs = sv.worldmodel->numleafs;
		return;
	}

	header = (visheader_t*)base;
	
	if (strncmp("QVIS", header->qvis, 4)) {
		Sys_Printf("Load_SpecVisData: %s: unknown file type\n", visname);
		return;
	}

	i = LittleLong (header->version);
	if (i != BSPVERSION) {
		Sys_Printf ("Load_SpecVisData: %s has wrong version number (%i should be %i)", visname, i, BSPVERSION);
		return;
	}

	for (i=0 ; i<sizeof(visheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

	COM_FileBase (visname, loadname);

	VIS_LoadPlanes(&header->lumps[VIS_LUMP_PLANES], base, loadname);
	VIS_LoadVisibility(&header->lumps[VIS_LUMP_VISIBILITY], base, loadname);
	VIS_LoadLeafs(&header->lumps[VIS_LUMP_LEAFS], base, loadname);
	VIS_LoadNodes(&header->lumps[VIS_LUMP_NODES], base, loadname);

	specworld.numleafs = header->visleafs;
}

/*
================
SV_SpawnServer

Change the server to a new map, taking all connected
clients along with it.

This is only called from the SV_Map_f() function.
================
*/
void D_FlushCaches ();
dfunction_t *ED_FindFunction (char *name);
void Sys_TimeOfDay(date_t *date);

void SV_SpawnServer (char *server)
{
	edict_t		*ent;
	int			i;
	extern cvar_t version;
	dfunction_t *f;

	Con_DPrintf ("SpawnServer: %s\n",server);
	
	SV_SaveSpawnparms ();
	SV_LoadAccounts();

	svs.spawncount++;		// any partially connected client will be
							// restarted

	sv.state = ss_dead;
	sv.paused = false;

	Mod_ClearAll ();
	Hunk_FreeToLowMark (host_hunklevel);

	if (coop.value)
		Cvar_Set (&deathmatch, "0");
	current_skill = (int)(skill.value + 0.5);
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 3)
		current_skill = 3;
	Cvar_Set (&skill, va("%d", (int)current_skill));


	// wipe the entire per-level structure
	memset (&sv, 0, sizeof(sv));

	sv.datagram.maxsize = sizeof(sv.datagram_buf);
	sv.datagram.data = sv.datagram_buf;
	sv.datagram.allowoverflow = true;

	sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
	sv.reliable_datagram.data = sv.reliable_datagram_buf;
	
	sv.multicast.maxsize = sizeof(sv.multicast_buf);
	sv.multicast.data = sv.multicast_buf;
	
	sv.master.maxsize = sizeof(sv.master_buf);
	sv.master.data = sv.master_buf;
	
	sv.signon.maxsize = sizeof(sv.signon_buffers[0]);
	sv.signon.data = sv.signon_buffers[0];
	sv.num_signon_buffers = 1;

	strcpy (sv.name, server);

	// load progs to get entity field count
	// which determines how big each edict is
	PR_LoadProgs ();

	// allocate edicts

	sv.edicts = Hunk_AllocName (MAX_EDICTS*pr_edict_size, "edicts");

	// leave slots at start for clients only
	sv.num_edicts = MAX_CLIENTS+1;
	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
//ZOID - make sure we update frags right
		svs.clients[i].old_frags = 0;
	}

	sv.time = sv.gametime = 1.0;
	
	strcpy (sv.name, server);
	sprintf (sv.modelname,"maps/%s.bsp", server);
	sv.worldmodel = Mod_ForName (sv.modelname, true);
	Load_SpecVisData(va("maps/%s.vis", server));
	SV_CalcPHS ();

	//
	// clear physics interaction links
	//

	SV_ClearWorld ();
	
	sv.sound_precache[0] = pr_strings;

	sv.model_precache[0] = pr_strings;
	sv.model_precache[1] = sv.modelname;
	sv.models[1] = sv.worldmodel;
	for (i=1 ; i<sv.worldmodel->numsubmodels ; i++)
	{
		sv.model_precache[1+i] = localmodels[i];
		sv.models[i+1] = Mod_ForName (localmodels[i], false);
	}

	//check player/eyes models for hacks
	sv.model_player_checksum = SV_CheckModel("progs/player.mdl");
	sv.eyes_player_checksum = SV_CheckModel("progs/eyes.mdl");

	//
	// spawn the rest of the entities on the map
	//	

	// precache and static commands can be issued during
	// map initialization
	sv.state = ss_loading;

	ent = EDICT_NUM(0);
	ent->free = false;
	ent->v.model = PR_SetString(sv.worldmodel->name);
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	// information about the server
	ent->v.netname = PR_SetString(version.string);
	ent->v.targetname = PR_SetString("mvdsv");
	ent->v.impulse = QWE_VERNUM;
	ent->v.items = pr_numbuiltins - 1;

	pr_global_struct->mapname = /*sv.name - pr_strings;*/PR_SetString(sv.name);
	// serverflags are for cross level information (sigils)
	pr_global_struct->serverflags = svs.serverflags;

	// run the frame start qc function to let progs check cvars
	SV_ProgStartFrame ();

	// load and spawn all other entities
	ED_LoadFromFile (sv.worldmodel->entities);

	// look up some model indexes for specialized message compression
	SV_FindModelNumbers ();

	// all spawning is completed, any further precache statements
	// or prog writes to the signon message are errors
	sv.state = ss_active;
	
	// run two frames to allow everything to settle
	sv_frametime = 0.1;
	SV_Physics ();
	SV_Physics ();

	// save movement vars
	SV_SetMoveVars();

	// create a baseline for more efficient communications
	SV_CreateBaseline ();
	sv.signon_buffer_size[sv.num_signon_buffers-1] = sv.signon.cursize;

	Info_SetValueForKey (svs.info, "map", sv.name, MAX_SERVERINFO_STRING);

	if ((f = ED_FindFunction ("timeofday")) != NULL) {
		date_t date;

		Sys_TimeOfDay(&date);

		G_FLOAT(OFS_PARM0) = (float)date.sec;
		G_FLOAT(OFS_PARM1) = (float)date.min;
		G_FLOAT(OFS_PARM2) = (float)date.hour;
		G_FLOAT(OFS_PARM3) = (float)date.day;
		G_FLOAT(OFS_PARM4) = (float)date.mon;
		G_FLOAT(OFS_PARM5) = (float)date.year;
		G_INT(OFS_PARM6) = PR_SetTmpString(date.str);

		pr_global_struct->time = sv.time;
		pr_global_struct->self = 0;

		PR_ExecuteProgram((func_t)(f - pr_functions));
	}

	Con_DPrintf ("Server spawned.\n");
}

