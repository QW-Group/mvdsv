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
// cl_parse.c  -- parse a message received from the server

#include "quakedef.h"
#include "pmove.h"
#include "sbar.h"
#include "sound.h"
#include "teamplay.h"
#include "version.h"

char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",			// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value
	
	"svc_serverdata",		// [long] version ...
	"svc_lightstyle",		// [byte] [string]
	"svc_updatename",		// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",			// [byte] impact [byte] blood [vec3] from
	
	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",
	
	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",

	"svc_cdtrack",
	"svc_sellscreen",

	"svc_smallkick",
	"svc_bigkick",

	"svc_updateping",
	"svc_updateentertime",

	"svc_updatestatlong",
	"svc_muzzleflash",
	"svc_updateuserinfo",
	"svc_download",
	"svc_playerinfo",
	"svc_nails",
	"svc_choke",
	"svc_modellist",
	"svc_soundlist",
	"svc_packetentities",
 	"svc_deltapacketentities",
	"svc_maxspeed",
	"svc_entgravity",

	"svc_setinfo",
	"svc_serverinfo",
	"svc_updatepl",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL"
};

int	oldparsecountmod;
int	parsecountmod;
double	parsecounttime;

int		cl_spikeindex, cl_playerindex, cl_flagindex;

int		cl_h_playerindex, cl_gib1index, cl_gib2index, cl_gib3index; // Tonik
int		cl_rocketindex, cl_grenadeindex; // Tonik

extern int stat_packets;
extern int	stat_headers;
extern int	stat_size;


//=============================================================================

int packet_latency[NET_TIMINGS];

int CL_CalcNet (void)
{
	int		a, i;
	frame_t	*frame;
	int lost;
	int packetcount;

	for (i=cls.netchan.outgoing_sequence-UPDATE_BACKUP+1
		; i <= cls.netchan.outgoing_sequence
		; i++)
	{
		frame = &cl.frames[i&UPDATE_MASK];
		if (frame->receivedtime == -1)
			packet_latency[i&NET_TIMINGSMASK] = 9999;	// dropped
		else if (frame->receivedtime == -2)
			packet_latency[i&NET_TIMINGSMASK] = 10000;	// choked
		else if (frame->receivedtime == -3)
			packet_latency[i&NET_TIMINGSMASK] = -1;	// choked by c2spps
		else if (frame->invalid)
			packet_latency[i&NET_TIMINGSMASK] = 9998;	// invalid delta
		else
			packet_latency[i&NET_TIMINGSMASK] = (frame->receivedtime - frame->senttime)*20;
	}

	lost = 0;
	packetcount = 0;
	for (a=0 ; a<NET_TIMINGS ; a++)
	{
		i = (cls.netchan.outgoing_sequence-a) & NET_TIMINGSMASK;
		if (packet_latency[i] == 9999)
			lost++;
		if (packet_latency[i] != -1)	// don't count packets choked by c2spps
			packetcount++;
	}
	if (packetcount == 0)
		return 100;
	else
		return lost * 100 / packetcount;
}

//=============================================================================

/*
===============
CL_CheckOrDownloadFile

Returns true if the file exists, otherwise it attempts
to start a download from the server.
===============
*/
qboolean	CL_CheckOrDownloadFile (char *filename)
{
	FILE	*f;

	if (strstr (filename, ".."))
	{
		Con_Printf ("Refusing to download a path with ..\n");
		return true;
	}

	COM_FOpenFile (filename, &f);
	if (f)
	{	// it exists, no need to download
		fclose (f);
		return true;
	}

	//ZOID - can't download when recording
	if (cls.demorecording) {
		Con_Printf("Unable to download %s in record mode.\n", cls.downloadname);
		return true;
	}
	//ZOID - can't download when playback
	if (cls.demoplayback)
		return true;

	strcpy (cls.downloadname, filename);
	Con_Printf ("Downloading %s...\n", cls.downloadname);

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension (cls.downloadname, cls.downloadtempname);
	strcat (cls.downloadtempname, ".tmp");

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, va("download %s", cls.downloadname));

	cls.downloadnumber++;

	return false;
}

/*
=================
Model_NextDownload
=================
*/
void Model_NextDownload (void)
{
	char	*s;
	int		i;
	extern	char gamedirfile[];

	if (cls.downloadnumber == 0)
	{
		Con_Printf ("Checking models...\n");
		cls.downloadnumber = 1;
	}

	cls.downloadtype = dl_model;
	for ( 
		; cl.model_name[cls.downloadnumber][0]
		; cls.downloadnumber++)
	{
		s = cl.model_name[cls.downloadnumber];
		if (s[0] == '*')
			continue;	// inline brush model
		if (!CL_CheckOrDownloadFile(s))
			return;		// started a download
	}

	for (i=1 ; i<MAX_MODELS ; i++)
	{
		if (!cl.model_name[i][0])
			break;

		cl.model_precache[i] = Mod_ForName (cl.model_name[i], false);

		if (!cl.model_precache[i])
		{
			Con_Printf ("\nThe required model file '%s' could not be found or downloaded.\n\n"
				, cl.model_name[i]);
			Con_Printf ("You may need to download or purchase a %s client "
				"pack in order to play on this server.\n\n", gamedirfile);
			CL_Disconnect ();
			return;
		}
	}

	// all done
	cl.worldmodel = cl.model_precache[1];	
	R_NewMap ();
	TP_NewMap ();
	Hunk_Check ();		// make sure nothing is hurt

	// done with modellist, request first of static signon messages
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
//	MSG_WriteString (&cls.netchan.message, va("prespawn %i 0 %i", cl.servercount, cl.worldmodel->checksum2));
	MSG_WriteString (&cls.netchan.message, va(prespawn_name, cl.servercount, cl.worldmodel->checksum2));
}

