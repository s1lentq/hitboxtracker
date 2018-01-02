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

CHud gHUD;

cvar_t *cl_lw;
cvar_t *cl_min_t;
cvar_t *cl_min_ct;
cvar_t *cl_shadows;
cvar_t *cl_righthand;
cvar_t *cl_minmodels;

cvar_t *default_fov;
cvar_t *sensitivity;

server_sync_t       sv;
player_sync_t       g_PlayerSyncInfo [MAX_PLAYERS + 1];
extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS + 1];	// additional player info sent directly to the client dll

GHUD_DECLARE_MESSAGE(SetFOV)
GHUD_DECLARE_MESSAGE(ShadowIdx)
GHUD_DECLARE_MESSAGE(ResetHUD)

CHud::CHud() : m_iFOV(0),
	m_flMouseSensitivity(0),
	m_Client(nullptr)
{

}

bool CHud::Init()
{
	m_Client = new CClient();

	if (!m_Client->Init("client.dll", "client.log")) {
		return false;
	}

	cl_lw        = gEngfuncs.pfnGetCvarPointer("cl_lw");
	cl_min_t     = gEngfuncs.pfnGetCvarPointer("cl_min_t");
	cl_min_ct    = gEngfuncs.pfnGetCvarPointer("cl_min_ct");
	cl_shadows   = gEngfuncs.pfnGetCvarPointer("cl_shadows");
	cl_righthand = gEngfuncs.pfnGetCvarPointer("cl_righthand");
	cl_minmodels = gEngfuncs.pfnGetCvarPointer("cl_minmodels");
	default_fov  = gEngfuncs.pfnGetCvarPointer("default_fov");
	sensitivity  = gEngfuncs.pfnGetCvarPointer("sensitivity");

	HOOK_MESSAGE(SetFOV);
	HOOK_MESSAGE(ShadowIdx);
	HOOK_MESSAGE(ResetHUD);

	return true;
}

void CHud::ShutDown()
{
	if (m_Client) {
		delete m_Client;
		m_Client = nullptr;
	}
}

int CHud::MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int newfov = READ_BYTE();
	int def_fov = default_fov->value;

	// Weapon prediction already takes care of changing the fog. (g_lastFOV).
	//if (IsGame("tfc") && cl_lw && cl_lw->value)
	//	return 1;

	g_lastFOV = newfov;

	if (newfov == 0)
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == def_fov)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * gEngfuncs.pfnGetCvarFloat("zoom_sensitivity_ratio");
	}

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int CHud::MsgFunc_ShadowIdx(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int idx = READ_LONG();
	g_StudioRenderer.StudioSetShadowSprite(idx);
	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

int CHud::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	m_flMouseSensitivity = 0;

	return g_ClientUserMsgsMap[pszName](pszName, iSize, pbuf);
}

bool CHud::IsGame(const char *game) const
{
	const char *gamedir = gEngfuncs.pfnGetGameDirectory();
	if (!gamedir || gamedir[0] == '\0') {
		return false;
	}

	char gd[1024];
	COM_FileBase(gamedir, gd);
	return Q_stricmp(gd, game) == 0 ? true : false;
}
