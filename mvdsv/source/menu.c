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
#include "quakedef.h"
#include "winquake.h"
#include "cl_slist.h"
#include "keys.h"
#include "sound.h"
#include "version.h"
#ifdef QW_BOTH
#include "progs.h"	// FIXME
#include "server.h"
#endif

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

enum {m_none, m_main, m_singleplayer, m_load, m_save, m_multiplayer,
	m_setup, m_net, m_options, m_video, m_keys, m_help, m_quit,
	m_serialconfig, m_modemconfig, m_lanconfig, m_gameoptions, 
	m_search, m_slist, m_sedit} m_state;

void M_Menu_Main_f (void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Load_f (void);
		void M_Menu_Save_f (void);
	void M_Menu_MultiPlayer_f (void);
		void M_Menu_SEdit_f (void);
		void M_Menu_Setup_f (void);
//		void M_Menu_Net_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Menu_Video_f (void);
	void M_Menu_Help_f (void);
	void M_Menu_Quit_f (void);
void M_Menu_SerialConfig_f (void);
	void M_Menu_ModemConfig_f (void);
void M_Menu_LanConfig_f (void);
void M_Menu_GameOptions_f (void);
void M_Menu_Search_f (void);
void M_Menu_ServerList_f (void);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
	void M_MultiPlayer_Draw (void);
		void M_Setup_Draw (void);
		void M_Net_Draw (void);
	void M_Options_Draw (void);
		void M_Keys_Draw (void);
		void M_Video_Draw (void);
	void M_Help_Draw (void);
	void M_Quit_Draw (void);
void M_SerialConfig_Draw (void);
	void M_ModemConfig_Draw (void);
void M_LanConfig_Draw (void);
void M_GameOptions_Draw (void);
void M_Search_Draw (void);
void M_ServerList_Draw (void);

void M_Main_Key (int key);
	void M_SinglePlayer_Key (int key);
		void M_Load_Key (int key);
		void M_Save_Key (int key);
	void M_MultiPlayer_Key (int key);
		void M_Setup_Key (int key);
		void M_Net_Key (int key);
	void M_Options_Key (int key);
		void M_Keys_Key (int key);
		void M_Video_Key (int key);
	void M_Help_Key (int key);
	void M_Quit_Key (int key);
void M_SerialConfig_Key (int key);
	void M_ModemConfig_Key (int key);
void M_LanConfig_Key (int key);
void M_GameOptions_Key (int key);
void M_Search_Key (int key);
void M_ServerList_Key (int key);

qboolean	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	m_recursiveDraw;

int			m_return_state;
qboolean	m_return_onerror;
char		m_return_reason [32];

#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)
#define SerialConfig	(m_net_cursor == 0)
#define DirectConfig	(m_net_cursor == 1)
#define	IPXConfig		(m_net_cursor == 2)
#define	TCPIPConfig		(m_net_cursor == 3)

void M_ConfigureNetSubsystem(void);

//=============================================================================
/* Support Routines */

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character ( cx + ((vid.width - 320)>>1), line, num);
}

void M_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_TransPic (x + ((vid.width - 320)>>1), y, pic);
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x + ((vid.width - 320)>>1), y, pic);
}

byte identityTable[256];
byte translationTable[256];

void M_BuildTranslationTable(int top, int bottom)
{
	int		j;
	byte	*dest, *source;

	for (j = 0; j < 256; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);

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


void M_DrawTransPicTranslate (int x, int y, qpic_t *pic)
{
	Draw_TransPicTranslate (x + ((vid.width - 320)>>1), y, pic, translationTable);
}


void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p);
}

//=============================================================================

int m_save_demonum;
		
/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (void)
{
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}
		key_dest = key_game;
		m_state = m_none;
		return;
	}
/*	if (key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}*/
	else
	{
		M_Menu_Main_f ();
	}
}

		
//=============================================================================
/* MAIN MENU */

int	m_main_cursor;
#define	MAIN_ITEMS	5


void M_Menu_Main_f (void)
{
	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
}
				

void M_Main_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_main.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mainmenu.lmp") );

	f = (int)(realtime * 10)%6;
	
	M_DrawTransPic (54, 32 + m_main_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		key_dest = key_game;
		m_state = m_none;
		cls.demonum = m_save_demonum;
		if (cls.demonum != -1 && !cls.demoplayback && cls.state == ca_disconnected)
			CL_NextDemo ();
		break;
		
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		break;

	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		m_main_cursor = 0;
		break;

	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		m_main_cursor = MAIN_ITEMS - 1;
		break;
	
	case K_ENTER:
		m_entersound = true;

		switch (m_main_cursor)
		{
		case 0:
			M_Menu_SinglePlayer_f ();
			break;

		case 1:
			M_Menu_MultiPlayer_f ();
			break;

		case 2:
			M_Menu_Options_f ();
			break;

		case 3:
			M_Menu_Help_f ();
			break;

		case 4:
			M_Menu_Quit_f ();
			break;
		}
	}
}


//=============================================================================
/* OPTIONS MENU */

#define	OPTIONS_ITEMS	16

#define	SLIDER_RANGE	10

int		options_cursor;

void M_Menu_Options_f (void)
{
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;
}