/*
=================
Sound_NextDownload
=================
*/
void Sound_NextDownload (void)
{
	char	*s;
	int		i;

	if (cls.downloadnumber == 0)
	{
		Con_Printf ("Checking sounds...\n");
		cls.downloadnumber = 1;
	}

	cls.downloadtype = dl_sound;
	for ( 
		; cl.sound_name[cls.downloadnumber][0]
		; cls.downloadnumber++)
	{
		s = cl.sound_name[cls.downloadnumber];
		if (!CL_CheckOrDownloadFile(va("sound/%s",s)))
			return;		// started a download
	}

	for (i=1 ; i<MAX_SOUNDS ; i++)
	{
		if (!cl.sound_name[i][0])
			break;
		cl.sound_precache[i] = S_PrecacheSound (cl.sound_name[i]);
	}

	// done with sounds, request models now
	memset (cl.model_precache, 0, sizeof(cl.model_precache));
	cl_playerindex = -1;
	cl_spikeindex = -1;
	cl_flagindex = -1;
// Tonik -->
	cl_h_playerindex = -1;
	cl_gib1index = cl_gib2index = cl_gib3index = -1;
	cl_rocketindex = cl_grenadeindex = -1;
// <-- Tonik
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
//	MSG_WriteString (&cls.netchan.message, va("modellist %i 0", cl.servercount));
	MSG_WriteString (&cls.netchan.message, va(modellist_name, cl.servercount, 0));
}


/*
======================
CL_RequestNextDownload
======================
*/
void CL_RequestNextDownload (void)
{
	switch (cls.downloadtype)
	{
	case dl_single:
		break;
	case dl_skin:
		Skin_NextDownload ();
		break;
	case dl_model:
		Model_NextDownload ();
		break;
	case dl_sound:
		Sound_NextDownload ();
		break;
	case dl_none:
	default:
		Con_DPrintf("Unknown download type.\n");
	}
}

/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
void CL_ParseDownload (void)
{
	int		size, percent;
	byte	name[1024];
	int		r;


	// read the data
	size = MSG_ReadShort ();
	percent = MSG_ReadByte ();

	if (cls.demoplayback) {
		if (size > 0)
			msg_readcount += size;
		return; // not in demo playback
	}

	if (size == -1)
	{
		Con_Printf ("File not found.\n");
		if (cls.download)
		{
			Con_Printf ("cls.download shouldn't have been set\n");
			fclose (cls.download);
			cls.download = NULL;
		}
		CL_RequestNextDownload ();
		return;
	}

	// open the file if not opened yet
	if (!cls.download)
	{
		if (strncmp(cls.downloadtempname,"skins/",6))
			sprintf (name, "%s/%s", com_gamedir, cls.downloadtempname);
		else
			sprintf (name, "qw/%s", cls.downloadtempname);

		COM_CreatePath (name);

		cls.download = fopen (name, "wb");
		if (!cls.download)
		{
			msg_readcount += size;
			Con_Printf ("Failed to open %s\n", cls.downloadtempname);
			CL_RequestNextDownload ();
			return;
		}
	}

	fwrite (net_message.data + msg_readcount, 1, size, cls.download);
	msg_readcount += size;

	if (percent != 100)
	{
// change display routines by zoid
		// request next block
#if 0
		Con_Printf (".");
		if (10*(percent/10) != cls.downloadpercent)
		{
			cls.downloadpercent = 10*(percent/10);
			Con_Printf ("%i%%", cls.downloadpercent);
		}
#endif
		cls.downloadpercent = percent;

		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, "nextdl");
	}
	else
	{
		char	oldn[MAX_OSPATH];
		char	newn[MAX_OSPATH];

#if 0
		Con_Printf ("100%%\n");
#endif

		fclose (cls.download);

		// rename the temp file to it's final name
		if (strcmp(cls.downloadtempname, cls.downloadname)) {
			if (strncmp(cls.downloadtempname,"skins/",6)) {
				sprintf (oldn, "%s/%s", com_gamedir, cls.downloadtempname);
				sprintf (newn, "%s/%s", com_gamedir, cls.downloadname);
			} else {
				sprintf (oldn, "qw/%s", cls.downloadtempname);
				sprintf (newn, "qw/%s", cls.downloadname);
			}
			r = rename (oldn, newn);
			if (r)
				Con_Printf ("failed to rename.\n");
		}

		cls.download = NULL;
		cls.downloadpercent = 0;

		// get another file if needed

		CL_RequestNextDownload ();
	}
}

static byte *upload_data;
static int upload_pos;
static int upload_size;

void CL_NextUpload(void)
{
	byte	buffer[1024];
	int		r;
	int		percent;
	int		size;

	if (!upload_data)
		return;

	r = upload_size - upload_pos;
	if (r > 768)
		r = 768;
	memcpy(buffer, upload_data + upload_pos, r);
	MSG_WriteByte (&cls.netchan.message, clc_upload);
	MSG_WriteShort (&cls.netchan.message, r);

	upload_pos += r;
	size = upload_size;
	if (!size)
		size = 1;
	percent = upload_pos*100/size;
	MSG_WriteByte (&cls.netchan.message, percent);
	SZ_Write (&cls.netchan.message, buffer, r);

Con_DPrintf ("UPLOAD: %6d: %d written\n", upload_pos - r, r);

	if (upload_pos != upload_size)
		return;

	Con_Printf ("Upload completed\n");

	free(upload_data);
	upload_data = 0;
	upload_pos = upload_size = 0;
}

