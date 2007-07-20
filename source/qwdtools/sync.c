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

void ReadPackets (void);

typedef struct
{
	int		running;
	float	time;
	float	diff;
	float	ratio;
	qbool synced;
} info_t;

static info_t	info[MAX_CLIENTS];
static sizebuf_t dummy;

enum {T_ENT, T_CL};

void *FindEntity(int type, frame_t *f, int num, int frame)
{
	//f = &d->frames[frame&UPDATE_MASK];
	f += frame&UPDATE_MASK;

	switch (type)
	{
	case T_ENT:
		{
			packet_entities_t *p;
			int i;

			if (f->invalid)
				return NULL;

			p = &f->packet_entities;
			for (i = 0; i < p->num_entities; i++)
				if (p->entities[i].number == num)
					return &p->entities[i];

			return NULL;
		}

	case T_CL:
		if (f->playerstate[num].messagenum == frame)
			return &f->playerstate[num];

		return NULL;
	default:
		return NULL;
	}
}

qbool app (vec3_t o1, vec3_t po1, vec3_t o2, vec3_t po2, float *sec, float t1, float t2)
{
	int i,j;

	for (i = 0; i < 3; i++)
		if (o1[i] - po1[i] != 0)
			break;

	for (j = 0; j < 3; j++)
		if (o2[j] - po2[j] != 0)
			break;

	if (i == 3)
		return false;

	if (j == 3)
		return false;

	if (po1[i] >= o2[i] && o1[i] <= o2[i] && po1[i] <= po2[i])
	{
		*sec = (o2[i] - o1[i])*t1/(po1[i] - o1[i]);
		//Sys_Printf("app:%f, %f, %f %f (%f %f)\n", o1[i], o2[i], po1[i], po2[i], t1, t2);
		return true;
	}

	if (po1[i] <= o2[i] && o1[i] >= o2[i] && po1[i] >= po2[i])
	{
		*sec = (o2[i] - o1[i])*t1/(o1[i] - po1[i]);
		//Sys_Printf("app:%f, %f, %f %f (%f %f)\n", o1[i], o2[i], po1[i], po2[i], t1, t2);
		return true;
	}

	if (po2[j] >= o1[j] && o2[j] <= o1[j] && po2[j] <= po1[j])
	{
		*sec = (o1[j] - o2[j])*t2/(po2[j] - o2[j]);
		//Sys_Printf("app:%f, %f, %f %f (%f %f)\n", o1[i], o2[i], po1[i], po2[i], t1, t2);
		return true;
	}

	if (po2[j] <= o1[j] && o2[j] >= o1[j] && po2[j] >= po1[j])
	{
		*sec = (o1[j] - o2[j])*t2/(o2[j] - po2[j]);
		//Sys_Printf("app:%f, %f, %f %f (%f %f)\n", o1[i], o2[i], po1[i], po2[i], t1, t2);
		return true;
	}

	//Sys_Printf("false\n");

	return false;
}

