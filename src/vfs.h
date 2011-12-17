/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *     
 *             
 */

#ifndef __VFS_H__
#define __VFS_H__


// FIXME: probably we can move some of that to .c files.


//=================================
// Quake filesystem
//=================================
extern hashtable_t *filesystemhash;
extern int fs_hash_dups;		
extern int fs_hash_files;		

typedef struct
{
	struct searchpath_s *	search;
	int						index;
	char					rawname[MAX_OSPATH];
	int						offset;
	int						len;
} flocation_t;

// FS_FLocateFile return type.
typedef enum
{
	FSLFRT_IFFOUND,			// return true if file found, false if not found.
	FSLFRT_LENGTH,			// return file length if found, -1 if not found.
	FSLFRT_DEPTH_OSONLY,	// return depth (no paks), 0x7fffffff if not found.
	FSLFRT_DEPTH_ANYPATH	// return depth, 0x7fffffff if not found.
} FSLF_ReturnType_e;

typedef enum
{
	VFSERR_NONE,
	VFSERR_EOF
} vfserrno_t;

typedef struct vfsfile_s
{
	int (*ReadBytes) (struct vfsfile_s *file, void *buffer, int bytestoread, vfserrno_t *err);
	int (*WriteBytes) (struct vfsfile_s *file, const void *buffer, int bytestowrite);
	int (*Seek) (struct vfsfile_s *file, unsigned long pos, int whence);	// Returns 0 on sucess, -1 otherwise
	unsigned long (*Tell) (struct vfsfile_s *file);
	unsigned long (*GetLen) (struct vfsfile_s *file);	// Could give some lag
	void (*Close) (struct vfsfile_s *file);
	void (*Flush) (struct vfsfile_s *file);
	qbool seekingisabadplan;
	qbool copyprotected;							// File found was in a pak
} vfsfile_t;

typedef struct
{
	void	(*PrintPath)(void *handle);
	void	(*ClosePath)(void *handle);
	void	(*BuildHash)(void *handle);
	// true if found (hashedresult can be NULL)
	// note that if rawfile and offset are set, many Com_FileOpens will 
	// read the raw file otherwise ReadFile will be called instead.
	qbool   (*FindFile)(void *handle, flocation_t *loc, const char *name, void *hashedresult);	
	// reads the entire file
	void	(*ReadFile)(void *handle, flocation_t *loc, char *buffer);	

	int		(*EnumerateFiles)(void *handle, char *match, int (*func)(char *, int, void *), void *parm);

	// returns a handle to a new pak/path
	void	*(*OpenNew)(vfsfile_t *file, const char *desc);	

	int		(*GeneratePureCRC) (void *handle, int seed, int usepure);

	vfsfile_t *(*OpenVFS)(void *handle, flocation_t *loc, char *mode);
} searchpathfuncs_t;

typedef struct searchpath_s
{
	searchpathfuncs_t *funcs;
	qbool copyprotected;	// don't allow downloads from here.
	qbool istemporary;
	void *handle;

	struct searchpath_s *next;

} searchpath_t;

// mostly analogs for stdio functions
void 			VFS_CLOSE  (struct vfsfile_s *vf);
unsigned long	VFS_TELL   (struct vfsfile_s *vf);
unsigned long	VFS_GETLEN (struct vfsfile_s *vf);
int				VFS_SEEK   (struct vfsfile_s *vf, unsigned long pos, int whence);
int				VFS_READ   (struct vfsfile_s *vf, void *buffer, int bytestoread, vfserrno_t *err);
int				VFS_WRITE  (struct vfsfile_s *vf, const void *buffer, int bytestowrite);
void			VFS_FLUSH  (struct vfsfile_s *vf);
// return null terminated string
char		   *VFS_GETS   (struct vfsfile_s *vf, char *buffer, int buflen); 
qbool			VFS_COPYPROTECTED(struct vfsfile_s *vf);

typedef enum
{
	FS_NONE_OS, // FIXME: probably must be removed, as not so secure...
				// Opened with OS functions (no paks).
				// filename.
				
	FS_GAME_OS, // Opened with OS functions (no paks).
				// fs_basedir/fs_gamedirfile/filename.

	FS_GAME,	// Searched on path as filename, including packs.

	FS_BASE_OS,	// Opened with OS functions (no paks).
				// fs_basedir/filename.

	FS_ANY		// That slightly evil, derived from ezquake.
				// 1) FS_GAME.
				// 2) FS_NONE_OS.
} relativeto_t;

vfsfile_t *FS_OpenVFS(const char *filename, char *mode, relativeto_t relativeto);
int FS_FLocateFile(const char *filename, FSLF_ReturnType_e returntype, flocation_t *loc);

//=================================
// STDIO Files (OS)
//=================================

vfsfile_t *FS_OpenTemp(void);
vfsfile_t *VFSOS_Open(char *osname, char *mode);

extern searchpathfuncs_t osfilefuncs;

//====================
// PACK (*pak) Support
//====================

extern searchpathfuncs_t packfilefuncs;

#endif /* __VFS_H__ */
