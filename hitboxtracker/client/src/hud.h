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

#pragma once

#define g_PlayerExtraInfo (*pg_PlayerExtraInfo)

enum
{
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

struct server_sync_t
{
	double time;
	double oldtime;
	float  frametime;
};

struct player_sync_t
{
	float  yaw;
	float  pitch;
	float  gaityaw;
	float  gaitframe;
	float  gaitmovement;
	int    gaitsequence;
	Vector prevgaitorigin;
};

struct extra_player_info_t
{
	short frags;
	short deaths;
	short team_id;
	int has_c4;
	int vip;
	Vector origin;
	float radarflash;
	int radarflashon;
	int radarflashes;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
	bool dead;
	float showhealth;
	int health;
	char location[32];
	int sb_health;
	int sb_account;
	int has_defuse_kit;
};

// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x)   HookUserMsg(#x, __MsgFunc_##x)
#define UNHOOK_MESSAGE(x) UnHookUserMsg(#x)

#define DECLARE_MESSAGE(y, x)\
int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf)\
{\
	return gHUD.y.MsgFunc_##x(pszName, iSize, pbuf);\
}

#define GHUD_DECLARE_MESSAGE(x)\
int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf)\
{\
	return gHUD.MsgFunc_##x(pszName, iSize, pbuf);\
}

#define DECLARE_MESSAGE_FUNC(x)\
int MsgFunc_##x(const char *pszName, int iSize, void *buf);

class CClient;
class CHud
{
public:
	CHud();

	bool Init();
	void ShutDown();
	bool IsGame(const char *game) const;

public:
	DECLARE_MESSAGE_FUNC(SetFOV);
	DECLARE_MESSAGE_FUNC(ShadowIdx);
	DECLARE_MESSAGE_FUNC(ResetHUD);

	int m_iFOV;
	float m_flMouseSensitivity;

private:
	CClient *m_Client;
};

extern CHud gHUD;

extern server_sync_t       sv;
extern player_sync_t       g_PlayerSyncInfo [MAX_PLAYERS + 1];
extern extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS + 1];

extern cvar_t *cl_lw;
extern cvar_t *cl_min_t;
extern cvar_t *cl_min_ct;
extern cvar_t *cl_shadows;
extern cvar_t *cl_righthand;
extern cvar_t *cl_minmodels;
extern cvar_t *default_fov;
extern cvar_t *sensitivity;