void CL_StartUpload (byte *data, int size)
{
	if (cls.state < ca_onserver)
		return; // gotta be connected

	// override
	if (upload_data)
		free(upload_data);

Con_DPrintf("Upload starting of %d...\n", size);

	upload_data = Q_Malloc (size);
	memcpy(upload_data, data, size);
	upload_size = size;
	upload_pos = 0;

	CL_NextUpload();
} 

qboolean CL_IsUploading(void)
{
	if (upload_data)
		return true;
	return false;
}

void CL_StopUpload(void)
{
	if (upload_data)
		free(upload_data);
	upload_data = NULL;
}

/*
=====================================================================

  SERVER CONNECTING MESSAGES

=====================================================================
*/

/*
==================
CL_ParseServerData
==================
*/
void CL_ParseServerData (void)
{
	char	*str;
	FILE	*f;
	char	fn[MAX_OSPATH];
	qboolean	cflag = false;
	extern	char	gamedirfile[MAX_OSPATH];
	int protover;
	extern cshift_t	cshift_empty;
	extern float nextdemotime, olddemotime;
	
	Con_DPrintf ("Serverdata packet received.\n");
//
// wipe the client_state_t struct
//
	CL_ClearState ();
	memset (&cshift_empty, 0, sizeof(cshift_empty));	// Tonik

// parse protocol version number
// allow 2.2 and 2.29 demos to play
	protover = MSG_ReadLong ();
	if (protover != PROTOCOL_VERSION && 
		!(cls.demoplayback && (protover == 26 || protover == 27 || protover == 28)))
		Host_EndGame ("Server returned version %i, not %i\nYou probably need to upgrade.\nCheck http://www.quakeworld.net/", protover, PROTOCOL_VERSION);

	cl.servercount = MSG_ReadLong ();

	// game directory
	str = MSG_ReadString ();

	cl.teamfortress = !Q_strcasecmp(str, "fortress");
	if (cl.teamfortress) {
		extern cvar_t	v_iyaw_cycle, v_iroll_cycle, v_ipitch_cycle,
			v_iyaw_level, v_iroll_level, v_ipitch_level, v_idlescale;
		cbuf_current = &cbuf_svc;	// hack
		Cvar_SetValue (&v_iyaw_cycle, 2);
		Cvar_SetValue (&v_iroll_cycle, 0.5);
		Cvar_SetValue (&v_ipitch_cycle, 1);
		Cvar_SetValue (&v_iyaw_level, 0.3);
		Cvar_SetValue (&v_iroll_level, 0.1);
		Cvar_SetValue (&v_ipitch_level, 0.3);
		Cvar_SetValue (&v_idlescale, 0);
	}

	if (stricmp(gamedirfile, str)) {
		// save current config
		Host_WriteConfiguration (); 
		cflag = true;
	}

	COM_Gamedir(str);

	//ZOID--run the autoexec.cfg in the gamedir
	//if it exists
	if (cflag) {
		int cl_warncmd_val = cl_warncmd.value;
		// FIXME: _snprintf is Win32 only
		_snprintf(fn, sizeof(fn), "%s/%s", com_gamedir, "config.cfg");
		if ((f = fopen(fn, "r")) != NULL) {
			fclose(f);
			Cbuf_AddText ("cl_warncmd 0\n");
			Cbuf_AddText ("exec config.cfg\n");
		}
		_snprintf(fn, sizeof(fn), "%s/%s", com_gamedir, "frontend.cfg");
		if ((f = fopen(fn, "r")) != NULL) {
			fclose(f);
			Cbuf_AddText ("cl_warncmd 0\n");
			Cbuf_AddText ("exec frontend.cfg\n");
		}
		_snprintf(fn,sizeof(fn), "cl_warncmd %d\n", cl_warncmd_val);
		Cbuf_AddText(fn);
	}

	if (cls.demoplayback2) {
		cls.netchan.last_received = nextdemotime = olddemotime = MSG_ReadFloat();
		cl.playernum = 31;
		cl.spectator = true;
	} else {
		// parse player slot, high bit means spectator
		cl.playernum = MSG_ReadByte ();
		if (cl.playernum & 128)
		{
			cl.spectator = true;
			cl.playernum &= ~128;
			//Con_Printf("pn:%d\n", cl.playernum);
		}
	}

	// get the full level name
	str = MSG_ReadString ();
	Q_strncpyz (cl.levelname, str, sizeof(cl.levelname));

	// get the movevars
	movevars.gravity			= MSG_ReadFloat();
	movevars.stopspeed          = MSG_ReadFloat();
	movevars.maxspeed           = MSG_ReadFloat();
	movevars.spectatormaxspeed  = MSG_ReadFloat();
	movevars.accelerate         = MSG_ReadFloat();
	movevars.airaccelerate      = MSG_ReadFloat();
	movevars.wateraccelerate    = MSG_ReadFloat();
	movevars.friction           = MSG_ReadFloat();
	movevars.waterfriction      = MSG_ReadFloat();
	movevars.entgravity         = MSG_ReadFloat();

	// seperate the printfs so the server message can have a color
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_Printf ("%c%s\n", 2, str);

	// ask for the sound list next
	memset(cl.sound_name, 0, sizeof(cl.sound_name));
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
//	MSG_WriteString (&cls.netchan.message, va("soundlist %i 0", cl.servercount));
	MSG_WriteString (&cls.netchan.message, va(soundlist_name, cl.servercount, 0));

	// now waiting for downloads, etc
	cls.state = ca_onserver;
}

