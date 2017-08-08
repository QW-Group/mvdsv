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

#ifndef __FS_H__
#define __FS_H__

/*
=============================================================================

VARIABLES

=============================================================================
*/

extern char fs_gamedir[MAX_OSPATH];

/*
=============================================================================

FUNCTION PROTOTYPES

=============================================================================
*/

void FS_Init (void);
void FS_ShutDown(void);
long FS_FileLength (FILE *f);
void FS_FileBase (char *in, char *out);
#define COM_FileBase FS_FileBase // ezquake compatibility

void FS_WriteFile (char *filename, void *data, int len);

byte *FS_LoadTempFile (char *path, int *len);
byte *FS_LoadHunkFile (char *path, int *len);
void FS_CreatePath (char *path);
char *FS_NextPath (char *prevpath);
void FS_SetGamedir (char *dir, qbool force);

qbool FS_UnsafeFilename(const char* name);

#endif /* !__FS_H__ */
