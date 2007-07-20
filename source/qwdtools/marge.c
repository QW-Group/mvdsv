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

    $Id$
*/

#include "defs.h"

#define HEADER (int)&((header_t*)0)->data
#define NextMsg(h) (h) = (header_t *)((h)->data + (h)->size)
#define Cpy(d,s) memcpy((d), (s), HEADER + (s)->size)
#define Cmp(a,b) (((a)->source & (b)->source) == 0 && (a)->size == (b)->size && *(a)->data == *(b)->data && !memcmp((a)->data, (b)->data, (a)->size) && TypeCmp((a),(b)))


void IntegrateMsgs(sizebuf_t *dest)
{
	byte buf[MAX_MSGLEN], *end;
	header_t *p, *t;
	sizebuf_t msg;

	msg.data = buf;

	p = (header_t *)dest->data;

	while ((byte*)p - dest->data < dest->bufsize)
	{
		t = (header_t *)(end = p->data + p->size);
		while ((byte*)t - dest->data < dest->bufsize)
		{
			if (t->type == p->type && t->to == p->to)
			{
				// if msg reached MAX_MSGLEN break
				if (p->size + t->size >= MAX_MSGLEN)
					break;

				memcpy(msg.data, t->data, t->size);
				msg.bufsize = t->size;

				// make room & cpy
				memmove(end + t->size, end, (byte*)t - end);
				memcpy(end, msg.data, msg.bufsize);

				// shrunk the buffer
				memmove((byte*)t + msg.bufsize, (byte*)t + HEADER + msg.bufsize, dest->bufsize - (((byte*)t - dest->data) + HEADER + msg.bufsize));
				p->size += msg.bufsize;
				dest->bufsize -= HEADER;

				t = (header_t *)((byte *)t + msg.bufsize);
			}
			else
				NextMsg(t);
		}

		NextMsg(p);
	}
}

void MoveFromDemoBuffer(source_t *who, int frame, int size)
{
	who->frames[frame&UPDATE_MASK].buf.data += size;
	who->frames[frame&UPDATE_MASK].buf.bufsize -= size;
	who->frames[frame&UPDATE_MASK].buf.maxsize -= size;

	who->dbuffer.start += size;
	if (who->dbuffer.start == who->dbuffer.last)
	{

		if (who->dbuffer.start == who->dbuffer.end)
		{
			who->dbuffer.end = 0; // demobuffer is empty
			who->dbuffer.msgbuf->data = who->dbuffer.data;
			who->dbuffer.msgbuf->bufsize = 0;
		}

		// go back to begining of the buffer
		who->dbuffer.last = who->dbuffer.end;
		who->dbuffer.start = 0;
	}

	who->dbuffer.msgbuf->maxsize = MAXSIZE(&who->dbuffer) + who->dbuffer.msgbuf->bufsize;
}

void SBCpy(sizebuf_t *d, sizebuf_t *s)
{
	d->bufsize = s->bufsize;
	memcpy(d->data,s->data,s->bufsize);

	//FIXME:
	d->h = NULL;
	d->cursize = 0;
}

static int desync_frames;
static int desync_source;

void Mrg(header_t *a, header_t *b, header_t *d, qbool update)
{
	if (!update)
		Cpy(d, a);

	if (b->frame - a->frame > desync_frames)
	{
		desync_frames = b->frame - a->frame;
		desync_source = b->source;
	}
	else if (a->frame - b->frame > desync_frames)
	{
		desync_frames = a->frame - b->frame;
		desync_source = a->source;
	}

	d->frame = a->frame < b->frame ? a->frame : b->frame;
	d->source = a->source | b->source;

	switch (a->type)
	{
	case dem_single:
		switch (b->type)
		{
		case dem_all:
			(d)->type = dem_all;
			(d)->to = 0;
			break;
		case dem_single:
			if (a->to != b->to)
			{
				(d)->type = dem_multiple;
				(d)->to = (1 << a->to) | (1 << b->to);
			}
			break;
		case dem_multiple:
			(d)->type = dem_multiple;
			(d)->to = (1 << a->to) | b->to;
			break;
		default: //FIXME ?
			(d)->type = a->type;
			(d)->to = a->to;
		}
		break;
	case dem_multiple:
		switch (b->type)
		{
		case dem_all:
			(d)->type = dem_all;
			(d)->to = 0;
			break;
		case dem_multiple:
			(d)->type = dem_multiple;
			(d)->to = a->to | b->to;
			break;
		case dem_single:
			(d)->type = dem_multiple;
			(d)->to = a->to | (1 << b->to);
			break;
		default:
			(d)->type = a->type;
			(d)->to = a->to;
		}
		break;
	case dem_stats:
		(d)->type = dem_stats;
		(d)->to = a->to;
		break;
	case dem_all:
		(d)->type = dem_all;
		(d)->to = 0;
		break;
	}
}