/*
==================
CL_ParseSoundlist
==================
*/
void CL_ParseSoundlist (void)
{
	int	numsounds;
	char	*str;
	int n;

// precache sounds
//	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));

	numsounds = MSG_ReadByte();

	for (;;) {
		str = MSG_ReadString ();
		if (!str[0])
			break;
		numsounds++;
		if (numsounds == MAX_SOUNDS)
			Host_EndGame ("Server sent too many sound_precache");
		strcpy (cl.sound_name[numsounds], str);
	}

	n = MSG_ReadByte();

	if (n) {
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
//		MSG_WriteString (&cls.netchan.message, va("soundlist %i %i", cl.servercount, n));
		MSG_WriteString (&cls.netchan.message, va(soundlist_name, cl.servercount, n));
		return;
	}

	cls.downloadnumber = 0;
	cls.downloadtype = dl_sound;
	Sound_NextDownload ();
}

/*
==================
CL_ParseModellist
==================
*/
void CL_ParseModellist (void)
{
	int	nummodels;
	char	*str;
	int n;

// precache models and note certain default indexes
	nummodels = MSG_ReadByte();

	for (;;)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		nummodels++;
		if (nummodels==MAX_MODELS)
			Host_EndGame ("Server sent too many model_precache");
		strcpy (cl.model_name[nummodels], str);

		if (!strcmp(cl.model_name[nummodels],"progs/spike.mdl"))
			cl_spikeindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/player.mdl"))
			cl_playerindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/flag.mdl"))
			cl_flagindex = nummodels;

// Tonik -->
		else if (!strcmp(cl.model_name[nummodels],"progs/h_player.mdl"))
			cl_h_playerindex = nummodels;
		else if (!strcmp(cl.model_name[nummodels],"progs/gib1.mdl"))
			cl_gib1index = nummodels;
		else if (!strcmp(cl.model_name[nummodels],"progs/gib2.mdl"))
			cl_gib2index = nummodels;
		else if (!strcmp(cl.model_name[nummodels],"progs/gib3.mdl"))
			cl_gib3index = nummodels;
		else if (!strcmp(cl.model_name[nummodels],"progs/missile.mdl"))
			cl_rocketindex = nummodels;
		else if (!strcmp(cl.model_name[nummodels],"progs/grenade.mdl"))
			cl_grenadeindex = nummodels;
// <-- Tonik
	}

	n = MSG_ReadByte();

	if (n) {
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
//		MSG_WriteString (&cls.netchan.message, va("modellist %i %i", cl.servercount, n));
		MSG_WriteString (&cls.netchan.message, va(modellist_name, cl.servercount, n));
		return;
	}

	cls.downloadnumber = 0;
	cls.downloadtype = dl_model;
	Model_NextDownload ();
}

/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (entity_state_t *es)
{
	int			i;
	
	es->modelindex = MSG_ReadByte ();
	es->frame = MSG_ReadByte ();
	es->colormap = MSG_ReadByte();
	es->skinnum = MSG_ReadByte();
	for (i=0 ; i<3 ; i++)
	{
		es->origin[i] = MSG_ReadCoord ();
		es->angles[i] = MSG_ReadAngle ();
	}
}



/*
=====================
CL_ParseStatic

Static entities are non-interactive world objects
like torches
=====================
*/
void CL_ParseStatic (void)
{
	entity_t *ent;
	int		i;
	entity_state_t	es;

	CL_ParseBaseline (&es);
		
	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_EndGame ("Too many static entities");
	ent = &cl_static_entities[i];
	cl.num_statics++;

// copy it to the current state
	ent->model = cl.model_precache[es.modelindex];
	ent->frame = es.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = es.skinnum;

	VectorCopy (es.origin, ent->origin);
	VectorCopy (es.angles, ent->angles);
	
	R_AddEfrags (ent);
}

/*
===================
CL_ParseStaticSound
===================
*/
void CL_ParseStaticSound (void)
{
	extern cvar_t	cl_staticsounds;
	vec3_t		org;
	int			sound_num, vol, atten;
	int			i;
	
	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	sound_num = MSG_ReadByte ();
	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();

	if (cl_staticsounds.value)
		S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}



/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
==================
CL_ParseStartSoundPacket
==================
*/
void CL_ParseStartSoundPacket(void)
{
    vec3_t  pos;
    int 	channel, ent, tracknum;
    int 	sound_num;
    int 	volume;
    float 	attenuation;  
 	int		i;
	           
    channel = MSG_ReadShort(); 

    if (channel & SND_VOLUME)
		volume = MSG_ReadByte ();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
    if (channel & SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;
	
	sound_num = MSG_ReadByte ();

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();
 
	ent = (channel>>3)&1023;
	channel &= 7;

	if (ent > MAX_EDICTS)
		Host_EndGame ("CL_ParseStartSoundPacket: ent = %i", ent);

	tracknum = Cam_TrackNum();
	if (cl.spectator && tracknum != -1 && ent == tracknum + 1)
		ent = cl.playernum + 1;

    S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);

	if (ent == cl.playernum+1) 
		TP_CheckPickupSound (cl.sound_name[sound_num]);
}       