void M_AdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (options_cursor)
	{
	case 3:	// screen size
		scr_viewsize.value += dir * 10;
		if (scr_viewsize.value < 30)
			scr_viewsize.value = 30;
		if (scr_viewsize.value > 120)
			scr_viewsize.value = 120;
		Cvar_SetValue (&scr_viewsize, scr_viewsize.value);
		break;
	case 4:	// gamma
		v_gamma.value -= dir * 0.05;
		if (v_gamma.value < 0.5)
			v_gamma.value = 0.5;
		if (v_gamma.value > 1)
			v_gamma.value = 1;
		Cvar_SetValue (&v_gamma, v_gamma.value);
		break;
	case 5:	// mouse speed
		sensitivity.value += dir * 0.5;
		if (sensitivity.value < 1)
			sensitivity.value = 1;
		if (sensitivity.value > 11)
			sensitivity.value = 11;
		Cvar_SetValue (&sensitivity, sensitivity.value);
		break;
	case 6:	// music volume
#ifdef _WIN32
		bgmvolume.value += dir * 1.0;
#else
		bgmvolume.value += dir * 0.1;
#endif
		if (bgmvolume.value < 0)
			bgmvolume.value = 0;
		if (bgmvolume.value > 1)
			bgmvolume.value = 1;
		Cvar_SetValue (&bgmvolume, bgmvolume.value);
		break;
	case 7:	// sfx volume
		volume.value += dir * 0.1;
		if (volume.value < 0)
			volume.value = 0;
		if (volume.value > 1)
			volume.value = 1;
		Cvar_SetValue (&volume, volume.value);
		break;
		
	case 8:	// always run
		if (cl_forwardspeed.value > 200)
		{
			Cvar_SetValue (&cl_forwardspeed, 200);
			Cvar_SetValue (&cl_backspeed, 200);
		}
		else
		{
			Cvar_SetValue (&cl_forwardspeed, 400);
			Cvar_SetValue (&cl_backspeed, 400);
		}
		break;
	
	case 9:	// invert mouse
		Cvar_SetValue (&m_pitch, -m_pitch.value);
		break;
	
	case 10:	// lookspring
		Cvar_SetValue (&lookspring, !lookspring.value);
		break;
	
	case 11:	// lookstrafe
		Cvar_SetValue (&lookstrafe, !lookstrafe.value);
		break;

	case 12:
		Cvar_SetValue (&cl_sbar, !cl_sbar.value);
		break;

	case 13:
		Cvar_SetValue (&cl_hudswap, !cl_hudswap.value);
		break;

	case 15:	// _windowed_mouse
		Cvar_SetValue (&_windowed_mouse, !_windowed_mouse.value);
		break;
	}
}


void M_DrawSlider (int x, int y, float range)
{
	int	i;

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 128);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*8, y, 129);
	M_DrawCharacter (x+i*8, y, 130);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 131);
}

void M_DrawCheckbox (int x, int y, int on)
{
#if 0
	if (on)
		M_DrawCharacter (x, y, 131);
	else
		M_DrawCharacter (x, y, 129);
#endif
	if (on)
		M_Print (x, y, "on");
	else
		M_Print (x, y, "off");
}

void M_Options_Draw (void)
{
	float		r;
	qpic_t	*p;
	
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	
	M_Print (16, 32, "    Customize controls");
	M_Print (16, 40, "         Go to console");
	M_Print (16, 48, "     Reset to defaults");

	M_Print (16, 56, "           Screen size");
	r = (scr_viewsize.value - 30) / (120 - 30);
	M_DrawSlider (220, 56, r);

	M_Print (16, 64, "            Brightness");
	r = (1.0 - v_gamma.value) / 0.5;
	M_DrawSlider (220, 64, r);

	M_Print (16, 72, "           Mouse Speed");
	r = (sensitivity.value - 1)/10;
	M_DrawSlider (220, 72, r);

	M_Print (16, 80, "       CD Music Volume");
	r = bgmvolume.value;
	M_DrawSlider (220, 80, r);

	M_Print (16, 88, "          Sound Volume");
	r = volume.value;
	M_DrawSlider (220, 88, r);

	M_Print (16, 96,  "            Always Run");
	M_DrawCheckbox (220, 96, cl_forwardspeed.value > 200);

	M_Print (16, 104, "          Invert Mouse");
	M_DrawCheckbox (220, 104, m_pitch.value < 0);

	M_Print (16, 112, "            Lookspring");
	M_DrawCheckbox (220, 112, lookspring.value);

	M_Print (16, 120, "            Lookstrafe");
	M_DrawCheckbox (220, 120, lookstrafe.value);

	M_Print (16, 128, "    Use old status bar");
	M_DrawCheckbox (220, 128, cl_sbar.value);

	M_Print (16, 136, "      HUD on left side");
	M_DrawCheckbox (220, 136, cl_hudswap.value);

	if (vid_menudrawfn)
		M_Print (16, 144, "         Video Options");

#ifdef _WIN32
	if (modestate == MS_WINDOWED)
	{
#endif
		M_Print (16, 152, "             Use Mouse");
		M_DrawCheckbox (220, 152, _windowed_mouse.value);
#ifdef _WIN32
	}
#endif

// cursor
	M_DrawCharacter (200, 32 + options_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
		
	case K_ENTER:
		m_entersound = true;
		switch (options_cursor)
		{
		case 0:
			M_Menu_Keys_f ();
			break;
		case 1:
			m_state = m_none;
			key_dest = key_console;
//			Con_ToggleConsole_f ();
			break;
		case 2:
			Cbuf_AddText ("exec default.cfg\n");
			break;
		case 14:
			M_Menu_Video_f ();
			break;
		default:
			M_AdjustSliders (1);
			break;
		}
		return;
	
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;	

	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		options_cursor = 0;
		break;

	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;
	}

	if (options_cursor == 14 && vid_menudrawfn == NULL)
	{
		if (k == K_UPARROW || k == K_END || k == K_PGDN)
			options_cursor = 13;
		else
			options_cursor = 0;
	}

	if ((options_cursor == 15) 
#ifdef _WIN32
	&& (modestate != MS_WINDOWED)
#endif
	)
	{
		if (k == K_UPARROW || k == K_END || k == K_PGDN)
			options_cursor = 14;
		else
			options_cursor = 0;
	}
}


//=============================================================================
/* KEYS MENU */

char *bindnames[][2] =
{
{"+attack", 		"attack"},
{"+jump", 			"jump"},
{"+forward", 		"walk forward"},
{"+back", 			"backpedal"},
{"+moveleft", 		"move left"},
{"+moveright", 		"move right"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},
{"impulse 10", 		"change weapon"},
{"+speed", 			"run"},
{"+strafe", 		"sidestep"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 			"mouse look"},
};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

