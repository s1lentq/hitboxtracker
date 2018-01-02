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

float g_lastFOV;
bool  g_bHoldingKnife;
bool  g_bHoldingShield;

int Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion)
{
	g_pEngfuncs = pEnginefuncs;
	Q_memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

	g_pClientfuncs->pHudInitFunc = HUD_Init;

	return gClientfuncs.pInitFunc(pEnginefuncs, iVersion);
}

void HUD_Init()
{
	g_pClientfuncs->pHudFrame = HUD_Frame;
	g_pClientfuncs->pPostRunCmd = HUD_PostRunCmd;
	g_pClientfuncs->pStudioInterface = HUD_GetStudioModelInterface;
	g_pClientfuncs->pDirectorMessage = HUD_DirectorMessage;

	gClientfuncs.pHudInitFunc();
	gHUD.Init();
}

void HUD_Frame(double time)
{
	net_status_t st;
	gEngfuncs.pNetAPI->Status(&st);

	const auto delay = 0.5;
	if (st.connected == 1 && st.connection_time > (delay - 0.1) && st.connection_time < delay)
	{
		// tell the server that i want to sync
		gEngfuncs.pfnServerCmd("__request_sync");
	}

	gClientfuncs.pHudFrame(time);
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

constexpr size_t DIRECTOR_UID = 0xabcdef00;

void HUD_DirectorMessage(int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	auto cmd = READ_BYTE();
	if (cmd != DRC_CMD_TIMESCALE)
	{
		gClientfuncs.pDirectorMessage(iSize, pbuf);
		return;
	}

	// make sure that this is our custom message
	auto uid = READ_LONG();
	if (uid != DIRECTOR_UID)
	{
		gClientfuncs.pDirectorMessage(iSize, pbuf);
		return;
	}

	auto index = READ_BYTE();

	// header packet
	if (index == 0)
	{
		// parse time
		split_double_t time, oldtime;

		time.lo      = READ_LONG();
		time.hi      = READ_LONG();

		oldtime.lo   = READ_LONG();
		oldtime.hi   = READ_LONG();

		sv.time      = time.d;
		sv.oldtime   = oldtime.d;
		sv.frametime = READ_FLOAT();
	}
	else
	{
		auto &sync = g_PlayerSyncInfo[index];

		// save current values of fields from server-side
		sync.prevgaitorigin[0] = READ_FLOAT();
		sync.prevgaitorigin[1] = READ_FLOAT();
		sync.prevgaitorigin[2] = READ_FLOAT();

		sync.yaw               = READ_FLOAT();
		sync.pitch             = READ_FLOAT();

		sync.gaityaw           = READ_FLOAT();
		sync.gaitframe         = READ_FLOAT();
		sync.gaitmovement      = READ_FLOAT();
		sync.gaitsequence      = READ_LONG ();
	}
}

void HUD_WeaponsPostThink(local_state_t *from, local_state_t *to, usercmd_t *cmd, double time, unsigned int random_seed)
{
	g_bHoldingKnife  = (from->client.m_iId == WEAPON_KNIFE);
	g_bHoldingShield = (from->client.iuser3 & PLAYER_HOLDING_SHIELD) == PLAYER_HOLDING_SHIELD;
}

void R_ForceCVars(qboolean multiplayer)
{
	// nothing to do here
	// block
}
