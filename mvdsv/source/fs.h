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

    $Id: fs.h,v 1.2 2006/06/05 12:43:55 vvd0 Exp $
*/

#ifndef __FS_H__
#define __FS_H__

/*
=============================================================================

CONSTANTS

=============================================================================
*/

#define MAX_FILES_IN_PACK 2048

/*
=============================================================================

TYPES

=============================================================================
*/

// ------ PAK files on disk ------ //
typedef struct dpackfile_s
{
	char name[56];
	int filepos, filelen;
} dpackfile_t;

typedef struct dpackheader_s
{
	char id[4];
	int dirofs;
	int dirlen;
} dpackheader_t;


// Packages in memory
typedef struct packfile_s
{
	char name[MAX_QPATH];
	int filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char filename[MAX_OSPATH];
	FILE *handle;
	int numfiles;
	packfile_t *files;
} pack_t;

// Search paths for files (including packages)
typedef struct searchpath_s
{
	// only one of filename / pack will be used
	char filename[MAX_OSPATH];
	pack_t *pack;
	struct searchpath_s *next;
} searchpath_t;


// ------ Variables ------ //

extern int fs_filesize;
extern char fs_gamedir[MAX_OSPATH];
extern char fs_basedir[MAX_OSPATH];

/*
=============================================================================

FUNCTION PROTOTYPES

=============================================================================
*/
// ------ Main functions ------ //
void FS_Init (void);
void FS_Init_Commands(void);
int FS_FileLength (FILE *f);
void COM_FileBase (char *in, char *out);

void COM_WriteFile (char *filename, void *data, int len);
int COM_FOpenFile (char *filename, FILE **file);

pack_t *COM_LoadPackFile (char *packfile);
byte *COM_LoadStackFile (char *path, void *buffer, int bufsize);
byte *COM_LoadTempFile (char *path);
byte *COM_LoadHunkFile (char *path);
void COM_CreatePath (char *path);
char *COM_NextPath (char *prevpath);
void COM_Gamedir (char *dir);

// ------ Other functions ------ //

#endif /* !__FS_H__ */
