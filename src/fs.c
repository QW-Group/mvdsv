/*
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
 

/*
All of Quake's data access is through a hierchal file system, but the contents
of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and
all game directories.  The sys_* files pass this to host_init in
quakeparms_t->basedir.  This can be overridden with the "-basedir" command
line parm to allow code debugging in a different directory.  The base
directory is only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that
all generated files (savegames, screenshots, demos, config files) will be
saved to.  This can be overridden with the "-game" command line parameter.
The game directory can never be changed while quake is executing.  This is a
precaution against having a malicious server instruct clients to write files
over areas they shouldn't.
*/

typedef enum
{
	FS_LOAD_NONE     = 1,
	FS_LOAD_FILE_PAK = 2,
	FS_LOAD_FILE_ALL = FS_LOAD_FILE_PAK
} FS_Load_File_Types;

/*
=============================================================================

VARIABLES

=============================================================================
*/

// WARNING: if you add some FS related global variable
// then made appropriate change to FS_ShutDown() too, if required.

static char		fs_basedir[MAX_OSPATH];		// c:/quake

char			fs_gamedir[MAX_OSPATH];		// c:/quake/qw
static char		fs_gamedirfile[MAX_QPATH];	// qw tf ctf and etc. In other words single dir name without path

static searchpath_t	*fs_searchpaths = NULL;
static searchpath_t	*fs_base_searchpaths = NULL;	// without gamedirs

hashtable_t		*filesystemhash = NULL;
qbool			filesystemchanged = true;
int				fs_hash_dups = 0;
int				fs_hash_files = 0;

cvar_t fs_cache = {"fs_cache", "1"};

/*
=============================================================================

FORWARD DEFINITION

=============================================================================
*/

static searchpath_t *FS_AddPathHandle(char *probablepath, searchpathfuncs_t *funcs, void *handle, qbool copyprotect, qbool istemporary, FS_Load_File_Types loadstuff);

/*
=============================================================================

COMMANDS

=============================================================================
*/

/*
============
FS_Path_f

============
*/
static void FS_Path_f (void)
{
	searchpath_t	*search;

	Con_Printf ("Current search path:\n");

	for (search = fs_searchpaths; search ; search = search->next)
	{
		if (search == fs_base_searchpaths)
			Con_Printf ("----------\n");

		search->funcs->PrintPath(search->handle);
	}
}

/*
=============================================================================

FUNCTIONS

=============================================================================
*/

/*
================
FS_GetCleanPath

================
*/
static const char *FS_GetCleanPath(const char *pattern, char *outbuf, int outlen)
{
	char *s;

	if (strchr(pattern, '\\'))
	{
		strlcpy(outbuf, pattern, outlen);
		pattern = outbuf;

		Con_Printf("Warning: \\ characters in filename %s\n", pattern);

		for (s = (char*)pattern; (s = strchr(s, '\\')); s++)
			*s = '/';
	}

	if (*pattern == '/' || strstr(pattern, "..") || strstr(pattern, ":"))
		Con_Printf("Error: absolute path in filename %s\n", pattern);
	else
		return pattern;

	return NULL;
}

/*
============
FS_FlushFSHash

Flush FS hash and mark FS as changed,
so FS_FLocateFile() will be forced to call FS_RebuildFSHash().
============
*/
void FS_FlushFSHash(void)
{
	if (filesystemhash)
	{
		Hash_Flush(filesystemhash);
	}

	filesystemchanged = true;
}

/*
============
FS_RebuildFSHash

Rebuild FS hash.
============
*/
static void FS_RebuildFSHash(void)
{
	searchpath_t	*search;
	if (!filesystemhash)
	{
		filesystemhash = Hash_InitTable(1024);
	}
	else
	{
		FS_FlushFSHash();
	}

	fs_hash_dups = 0;
	fs_hash_files = 0;

	for (search = fs_searchpaths ; search ; search = search->next)
	{
		search->funcs->BuildHash(search->handle);
	}

	filesystemchanged = false;

	Con_DPrintf("FS_RebuildFSHash: %i unique files, %i duplicates\n", fs_hash_files, fs_hash_dups);
}

