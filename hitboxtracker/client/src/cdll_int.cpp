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
	g_pClientfuncs->pPostRunCmd = HUD_PostRunCmd;
	g_pClientfuncs->pProcessPlayerState = HUD_ProcessPlayerState;
	g_pClientfuncs->pStudioInterface = HUD_GetStudioModelInterface;

	gClientfuncs.pHudInitFunc();
	gHUD.Init();
}

void HUD_WeaponsPostThink(local_state_t *from, local_state_t *to, usercmd_t *cmd, double time, unsigned int random_seed)
{
	int flags = from->client.iuser3;

	g_bHoldingKnife = (from->client.m_iId == WEAPON_KNIFE);
	g_bHoldingShield = (flags & PLAYER_HOLDING_SHIELD) != 0;
}

void HUD_ProcessPlayerState(entity_state_t *dst, entity_state_t *src)
{
	// save current values of fields from server-side
	auto &sync             = g_PlayerSyncInfo[src->number];

	sync.yaw               = src->vuser1[0];
	sync.gaityaw           = src->vuser1[1];
	sync.gaitframe         = src->vuser1[2];

	sync.gaitmovement      = src->vuser2[0];
	sync.gaitsequence      = src->vuser2[1];
	sync.pitch             = src->vuser2[2];

	sync.prevgaitorigin[0] = src->maxs[0];
	sync.prevgaitorigin[1] = src->maxs[1];
	sync.prevgaitorigin[2] = src->maxs[2];

	sync.time              = src->mins[0]; // m_clTime
	sync.oldtime           = src->mins[1]; // m_clOldTime
	sync.frametime         = src->mins[2]; // m_clTime - m_clOldTime?

	gClientfuncs.pProcessPlayerState(dst, src);
}

void R_ForceCVars(qboolean multiplayer)
{
	// nothing to do here
	// block
}