/*
==================
CL_ParseClientdata

Server information pertaining to this client only, sent every frame
==================
*/
void CL_ParseClientdata (void)
{
	int				i;
	float		latency;
	frame_t		*frame;

// calculate simulated time of message
	oldparsecountmod = parsecountmod;

	i = cls.netchan.incoming_acknowledged;
	if (cls.demoplayback2)
		cl.oldparsecount = i - 1;
	//else
	//	cl.oldparsecount = cl.parsecount;

	cl.parsecount = i;
	i &= UPDATE_MASK;
	parsecountmod = i;
	frame = &cl.frames[i];
	if (cls.demoplayback2)
		frame->senttime = realtime - host_frametime;//realtime;
	parsecounttime = cl.frames[i].senttime;

	frame->receivedtime = realtime;

// calculate latency
	latency = frame->receivedtime - frame->senttime;

	if (latency < 0 || latency > 1.0)
	{
//		Con_Printf ("Odd latency: %5.2f\n", latency);
	}
	else
	{
	// drift the average latency towards the observed latency
		if (latency < cls.latency)
			cls.latency = latency;
		else
			cls.latency += 0.001;	// drift up, so correction are needed
	}	
}

/*
=====================
CL_NewTranslation
=====================
*/
#ifdef GLQUAKE
void CL_NewTranslation (int slot)
{
	int		teamplay, tracknum;
	//char	s[512];
	player_info_t	*player;
	static char *team = "";

	if (slot > MAX_CLIENTS)
		Sys_Error ("CL_NewTranslation: slot > MAX_CLIENTS");

	player = &cl.players[slot];
	if (player->spectator)
		return;

// teamcolor/enemycolor -->
	player->topcolor = player->real_topcolor;
	player->bottomcolor = player->real_bottomcolor;

	if (cl.spectator)
	{
		tracknum = Cam_TrackNum();
		if (tracknum != -1)
		//	team = NULL;
		//else 
			team = cl.players[tracknum].team;
	} else
		team = cl.players[cl.playernum].team;

	//strcpy (s, cl.players[cl.playernum].team);
	teamplay = atoi(Info_ValueForKey(cl.serverinfo, "teamplay"));

	if ( !cl.teamfortress && !(cl.fpd & FPD_NO_FORCE_COLOR) ) {
		if (cl_teamtopcolor >= 0 && teamplay && team != NULL &&
			!strcmp(player->team, team))
		{
			player->topcolor = cl_teamtopcolor;
			player->bottomcolor = cl_teambottomcolor;
		}
		
		if (cl_enemytopcolor >= 0 && slot != cl.playernum &&
			(!teamplay || team == NULL || strcmp(player->team, team)))
		{
			player->topcolor = cl_enemytopcolor;
			player->bottomcolor = cl_enemybottomcolor;
		}
	}
// <--

	R_TranslatePlayerSkin(slot);
}
#else
void CL_NewTranslation (int slot)
{
	int		teamplay, tracknum;
	int		i, j;
	int		top, bottom;
	byte	*dest, *source;
	player_info_t	*player;
	char	s[512];
	static char *team = "";

	if (slot > MAX_CLIENTS)
		Sys_Error ("CL_NewTranslation: slot > MAX_CLIENTS");

	player = &cl.players[slot];
	if (player->spectator)
		return;

	strcpy(s, Info_ValueForKey(player->userinfo, "skin"));
	COM_StripExtension(s, s);
	if (player->skin && !stricmp(s, player->skin->name))
		player->skin = NULL;

// teamcolor/enemycolor -->
	player->topcolor = player->real_topcolor;
	player->bottomcolor = player->real_bottomcolor;

	if (cl.spectator)
	{
		tracknum = Cam_TrackNum();
		if (tracknum != -1)
		//	team = NULL;
		//else 
			team = cl.players[tracknum].team;
	} else {
		team = cl.players[cl.playernum].team;
	}

	teamplay = atoi(Info_ValueForKey(cl.serverinfo, "teamplay"));

	if ( !cl.teamfortress && !(cl.fpd & FPD_NO_FORCE_COLOR) ) {
		if (cl_teamtopcolor >= 0 && teamplay && team != NULL &&
			!strcmp(player->team, team))
		{
			player->topcolor = cl_teamtopcolor;
			player->bottomcolor = cl_teambottomcolor;
		}
		
		if (cl_enemytopcolor >= 0 && slot != cl.playernum &&
			(!teamplay || team == NULL || strcmp(player->team, team)))
		{
			player->topcolor = cl_enemytopcolor;
			player->bottomcolor = cl_enemybottomcolor;
		}
	}
// <--

	if (player->_topcolor != player->topcolor ||
		player->_bottomcolor != player->bottomcolor || !player->skin)
	{
		player->_topcolor = player->topcolor;
		player->_bottomcolor = player->bottomcolor;

		dest = player->translations;
		source = vid.colormap;
		memcpy (dest, vid.colormap, sizeof(player->translations));
		top = player->topcolor;
		if (top > 13 || top < 0)
			top = 13;
		top *= 16;
		bottom = player->bottomcolor;
		if (bottom > 13 || bottom < 0)
			bottom = 13;
		bottom *= 16;

		for (i=0 ; i<VID_GRADES ; i++, dest += 256, source+=256)
		{
			if (top < 128)	// the artists made some backwards ranges.  sigh.
				memcpy (dest + TOP_RANGE, source + top, 16);
			else
				for (j=0 ; j<16 ; j++)
					dest[TOP_RANGE+j] = source[top+15-j];
					
			if (bottom < 128)
				memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
			else
				for (j=0 ; j<16 ; j++)
					dest[BOTTOM_RANGE+j] = source[bottom+15-j];		
		}
	}
}
#endif	// !GLQUAKE

