/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/

#include "precompiled.h"

server_t *sv;

int AddToFullPack_Post(entity_state_t *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)
{
	if (ent->v.modelindex <= 0 || ent->v.modelindex >= MAX_MODELS)
	{
		RETURN_META_VALUE(MRES_IGNORED, 0);
	}

	// we need to call every time SV_StudioSetupBones
	// to keep gaityaw and other synced on client-side
	TraceResult tr;
	TRACE_LINE(ent->v.origin - Vector(1, 1, 1), host->v.origin + Vector(1, 1, 1), 0, nullptr, &tr);

	CBasePlayer *plr = UTIL_PlayerByIndexSafe(e);
	if (plr)
	{
		state->vuser1[0] = plr->m_flYaw;
		state->vuser1[1] = plr->m_flGaityaw;
		state->vuser1[2] = plr->m_flGaitframe;

		state->vuser2[0] = plr->m_flGaitMovement;
		state->vuser2[1] = plr->m_iGaitsequence;
		state->vuser2[2] = plr->m_flPitch;

		state->maxs[0] = plr->m_prevgaitorigin[0];
		state->maxs[1] = plr->m_prevgaitorigin[0];
		state->maxs[2] = plr->m_prevgaitorigin[0];

		state->mins[0] = sv->time;
		state->mins[1] = sv->oldtime;
		state->mins[2] = gpGlobals->frametime;
	}

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax)
{
	// HACK HACK, we need sv.oldtime, sv.time
	sv = (server_t *)((const char *)((size_t)gpGlobals->mapname + (size_t)gpGlobals->pStringBase) - offsetof(server_t, name));
	RETURN_META(MRES_IGNORED);
}