#define depth 3
float demcmp(source_t *d1, source_t *d2)
{
	frame_t *f1, *f2;
	packet_entities_t *e1, *e2;
	player_state_t *p1, *p2, *op1, *op2;
	entity_state_t *pe1, *pe2;
	int i,j, c, k, p;
	float	sr;
	vec3_t vec;

	if (!d1->running)
		return -1;
	if (!d2->running)
		return -1;

	f1 = &d1->frames[d1->parsecount&UPDATE_MASK];
	f2 = &d2->frames[d2->parsecount&UPDATE_MASK];

	if (f1->invalid)
		return -1;
	if (f2->invalid)
		return -1;

	c = 0;
	sr = 0;
	p = 0;
	// compare players
	p1 = f1->playerstate;
	p2 = f2->playerstate;
	for (i = 0; i < MAX_CLIENTS; i++, p1++, p2++)
	{
		if (p1->messagenum != d1->parsecount)
			continue;
		if (p2->messagenum != d2->parsecount)
			continue;

		//VectorSubtract(p1->origin, p2->origin, vec);

		for (k = 1; k < depth; k++)
		{
			op1 = (player_state_t*)FindEntity(T_CL, d1->frames, i, d1->parsecount - k);

			if (!op1)
				continue;

			VectorSubtract(p1->origin, op1->origin, vec);
			if (VectorLength(vec) > 0 && VectorLength(vec) < 100)
				break;
		}

		if (k == depth)
			op1 = NULL;

		for (k = 1; k < depth; k++)
		{
			op2 = (player_state_t*)FindEntity(T_CL, d2->frames, i, d2->parsecount - k);

			if (!op2)
				continue;

			VectorSubtract(p2->origin, op2->origin, vec);
			if (VectorLength(vec) > 0 && VectorLength(vec) < 100)
				break;
		}

		if (k == depth)
			op2 = NULL;

		if (op1 && op2)
		{
			vec3_t vec2;

			VectorSubtract(p1->origin, p2->origin, vec);
			VectorSubtract(p1->origin, op1->origin, vec2);
			//Sys_Printf("ent:%d %f\n",e1->entities[i].number, VectorLength(vec)/VectorLength(vec2));
			//return VectorLength(vec)/VectorLength(vec2);
			sr += VectorLength(vec)/VectorLength(vec2);
			c++;
		}

		//sr += VectorLength(vec);
		//c++;
		//p++;
		//if (VectorLength(vec))
		//	Sys_Printf("cl:%d %f %s\n", i, VectorLength(vec), d1->players[i].name);
	}

	// compare entities
	i = 0;
	j = 0;

	e1 = &f1->packet_entities;
	e2 = &f2->packet_entities;

	while (i < e1->num_entities && j < e2->num_entities)
	{
		if (e1->entities[i].number < e2->entities[j].number)
			i++;
		else if (e1->entities[i].number > e2->entities[j].number)
			j++;
		else
		{
			for (k = 1; k < depth; k++)
			{
				pe1 = (entity_state_t*)FindEntity(T_ENT, d1->frames, e1->entities[i].number, d1->parsecount - k);

				if (!pe1)
					continue;

				VectorSubtract(e1->entities[i].origin, pe1->origin, vec);
				if (VectorLength(vec) > 0)
					break;
			}

			if (k == depth)
				pe1 = NULL;

			for (k = 1; k < depth; k++)
			{
				pe2 = (entity_state_t*)FindEntity(T_ENT, d2->frames, e2->entities[j].number, d2->parsecount - k);

				if (!pe2)
					continue;

				VectorSubtract(e2->entities[j].origin, pe2->origin, vec);
				if (VectorLength(vec) > 0)
					break;
			}
			if (k == depth)
				pe2 = NULL;

			if (pe1 && pe2)
			{
				vec3_t vec2;

				VectorSubtract(e1->entities[i].origin, e2->entities[j].origin, vec);
				VectorSubtract(e1->entities[i].origin, pe1->origin, vec2);
				//Sys_Printf("ent:%d %f\n",e1->entities[i].number, VectorLength(vec)/VectorLength(vec2));
				//return VectorLength(vec)/VectorLength(vec2);
				sr += VectorLength(vec)/VectorLength(vec2);
				c++;
			}

			i++;
			j++;
		}
	}

	if (!c)
		return -1;

	return sr/c;
}

qbool SetComparisionDemo(source_t *dem, float time1)
{
	int num;

	from = dem;
	num = from - sources;

	dem->running = (1 << (dem - sources));
	if (from->lastframe > time1) // rewind
	{
		from->netchan.incoming_sequence = 0;
		from->netchan.incoming_acknowledged= 0;
		from->netchan.incoming_reliable_acknowledged = 0;
		from->prevtime = 0;
		from->time = from->worldtime = from->lastframe = from->lasttime = 0;
		from->latency = 0;
		from->parsecount = 0;
		rewind(sworld.from[from-sources].file);
		from->lastframe = -1;
		ReadPackets();
	}

	if (time1 > (from->lastframe))
		from->lastframe = time1;

	ReadPackets();

	return from->running != 0;
}