/*
============
FS_FLocateFile

Finds the file in the search path.
Look FSLF_ReturnType_e definition so you know that it returns.
============
*/
int FS_FLocateFile(const char *filename, FSLF_ReturnType_e returntype, flocation_t *loc)
{
	int				depth = 0;
	int				len;
	searchpath_t	*search;
	char			cleanpath[MAX_OSPATH];
	void			*pf = NULL;

	filename = FS_GetCleanPath(filename, cleanpath, sizeof(cleanpath));
	if (!filename)
		goto fail;

 	if (fs_cache.value)
	{
		if (filesystemchanged)
			FS_RebuildFSHash();
		pf = Hash_GetInsensitive(filesystemhash, filename);
		if (!pf)
			goto fail;
	}

	//
	// search through the path, one element at a time.
	//
	for (search = fs_searchpaths ; search ; search = search->next)
	{
		if (search->funcs->FindFile(search->handle, loc, filename, pf))
		{
			if (loc)
			{
				loc->search = search;
				len = loc->len;
			}
			else
			{
				len = 0;
			}

			goto out;
		}

		depth += (search->funcs == &osfilefuncs || returntype == FSLFRT_DEPTH_ANYPATH);
	}
	
fail:
	if (loc)
		loc->search = NULL;
	depth = 0x7fffffff; // NOTE: weird, we return it on fail in some cases, may cause mistakes by user.
	len = -1;

out:

/*	Debug printing removed
 *	if (len>=0)
	{
		if (loc)
			Con_Printf("Found %s:%i\n", loc->rawname, loc->len);
		else
			Con_Printf("Found %s\n", filename);
	}
	else
		Con_Printf("Failed\n");
*/

	if (returntype == FSLFRT_IFFOUND)
		return len != -1;
	else if (returntype == FSLFRT_LENGTH)
		return len;
	else
		return depth;
}

// internal struct, no point to expose it outside.
typedef struct
{
	searchpathfuncs_t *funcs;
	searchpath_t *parentpath;
	char *parentdesc;
} wildpaks_t;

/*
================
FS_AddWildDataFiles

Add pack files masked by wildcards, e.g. '*.pak' .
That allow add not only packs named like pak0.pak pak1.pak but with random names too.
================
*/
static int FS_AddWildDataFiles (char *descriptor, int size, void *vparam)
{
	wildpaks_t			*param = vparam;
	vfsfile_t			*vfs;
	searchpathfuncs_t	*funcs = param->funcs;
	searchpath_t		*search;
	void 				*pak;
	char				pakfile[MAX_OSPATH];
	flocation_t			loc = {0};

	snprintf (pakfile, sizeof (pakfile), "%s%s", param->parentdesc, descriptor);

	for (search = fs_searchpaths; search; search = search->next)
	{
		if (search->funcs != funcs)
			continue;
		if (!strcasecmp((char*)search->handle, pakfile))	//assumption: first member of structure is a char array
			return true; //already loaded (base paths?)
	}

	search = param->parentpath;

	if (!search->funcs->FindFile(search->handle, &loc, descriptor, NULL))
		return true;	//not found..

	vfs = search->funcs->OpenVFS(search->handle, &loc, "rb");
	if (!vfs)
		return true;
	pak = funcs->OpenNew (vfs, pakfile);
	if (!pak)
	{
		VFS_CLOSE(vfs);
		return true;
	}

	snprintf (pakfile, sizeof (pakfile), "%s%s/", param->parentdesc, descriptor);
	FS_AddPathHandle(pakfile, funcs, pak, true, false, FS_LOAD_FILE_ALL);

	return true;
}

/*
================
FS_AddDataFiles

Load pack files.
================
*/
static void FS_AddDataFiles(char *pathto, searchpath_t *parent, char *extension, searchpathfuncs_t *funcs)
{
	int				i;
	char			pakfile[MAX_OSPATH];
	wildpaks_t		wp = {0};

	// first load all the numbered pak files.
	for (i = 0; ; i++)
	{
		void		*handle;
		vfsfile_t	*vfs;
		flocation_t	loc = {0};

		snprintf (pakfile, sizeof(pakfile), "pak%i.%s", i, extension);
		if (!parent->funcs->FindFile(parent->handle, &loc, pakfile, NULL))
			break;	//not found..
		snprintf (pakfile, sizeof(pakfile), "%spak%i.%s", pathto, i, extension);
		vfs = parent->funcs->OpenVFS(parent->handle, &loc, "rb");
		if (!vfs)
			break;
		handle = funcs->OpenNew (vfs, pakfile);
		if (!handle)
		{
			VFS_CLOSE(vfs);
			break;
		}
		snprintf (pakfile, sizeof(pakfile), "%spak%i.%s/", pathto, i, extension);
		FS_AddPathHandle(pakfile, funcs, handle, true, false, FS_LOAD_FILE_ALL);
	}

	// now load the random ones.
	snprintf (pakfile, sizeof (pakfile), "*.%s", extension);
	wp.funcs = funcs;
	wp.parentdesc = pathto;
	wp.parentpath = parent;
	parent->funcs->EnumerateFiles(parent->handle, pakfile, FS_AddWildDataFiles, &wp);
}