int		keys_cursor;
int		bind_grab;

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}


void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}


void M_Keys_Draw (void)
{
	int		i, l;
	int		keys[2];
	char	*name;
	int		x, y;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	if (bind_grab)
		M_Print (12, 32, "Press a key or button for this action");
	else
		M_Print (18, 32, "Enter to change, backspace to clear");
		
// search for known bindings
	for (i=0 ; i<NUMCOMMANDS ; i++)
	{
		y = 48 + 8*i;

		M_Print (16, y, bindnames[i][1]);

		l = strlen (bindnames[i][0]);
		
		M_FindKeysForCommand (bindnames[i][0], keys);
		
		if (keys[0] == -1)
		{
			M_Print (140, y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
			M_Print (140, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (140 + x + 8, y, "or");
				M_Print (140 + x + 32, y, Key_KeynumToString (keys[1]));
			}
		}
	}
	
	if (bind_grab)
		M_DrawCharacter (130, 48 + keys_cursor*8, '=');
	else
		M_DrawCharacter (130, 48 + keys_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[2];
	
	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menu1.wav");
		if (k == K_ESCAPE)
		{
			bind_grab = false;
		}
		else if (k != '`')
		{
			sprintf (cmd, "bind %s \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);			
			Cbuf_InsertText (cmd);
		}
		
		bind_grab = false;
		return;
	}
	
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor = 0;
		break;

	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor = NUMCOMMANDS - 1;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
		break;

	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}

//=============================================================================
/* VIDEO MENU */

void M_Menu_Video_f (void)
{
	key_dest = key_menu;
	m_state = m_video;
	m_entersound = true;
}


void M_Video_Draw (void)
{
	(*vid_menudrawfn) ();
}


void M_Video_Key (int key)
{
	(*vid_menukeyfn) (key);
}

//=============================================================================
/* HELP MENU */

int		help_page;
#define	NUM_HELP_PAGES	6


void M_Menu_Help_f (void)
{
	key_dest = key_menu;
	m_state = m_help;
	m_entersound = true;
	help_page = 0;
}



void M_Help_Draw (void)
{
	M_DrawPic (0, 0, Draw_CachePic ( va("gfx/help%i.lmp", help_page)) );
}


void M_Help_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
		
	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_HELP_PAGES-1;
		break;
	}

}

//=============================================================================
/* QUIT MENU */

int		msgNumber;
int		m_quit_prevstate;
qboolean	wasInMenus;

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}


void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		key_dest = key_console;
		CL_Disconnect ();
		Sys_Quit ();
		break;

	default:
		break;
	}

}

//=============================================================================
/* SINGLE PLAYER MENU */

#ifdef QW_BOTH
int	m_singleplayer_cursor;
#define	SINGLEPLAYER_ITEMS	3

extern	cvar_t	maxclients;

void M_Menu_SinglePlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_singleplayer;
//	m_entersound = true;
	S_LocalSound ("misc/menu1.wav");
}


void M_SinglePlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/sp_menu.lmp") );

	f = (int)(realtime * 10)%6;

	M_DrawTransPic (54, 32 + m_singleplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_SinglePlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;

	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		m_singleplayer_cursor = 0;
		break;

	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
//		m_entersound = true;
//		S_LocalSound ("misc/menu1.wav");

		switch (m_singleplayer_cursor)
		{
		case 0:
			if (sv.state != ss_dead)
				if (!SCR_ModalMessage("Are you sure you want to\nstart a new game?\n"))
					break;
			key_dest = key_game;
			Cvar_Set (&maxclients, "1");
//			Cvar_Set (&maxspectators, "0");
			Cvar_Set (&teamplay, "0");
			Cvar_Set (&timelimit, "0");
			Cvar_Set (&deathmatch, "0");
			Cvar_Set (&coop, "0");
			if (sv.state != ss_dead)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("gamedir single\n");	// FIXME
			Cbuf_AddText ("map start\n");
			break;

		case 1:
			M_Menu_Load_f ();
			break;

		case 2:
			M_Menu_Save_f ();
			break;
		}
	}
}

#else	// QW_BOTH

void M_Menu_SinglePlayer_f (void)
{
	m_state = m_singleplayer;
}

void M_SinglePlayer_Draw (void)
{
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
//	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/sp_menu.lmp") );

	M_DrawTextBox (60, 10*8, 23, 4);	
	M_PrintWhite (92, 12*8, "QuakeWorld is for");
	M_PrintWhite (88, 13*8, "Internet play only");
}

void M_SinglePlayer_Key (key)
{
	if (key == K_ESCAPE || key == K_ENTER)
		m_state = m_main;
}
#endif	// QW_BOTH


//=============================================================================
/* LOAD/SAVE MENU */

/*
int		load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES

#define	MAX_SAVEGAMES		12
char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
int		loadable[MAX_SAVEGAMES];

void M_ScanSaves (void)
{
	int		i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int		version;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		sprintf (name, "%s/s%i.sav", com_gamedir, i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		strncpy (m_filenames[i], name, sizeof(m_filenames[i])-1);

	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}
*/

void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;
	key_dest = key_menu;
//	M_ScanSaves ();
}


void M_Menu_Save_f (void)
{
/*	if (!sv.active)
		return;
	if (cl.intermission)
		return;
	if (svs.maxclients != 1)
		return;
*/
	m_entersound = true;
	m_state = m_save;
	key_dest = key_menu;
//	M_ScanSaves ();
}



void M_Load_Draw (void)
{
//	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_load.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

/*	for (i=0 ; i< MAX_SAVEGAMES; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
*/

	M_DrawTextBox (60, 10*8, 23, 4);	
	M_PrintWhite (80, 12*8, "Savegames are not yet");
	M_PrintWhite (88, 13*8, "supported by ZQuake");
}