qbool TypeCmp(header_t *a, header_t *b)
{
	switch (a->type)
	{
	case dem_stats:
		if (b->type == a->type && b->to == a->to)
			return true;
		return false;
	case dem_single:
		if (b->type == dem_single || b->type == dem_multiple)
			return true;
		return false;
	case dem_multiple:
		if (b->type == dem_single || b->type == dem_multiple)
			return true;
		return false;
	case dem_all:
		if (b->type == dem_all)
			return true;
		return false;
	}

	return false;
}

typedef struct
{
	header_t *start, *end;
	int fin, size;
}
zn_t;

void CpyPrev(source_t *who, header_t **d, int start, int end, int size)
{
	int i;
	sizebuf_t *buf;
	header_t *s, *h;

	for (i = start; i < end; i++)
		if (who->frames[i&UPDATE_MASK].buf.bufsize)
		{
			buf = &who->frames[i&UPDATE_MASK].buf;
			s = (header_t *)buf->data;

			for ( h = s; (byte*)h - (byte*)s < buf->bufsize; NextMsg(h), NextMsg(*d))
				Cpy(*d,h);

			MoveFromDemoBuffer(who, i, buf->bufsize);
		}

	MoveFromDemoBuffer(who, end, size);
}

void CheckMsg(zn_t *zn1, zn_t *zn2, zn_t *znd)//header_t **d, source_t *who, int start, int end)
{
	header_t *c, *t;

	// compare the message with dest
	for (t = znd->start; t != znd->end; NextMsg(t))
		if (Cmp(zn1->end, t))
			break;

	if (t != znd->end)
	{
		int size = ((byte*)zn1->end - (byte*)zn1->start);

		// make room
		memmove((byte*)t + size, t, (byte*)znd->end - (byte*)t);
		znd->end += size;

		// copy
		for (c = zn1->start; c != zn1->end; NextMsg(c), NextMsg(t))
			Cpy(t, c);

		Mrg(zn1->end, t, t, true);

		NextMsg(zn1->end);
		zn1->start = zn1->end;

		return;
	}

	for (t = zn2->start; t != zn2->end; NextMsg(t))
		if (Cmp(zn1->end, t))
			break;

	if (t != zn2->end)
	{

		for (c = zn2->start; c != t; NextMsg(c), NextMsg(znd->end))
			Cpy(znd->end, c);

		for (c = zn1->start; c != zn1->end; NextMsg(c), NextMsg(znd->end))
			Cpy(znd->end, c);

		Mrg(zn1->end, t, znd->end, false);
		NextMsg(znd->end);

		NextMsg(zn1->end);
		NextMsg(t);

		zn1->start = zn1->end;
		zn2->start = t;
	}
	else
	{
		NextMsg(zn1->end);
	}
}

