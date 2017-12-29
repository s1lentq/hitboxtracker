/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_int.h
//
// 4-23-98
// JOHN:  client dll interface declarations
//
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"

// this file is included by both the engine and the client-dll,
// so make sure engine declarations aren't done twice

enum
{
	SCRINFO_SCREENFLASH = 1,
	SCRINFO_STRETCHED
};

typedef struct SCREENINFO_s
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
} SCREENINFO;

typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
//	int		iAccessoryBits;
	float	fov;	// field of view
} client_data_t;

typedef struct client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;

typedef struct hud_player_info_s
{
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

	byte spectator;
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

	uint64 m_nSteamID;
} hud_player_info_t;

typedef struct module_s
{
	unsigned char				ucMD5Hash[16];	// hash over code
	qboolean					fLoaded;		// true if successfully loaded
} module_t;

#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

const int CLDLL_INTERFACE_VERSION = 7;

extern void LoadSecurityModuleFromDisk(char * pszDllName);
extern void LoadSecurityModuleFromMemory( unsigned char * pCode, int nSize );
extern void CloseSecurityModule();

extern void ClientDLL_Init( void ); // from cdll_int.c
extern void ClientDLL_Shutdown( void );
extern void ClientDLL_HudInit( void );
extern void ClientDLL_HudVidInit( void );
extern void ClientDLL_UpdateClientData( void );
extern void ClientDLL_Frame( double time );
extern void ClientDLL_HudRedraw( int intermission );
extern void ClientDLL_MoveClient( struct playermove_s *ppmove );
extern void ClientDLL_ClientMoveInit( struct playermove_s *ppmove );
extern char ClientDLL_ClientTextureType( char *name );

extern void ClientDLL_CreateMove( float frametime, struct usercmd_s *cmd, int active );
extern void ClientDLL_ActivateMouse( void );
extern void ClientDLL_DeactivateMouse( void );
extern void ClientDLL_MouseEvent( int mstate );
extern void ClientDLL_ClearStates( void );
extern int ClientDLL_IsThirdPerson( void );
extern void ClientDLL_GetCameraOffsets( float *ofs );
extern int ClientDLL_GraphKeyDown( void );
extern struct kbutton_s *ClientDLL_FindKey( const char *name );
extern void ClientDLL_CAM_Think( void );
extern void ClientDLL_IN_Accumulate( void );
extern void ClientDLL_CalcRefdef( struct ref_params_s *pparams );
extern int ClientDLL_AddEntity( int type, struct cl_entity_s *ent );
extern void ClientDLL_CreateEntities( void );

extern void ClientDLL_DrawNormalTriangles( void );
extern void ClientDLL_DrawTransparentTriangles( void );
extern void ClientDLL_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
extern void ClientDLL_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
extern void ClientDLL_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
extern void ClientDLL_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
extern void ClientDLL_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
extern void ClientDLL_ReadDemoBuffer( int size, unsigned char *buffer );
extern int ClientDLL_ConnectionlessPacket( const struct netadr_s *net_from_, const char *args, char *response_buffer, int *response_buffer_size );
extern int ClientDLL_GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void ClientDLL_VGui_ConsolePrint(const char* text);

extern int ClientDLL_Key_Event( int down, int keynum, const char *pszCurrentBinding );
extern void ClientDLL_TempEntUpdate( double ft, double ct, double grav, struct tempent_s **ppFreeTE, struct tempent_s **ppActiveTE, int ( *addTEntity )( struct cl_entity_s *pEntity ), void ( *playTESound )( struct tempent_s *pTemp, float damp ) );
extern struct cl_entity_s *ClientDLL_GetUserEntity( int index );
extern void ClientDLL_VoiceStatus(int entindex, qboolean bTalking);
extern void ClientDLL_DirectorMessage( int iSize, void *pbuf );
extern void ClientDLL_ChatInputPosition( int *x, int *y );

//#include "server.h" // server_static_t define for apiproxy
#include "APIProxy.h"

extern cldll_func_t cl_funcs;
extern cl_enginefunc_t cl_engsrcProxies;

#ifdef HOOK_ENGINE
#define g_engdstAddrs (*pg_engdstAddrs)
#define g_module (*pg_module)
#endif

extern cl_enginefunc_dst_t g_engdstAddrs;
extern module_t	g_module;