void M_Save_Draw (void)
{
//	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_save.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

/*	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
*/

	M_DrawTextBox (60, 10*8, 23, 4);	
	M_PrintWhite (80, 12*8, "Savegames are not yet");
	M_PrintWhite (88, 13*8, "supported by ZQuake");
}


void M_Load_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_ENTER:
		M_Menu_SinglePlayer_f ();
		break;
	}
}

void M_Save_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_ENTER:
		M_Menu_SinglePlayer_f ();
		break;
	}
}



//=============================================================================
/* MULTIPLAYER MENU */

int	m_multiplayer_cursor;
#define	MULTIPLAYER_ITEMS	3


void M_Menu_MultiPlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_multiplayer;
	m_entersound = true;
}


void M_MultiPlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );

	f = (int)(realtime * 10)%6;

	M_DrawTransPic (54, 32 + m_multiplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );

/*	if (serialAvailable || ipxAvailable || tcpipAvailable)
		return;
	M_PrintWhite ((320/2) - ((27*8)/2), 148, "No Communications Available");
*/
}


void M_MultiPlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_multiplayer_cursor >= MULTIPLAYER_ITEMS)
			m_multiplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_multiplayer_cursor < 0)
			m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;

	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		m_multiplayer_cursor = 0;
		break;

	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;
		switch (m_multiplayer_cursor)
		{
		case 0:
//			if (serialAvailable || ipxAvailable || tcpipAvailable)
//				M_Menu_Net_f ();
			M_Menu_ServerList_f ();
			break;

		case 1:
//			if (serialAvailable || ipxAvailable || tcpipAvailable)
//				M_Menu_ServerList_f ();
#ifdef QW_BOTH
			M_Menu_GameOptions_f ();
#endif
			break;

		case 2:
			M_Menu_Setup_f ();
			break;
		}
	}
}


//=============================================================================
/* GAME OPTIONS MENU */

#ifdef QW_BOTH

typedef struct
{
	char	*name;
	char	*description;
} level_t;

level_t		levels[] =
{
	{"start", "Entrance"},	// 0

	{"e1m1", "Slipgate Complex"},				// 1
	{"e1m2", "Castle of the Damned"},
	{"e1m3", "The Necropolis"},
	{"e1m4", "The Grisly Grotto"},
	{"e1m5", "Gloom Keep"},
	{"e1m6", "The Door To Chthon"},
	{"e1m7", "The House of Chthon"},
	{"e1m8", "Ziggurat Vertigo"},

	{"e2m1", "The Installation"},				// 9
	{"e2m2", "Ogre Citadel"},
	{"e2m3", "Crypt of Decay"},
	{"e2m4", "The Ebon Fortress"},
	{"e2m5", "The Wizard's Manse"},
	{"e2m6", "The Dismal Oubliette"},
	{"e2m7", "Underearth"},

	{"e3m1", "Termination Central"},			// 16
	{"e3m2", "The Vaults of Zin"},
	{"e3m3", "The Tomb of Terror"},
	{"e3m4", "Satan's Dark Delight"},
	{"e3m5", "Wind Tunnels"},
	{"e3m6", "Chambers of Torment"},
	{"e3m7", "The Haunted Halls"},

	{"e4m1", "The Sewage System"},				// 23
	{"e4m2", "The Tower of Despair"},
	{"e4m3", "The Elder God Shrine"},
	{"e4m4", "The Palace of Hate"},
	{"e4m5", "Hell's Atrium"},
	{"e4m6", "The Pain Maze"},
	{"e4m7", "Azure Agony"},
	{"e4m8", "The Nameless City"},

	{"end", "Shub-Niggurath's Pit"},			// 31

	{"dm1", "Place of Two Deaths"},				// 32
	{"dm2", "Claustrophobopolis"},
	{"dm3", "The Abandoned Base"},
	{"dm4", "The Bad Place"},
	{"dm5", "The Cistern"},
	{"dm6", "The Dark Zone"}
};

typedef struct
{
	char	*description;
	int		firstLevel;
	int		levels;
} episode_t;

episode_t	episodes[] =
{
	{"Welcome to Quake", 0, 1},
	{"Doomed Dimension", 1, 8},
	{"Realm of Black Magic", 9, 7},
	{"Netherworld", 16, 7},
	{"The Elder World", 23, 8},
	{"Final Level", 31, 1},
	{"Deathmatch Arena", 32, 6}
};

extern cvar_t maxclients, maxspectators;

int	startepisode;
int	startlevel;
int _maxclients, _maxspectators;
int _deathmatch, _teamplay, _skill, _coop;
int _fraglimit, _timelimit;

void M_Menu_GameOptions_f (void)
{
	key_dest = key_menu;
	m_state = m_gameoptions;
	m_entersound = true;

	// 16 and 8 are not really limits --- just sane values
	// for these variables...
	_maxclients = min(16, (int)maxclients.value);
	if (_maxclients < 2) _maxclients = 8;
	_maxspectators = max(0, min((int)maxspectators.value, 8));

	_deathmatch = max (0, min((int)deathmatch.value, 5));
	_teamplay = max (0, min((int)teamplay.value, 2));
	_skill = max (0, min((int)skill.value, 3));
	_fraglimit = max (0, min((int)fraglimit.value, 100));
	_timelimit = max (0, min((int)timelimit.value, 60));
}


int gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 96, 104, 120, 128};
#define	NUM_GAMEOPTIONS	9
int		gameoptions_cursor;

