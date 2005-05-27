/*
 * sv_mod_frags.c
 * QuakeWorld message definitions
 * For glad & vvd
 * (C) kreon 2005
 * Messages from fuhquake's fragfile.dat
 */
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

	$Id: sv_mod_frags.c,v 1.1 2005/05/27 15:38:33 vvd0 Exp $
*/

#include "qwsvdef.h"
#define BREAK_SYM "\\"
/*
 * Weapon structures
 * It defines weapon number and fragfile message
 * Like a:
 * doom/doom/die
 * doom/del/axe
 * etc...
 */

char *qweapon[] = {
    "die", 
    "axe",
    "sg",
    "ssg",
    "ng",
    "sng",
    "gl",
    "rl",
    "lg",
    "rail",
    "drown",
    "trap",
    "tele",
    "dschrge",
    "squish",
    "fall",
    "team"
};

/*
 * MOD messages templates
 * Standart: first - frag, second - fragger
 * Reverse: first - fragger, second - frag
 * Like a: kreon rips glad a new one
 * And: kreon accepts glad's shaft
 */

struct qw_msg
{
    int wp_num;
    int pl_num; // 1 or 2;
    char *str;
    int reverse;
}
qwmsg[] =
{
    {10, 1, " sleeps with the fishes", 0},
    {10, 1, " sucks it down", 0},
    {10, 1, " gulped a load of slime", 0},
    {10, 1, " can't exist on slime alone", 0},
    {10, 1, " burst into flames", 0},
    {10, 1, " turned into hot slag", 0},
    {10, 1, " visits the Volcano God", 0},
    {15, 1, " cratered", 0},
    {15, 1, " fell to his death", 0},
    {15, 1, " fell to her death", 0},
    {11, 1, " blew up", 0},
    {11, 1, " was spiked", 0},
    {11, 1, " was zapped", 0},
    {11, 1, " ate a lavaball", 0},
    {12, 1, " was telefragged by his teammate", 0},
    {12, 1, " was telefragged by her teammate", 0},
    { 0, 1, " died", 0},
    { 0, 1, " tried to leave", 0},
    {14, 1, " was squished", 0},
    { 0, 1, " suicides", 0},
    { 6, 1, " tries to put the pin back in", 0},
    { 7, 1, " becomes bored with life", 0},
    { 7, 1, " discovers blast radius", 0},
    {13, 1, " electrocutes himself.", 0},
    {13, 1, " electrocutes herself.", 0},
    {13, 1, " discharges into the slime", 0},
    {13, 1, " discharges into the lava", 0},
    {13, 1, " discharges into the water", 0},
    {13, 1, " heats up the water", 0},
    {16, 1, " squished a teammate", 0},
    {16, 1, " mows down a teammate", 0},
    {16, 1, " checks his glasses", 0},
    {16, 1, " checks her glasses", 0},
    {16, 1, " gets a frag for the other team", 0},
    {16, 1, " loses another friend", 0},
    { 1, 2, " was ax-murdered by \\", 0},
    { 2, 2, " was lead poisoned by \\", 0},
    { 2, 2, " chewed on \\'s boomstick", 0},
    { 3, 2, " ate 8 loads of \\'s buckshot", 0},
    { 3, 2, " ate 2 loads of \\'s buckshot", 0},
    { 4, 2, " was body pierced by \\", 0},
    { 4, 2, " was nailed by \\", 0},
    { 5, 2, " was perforated by \\", 0},
    { 5, 2, " was punctured by \\", 0},
    { 5, 2, " was ventilated by \\", 0},
    { 5, 2, " was straw-cuttered by \\", 0},
    { 6, 2, " eats \\'s pineapple", 0},
    { 6, 2, " was gibbed by \\'s grenade", 0},
    { 7, 2, " was smeared by \\'s quad rocket", 0},
    { 7, 2, " was brutalized by \\'s quad rocket", 0},
    { 7, 2, " rips \\ a new one", 1},
    { 7, 2, " was gibbed by \\'s rocket", 0},
    { 7, 2, " rides \\'s rocket", 0},
    { 8, 2, " accepts \\'s shaft", 0},
    { 9, 2, " was railed by \\", 0},
    {12, 2, " was telefragged by \\", 0},
    {14, 2, " squishes \\", 1},
    {13, 2, " accepts \\'s discharge", 0},
    {13, 2, " drains \\'s batteries", 0},
    { 8, 2, " gets a natural disaster from \\", 0},
    { 0, 0, NULL, 0}
};

static char *x_strncpy(char *src, int len)
{
    char *ret_val;
    ret_val = (char *)Q_Malloc(++len);
    strlcpy(ret_val, src, len);
    return ret_val;
}

// checks lexem...
char* x_check_lex(char *str, struct qw_msg *lex)
{
    char *str_buf;
    char *nick_1, *nick_2;
    int nick_1_len, nick_2_len, len;
    switch (lex->pl_num)
    {
    case 1:
        if (str_buf = strstr(str, lex->str))
        {
            nick_1_len = strlen(str) - strlen(lex->str);
            nick_1 = x_strncpy(str, nick_1_len);
            len = strlen(nick_1) * 2 + strlen(qweapon[lex->wp_num]) + 4;
            str_buf = (char *)Q_Malloc(len);
            snprintf(str_buf, len, "%s/%s/%s\n", nick_1, nick_1, qweapon[lex->wp_num]);
            Q_Free(nick_1);
            return str_buf;
        }
        break;
    case 2:
        {
        char *tok_1, *tok_2, *tok;

        tok_1 = x_strncpy(lex->str, strlen(lex->str));
        if (!(tok = strtok(tok_1, BREAK_SYM)))
        {
            Q_Free(tok_1);
            return NULL;
        }

        tok_2 = x_strncpy(tok, strlen(tok));
        if (str_buf = strstr(str, tok_2))
        {
            nick_1_len = str_buf - str;
            nick_1 = x_strncpy(str, nick_1_len);
            tok = strtok(NULL, BREAK_SYM);
            if (tok)
            {
                tok = x_strncpy(tok, strlen(tok));
                Q_Free(tok_1);
                tok_1 = tok;
                if(strcmp(str + strlen(str) - strlen(tok_1), tok_1))
                {
                    Q_Free(tok_1);
                    Q_Free(tok_2);
                    return NULL;
                }
            }
            else
                *tok_1 = '\0';

            nick_2_len = strlen(str) - nick_1_len - strlen(tok_1) - strlen(tok_2);
            nick_2 = x_strncpy(str + nick_1_len + strlen(tok_2), nick_2_len);
            if (lex->reverse)
            {
                tok = nick_1;
                nick_1 = nick_2;
                nick_2 = tok;
            }
            len = strlen(nick_1) + strlen(nick_2) + strlen(qweapon[lex->wp_num]) + 4;
            str_buf = (char *)Q_Malloc(len);
            snprintf(str_buf, len, "%s/%s/%s\n", nick_1, nick_2, qweapon[lex->wp_num]);
            Q_Free(tok_1);
            Q_Free(tok_2);
            Q_Free(nick_1);
            Q_Free(nick_2);
            return str_buf;
        }
        Q_Free(tok_1);
        Q_Free(tok_2);
        break;
    default:;
        }
    }
    return NULL;
}

char *parse_mod_string(char *str)
{
    int i;
    char *ret;
    for (i = 0; qwmsg[i].pl_num; i++)
        if (ret = x_check_lex(str, &qwmsg[i]))
            return ret;
    return NULL;
}