// Module exports
extern modfuncs_t g_modfuncs;

// Macros for exported engine funcs
#define RecEngSPR_Load(a)									(g_engdstAddrs.pfnSPR_Load(&a))
#define RecEngSPR_Frames(a)									(g_engdstAddrs.pfnSPR_Frames(&a))
#define RecEngSPR_Height(a, b)								(g_engdstAddrs.pfnSPR_Height(&a, &b))
#define RecEngSPR_Width(a, b)								(g_engdstAddrs.pfnSPR_Width(&a, &b))
#define RecEngSPR_Set(a, b, c, d)							(g_engdstAddrs.pfnSPR_Set(&a, &b, &c, &d))
#define RecEngSPR_Draw(a, b, c, d)							(g_engdstAddrs.pfnSPR_Draw(&a, &b, &c, &d))
#define RecEngSPR_DrawHoles(a, b, c, d)						(g_engdstAddrs.pfnSPR_DrawHoles(&a, &b, &c, &d))
#define RecEngSPR_DrawAdditive(a, b, c, d)					(g_engdstAddrs.pfnSPR_DrawAdditive(&a, &b, &c, &d))
#define RecEngSPR_EnableScissor(a, b, c, d)					(g_engdstAddrs.pfnSPR_EnableScissor(&a, &b, &c, &d))
#define RecEngSPR_DisableScissor()							(g_engdstAddrs.pfnSPR_DisableScissor())
#define RecEngSPR_GetList(a, b)								(g_engdstAddrs.pfnSPR_GetList(&a, &b))
#define RecEngDraw_FillRGBA(a, b, c, d, e, f, g, h)			(g_engdstAddrs.pfnFillRGBA(&a, &b, &c, &d, &e, &f, &g, &h))
#define RecEnghudGetScreenInfo(a)							(g_engdstAddrs.pfnGetScreenInfo(&a))
#define RecEngSetCrosshair(a, b, c, d, e)					(g_engdstAddrs.pfnSetCrosshair(&a, &b, &c, &d, &e))
#define RecEnghudRegisterVariable(a, b, c)					(g_engdstAddrs.pfnRegisterVariable(&a, &b, &c))
#define RecEnghudGetCvarFloat(a)							(g_engdstAddrs.pfnGetCvarFloat(&a))
#define RecEnghudGetCvarString(a)							(g_engdstAddrs.pfnGetCvarString(&a))
#define RecEnghudAddCommand(a, b)							(g_engdstAddrs.pfnAddCommand(&a, &b))
#define RecEnghudHookUserMsg(a, b)							(g_engdstAddrs.pfnHookUserMsg(&a, &b))
#define RecEnghudServerCmd(a)								(g_engdstAddrs.pfnServerCmd(&a))
#define RecEnghudClientCmd(a)								(g_engdstAddrs.pfnClientCmd(&a))
#define RecEngPrimeMusicStream(a, b)						(g_engdstAddrs.pfnPrimeMusicStream(&a, &b))
#define RecEnghudGetPlayerInfo(a, b)						(g_engdstAddrs.pfnGetPlayerInfo(&a, &b))
#define RecEnghudPlaySoundByName(a, b)						(g_engdstAddrs.pfnPlaySoundByName(&a, &b))
#define RecEnghudPlaySoundByNameAtPitch(a, b, c)			(g_engdstAddrs.pfnPlaySoundByNameAtPitch(&a, &b, &c))
#define RecEnghudPlaySoundVoiceByName(a, b)					(g_engdstAddrs.pfnPlaySoundVoiceByName(&a, &b))
#define RecEnghudPlaySoundByIndex(a, b)						(g_engdstAddrs.pfnPlaySoundByIndex(&a, &b))
#define RecEngAngleVectors(a, b, c, d)						(g_engdstAddrs.pfnAngleVectors(&a, &b, &c, &d))
#define RecEngTextMessageGet(a)								(g_engdstAddrs.pfnTextMessageGet(&a))
#define RecEngTextMessageDrawCharacter(a, b, c, d, e, f)	(g_engdstAddrs.pfnDrawCharacter(&a, &b, &c, &d, &e, &f))
#define RecEngDrawConsoleString(a, b, c)					(g_engdstAddrs.pfnDrawConsoleString(&a, &b, &c))
#define RecEngDrawSetTextColor(a, b, c)						(g_engdstAddrs.pfnDrawSetTextColor(&a, &b, &c))
#define RecEnghudDrawConsoleStringLen(a, b, c)				(g_engdstAddrs.pfnDrawConsoleStringLen(&a, &b, &c))
#define RecEnghudConsolePrint(a)							(g_engdstAddrs.pfnConsolePrint(&a))
#define RecEnghudCenterPrint(a)								(g_engdstAddrs.pfnCenterPrint(&a))
#define RecEnghudCenterX()									(g_engdstAddrs.GetWindowCenterX())
#define RecEnghudCenterY()									(g_engdstAddrs.GetWindowCenterY())
#define RecEnghudGetViewAngles(a)							(g_engdstAddrs.GetViewAngles(&a))
#define RecEnghudSetViewAngles(a)							(g_engdstAddrs.SetViewAngles(&a))
#define RecEnghudGetMaxClients()							(g_engdstAddrs.GetMaxClients())
#define RecEngCvar_SetValue(a, b)							(g_engdstAddrs.Cvar_SetValue(&a, &b))
#define RecEngCmd_Argc()									(g_engdstAddrs.Cmd_Argc())
#define RecEngCmd_Argv(a)									(g_engdstAddrs.Cmd_Argv(&a))
#define RecEngCon_Printf(a)									(g_engdstAddrs.Con_Printf(&a))
#define RecEngCon_DPrintf(a)								(g_engdstAddrs.Con_DPrintf(&a))
#define RecEngCon_NPrintf(a, b)								(g_engdstAddrs.Con_NPrintf(&a, &b))
#define RecEngCon_NXPrintf(a, b)							(g_engdstAddrs.Con_NXPrintf(&a, &b))
#define RecEnghudPhysInfo_ValueForKey(a)					(g_engdstAddrs.PhysInfo_ValueForKey(&a))
#define RecEnghudServerInfo_ValueForKey(a)					(g_engdstAddrs.ServerInfo_ValueForKey(&a))
#define RecEnghudGetClientMaxspeed()						(g_engdstAddrs.GetClientMaxspeed())
#define RecEnghudCheckParm(a, b)							(g_engdstAddrs.CheckParm(&a, &b))
#define RecEngKey_Event(a, b)								(g_engdstAddrs.Key_Event(&a, &b))
#define RecEnghudGetMousePosition(a, b)						(g_engdstAddrs.GetMousePosition(&a, &b))
#define RecEnghudIsNoClipping()								(g_engdstAddrs.IsNoClipping())
#define RecEnghudGetLocalPlayer()							(g_engdstAddrs.GetLocalPlayer())
#define RecEnghudGetViewModel()								(g_engdstAddrs.GetViewModel())
#define RecEnghudGetEntityByIndex(a)						(g_engdstAddrs.GetEntityByIndex(&a))
#define RecEnghudGetClientTime()							(g_engdstAddrs.GetClientTime())
#define RecEngV_CalcShake()									(g_engdstAddrs.V_CalcShake())
#define RecEngV_ApplyShake(a, b, c)							(g_engdstAddrs.V_ApplyShake(&a, &b, &c))
#define RecEngPM_PointContents(a, b)						(g_engdstAddrs.PM_PointContents(&a, &b))
#define RecEngPM_WaterEntity(a)								(g_engdstAddrs.PM_WaterEntity(&a))
#define RecEngPM_TraceLine(a, b, c, d, e)					(g_engdstAddrs.PM_TraceLine(&a, &b, &c, &d, &e))
#define RecEngCL_LoadModel(a, b)							(g_engdstAddrs.CL_LoadModel(&a, &b))
#define RecEngCL_CreateVisibleEntity(a, b)					(g_engdstAddrs.CL_CreateVisibleEntity(&a, &b))
#define RecEnghudGetSpritePointer(a)						(g_engdstAddrs.GetSpritePointer(&a))
#define RecEnghudPlaySoundByNameAtLocation(a, b, c)			(g_engdstAddrs.pfnPlaySoundByNameAtLocation(&a, &b, &c))
#define RecEnghudPrecacheEvent(a, b)						(g_engdstAddrs.pfnPrecacheEvent(&a, &b))
#define RecEnghudPlaybackEvent(a, b, c, d, e, f, g, h, i, j, k, l)	(g_engdstAddrs.pfnPlaybackEvent(&a, &b, &c, &d, &e, &f, &g, &h, &i, &j, &k, &l))
#define RecEnghudWeaponAnim(a, b)							(g_engdstAddrs.pfnWeaponAnim(&a, &b))
#define RecEngRandomFloat(a, b)								(g_engdstAddrs.pfnRandomFloat(&a, &b))
#define RecEngRandomLong(a, b)								(g_engdstAddrs.pfnRandomLong(&a, &b))
#define RecEngCL_HookEvent(a, b)							(g_engdstAddrs.pfnHookEvent(&a, &b))
#define RecEngCon_IsVisible()								(g_engdstAddrs.Con_IsVisible())
#define RecEnghudGetGameDir()								(g_engdstAddrs.pfnGetGameDirectory())
#define RecEngCvar_FindVar(a)								(g_engdstAddrs.pfnGetCvarPointer(&a))
#define RecEngKey_NameForBinding(a)							(g_engdstAddrs.Key_LookupBinding(&a))
#define RecEnghudGetLevelName()								(g_engdstAddrs.pfnGetLevelName())
#define RecEnghudGetScreenFade(a)							(g_engdstAddrs.pfnGetScreenFade(&a))
#define RecEnghudSetScreenFade(a)							(g_engdstAddrs.pfnSetScreenFade(&a))
#define RecEngVGuiWrap_GetPanel()							(g_engdstAddrs.VGui_GetPanel())
#define RecEngVGui_ViewportPaintBackground(a)				(g_engdstAddrs.VGui_ViewportPaintBackground(&a))
#define RecEngCOM_LoadFile(a, b, c)							(g_engdstAddrs.COM_LoadFile(&a, &b, &c))
#define RecEngCOM_ParseFile(a, b)							(g_engdstAddrs.COM_ParseFile(&a, &b))
#define RecEngCOM_FreeFile(a)								(g_engdstAddrs.COM_FreeFile(&a))
#define RecEngCL_IsSpectateOnly()							(g_engdstAddrs.IsSpectateOnly())
#define RecEngR_LoadMapSprite(a)							(g_engdstAddrs.LoadMapSprite(&a))
#define RecEngCOM_AddAppDirectoryToSearchPath(a, b)			(g_engdstAddrs.COM_AddAppDirectoryToSearchPath(&a, &b))
#define RecEngClientDLL_ExpandFileName(a, b, c)				(g_engdstAddrs.COM_ExpandFilename(&a, &b, &c))
#define RecEngPlayerInfo_ValueForKey(a, b)					(g_engdstAddrs.PlayerInfo_ValueForKey(&a, &b))
#define RecEngPlayerInfo_SetValueForKey(a, b)				(g_engdstAddrs.PlayerInfo_SetValueForKey(&a, &b))
#define RecEngGetPlayerUniqueID(a, b)						(g_engdstAddrs.GetPlayerUniqueID(&a, &b))
#define RecEngGetTrackerIDForPlayer(a)						(g_engdstAddrs.GetTrackerIDForPlayer(&a))
#define RecEngGetPlayerForTrackerID(a)						(g_engdstAddrs.GetPlayerForTrackerID(&a))
#define RecEnghudServerCmdUnreliable(a)						(g_engdstAddrs.pfnServerCmdUnreliable(&a))
#define RecEngGetMousePos(a)								(g_engdstAddrs.pfnGetMousePos(&a))
#define RecEngSetMousePos(a, b)								(g_engdstAddrs.pfnSetMousePos(&a, &b))
#define RecEngSetMouseEnable(a)								(g_engdstAddrs.pfnSetMouseEnable(&a))
#define RecEngSetFilterMode(a)								(g_engdstAddrs.pfnSetFilterMode(&a))
#define RecEngSetFilterColor(a,b,c)							(g_engdstAddrs.pfnSetFilterColor(&a,&b,&c))
#define RecEngSetFilterBrightness(a)						(g_engdstAddrs.pfnSetFilterBrightness(&a))
#define RecEngSequenceGet(a,b)								(g_engdstAddrs.pfnSequenceGet(&a,&b))
#define RecEngSPR_DrawGeneric(a,b,c,d,e,f,g,h)				(g_engdstAddrs.pfnSPR_DrawGeneric(&a, &b, &c, &d, &e, &f, &g, &h))
#define RecEngSequencePickSentence(a,b,c)					(g_engdstAddrs.pfnSequencePickSentence(&a, &b, &c))
#define RecEngLocalPlayerInfo_ValueForKey(a)				(g_engdstAddrs.LocalPlayerInfo_ValueForKey(&a))
#define RecEngProcessTutorMessageDecayBuffer(a, b)			(g_engdstAddrs.pfnProcessTutorMessageDecayBuffer(&a, &b))
#define RecEngConstructTutorMessageDecayBuffer(a, b)		(g_engdstAddrs.pfnConstructTutorMessageDecayBuffer(&a, &b))
#define RecEngResetTutorMessageDecayBuffer()				(g_engdstAddrs.pfnResetTutorMessageDecayBuffer())
#define RecEngDraw_FillRGBABlend(a, b, c, d, e, f, g, h)	(g_engdstAddrs.pfnFillRGBABlend(&a, &b, &c, &d, &e, &f, &g, &h))