void M_GameOptions_Draw (void)
{
	qpic_t	*p;
	int		x;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_DrawTextBox (152, 32, 10, 1);
	M_Print (160, 40, "begin game");

	M_Print (0, 56, "        game type");
	if (!_deathmatch)
		M_Print (160, 56, "cooperative");
	else
		M_Print (160, 56, va("deathmatch %i", _deathmatch));

	M_Print (0, 64, "         teamplay");
	{
		char *msg;

		switch(_teamplay)
		{
			default: msg = "Off"; break;
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
		}
		M_Print (160, 64, msg);
	}

	if (_deathmatch == 0)
	{
		M_Print (0, 72, "            skill");
		switch (_skill)
		{
		case 0:  M_Print (160, 72, "Easy"); break;
		case 1:  M_Print (160, 72, "Normal"); break;
		case 2:  M_Print (160, 72, "Hard"); break;
		default: M_Print (160, 72, "Nightmare");
		}
	}
	else
	{
		M_Print (0, 72, "        fraglimit");
		if (_fraglimit == 0)
			M_Print (160, 72, "none");
		else
			M_Print (160, 72, va("%i frags", _fraglimit));
		
		M_Print (0, 80, "        timelimit");
		if (_timelimit == 0)
			M_Print (160, 80, "none");
		else
			M_Print (160, 80, va("%i minutes", _timelimit));
	}
	M_Print (0, 96, "       maxclients");
	M_Print (160, 96, va("%i", _maxclients) );
	
	M_Print (0, 104, "       maxspect.");
	M_Print (160, 104, va("%i", _maxspectators) );

	M_Print (0, 120, "         Episode");
    M_Print (160, 120, episodes[startepisode].description);

	M_Print (0, 128, "           Level");
    M_Print (160, 128, levels[episodes[startepisode].firstLevel + startlevel].description);
	M_Print (160, 136, levels[episodes[startepisode].firstLevel + startlevel].name);

// line cursor
	M_DrawCharacter (144, gameoptions_cursor_table[gameoptions_cursor], 12+((int)(realtime*4)&1));
}


void M_NetStart_Change (int dir)
{
	int count;

	switch (gameoptions_cursor)
	{
	case 1:
		_deathmatch += dir;
		if (_deathmatch < 0) _deathmatch = 5;
		else if (_deathmatch > 5) _deathmatch = 0;
		break;

	case 2:
		_teamplay += dir;
		if (_teamplay < 0) _teamplay = 2;
		else if (_teamplay > 2) _teamplay = 0;
		break;

	case 3:
		if (_deathmatch == 0)
		{
			_skill += dir;
			if (_skill < 0) _skill = 3;
			else if (_skill > 3) _skill = 0;
		}
		else
		{
			_fraglimit += dir*10;
			if (_fraglimit < 0) _fraglimit = 100;
			else if (_fraglimit > 100) _fraglimit = 0;
		}
		break;

	case 4:
		_timelimit += dir*5;
		if (_timelimit < 0) _timelimit = 60;
		else if (_timelimit > 60) _timelimit = 0;
		break;

	case 5:
		_maxclients += dir;
		if (_maxclients > 16)
			_maxclients = 2;
		else if (_maxclients < 2)
			_maxclients = 16;
		break;

	case 6:
		_maxspectators += dir;
		if (_maxspectators > 8)
			_maxspectators = 0;
		else if (_maxspectators < 0)
			_maxspectators = 8;
		break;

	case 7:
		startepisode += dir;
		if (registered.value)
			count = 7;
		else
			count = 2;

		if (startepisode < 0)
			startepisode = count - 1;

		if (startepisode >= count)
			startepisode = 0;

		startlevel = 0;
		break;

	case 8:
		startlevel += dir;
		count = episodes[startepisode].levels;

		if (startlevel < 0)
			startlevel = count - 1;

		if (startlevel >= count)
			startlevel = 0;
		break;
	}
}

void M_GameOptions_Key (int key)
{
	char *p;

	switch (key)
	{
	case K_ESCAPE:
//		M_Menu_Net_f ();
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor--;
		if (!_deathmatch && gameoptions_cursor == 4)
			gameoptions_cursor--;
		if (gameoptions_cursor < 0)
			gameoptions_cursor = NUM_GAMEOPTIONS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor++;
		if (!_deathmatch && gameoptions_cursor == 4)
			gameoptions_cursor++;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS)
			gameoptions_cursor = 0;
		break;

	case K_LEFTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menu3.wav");
		M_NetStart_Change (-1);
		break;

	case K_RIGHTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menu3.wav");
		M_NetStart_Change (1);
		break;

	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
//		if (gameoptions_cursor == 0)
		{
			key_dest = key_game;

			// Kill the server, unless we continue playing 
			// deathmatch on another level
			if (!_deathmatch || !deathmatch.value)
				Cbuf_AddText ("disconnect\n");

			if (_deathmatch == 0)
			{
				_coop = 1;
				_timelimit = 0;
				_fraglimit = 0;
			}
			else
				_coop = 0;

			Cvar_Set (&deathmatch, va("%i", _deathmatch));
			Cvar_Set (&skill, va("%i", _skill));
			Cvar_Set (&coop, va("%i", _coop));
			Cvar_Set (&fraglimit, va("%i", _fraglimit));
			Cvar_Set (&timelimit, va("%i", _timelimit));
			Cvar_Set (&teamplay, va("%i", _teamplay));
			Cvar_Set (&maxclients, va("%i", _maxclients));
			Cvar_Set (&maxspectators, va("%i", _maxspectators));

			// FIXME...
			if (_deathmatch)
			{
				p = Info_ValueForKey (svs.info, "*gamedir");
				if (p && !strcmp(p, "single"))
				{
					Cbuf_AddText ("gamedir qw\n");
				}
			}
			else
				Cbuf_AddText ("gamedir single\n");

			Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + startlevel].name) );

			return;
		}

//		M_NetStart_Change (1);
		break;
	}
}
#endif	// QW_BOTH


//=============================================================================
/* SETUP MENU */

int		setup_cursor = 0;
int		setup_cursor_table[] = {40, 56, 80, 104, 140};

char	setup_name[16];
char	setup_team[16];
int		setup_oldtop;
int		setup_oldbottom;
int		setup_top;
int		setup_bottom;