void Marge (sizebuf_t *dest, int start, int end)
{
	zn_t zn1, zn2, znd;
	header_t *d, *t;
	source_t *who;
	sizebuf_t tmp, *m1, *m2;
	byte	buffer[45*MAX_MSGLEN];
	int num, n2, n3, num2;

	if (sworld.fromcount == 1)
	{
		SBCpy(dest,&sources->frames[start&UPDATE_MASK].buf);
		MoveFromDemoBuffer(sources, start, sources->frames[start&UPDATE_MASK].buf.bufsize);
		return;
	}

	desync_frames = desync_source = 0;

	//Sys_Printf("%d %d %d\n", start, end, world.messages.bufsize);
	for (; world.lastmarged < end; world.lastmarged++)
	{
		memset(&tmp, 0, sizeof(tmp));
		tmp.data = buffer;
		tmp.maxsize = sizeof(buffer);

		for (who = sources; who - sources < sworld.fromcount; who++)
		{
			//Sys_Printf("marge: %d, %d\n", world.lastmarged, who - sources);
			m1 = &world.messages;
			m2 = &who->frames[world.lastmarged&UPDATE_MASK].buf;
			//Sys_Printf("%d ", m1->bufsize ? ((header_t*)m1->data)->frame : -1);

			memset(&zn1, 0, sizeof(zn1));
			memset(&zn2, 0, sizeof(zn2));

			zn1.fin = true;//!m1->bufsize;
			zn2.fin = !m2->bufsize;

			zn1.start = (header_t *)m1->data;
			zn1.end = (header_t *)(m1->data + m1->bufsize);
			zn2.start = (header_t *)m2->data;
			zn2.end = (header_t *)m2->data;

			d = (header_t *)tmp.data;
			znd.start = (header_t *)tmp.data;
			znd.end = (header_t *)tmp.data;

			while (!zn2.fin)
			{
				for (t = zn1.start; t != zn1.end; NextMsg(t))
					if (Cmp(zn2.end, t))
						break;

				if (t != zn1.end)
				{
					Mrg(zn2.end, t, t, true);
				}
				else
				{
					Cpy(zn1.end, zn2.end);
					NextMsg(zn1.end);
				}
				NextMsg(zn2.end);

				zn2.fin = ((byte*)zn2.end - m2->data) == m2->bufsize;
			}

			m1->bufsize = (byte*)zn1.end - (byte*)zn1.start;
			MoveFromDemoBuffer(who, world.lastmarged, m2->bufsize);
#if 0
			//Sys_Printf("petla:%d, %d\n", m1->bufsize, m2->bufsize);
			while(!zn1.fin || !zn2.fin)
			{
				//Sys_Printf("petla:%d, %d, %d\n", m1->bufsize, m2->bufsize,(byte*)znd.end - (byte*)znd.start);
				if (!zn1.fin)
					CheckMsg(&zn1, &zn2, &znd);

				if (!zn2.fin)
					CheckMsg(&zn2, &zn1, &znd);


				zn1.fin = ((byte*)zn1.end - m1->data) == m1->bufsize;
				zn2.fin = ((byte*)zn2.end - m2->data) == m2->bufsize;
			}
			//Sys_Printf("koniec\n");

			if (!m2->bufsize)
				zn2.start = zn2.end;

			//Sys_Printf("copy\n");
			// copy what's left
			for (; zn1.start != zn1.end; NextMsg(zn1.start), NextMsg(znd.end))
				Cpy(znd.end,zn1.start);

			for (; zn2.start != zn2.end; NextMsg(zn2.start), NextMsg(znd.end))
				Cpy(znd.end,zn2.start);

			//Sys_Printf("move\n");

			MoveFromDemoBuffer(who, world.lastmarged, m2->bufsize);

			tmp.bufsize = (byte*)znd.end - (byte*)znd.start;
			//Sys_Printf("copy to world %d\n",tmp.bufsize);

			SBCpy(&world.messages, &tmp);
			//Sys_Printf("%d done %d\n", who - sources, world.messages.bufsize);
#endif

		}
		//Sys_Printf("\n");
	}

	// find where messages for current frame ends
	t = d = (header_t*)world.messages.data;
	num = n2 = world.lastwritten;
	n3 = -1;
	//if (world.messages.bufsize)
	//	Sys_Printf ("%d %d:", world.lastwritten, world.lastmarged);
	while ((byte*)t - (byte*)world.messages.data < world.messages.bufsize)
	{
		//Sys_Printf("%d ", t->frame);
		if (t->frame <= world.lastwritten)
		{
			d = t;
			num = n2;
			num2 = n3;
		}

		if (t->frame > n2)
		{
			n2 = t->frame;
			n3 = t->source;
		}

		NextMsg(t);
	}

	if (world.messages.bufsize)
	{
		//Sys_Printf("\n");
		//if (num > world.lastwritten) {
		//	for (num = 0; num < sworld.fromcount; num++)
		//		if (num2
		//	Sys_Printf("%d\n", num - world.lastwritten);
		if (d->frame <= world.lastwritten)
			NextMsg(d);
	}
	//if (desync_frames > 0)
	//	Sys_Printf("%d %d %d\n", world.lastwritten, desync_frames, desync_source);
#if 0
	if (desync_frames > 1)
	{
		//Sys_Printf("%d %d %d\n", world.lastwritten, desync_frames, desync_source);
		for (num = 0; num < sworld.fromcount; num++)
			if (desync_source & (1 << num))
				sources[num].sync -= 0.0005*desync_frames;
	}
#endif

	dest->bufsize = (byte*)d - (byte*)world.messages.data;
	//Sys_Printf("here:%d, %d\n", dest->bufsize, world.messages.bufsize);
	memcpy(dest->data,world.messages.data, dest->bufsize);
	memmove(world.messages.data, &world.messages.data[dest->bufsize],world.messages.bufsize - dest->bufsize);
	world.messages.bufsize -= dest->bufsize;

	//Sys_Printf("out\n");

	IntegrateMsgs(dest);
	//Sys_Printf("integrated\n");
}