qbool Synchronize (void)
{
	int i, j, bad;
	info_t *p;
	source_t *dem1, *dem2;
	float prevdesync = 0.0, before = 0.0, after = 0.0, diff, desync = 0.0, f = 0.0, newdesync = 0.0, x = 0.0, y = 0.0, mindiff = 0.0, m = 0.0, sr = 0.0, c = 0.0, sr2 = 0.0, sec2 = 0.0, test = 0.0, test2 = 0.0;
	byte buf[15*MAX_MSGLEN];
	qbool done;

	if (sworld.fromcount == 1)
		return true;

	sworld.options |= O_QWDSYNC;

	memset(&dummy, 0, sizeof(dummy));
	dummy.data = buf;
	dummy.maxsize = sizeof(buf);

	msgbuf = &dummy;
	world.time = 0;

	// get starting info about demos
	for (p = info, from = sources; from - sources < sworld.fromcount; from++, p++)
	{
		if (!from->running)
			continue;

		rewind(sworld.from[from-sources].file);
		memset(&from->netchan, 0, sizeof(from->netchan));
		from->latency = from->worldtime = from->lastframe = from->lasttime = 0.0; // float
		from->prevtime = 0; // long
		from->parsecount = 0; // int

		p->running = from->running;
		p->diff = 0;
		p->synced = false;
		from->lastframe = -1;
		ReadPackets();
		p->time = from->time;
		p->ratio = 1;
	}

	dem1 = sources;
	info[0].synced = true;

	// now read another demo and try to compare
	for (i = 1, dem2 = sources + 1; i < sworld.fromcount; dem2++, i++)
	{
		if (!dem2->running)
			continue;

		// initial guess for time desync
		x = 1;
		y = info[i].time - info[0].time;
		// rewind dem1
		if (!SetComparisionDemo(dem1, info->time))
		{
			sworld.options -= O_QWDSYNC;
			return false;
		}

		// read 10 sec from dem2
		while (dem1->running)
		{
			j = 0;
			diff = 0;
			if (!dem2->running)
				SetComparisionDemo(dem2, info[i].time);

			while (dem1->running && dem2->running && dem2->time - dem1->time < x + y)
			{
				if (diff <0)
				{
					f = dem2->time - dem1->time;

					from = dem1;
					from->lastframe += 1;
					ReadPackets();

					from = dem2;
					while (from->running && from->time < dem1->time + f)
						ReadPackets();
				}
				else
				{
					from = dem2;
					ReadPackets();
				}

				diff = demcmp(dem1, dem2);

				//Sys_Printf("diff:%f, %f, %f %f\n", diff, dem2->worldtime - dem1->worldtime, dem1->worldtime, dem2->worldtime);

				if (diff < 0 || diff > 5)
					continue;

				f = dem2->time + 1;
				desync = dem1->time - dem2->time;
				from = dem2;

				//Sys_Printf("diff:%f, desync:%f\n", diff, desync);
				while (from->running && from->time < f)
				{
					ReadPackets();
					m = demcmp(dem1, dem2);
					if (m < 0 || m >= diff)
						continue;

					diff = m;
					desync = dem1->time - dem2->time;
				}

				//Sys_Printf("diff:%f, desync:%f\n", diff, desync);

				bad = 0;
				j = 0;
				sr = 0;
				c = 0;
				sr2 = 0;

				//dem1->lastframe += 1;
				test = desync;

				while (dem1->running && dem2->running && bad < 5 && j < 30)
				{
					from = dem1;
					//from->lastframe += 0.5;
					f = from->time + 0.5;
					while (from->running && from->time < f)
						ReadPackets();

					from = dem2;
					f = -1;
					m = -1;
					while (from->running && from->time + desync < dem1->time + 0.25)
					{
						ReadPackets();
						m = demcmp(dem1, dem2);
						if ( m < 0)
							continue;

						if (f < 0 || f > m)
						{
							f = m;
							newdesync = dem1->time - dem2->time;
						}
					}

					//m = demcmp(dem1, dem2);

					if (f < 0)
						continue;

					//Sys_Printf("diff:%f\n", f);
					if (f > 1)
					{
						bad++;
						continue;
					}

					m = newdesync - test;
					test += m < 0 ? max(-0.025, m) : min(0.025, m);

					if (!f)
						f = 0.01f;

					f = 1;
					sr2 += 1/(f*f);
					sr += newdesync/(f*f);
					c++;

					desync = test;

					if (c > 10)
						desync = sr/sr2;

					j++;
				}

				//Sys_Printf("\n%f %d %d, %d, %f\n",desync, j , bad, c, x);

				if (bad == 5)
				{
					diff = 0;
					continue;
				}

				if (j == 30)
					break;
			}

			if (j == 30)
			{
				Sys_Printf("sync1:%f %f\n", desync, sr/sr2);
				if (c)
					desync = sr/sr2;

				desync = test;
				before = dem2->time;
				prevdesync = desync;


				sr = 0;
				c = 0;
				sr2 = 0;
				j = 0;
#if 1
				//desync = 0.5;
				if (desync < 0)
				{
					SetComparisionDemo(dem1, info->time - desync + 2);
					SetComparisionDemo(dem2, 0);
				}
				else
				{
					SetComparisionDemo(dem1, info->time + 2);
					SetComparisionDemo(dem2, info[i].time + desync);
				}

				test = desync;

				while (dem1->running && dem2->running && j < 800)
				{
					from = dem1;
					f = from->time + 2;
					while (from->running && from->time < f)
						ReadPackets();
					//from->lastframe += 1;
					//ReadPackets();

					from = dem2;
					f = -999;
					m = -999;
					sec2 = 0;
					while (from->running && from->time + desync < dem1->time + 1)
					{
						ReadPackets();
						sec2 = 0;

						m = demcmp(dem1, dem2);
						if ( m < 0)
							continue;

						if (f < 0 || f > m)
						{
							f = m;
							newdesync = dem1->time - dem2->time;
						}

					}

					if (fabs(f) < 1)
					{
						test2 = 0;
						m = newdesync - test;
						test += m < 0 ? max(-0.025, m) : min(0.025, m);

						//Sys_Printf("%f %f %f %f, %f %f\n",newdesync, f, desync, test, dem1->time, dem1->time - newdesync);
						if (!f)
							f = 0.1f;
						f = 1;
						sr2 += 1/(f*f);
						sr += newdesync/(f*f);
						c++;
					}
					else if (f > 4)
					{
						test2++;
						if (test2 == 5)
							Sys_Printf("lag:%f, %d\n", dem1->worldtime, dem2-sources);

					}

					if (f < 0)
						continue;

					if (c > 10)
						//desync = sr/sr2;
						desync = test;

					after = dem2->time;
					j++;
				}
				//Sys_Printf("j:%d sync:%f\n", j, desync);

				if (c)
				{
					//desync = sr/sr2;
					desync = test;
					//Sys_Printf("time:%f, b:%f, a:%f, p:%f, d:%f\n", info[i].time, before, after, prevdesync, desync);
					info[i].ratio = (after - before + desync - prevdesync)/(after - before);
					//Sys_Printf("ratio:%f\n", info[i].ratio);
					f = (desync - prevdesync)/(after - before);
					desync += f*(info[i].time - after);
				}


#endif
				info[i].diff = desync;
				info[i].synced = true;
				if (mindiff > desync)
					mindiff = desync;
				break;
			}

			//Sys_Printf("another try\n");
			x *= 2;
			SetComparisionDemo(dem1, info->time + x);
			SetComparisionDemo(dem2, info[i].time);
		}

	}

	done = true;
	diff = -1;
	for (i = 0, p = info, from = sources; i < sworld.fromcount; i++, p++, from++)
	{
		if (!p->running)
			continue;

		from->running = p->running;
		rewind(sworld.from[i].file);
		from->netchan.incoming_sequence = 0;
		from->netchan.incoming_acknowledged = 0;
		from->sync = p->diff - mindiff;
		from->worldtime = from->latency = from->time = from->lasttime = from->lastframe = 0;
		from->prevtime = 0;
		from->parsecount = 0;
		from->ratio = p->ratio;
		if (!p->synced)
		{
			Sys_Printf(" couldn't synchronize %s\n", sworld.from[i].name);
			done = false;
		}
		else
			Sys_Printf(" time offset:%f, time ratio:%f\n", from->sync, from->ratio);

		from->lastframe = -1;
		ReadPackets();
		if (diff < from->worldtime)
			diff = from->worldtime;
	}

	world.signonstats = true;
	for (from = sources; from - sources < sworld.fromcount; from++)
	{
		while (from->running && from->worldtime < diff)
			ReadPackets();
	}

	world.signonstats = false;
	sworld.options -= O_QWDSYNC;

	return done;
}