extern int Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion);
extern int HUD_VidInit();
extern void HUD_Init();
extern int HUD_Redraw(float time, int intermission);
extern int HUD_UpdateClientData(client_data_t *pcldata, float flTime);
extern void HUD_Reset();
extern void HUD_PlayerMove(playermove_t *ppmove, int server);
extern void HUD_PlayerMoveInit(playermove_t *ppmove);
extern char HUD_PlayerMoveTexture(char *name);
extern void IN_ActivateMouse();
extern void IN_DeactivateMouse();
extern void IN_MouseEvent(int mstate);
extern void IN_ClearStates();
extern void IN_Accumulate();
extern void CL_CreateMove(float frametime, usercmd_t *cmd, int active);
extern int CL_IsThirdPerson();
extern void CL_CameraOffset(float *ofs);
extern void CAM_Think();
extern kbutton_t *KB_Find(const char *name);
extern void V_CalcRefdef(ref_params_t *pparams);
extern int HUD_AddEntity(int type, cl_entity_t *ent, const char *modelname);
extern void HUD_CreateEntities();
extern void HUD_DrawNormalTriangles();
extern void HUD_DrawTransparentTriangles();
extern void HUD_StudioEvent(const mstudioevent_s *event, const cl_entity_t *entity);
extern void HUD_Shutdown();
extern void HUD_TxferLocalOverrides(entity_state_s *state, const clientdata_t *client);
extern void HUD_ProcessPlayerState(entity_state_t *dst, entity_state_t *src);
extern void HUD_TxferPredictionData(entity_state_t *ps, const entity_state_t *pps, clientdata_t *pcd, const clientdata_t *ppcd, weapon_data_t *wd, const weapon_data_t *pwd);
extern void Demo_ReadBuffer(int size, unsigned char *buffer);
extern int HUD_ConnectionlessPacket(const netadr_t *net_from, const char *args, char *response_buffer, int *response_buffer_size);
extern int HUD_GetHullBounds(int hullnumber, float *mins, float *maxs);
extern void HUD_Frame(double time);
extern int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding);
extern void HUD_PostRunCmd(local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed);
extern void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, TEMPENTITY **ppTempEntFree, TEMPENTITY **ppTempEntActive, int (*Callback_AddVisibleEntity)(cl_entity_t *), void (*Callback_TempEntPlaySound)(TEMPENTITY *, float));
extern cl_entity_t *HUD_GetUserEntity(int index);
extern void HUD_VoiceStatus(int entindex, qboolean bTalking);
extern void HUD_DirectorMessage(int iSize, void *pbuf);
extern void HUD_WeaponsPostThink(local_state_t *from, local_state_t *to, usercmd_t *cmd, double time, unsigned int random_seed);
extern int HUD_GetStudioModelInterface(int version, r_studio_interface_t **ppinterface, engine_studio_api_t *pstudio);
extern void HUD_ChatInputPosition(int *x, int *y);
extern int HUD_GetPlayerTeam(int iplayer);
extern void *ClientFactory();

#ifdef __cplusplus
}
#endif

extern float g_lastFOV;
extern bool  g_bHoldingKnife;
extern bool  g_bHoldingShield;