#if 0
void Marge (sizebuf_t *dest, int start, int end)
{
	zn_t zn1, zn2, zn3;
	header_t *d;
	source_t *who;
	sizebuf_t tmp, *m1, *m2, *m3;
	byte	buffer[45*MAX_MSGLEN];
	int num;

	memset(&tmp, 0, sizeof(tmp));
	tmp.data = buffer;
	tmp.maxsize = sizeof(buffer);

	SBCpy(dest,&sources->frames[start&UPDATE_MASK].buf);

	// mark place after this message as free
	//MoveFromDemoBuffer(sources, who->frames[world.lastwritten&UPDATE_MASK].buf.bufsize);

	for (who = sources + 1; who - sources < sworld.fromcount; who++)
	{
		m1 = dest;
		m2 = &who->frames[start&UPDATE_MASK].buf;

		memset(&zn1, 0, sizeof(zn1));
		memset(&zn2, 0, sizeof(zn2));

		zn1.fin = !m1->bufsize;
		zn2.fin = !m2->bufsize;

		(byte*)zn1.start = m1->data;
		(byte*)zn1.end = m1->data;
		(byte*)zn2.start = m2->data;
		(byte*)zn2.end = m2->data;

		(byte*)d = tmp.data;

		while(!zn1.fin || !zn2.fin)
		{
			if (!zn1.fin)
				CheckMsg(&zn1, &zn2, &d, NULL,0,0);

			if (!zn2.fin)
				CheckMsg(&zn2, &zn1, &d, NULL,0,0);

			zn1.fin = ((byte*)zn1.end - m1->data) == m1->bufsize;
			zn2.fin = ((byte*)zn2.end - m2->data) == m2->bufsize;
		}

		// mark place after this message as free
		MoveFromDemoBuffer(who, start, (byte*)zn2.start - m2->data);

		if (who - sources == 1)
		{
			MoveFromDemoBuffer(sources, start, (byte*)zn1.start - m1->data);
			for (num = start + 1; zn2.start != zn2.end && num < end; num++)
			{
				m3 = &sources->frames[num&UPDATE_MASK].buf;
				memset(&zn3, 0, sizeof(zn3));

				(byte*)zn3.start = m3->data;
				(byte*)zn3.end = m3->data;

				while ((byte*)zn3.end - (byte*)zn3.start < m3->bufsize)
					CheckMsg(&zn3, &zn2, &d, sources, start, num);

			}

			if (!sources->frames[start&UPDATE_MASK].buf.bufsize)
				zn1.start = zn1.end;
		}

		MoveFromDemoBuffer(who, start, (byte*)zn2.start - m2->data);

		for (num = start + 1; zn1.start != zn1.end && num < end; num++)
		{
			m3 = &who->frames[num&UPDATE_MASK].buf;
			memset(&zn3, 0, sizeof(zn3));

			(byte*)zn3.start = m3->data;
			(byte*)zn3.end = m3->data;

			while ((byte*)zn3.end - (byte*)zn3.start < m3->bufsize)
				CheckMsg(&zn3, &zn1, &d, who, start, num);

		}

		if (!m2->bufsize)
			zn2.start = zn2.end;

		// copy what's left
		for (; zn1.start != zn1.end; NextMsg(zn1.start), NextMsg(d))
			Cpy(d,zn1.start);
		for (; zn2.start != zn2.end; NextMsg(zn2.start), NextMsg(d))
			Cpy(d,zn2.start);

		MoveFromDemoBuffer(who, start, m2->bufsize);

		tmp.bufsize = (byte*)d - tmp.data;
		SBCpy(dest, &tmp);
	}

	MoveFromDemoBuffer(sources, start, sources->frames[start&UPDATE_MASK].buf.bufsize);

	IntegrateMsgs(dest);
}
#endif