/*
==============
CL_ProcessUserInfo
==============
*/
void CL_ProcessUserInfo (int slot, player_info_t *player)
{
	Q_strncpyz (player->name, Info_ValueForKey (player->userinfo, "name"), sizeof(player->name));
	player->real_topcolor = atoi(Info_ValueForKey (player->userinfo, "topcolor"));
	player->real_bottomcolor = atoi(Info_ValueForKey (player->userinfo, "bottomcolor"));
	strcpy (player->team, Info_ValueForKey (player->userinfo, "team"));

	if (Info_ValueForKey (player->userinfo, "*spectator")[0])
		player->spectator = true;
	else
		player->spectator = false;

	if (cls.state == ca_active)
		Skin_Find (player);

	Sbar_Changed ();
	if (slot == cl.playernum && (cl_teamtopcolor >= 0 || cl_enemytopcolor >= 0) &&
		strcmp(player->team, player->_team))
	{
		int i;
		//strcpy (cl_playerteam, Info_ValueForKey(player->userinfo, "team"));
		for (i=0 ; i < MAX_CLIENTS ; i++)
			CL_NewTranslation (i);
	}
	else
		CL_NewTranslation (slot);

	strcpy (player->_team, player->team);
}

/*
==============
CL_UpdateUserinfo
==============
*/
void CL_UpdateUserinfo (void)
{
	int		slot;
	player_info_t	*player;

	slot = MSG_ReadByte ();
	if (slot >= MAX_CLIENTS)
		Host_EndGame ("CL_ParseServerMessage: svc_updateuserinfo > MAX_CLIENTS");

	player = &cl.players[slot];
	player->userid = MSG_ReadLong ();
	Q_strncpyz (player->userinfo, MSG_ReadString(), sizeof(player->userinfo));

	CL_ProcessUserInfo (slot, player);
}

/*
==============
CL_SetInfo
==============
*/
void CL_SetInfo (void)
{
	int		slot;
	player_info_t	*player;
	char key[MAX_INFO_STRING];
	char value[MAX_INFO_STRING];

	slot = MSG_ReadByte ();
	if (slot >= MAX_CLIENTS)
		Host_EndGame ("CL_ParseServerMessage: svc_setinfo > MAX_CLIENTS");

	player = &cl.players[slot];

	Q_strncpyz (key, MSG_ReadString(), sizeof(key));
	Q_strncpyz (value, MSG_ReadString(), sizeof(value));

	Con_DPrintf("SETINFO %s: %s=%s\n", player->name, key, value);

	Info_SetValueForKey (player->userinfo, key, value, MAX_INFO_STRING);

	CL_ProcessUserInfo (slot, player);
}


/*
==============
CL_ProcessServerInfo

Called by CL_FullServerinfo_f and CL_ParseServerInfoChange
==============
*/
void CL_ProcessServerInfo (void)
{
	char	*p;
	static int old_teamplay = 0;
	static int old_fpd = 0;
	int teamplay;	// FIXME: make it cl.teamplay?
	int i;

	cl.fpd = Q_atof(Info_ValueForKey(cl.serverinfo, "fpd"));
	teamplay = Q_atof(Info_ValueForKey(cl.serverinfo, "teamplay"));
	
	p = Info_ValueForKey(cl.serverinfo, "deathmatch");
	if (*p)
		cl.gametype = Q_atof(p) ? GAME_DEATHMATCH : GAME_COOP;
	else
		cl.gametype = GAME_DEATHMATCH;	// assume GAME_DEATHMATCH by default

	if (teamplay != old_teamplay || cl.fpd != old_fpd) {
		old_teamplay = teamplay;
		old_fpd = cl.fpd;
		if (cls.state >= ca_connected) {
			for (i = 0; i < MAX_CLIENTS ; i++)
				CL_NewTranslation (i);
		}
	}
}

/*
==============
CL_ParseServerInfoChange
==============
*/
void CL_ParseServerInfoChange (void)
{
	char key[MAX_INFO_STRING];
	char value[MAX_INFO_STRING];

	Q_strncpyz (key, MSG_ReadString(), sizeof(key));
	Q_strncpyz (value, MSG_ReadString(), sizeof(value));

	Con_DPrintf("SERVERINFO: %s=%s\n", key, value);

	Info_SetValueForKey (cl.serverinfo, key, value, MAX_SERVERINFO_STRING);

	CL_ProcessServerInfo ();
}


/*
==============
CL_ParsePrint
==============
*/

void CL_ParsePrint (void)
{
	char	*s, str[1024];
	char	*p;
	int		len;
	int		level;

	level = MSG_ReadByte ();
	s = MSG_ReadString ();

	//stat_size += 3 + sizeof(s);
	//Con_Printf("msg_size:%d\n", stat_size);

	strncat (cl.sprint_buf, s, sizeof(cl.sprint_buf)-1);

	while ( (p=strchr(cl.sprint_buf, '\n')) != NULL ) {
		len = p - cl.sprint_buf + 1;
		memcpy(str, cl.sprint_buf, len);
		str[len] = '\0';
		strcpy (cl.sprint_buf, p+1);
		
		if (level == PRINT_CHAT)
		{
			char *p;
			int flags;

			flags = TP_CategorizeMessage (str);
			TP_CheckVersionRequest (str);
			if (cl_nofake.value == 1 || (cl_nofake.value == 2 && flags != 2)) {
				for (p = str; *p; p++)
					if (*p == 13)
						*p = '#'; 
			}
			con_ormask = 128;
			S_LocalSound ("misc/talk.wav");
		}
		Con_Printf ("%s", str);
		con_ormask = 0;
		TP_SearchForMsgTriggers (str, level);
	}

}