extern cvar_t	name, team;
extern cvar_t	topcolor, bottomcolor;

#define	NUM_SETUP_CMDS	5

void M_Menu_Setup_f (void)
{
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;
	strncpy(setup_name, name.string, sizeof(setup_name));
	setup_name[15] = 0;
	strncpy(setup_team, team.string, sizeof(setup_team));
	setup_team[15] = 0;
	setup_top = setup_oldtop = (int)topcolor.value;
	setup_bottom = setup_oldbottom = (int)bottomcolor.value;
}


void M_Setup_Draw (void)
{
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_Print (64, 40, "Your name");
	M_DrawTextBox (160, 32, 16, 1);
	M_PrintWhite (168, 40, setup_name);

	M_Print (64, 56, "Your team");
	M_DrawTextBox (160, 48, 16, 1);
	M_PrintWhite (168, 56, setup_team);

	M_Print (64, 80, "Shirt color");
	M_Print (64, 104, "Pants color");

	M_DrawTextBox (64, 140-8, 14, 1);
	M_Print (72, 140, "Accept Changes");

	p = Draw_CachePic ("gfx/bigbox.lmp");
	M_DrawTransPic (160, 64, p);
	p = Draw_CachePic ("gfx/menuplyr.lmp");
	M_BuildTranslationTable(setup_top*16, setup_bottom*16);
	M_DrawTransPicTranslate (172, 72, p);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 0)
		M_DrawCharacter (168 + 8*strlen(setup_name), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (168 + 8*strlen(setup_team), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}


void M_Setup_Key (int k)
{
	int		l;

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 2)
			setup_top = setup_top - 1;
		if (setup_cursor == 3)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
//forward:
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 2)
			setup_top = setup_top + 1;
		if (setup_cursor == 3)
			setup_bottom = setup_bottom + 1;
		break;

	case K_ENTER:
//		if (setup_cursor == 0 || setup_cursor == 1)
//			return;

//		if (setup_cursor == 2 || setup_cursor == 3)
//			goto forward;

		// setup_cursor == 4 (OK)
		Cvar_Set (&name, setup_name);
		Cvar_Set (&team, setup_team);
		Cvar_Set (&topcolor, va("%i", setup_top));
		Cvar_Set (&bottomcolor, va("%i", setup_bottom));
		m_entersound = true;
		M_Menu_MultiPlayer_f ();
		break;

	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_name))
				setup_name[strlen(setup_name)-1] = 0;
		}

		if (setup_cursor == 1)
		{
			if (strlen(setup_team))
				setup_team[strlen(setup_team)-1] = 0;
		}
		break;

	default:
		if (k < 32 || k > 127)
			break;
		if (setup_cursor == 0)
		{
			l = strlen(setup_name);
			if (l < 15)
			{
				setup_name[l+1] = 0;
				setup_name[l] = k;
			}
		}
		if (setup_cursor == 1)
		{
			l = strlen(setup_team);
			if (l < 15)
			{
				setup_team[l+1] = 0;
				setup_team[l] = k;
			}
		}
	}

	if (setup_top > 13)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 13;
	if (setup_bottom > 13)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 13;
}


// SLIST -->

#define MENU_X 50
#define MENU_Y 30
#define STAT_X 50
#define STAT_Y 122

int m_multip_cursor=0;
int m_multip_mins;
int m_multip_maxs;
int m_multip_horiz;
int m_multip_state;

void M_Menu_ServerList_f (void) {
	key_dest = key_menu;
	m_entersound = true;
//	m_state = m_multiplayer;
	m_state = m_slist;
//	m_multip_cursor = 0;
	m_multip_mins = 0;
	m_multip_maxs = 10;
	m_multip_horiz = 0;
	m_multip_state = 0;
}

