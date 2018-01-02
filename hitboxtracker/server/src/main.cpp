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

struct player_t
{
	struct player_info_t
	{
		bool read_packet  : 1;
		bool request_sync : 1;
	};

	player_info_t &operator[](size_t i)             { return m_data[i - 1]; }
	player_info_t &operator[](edict_t *pEdict)      { return m_data[ENTINDEX(pEdict) - 1]; }
	player_info_t &operator[](CBasePlayer *pPlayer) { return m_data[pPlayer->entindex() - 1]; }

	player_info_t m_data[MAX_CLIENTS];
};

player_t g_Players;
constexpr size_t DIRECTOR_UID = 0xabcdef00;

void ClientCommand_Post(edict_t *pEdict)
{
	const auto cmd = CMD_ARGV(0);
	if (!Q_strcmp(cmd, "__request_sync"))
	{
		// want sync
		g_Players[pEdict].request_sync = true;
	}

	RETURN_META(MRES_IGNORED);
}

void SendPlayerSyncInfo(CBasePlayer *pPlayer)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		auto plr = UTIL_PlayerByIndex(i);
		if (!plr)
			continue;

		if (plr->IsDormant() || plr->has_disconnected)
			continue;

		if (!plr->IsNetClient())
			continue;

		if (g_Players[plr].request_sync)
		{
			MESSAGE_BEGIN(MSG_ONE, SVC_DIRECTOR, nullptr, plr->pev);
				WRITE_BYTE(42);						// number of bytes this message
				WRITE_BYTE(DRC_CMD_TIMESCALE);
				WRITE_LONG(DIRECTOR_UID);			// unique message id
				WRITE_BYTE(pPlayer->entindex());
				WRITE_LONG(*(int *)&pPlayer->m_prevgaitorigin[0]);
				WRITE_LONG(*(int *)&pPlayer->m_prevgaitorigin[1]);
				WRITE_LONG(*(int *)&pPlayer->m_prevgaitorigin[2]);
				WRITE_LONG(*(int *)&pPlayer->m_flYaw);
				WRITE_LONG(*(int *)&pPlayer->m_flPitch);
				WRITE_LONG(*(int *)&pPlayer->m_flGaityaw);
				WRITE_LONG(*(int *)&pPlayer->m_flGaitframe);
				WRITE_LONG(*(int *)&pPlayer->m_flGaitMovement);
				WRITE_LONG(pPlayer->m_iGaitsequence);
			MESSAGE_END();
		}
	}
}

void PlayerPostThink_Post(edict_t *pEdict)
{
	g_Players[pEdict].read_packet = true;
	RETURN_META(MRES_IGNORED);
}

void ClientDisconnect_Post(edict_t *pEdict)
{
	g_Players[pEdict].read_packet  = false;
	g_Players[pEdict].request_sync = false;
	RETURN_META(MRES_IGNORED);
}

union split_double_t
{
	split_double_t() : d (0.0) {}
	split_double_t(double f) : d(f) {}

	double  d;
	struct {
		int32_t lo;
		int32_t hi;
	};
};

void StartFrame_Post()
{
	int count = 0;
	CBasePlayer *plrList[MAX_CLIENTS];

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		auto pPlayer = UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		if (pPlayer->IsDormant() || pPlayer->has_disconnected)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		if (!g_Players[i].read_packet)
		{
			RETURN_META(MRES_IGNORED);
		}

		g_Players[i].read_packet = false;
		plrList[count++] = pPlayer;
	}

	if (count > 0)
	{
		split_double_t time(sv->time), oldtime(sv->oldtime);

		MESSAGE_BEGIN(MSG_ALL, SVC_DIRECTOR);
			WRITE_BYTE(26);								// number of bytes this message
			WRITE_BYTE(DRC_CMD_TIMESCALE);
			WRITE_LONG(DIRECTOR_UID);					// unique message id
			WRITE_BYTE(0);
			WRITE_LONG(time.lo);
			WRITE_LONG(time.hi);
			WRITE_LONG(oldtime.lo);
			WRITE_LONG(oldtime.hi);
			WRITE_LONG(*(int *)&gpGlobals->frametime);
		MESSAGE_END();

		// 26 + (42 * count) the total number of payload bytes
		for (int i = 0; i < count; i++) {
			SendPlayerSyncInfo(plrList[i]);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax)
{
	// HACK HACK, we need sv.oldtime, sv.time
	sv = (server_t *)((const char *)((size_t)gpGlobals->mapname + (size_t)gpGlobals->pStringBase) - offsetof(server_t, name));
	RETURN_META(MRES_IGNORED);
}