/*
==============
CL_ParseStufftext
==============
*/
void CL_ParseStufftext (void)
{
	char	*s;

	s = MSG_ReadString ();

	Con_DPrintf ("stufftext: %s\n", s);
	//if (!strncmp(s, "fullserverinfo", 10))
	//	return;

	Cbuf_AddTextEx (&cbuf_svc, s);

	// QW servers send this without the ending \n
	if ( !strcmp (s, "cmd snap") ||
		// QuakeForge servers up to Beta 6 send this without the \n
		(!strncmp (s, "r_skyname ", 10) && !strchr (s, '\n')) )
	{
		Cbuf_AddTextEx (&cbuf_svc, "\n");
	}
}


/*
=====================
CL_SetStat
=====================
*/

void CL_SetStat (int stat, int value)
{
	int	j;
	if (stat < 0 || stat >= MAX_CL_STATS)
		Host_EndGame ("CL_SetStat: %i is invalid", stat);

	if (cls.demoplayback2) {
		cl.players[cls.lastto].stats[stat]=value;
		if ( Cam_TrackNum() != cls.lastto )
			return;
	}

	Sbar_Changed ();
	
	if (stat == STAT_ITEMS)
	{	// set flash times
		Sbar_Changed ();
		for (j=0 ; j<32 ; j++)
			if ( (value & (1<<j)) && !(cl.stats[stat] & (1<<j)))
				cl.item_gettime[j] = cl.time;
	}

	cl.stats[stat] = value;

	TP_StatChanged(stat, value);
}

/*
==============
CL_MuzzleFlash
==============
*/
void CL_MuzzleFlash (void)
{
	vec3_t		fv, rv, uv;
	dlight_t	*dl;
	int			i;
	int			j, num_ent;
	entity_state_t	*ent;
	player_state_t	*pl;

	i = MSG_ReadShort ();

	if (!cl_muzzleflash.value)
		return;

	if ((unsigned)(i-1) >= MAX_CLIENTS)
	{
		// a monster firing
		num_ent = cl.frames[cls.netchan.incoming_sequence&UPDATE_MASK].packet_entities.num_entities;
		for (j = 0; j < num_ent; j++)
		{
			ent = &cl.frames[cls.netchan.incoming_sequence&UPDATE_MASK].packet_entities.entities[j];
			if (ent->number == i)
			{
				dl = CL_AllocDlight (-i);
				VectorCopy (ent->origin,  dl->origin);
				AngleVectors (ent->angles, fv, rv, uv);
				VectorMA (dl->origin, 18, fv, dl->origin);
				dl->radius = 200 + (rand()&31);
				dl->minlight = 32;
				dl->die = cl.time + 0.1;
				dl->color[0] = 0.2;
				dl->color[1] = 0.1;
				dl->color[2] = 0.05;
				dl->color[3] = 0.7;
				break;
			}
		}
		return;
	}

#ifdef GLQUAKE
	// don't draw our own muzzle flash in gl if flashblending
	if (i-1 == cl.playernum && gl_flashblend.value)
		return;
#endif

	if (cl_muzzleflash.value == 2 && i-1 == cl.playernum)
		return;

	pl = &cl.frames[parsecountmod].playerstate[i-1];

	dl = CL_AllocDlight (-i);
	VectorCopy (pl->origin,  dl->origin);
	AngleVectors (pl->viewangles, fv, rv, uv);
		
	VectorMA (dl->origin, 18, fv, dl->origin);
	dl->radius = 200 + (rand()&31);
	dl->minlight = 32;
	dl->die = cl.time + 0.1;
	dl->color[0] = 0.2;
	dl->color[1] = 0.1;
	dl->color[2] = 0.05;
	dl->color[3] = 0.7;
}