void M_ServerList_Draw (void) {
	int serv;
	int line = 1;
	qpic_t *p;
	//int f;

	M_DrawTransPic(16,4,Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic("gfx/p_multi.lmp");
	M_DrawPic((320-p->width)/2,4,p);

	if (!(slist[0].server)) {
		M_DrawTextBox(60,80,23,4);
		M_PrintWhite(110,12*8,"No server list");
		M_PrintWhite(140,13*8,"found.");
		return;
	}
	M_DrawTextBox(STAT_X,STAT_Y,23,4);
	//M_DrawTextBox(STAT_X+96,STAT_Y+38,12,3);
	M_DrawTextBox(STAT_X,STAT_Y+38,23,3);
	M_DrawTextBox(MENU_X,MENU_Y,23,(m_multip_maxs - m_multip_mins)+1);
	for (serv = m_multip_mins; serv <= m_multip_maxs; serv++) {
		if (slist[serv].server) {
			M_Print(MENU_X+18,line*8+MENU_Y,
				va("%1.21s",
				strlen(slist[serv].description) <= m_multip_horiz ? "" : slist[serv].description+m_multip_horiz));
			line++;
		}
	}
	M_PrintWhite(STAT_X+18,STAT_Y+16,"IP/Hostname:");
	M_Print(STAT_X+18,STAT_Y+24,slist[m_multip_cursor].server);
	M_DrawCharacter(MENU_X+8,(m_multip_cursor - m_multip_mins + 1) * 8+MENU_Y,
		12+((int)(realtime*4)&1));
}

void M_ServerList_Key (key)
{
//	server_entry_t *pt;
	if (!(slist[0].server) && key != K_ESCAPE && key != K_INS)
		return;
	switch(key)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f();
		break;

	case K_UPARROW:
		S_LocalSound("misc/menu1.wav");
		if (m_multip_cursor > 0)
		{
			if (keydown[K_CTRL])
			{
				Server_List_Switch(m_multip_cursor,m_multip_cursor-1);
				m_multip_cursor--;
			}
			else
				m_multip_cursor--;
		}
		break;

	case K_DOWNARROW:
		S_LocalSound("misc/menu1.wav");
		if (keydown[K_CTRL])
		{
			if (m_multip_cursor != Server_List_Len() - 1) {
				Server_List_Switch(m_multip_cursor,m_multip_cursor+1);
				m_multip_cursor++;
			}
		}
		else if (m_multip_cursor < (MAX_SERVER_LIST-1) && slist[m_multip_cursor+1].server) {
			m_multip_cursor++;
		}
		break;

	case K_HOME:
		S_LocalSound("misc/menu1.wav");
		m_multip_cursor = 0;
		break;

	case K_END:
		S_LocalSound("misc/menu1.wav");
		m_multip_cursor = Server_List_Len() - 1;
		break;
		
	case K_PGUP:
		S_LocalSound("misc/menu1.wav");
		m_multip_cursor -= (m_multip_maxs - m_multip_mins);
		if (m_multip_cursor < 0)
			m_multip_cursor = 0;
		break;

	case K_PGDN:
		S_LocalSound("misc/menu1.wav");
		m_multip_cursor += (m_multip_maxs - m_multip_mins);
		if (m_multip_cursor >= MAX_SERVER_LIST)
			m_multip_cursor = MAX_SERVER_LIST - 1;
		while (!(slist[m_multip_cursor].server))
			m_multip_cursor--;
		break;

	case K_RIGHTARROW:
		S_LocalSound("misc/menu1.wav");
		if (m_multip_horiz < 256)
			m_multip_horiz++;
		break;

	case K_LEFTARROW:
		S_LocalSound("misc/menu1.wav");
		if (m_multip_horiz > 0 )
			m_multip_horiz--;
		break;

	case K_ENTER:
		if (keydown[K_CTRL])
		{
			M_Menu_SEdit_f();
			break;
		}
		m_state = m_main;
		M_ToggleMenu_f();
		CL_Disconnect();
		strncpy(cls.servername,slist[m_multip_cursor].server,sizeof(cls.servername)-1);
		CL_BeginServerConnect();
		break;

	case 'e':
	case 'E':
		M_Menu_SEdit_f();
		break;

	case K_INS:
		S_LocalSound("misc/menu2.wav");
		if (Server_List_Len() < (MAX_SERVER_LIST-1)) {
			memmove(&slist[m_multip_cursor+1],
				&slist[m_multip_cursor],
				(Server_List_Len() - m_multip_cursor)*sizeof(slist[0]));
			Server_List_Reset_NoFree(m_multip_cursor);
			Server_List_Set(m_multip_cursor,"127.0.0.1","<BLANK>");
		}
		break;

	case K_DEL:
		S_LocalSound("misc/menu2.wav");
		if (Server_List_Len() > 0) {
			free(slist[m_multip_cursor].server);
			free(slist[m_multip_cursor].description);
			if (Server_List_Len()-1 == m_multip_cursor) {
				Server_List_Reset_NoFree(m_multip_cursor);
				m_multip_cursor = !m_multip_cursor ? 0 : m_multip_cursor-1;

			}
			else {
				memmove(&slist[m_multip_cursor],
				&slist[m_multip_cursor+1],
				(Server_List_Len()-m_multip_cursor-1) * sizeof(slist[0]));
				Server_List_Reset_NoFree(Server_List_Len()-1);
			}
		}
		break;
	default:
		break;
	}
	if (m_multip_cursor < m_multip_mins) {
		m_multip_maxs -= (m_multip_mins - m_multip_cursor);
		m_multip_mins = m_multip_cursor;
	}
	if (m_multip_cursor > m_multip_maxs) {
		m_multip_mins += (m_multip_cursor - m_multip_maxs);
		m_multip_maxs = m_multip_cursor;
	}
}
#define SERV_X 60
#define SERV_Y 64
#define DESC_X 60
#define DESC_Y 40
#define SERV_L 22
#define DESC_L 22

char serv[256];
char desc[256];
int serv_max;
int serv_min;
int desc_max;
int desc_min;
int sedit_state;

void M_Menu_SEdit_f (void) {
	key_dest = key_menu;
	m_entersound = true;
	m_state = m_sedit;
	sedit_state = 0;
	strncpy(serv,slist[m_multip_cursor].server,255);
	serv[strlen(slist[m_multip_cursor].server) + 1] = 0;
	strncpy(desc,slist[m_multip_cursor].description,255);
	desc[strlen(slist[m_multip_cursor].description) + 1] = 0;
	serv_max = strlen(serv) > SERV_L ? strlen(serv) : SERV_L;
	serv_min = serv_max - (SERV_L);
	desc_max = strlen(desc) > DESC_L ? strlen(desc) : DESC_L;
	desc_min = desc_max - (DESC_L);
}