/*
================
FS_AddPathHandle

Adds searchpath and load pack files in it if requested.
================
*/
static searchpath_t *FS_AddPathHandle(char *probablepath, searchpathfuncs_t *funcs, void *handle, qbool copyprotect, qbool istemporary, FS_Load_File_Types loadstuff)
{
	searchpath_t *search;

	// allocate new search path and init it.
	search = (searchpath_t*)Q_malloc (sizeof(searchpath_t));
	search->copyprotected = copyprotect;
	search->istemporary = istemporary;
	search->handle = handle;
	search->funcs = funcs;

	// link seach path in.
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	// mark file system is changed.
	filesystemchanged = true;

	// add any data files too.
	if (loadstuff & FS_LOAD_FILE_PAK)
		FS_AddDataFiles(probablepath, search, "pak", &packfilefuncs);//q1/hl/h2/q2

	return search;
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak0.pak pak1.pak ...
================
*/
static void FS_AddGameDirectory (char *dir, FS_Load_File_Types loadstuff)
{
	char *p;
	searchpath_t *search;

	if ((p = strrchr(dir, '/')))
		strlcpy (fs_gamedirfile, ++p, sizeof (fs_gamedirfile));
	else
		strlcpy (fs_gamedirfile, dir, sizeof (fs_gamedirfile));

	strlcpy (fs_gamedir, dir, sizeof (fs_gamedir));

	for (search = fs_searchpaths; search; search = search->next)
	{
		if (search->funcs != &osfilefuncs)
			continue; // ignore packs and such.

		if (!strcasecmp(search->handle, fs_gamedir))
			return; //already loaded (base paths?)
	}

	// add the directory to the search path
	FS_AddPathHandle (va("%s/", dir), &osfilefuncs, Q_strdup(dir), false, false, loadstuff);
}

/*
============
FS_SetGamedir

Sets the gamedir and path to a different directory.
That is basically handy wrapper around FS_AddGameDirectory() for "gamedir" command.
============
*/
void FS_SetGamedir (char *dir, qbool force)
{
	if (strstr(dir, "..") || strstr(dir, "/") || strstr(dir, "\\") || strstr(dir, ":")) 
	{
		Con_Printf ("Gamedir should be a single filename, not a path\n");
		return;
	}

	if (!force && !strcmp(fs_gamedirfile, dir))
		return;		// Still the same, unless we forced.

	// FIXME: do we need it? since it will be set in FS_AddGameDirectory().
	strlcpy (fs_gamedirfile, dir, sizeof(fs_gamedirfile));

	// Free up any current game dir info.
	FS_FlushFSHash();

	// free up any current game dir info
	while (fs_searchpaths != fs_base_searchpaths)
	{
		searchpath_t  *next;

		fs_searchpaths->funcs->ClosePath(fs_searchpaths->handle);
		next = fs_searchpaths->next;
		Q_free (fs_searchpaths);
		fs_searchpaths = next;
	}

	// mark file system is changed.
	filesystemchanged = true;

	// Flush all data, so it will be forced to reload.
// well, mvdsv does not use cache anyway.
//	Cache_Flush ();

	// FIXME: do we need it? since it will be set in FS_AddGameDirectory().
	snprintf (fs_gamedir, sizeof (fs_gamedir), "%s/%s", fs_basedir, dir);

	FS_AddGameDirectory(va("%s/%s", fs_basedir, dir), FS_LOAD_FILE_ALL);
}


/*
================
FS_InitModule

Add commands and cvars.
================
*/
void FS_InitModule(void)
{
	Cmd_AddCommand ("path", FS_Path_f);
	Cvar_Register(&fs_cache);
}

/*
================
FS_Init

================
*/
void FS_InitEx(void)
{
	int i;
	char *s;

	FS_ShutDown();

	if ((i = COM_CheckParm ("-basedir")) && i < COM_Argc() - 1)
	{
		// -basedir <path>
		// Overrides the system supplied base directory (under id1)
		strlcpy (fs_basedir, COM_Argv(i + 1), sizeof(fs_basedir));
	}
 	else
	{
#if 0 // FIXME: made fs_basedir equal to cwd
		Sys_getcwd(fs_basedir, sizeof(fs_basedir) - 1);
#else
		strlcpy (fs_basedir, ".", sizeof(fs_basedir));
#endif
	}

	// replace backslahes with slashes.
	for (s = fs_basedir; (s = strchr(s, '\\')); s++)
		*s = '/';

	// remove terminating slash if any.
	i = (int)strlen(fs_basedir) - 1;
	if (i >= 0 && fs_basedir[i] == '/')
		fs_basedir[i] = 0;

	// start up with id1 by default
	FS_AddGameDirectory(va("%s/%s", fs_basedir, "id1"),     FS_LOAD_FILE_ALL);
	FS_AddGameDirectory(va("%s/%s", fs_basedir, "qw"),      FS_LOAD_FILE_ALL);

	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// the user might want to override default game directory
	if (!(i = COM_CheckParm ("-game")))
		i = COM_CheckParm ("+gamedir");
	if (i && i < COM_Argc() - 1)
	{
		FS_SetGamedir (COM_Argv(i + 1), true);
		// FIXME: move in FS_SetGamedir() instead!!!
		Info_SetValueForStarKey (svs.info, "*gamedir", COM_Argv(i + 1), MAX_SERVERINFO_STRING);
	}
}

/*
================
FS_Init

================
*/
void FS_Init( void )
{
	FS_InitModule(); // init commands and variables.
	FS_InitEx();
}

/*
================
FS_ShutDown

================
*/
void FS_ShutDown( void )
{
	// free data
	while (fs_searchpaths)
	{
		searchpath_t  *next = fs_searchpaths->next;
		Q_free (fs_searchpaths); // FIXME: close handles and such!!!
		fs_searchpaths = next;
	}

	// flush all data, so it will be forced to reload
// well, mvdsv does not use cache anyway.
//	Cache_Flush ();

	// reset globals

	fs_base_searchpaths = fs_searchpaths = NULL;

	fs_gamedir[0]		= 0;
	fs_gamedirfile[0]	= 0;

	fs_basedir[0]		= 0;

	// FIXME:
//hashtable_t		*filesystemhash = NULL;
//qbool				filesystemchanged = true;
//int				fs_hash_dups = 0;
//int				fs_hash_files = 0;
}

/*
================
FS_OpenVFS

This should be how all files are opened.
================
*/
vfsfile_t *FS_OpenVFS(const char *filename, char *mode, relativeto_t relativeto)
{
	flocation_t loc = {0};
	vfsfile_t *vfs = NULL;
	char cleanname[MAX_OSPATH];
	char fullname[MAX_OSPATH];

	//blanket-bans
	filename = FS_GetCleanPath(filename, cleanname, sizeof(cleanname));
	if (!filename)
		return NULL;

	if (strcmp(mode, "rb"))
		if (strcmp(mode, "wb"))
			if (strcmp(mode, "ab"))
				return NULL; //urm, unable to write/append

	/* General opening of files */
	switch (relativeto)
	{
	case FS_NONE_OS: 	//OS access only, no paks, open file as is
		snprintf(fullname, sizeof(fullname), "%s", filename);
		return VFSOS_Open(fullname, mode);

	case FS_GAME_OS:	//OS access only, no paks
		snprintf(fullname, sizeof(fullname), "%s/%s/%s", fs_basedir, fs_gamedirfile, filename);
		if (strchr(mode, 'w') || strchr(mode, 'a'))
			FS_CreatePath(fullname); // FIXME: It should be moved to VFSOS_Open() itself? 
		return VFSOS_Open(fullname, mode);

	case FS_GAME:
//		snprintf(fullname, sizeof(fullname), "%s/%s/%s", fs_basedir, fs_gamedirfile, filename);

		// That an error attempt to write with FS_GAME, since file can be in pack file. redirect.
		if (strchr(mode, 'w') || strchr(mode, 'a'))
			return FS_OpenVFS(filename, mode, FS_GAME_OS);

		// Search on path, try to open if found.
		if (FS_FLocateFile(filename, FSLFRT_IFFOUND, &loc))
			return loc.search->funcs->OpenVFS(loc.search->handle, &loc, mode);

		return NULL;

	case FS_BASE_OS:	//OS access only, no paks
		snprintf(fullname, sizeof(fullname), "%s/%s", fs_basedir, filename);
		return VFSOS_Open(fullname, mode);

	case FS_ANY:
		// That an error attempt to write with FS_ANY, since file can be in pack file.
		vfs = FS_OpenVFS(filename, mode, FS_GAME);
		if (vfs)
			return vfs;

		vfs = FS_OpenVFS(filename, mode, FS_NONE_OS);
		if (vfs)
			return vfs;

		return NULL;

	default:
		Sys_Error("FS_OpenVFS: Bad relative path (%i)", relativeto);
		break;
	}

	return NULL;
}

//=======================================================================

void VFS_CHECKCALL (struct vfsfile_s *vf, void *fld, char *emsg)
{
	if (!fld)
		Sys_Error("%s", emsg);
}

void VFS_CLOSE (struct vfsfile_s *vf)
{
	assert(vf);
	VFS_CHECKCALL(vf, vf->Close, "VFS_CLOSE");
	vf->Close(vf);
}

unsigned long VFS_TELL (struct vfsfile_s *vf)
{
	assert(vf);
	VFS_CHECKCALL(vf, vf->Tell, "VFS_TELL");
	return vf->Tell(vf);
}

unsigned long VFS_GETLEN (struct vfsfile_s *vf)
{
	assert(vf);
	VFS_CHECKCALL(vf, vf->GetLen, "VFS_GETLEN");
	return vf->GetLen(vf);
}

/**
 * VFS_SEEK() reposition a stream
 * If whence is set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset  is
 * relative to the  start of the file, the current position indicator, or
 * end-of-file, respectively.
 * Return Value
 * Upon successful completion, VFS_SEEK(), returns 0. 
 * Otherwise, -1 is returned
 */
int VFS_SEEK (struct vfsfile_s *vf, unsigned long pos, int whence)
{
	assert(vf);
	VFS_CHECKCALL(vf, vf->Seek, "VFS_SEEK");
	return vf->Seek(vf, pos, whence);
}

int VFS_READ (struct vfsfile_s *vf, void *buffer, int bytestoread, vfserrno_t *err)
{
	assert(vf);
	VFS_CHECKCALL(vf, vf->ReadBytes, "VFS_READ");
	return vf->ReadBytes(vf, buffer, bytestoread, err);
}

int VFS_WRITE (struct vfsfile_s *vf, const void *buffer, int bytestowrite)
{
	assert(vf);
	VFS_CHECKCALL(vf, vf->WriteBytes, "VFS_WRITE");
	return vf->WriteBytes(vf, buffer, bytestowrite);
}

void VFS_FLUSH (struct vfsfile_s *vf)
{
	assert(vf);
	if(vf->Flush)
		vf->Flush(vf);
}

// return null terminated string
char *VFS_GETS(struct vfsfile_s *vf, char *buffer, int buflen)
{
	char in;
	char *out = buffer;
	int len = buflen-1;

	assert(vf);
	VFS_CHECKCALL(vf, vf->ReadBytes, "VFS_GETS");

//	if (len == 0)
//		return NULL;

// FIXME: I am not sure how to handle this better
	if (len <= 0)
		Sys_Error("VFS_GETS: len <= 0");

	while (len > 0)
	{
		if (!VFS_READ(vf, &in, 1, NULL))
		{
			if (len == buflen-1)
				return NULL;
			*out = '\0';
			return buffer;
		}
		if (in == '\n')
			break;
		*out++ = in;
		len--;
	}
	*out = '\0';

	return buffer;
}

qbool VFS_COPYPROTECTED(struct vfsfile_s *vf)
{
	assert(vf);
	return vf->copyprotected;
}

/*
=============================================================================

LEGACY FUNCTIONS

=============================================================================
*/

/*
================
FS_FileLength

================
*/
long FS_FileLength (FILE *f)
{
	long pos, end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

/*
============
FS_FileBase

============
*/
void FS_FileBase (char *in, char *out)
{
	char *begin, *end;
	int len;

	if (!(end = strrchr (in, '.')))
		end = in + strlen (in);

	if (!(begin = strchr (in, '/')))
		begin = in;
	else
		begin++;

	len = end - begin + 1;
	if (len < 1)
		strlcpy (out, "?model?", 8);
	else
		strlcpy (out, begin, min (len, MAX_OSPATH));
}

/*
============
FS_WriteFile

The filename will be prefixed by the current game directory
============
*/
void FS_WriteFile (char *filename, void *data, int len)
{
	FILE *f;
	char name[MAX_OSPATH];

	snprintf (name, MAX_OSPATH, "%s/%s", fs_gamedir, filename);

	f = fopen (name, "wb");
	if (!f)
	{
		Sys_mkdir (fs_gamedir);
		f = fopen (name, "wb");
		if (!f)
			Sys_Error ("Error opening %s", filename);
	}

	Sys_Printf ("FS_WriteFile: %s\n", name);
	fwrite (data, 1, len, f);
	fclose (f);
}


/*
============
FS_CreatePath

Only used for CopyFile and download
============
*/
void FS_CreatePath(char *path)
{
	char *s, save;

	if (!*path)
		return;

	for (s = path + 1; *s; s++)
	{
#ifdef _WIN32
		if (*s == '/' || *s == '\\')
		{
#else
		if (*s == '/')
		{
#endif
			save = *s;
			*s = 0;
			Sys_mkdir(path);
			*s = save;
		}
	}
}

/*
============
FS_LoadFile

Filename are relative to the quake directory.
Always appends a 0 byte to the loaded data.
============
*/
static byte *FS_LoadFile (char *path, void *allocator, int *file_length)
{
	vfsfile_t *f = NULL;
	vfserrno_t err;
	flocation_t loc = {0};
	byte *buf;
	int len;

	// Look for it in the filesystem or pack files.
    FS_FLocateFile(path, FSLFRT_LENGTH, &loc);
	if (loc.search)
	{
		f = loc.search->funcs->OpenVFS(loc.search->handle, &loc, "rb");
	}

	if (!f)
		return NULL;

	// FIXME: should we use loc.len instead?
	len = VFS_GETLEN(f);
	if (file_length)
		*file_length = len;

	if (allocator == Hunk_AllocName)
	{
		char base[32];
		// Extract the filename base name for hunk tag.
		FS_FileBase (path, base);
		buf = (byte *) Hunk_AllocName (len + 1, base);
	}
	else if (allocator == Hunk_TempAlloc)
	{
		buf = (byte *) Hunk_TempAlloc (len + 1);
	}
#if 0
	else if (allocator == Q_malloc)
	{
		buf = Q_malloc (len + 1);
	}
#endif
	else
	{
		Sys_Error ("FS_LoadFile: bad usehunk\n");
		return NULL;
	}

	if (!buf)
	{
		Sys_Error ("FS_LoadFile: not enough space for %s\n", path);
		return NULL;
	}

	buf[len] = 0;

	VFS_READ(f, buf, len, &err);
	VFS_CLOSE(f);

	return buf;
}

byte *FS_LoadHunkFile (char *path, int *len)
{
	return FS_LoadFile (path, Hunk_AllocName, len);
}

byte *FS_LoadTempFile (char *path, int *len)
{
	return FS_LoadFile (path, Hunk_TempAlloc, len);
}

/*
============
FS_NextPath

Iterate along searchpaths (no packs).
============
*/
char *FS_NextPath (char *prevpath)
{
	searchpath_t	*s;
	char			*prev;

	if (!prevpath)
		return fs_gamedir;

	prev = fs_gamedir;
	for (s = fs_searchpaths; s ; s = s->next)
	{
		if (s->funcs != &osfilefuncs)
			continue;

		if (prevpath == prev)
			return s->handle;
		prev = s->handle;
	}

	return NULL;
}

/*
===========
FS_UnsafeFilename

Returns true if user-specified path is unsafe
===========
*/
qbool FS_UnsafeFilename(const char* fileName)
{
	return !fileName ||
		!*fileName || // invalid name.
		fileName[1] == ':' ||	// dos filename absolute path specified - reject.
		*fileName == '\\' ||
		*fileName == '/' ||	// absolute path was given - reject.
		strstr(fileName, "..");
}