#define SHOWNET(x) if(cl_shownet.value==2)Con_Printf ("%3i:%s\n", msg_readcount-1, x);
/*
=====================
CL_ParseServerMessage
=====================
*/
int	received_framecount;
extern int	fixangle;
void CL_ParseServerMessage (void)
{
	int			cmd;
	int			i, j;

	received_framecount = host_framecount;
	cl.last_servermessage = realtime;

//
// if recording demos, copy the message out
//
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Printf ("------------------\n");


	CL_ParseClientdata ();
	CL_ClearProjectiles();

//
// parse the message
//
	while (1)
	{
		if (msg_badread)
		{
			Host_EndGame ("CL_ParseServerMessage: Bad server message");
			break;
		}

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
			msg_readcount++;	// so the EOM showner has the right value
			SHOWNET("END OF MESSAGE");
			break;
		}

		SHOWNET(svc_strings[cmd]);
	
	// other commands
		switch (cmd)
		{
		default:
			Host_EndGame ("CL_ParseServerMessage: Illegible server message");
			break;
			
		case svc_nop:
//			Con_Printf ("svc_nop\n");
			break;
			
		case svc_disconnect:
			if (cls.state == ca_connected)
				Host_EndGame ("Server disconnected\n"
					"Server version may not be compatible");
			else
				Host_EndGame ("Server disconnected");
			break;

		case svc_print:
			CL_ParsePrint ();
			break;
			
		case svc_centerprint:
			SCR_CenterPrint (MSG_ReadString ());
			break;
			
		case svc_stufftext:
			CL_ParseStufftext ();
			break;
			
		case svc_damage:
			V_ParseDamage ();
			break;
			
		case svc_serverdata:
			Cbuf_Execute ();		// make sure any stuffed commands are done
			CL_ParseServerData ();
			vid.recalc_refdef = true;	// leave full screen intermission
			break;
			
		case svc_setangle:
			if (cls.demoplayback2) {
				j = MSG_ReadByte();
				fixangle |= 1 << j;
				if (j != Cam_TrackNum())
					for (i=0; i<3; i++)
						MSG_ReadAngle();
			}

			if (!cls.demoplayback2 || (cls.demoplayback2 && j == Cam_TrackNum()))
			{
				for (i=0 ; i<3 ; i++)
					cl.viewangles[i] = MSG_ReadAngle ();
			}
//			cl.viewangles[PITCH] = cl.viewangles[ROLL] = 0;
			break;
			
		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Host_EndGame ("svc_lightstyle > MAX_LIGHTSTYLES");
			strncpy (cl_lightstyle[i].map,  MSG_ReadString(), MAX_STYLESTRING);
			cl_lightstyle[i].length = strlen(cl_lightstyle[i].map);
			break;
			
		case svc_sound:
			CL_ParseStartSoundPacket();
			break;
			
		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;
		
		case svc_updatefrags:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updatefrags > MAX_CLIENTS");
			cl.players[i].frags = MSG_ReadShort ();
			break;			

		case svc_updateping:
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updateping > MAX_CLIENTS");
			cl.players[i].ping = MSG_ReadShort ();
			break;
			
		case svc_updatepl:
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updatepl > MAX_CLIENTS");
			cl.players[i].pl = MSG_ReadByte ();
			break;
			
		case svc_updateentertime:
		// time is sent over as seconds ago
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updateentertime > MAX_CLIENTS");
			cl.players[i].entertime = realtime - MSG_ReadFloat ();
			//Con_Printf("enter:%f, real:%f\n", realtime - cl.players[i].entertime, realtime);
			break;
			
		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			CL_ParseBaseline (&cl_baselines[i]);
			break;
		case svc_spawnstatic:
			CL_ParseStatic ();
			break;			
		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_killedmonster:
			cl.stats[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			cl.stats[STAT_SECRETS]++;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();
			j = MSG_ReadByte ();
			//stat_size += 8;
			CL_SetStat (i, j);
			break;
		case svc_updatestatlong:
			i = MSG_ReadByte ();
			j = MSG_ReadLong ();
			//stat_size += 11;
			CL_SetStat (i, j);
			break;
			
		case svc_spawnstaticsound:
			CL_ParseStaticSound ();
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			CDAudio_Play ((byte)cl.cdtrack, true);
			break;

		case svc_intermission:
			cl.intermission = 1;
			cl.completed_time = realtime;
			vid.recalc_refdef = true;	// go to full screen
			for (i=0 ; i<3 ; i++)
				cl.simorg[i] = MSG_ReadCoord ();			
			for (i=0 ; i<3 ; i++)
				cl.simangles[i] = MSG_ReadAngle ();
			VectorCopy (vec3_origin, cl.simvel);
			TP_ExecTrigger ("f_mapend");
			break;

		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = realtime;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());			
			break;
			
		case svc_sellscreen:
			Cmd_ExecuteString ("help");
			break;

		case svc_smallkick:
			cl.punchangle = -2;
			break;
		case svc_bigkick:
			cl.punchangle = -4;
			break;

		case svc_muzzleflash:
			CL_MuzzleFlash ();
			break;

		case svc_updateuserinfo:
			CL_UpdateUserinfo ();
			break;

		case svc_setinfo:
			CL_SetInfo ();
			break;

		case svc_serverinfo:
			CL_ParseServerInfoChange ();
			break;

		case svc_download:
			CL_ParseDownload ();
			break;

		case svc_playerinfo:
			CL_ParsePlayerinfo ();
			break;

		case svc_nails:
			CL_ParseProjectiles (false);
			break;
		case svc_nails2:
			CL_ParseProjectiles (true);
			break;

		case svc_chokecount:		// some preceding packets were choked
			i = MSG_ReadByte ();
			for (j=0 ; j<i ; j++)
				cl.frames[ (cls.netchan.incoming_acknowledged-1-j)&UPDATE_MASK ].receivedtime = -2;
			break;

		case svc_modellist:
			CL_ParseModellist ();
			break;

		case svc_soundlist:
			CL_ParseSoundlist ();
			break;

		case svc_packetentities:
			CL_ParsePacketEntities (false);
			break;

		case svc_deltapacketentities:
			CL_ParsePacketEntities (true);
			break;

		case svc_maxspeed :
			movevars.maxspeed = MSG_ReadFloat();
			break;

		case svc_entgravity :
			movevars.entgravity = MSG_ReadFloat();
			break;

		case svc_setpause:
			cl.paused = (cl.paused &~ 1) | (MSG_ReadByte () != 0);
			if (cl.paused)	// FIXME
				CDAudio_Pause ();
			else
				CDAudio_Resume ();
			break;

		}
	}

	CL_SetSolidEntities ();
}