void M_SEdit_Draw (void) {
	qpic_t *p;

	M_DrawTransPic(16,4,Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic("gfx/p_multi.lmp");
	M_DrawPic((320-p->width)/2,4,p);

	M_DrawTextBox(SERV_X,SERV_Y,23,1);
	M_DrawTextBox(DESC_X,DESC_Y,23,1);
	M_PrintWhite(SERV_X,SERV_Y-4,"Hostname/IP:");
	M_PrintWhite(DESC_X,DESC_Y-4,"Description:");
	M_Print(SERV_X+9,SERV_Y+8,va("%1.22s",serv+serv_min));
	M_Print(DESC_X+9,DESC_Y+8,va("%1.22s",desc+desc_min));
	if (sedit_state == 0)
		M_DrawCharacter(SERV_X+9+8*(strlen(serv)-serv_min),
			SERV_Y+8,10+((int)(realtime*4)&1));
	if (sedit_state == 1)
		M_DrawCharacter(DESC_X+9+8*(strlen(desc)-desc_min),
			DESC_Y+8,10+((int)(realtime*4)&1));
}

void M_SEdit_Key (int key) {
	int	l;
	switch (key) {
		case K_ESCAPE:
			M_Menu_ServerList_f ();
			break;
		case K_ENTER:
			Server_List_Set(m_multip_cursor,serv,desc);
			M_Menu_ServerList_f ();
			break;
		case K_UPARROW:
			S_LocalSound("misc/menu1.wav");
			sedit_state = sedit_state == 0 ? 1 : 0;
			break;
		case K_DOWNARROW:
			S_LocalSound("misc/menu1.wav");
			sedit_state = sedit_state == 1 ? 0 : 1;
			break;
		case K_BACKSPACE:
			switch (sedit_state) {
				case 0:
					if ((l = strlen(serv)))
						serv[--l] = 0;
					if (strlen(serv)-6 < serv_min && serv_min) {
						serv_min--;
						serv_max--;
					}
					break;
				case 1:
					if ((l = strlen(desc)))
						desc[--l] = 0;
					if (strlen(desc)-6 < desc_min && desc_min) {
						desc_min--;
						desc_max--;
					}
					break;
				default:
					break;
			}
			break;
		default:
			if (key < 32 || key > 127)
				break;
			switch(sedit_state) {
				case 0:
					l = strlen(serv);
					if (l < 254) {
						serv[l+1] = 0;
						serv[l] = key;
					}
					if (strlen(serv) > serv_max) {
						serv_min++;
						serv_max++;
					}
					break;
				case 1:
					l = strlen(desc);
					if (l < 254) {
						desc[l+1] = 0;
						desc[l] = key;
					}
					if (strlen(desc) > desc_max) {
						desc_min++;
						desc_max++;
					}
					break;
			}
			break;
	}
}

// <-- SLIST

void M_Quit_Draw (void)
{
#define VSTR(x) #x
#define VSTR2(x) VSTR(x)
	char *cmsg[] = {
//    0123456789012345678901234567890123456789
	"0            QuakeWorld",
	"1    version " VSTR2(VERSION) " by id Software",
	"0Programming",
	"1 John Carmack    Michael Abrash",
	"1 John Cash       Christian Antkow",
	"0Additional Programming",
	"1 Dave 'Zoid' Kirsch",
	"1 Jack 'morbid' Mathews",
	"0Id Software is not responsible for",
    "0providing technical support for",
	"0QUAKEWORLD(tm). (c)1996 Id Software,",
	"0Inc.  All Rights Reserved.",
	"0QUAKEWORLD(tm) is a trademark of Id",
	"0Software, Inc.",
	"1NOTICE: THE COPYRIGHT AND TRADEMARK",
	"1NOTICES APPEARING  IN YOUR COPY OF",
	"1QUAKE(r) ARE NOT MODIFIED BY THE USE",
	"1OF QUAKEWORLD(tm) AND REMAIN IN FULL",
	"1FORCE.",
	"0NIN(r) is a registered trademark",
	"0licensed to Nothing Interactive, Inc.",
	"0All rights reserved. Press y to exit",
	NULL };
	char **p;
	int y;

	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

	M_DrawTextBox (0, 0, 38, 23);
	y = 12;
	for (p = cmsg; *p; p++, y += 8) {
		if (**p == '0')
			M_PrintWhite (16, y, *p + 1);
		else
			M_Print (16, y,	*p + 1);
	}
}



//=============================================================================
/* Menu Subsystem */


void M_Init (void)
{
	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);

	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
#ifdef QW_BOTH
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_load", M_Menu_Load_f);
	Cmd_AddCommand ("menu_save", M_Menu_Save_f);
#endif
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
}


void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
		return;

	if (!m_recursiveDraw)
	{
		scr_copyeverything = 1;

		if (scr_con_current)
		{
			Draw_ConsoleBackground (vid.height);
			VID_UnlockBuffer ();
			S_ExtraUpdate ();
			VID_LockBuffer ();
		}
		else
			Draw_FadeScreen ();

		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}

	switch (m_state)
	{
	case m_none:
		break;

	case m_main:
		M_Main_Draw ();
		break;

	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;

	case m_load:
		M_Load_Draw ();
		break;

	case m_save:
		M_Save_Draw ();
		break;

	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_net:
//		M_Net_Draw ();
		break;

	case m_options:
		M_Options_Draw ();
		break;

	case m_keys:
		M_Keys_Draw ();
		break;

	case m_video:
		M_Video_Draw ();
		break;

	case m_help:
		M_Help_Draw ();
		break;

	case m_quit:
		M_Quit_Draw ();
		break;

	case m_serialconfig:
//		M_SerialConfig_Draw ();
		break;

	case m_modemconfig:
//		M_ModemConfig_Draw ();
		break;

	case m_lanconfig:
//		M_LanConfig_Draw ();
		break;

#ifdef QW_BOTH
	case m_gameoptions:
		M_GameOptions_Draw ();
		break;
#endif

	case m_search:
//		M_Search_Draw ();
		break;

	case m_slist:
		M_ServerList_Draw ();
		break;

	case m_sedit:
		M_SEdit_Draw ();
		break;
	}

	if (m_entersound)
	{
		S_LocalSound ("misc/menu2.wav");
		m_entersound = false;
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


void M_Keydown (int key)
{
	switch (m_state)
	{
	case m_none:
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_load:
		M_Load_Key (key);
		return;

	case m_save:
		M_Save_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_net:
//		M_Net_Key (key);
		return;

	case m_options:
		M_Options_Key (key);
		return;

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_video:
		M_Video_Key (key);
		return;

	case m_help:
		M_Help_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_serialconfig:
//		M_SerialConfig_Key (key);
		return;

	case m_modemconfig:
//		M_ModemConfig_Key (key);
		return;

	case m_lanconfig:
//		M_LanConfig_Key (key);
		return;

#ifdef QW_BOTH
	case m_gameoptions:
		M_GameOptions_Key (key);
		return;
#endif

	case m_search:
//		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;

	case m_sedit:
		M_SEdit_Key (key);
		break;
	}
}


