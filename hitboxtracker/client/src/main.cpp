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

#define CLDLL_ENTRY(field, name)  { offsetof(cldll_func_t, field), #name }
#define CLDLL_ENTRY_H(name)       { #name, name }

struct cldll_table_t
{
	size_t offset;
	const char *name;
} g_cldllapi_info[] =
{
	CLDLL_ENTRY(pInitFunc,                 Initialize),
	CLDLL_ENTRY(pHudInitFunc,              HUD_Init),
	CLDLL_ENTRY(pHudVidInitFunc,           HUD_VidInit),
	CLDLL_ENTRY(pHudRedrawFunc,            HUD_Redraw),
	CLDLL_ENTRY(pHudUpdateClientDataFunc,  HUD_UpdateClientData),
	CLDLL_ENTRY(pHudResetFunc,             HUD_Reset),
	CLDLL_ENTRY(pClientMove,               HUD_PlayerMove),
	CLDLL_ENTRY(pClientMoveInit,           HUD_PlayerMoveInit),
	CLDLL_ENTRY(pClientTextureType,        HUD_PlayerMoveTexture),
	CLDLL_ENTRY(pIN_ActivateMouse,         IN_ActivateMouse),
	CLDLL_ENTRY(pIN_DeactivateMouse,       IN_DeactivateMouse),
	CLDLL_ENTRY(pIN_MouseEvent,            IN_MouseEvent),
	CLDLL_ENTRY(pIN_ClearStates,           IN_ClearStates),
	CLDLL_ENTRY(pIN_Accumulate,            IN_Accumulate),
	CLDLL_ENTRY(pCL_CreateMove,            CL_CreateMove),
	CLDLL_ENTRY(pCL_IsThirdPerson,         CL_IsThirdPerson),
	CLDLL_ENTRY(pCL_GetCameraOffsets,      CL_CameraOffset),
	CLDLL_ENTRY(pFindKey,                  KB_Find),
	CLDLL_ENTRY(pCamThink,                 CAM_Think),
	CLDLL_ENTRY(pCalcRefdef,               V_CalcRefdef),
	CLDLL_ENTRY(pAddEntity,                HUD_AddEntity),
	CLDLL_ENTRY(pCreateEntities,           HUD_CreateEntities),
	CLDLL_ENTRY(pDrawNormalTriangles,      HUD_DrawNormalTriangles),
	CLDLL_ENTRY(pDrawTransparentTriangles, HUD_DrawTransparentTriangles),
	CLDLL_ENTRY(pStudioEvent,              HUD_StudioEvent),
	CLDLL_ENTRY(pPostRunCmd,               HUD_PostRunCmd),
	CLDLL_ENTRY(pShutdown,                 HUD_Shutdown),
	CLDLL_ENTRY(pTxferLocalOverrides,      HUD_TxferLocalOverrides),
	CLDLL_ENTRY(pProcessPlayerState,       HUD_ProcessPlayerState),
	CLDLL_ENTRY(pTxferPredictionData,      HUD_TxferPredictionData),
	CLDLL_ENTRY(pReadDemoBuffer,           Demo_ReadBuffer),
	CLDLL_ENTRY(pConnectionlessPacket,     HUD_ConnectionlessPacket),
	CLDLL_ENTRY(pGetHullBounds,            HUD_GetHullBounds),
	CLDLL_ENTRY(pHudFrame,                 HUD_Frame),
	CLDLL_ENTRY(pKeyEvent,                 HUD_Key_Event),
	CLDLL_ENTRY(pTempEntUpdate,            HUD_TempEntUpdate),
	CLDLL_ENTRY(pGetUserEntity,            HUD_GetUserEntity),
	CLDLL_ENTRY(pVoiceStatus,              HUD_VoiceStatus),
	CLDLL_ENTRY(pDirectorMessage,          HUD_DirectorMessage),
	CLDLL_ENTRY(pStudioInterface,          HUD_GetStudioModelInterface),
	CLDLL_ENTRY(pChatInputPosition,        HUD_ChatInputPosition),
	CLDLL_ENTRY(pGetPlayerTeam,            HUD_GetPlayerTeam),
	CLDLL_ENTRY(pClientFactory,            ClientFactory),
};

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD type, LPVOID lpReserved)
{
	if (type == DLL_PROCESS_ATTACH)
	{
		g_EngineLib = new CEngine();

		const char *engine = registry->ReadString("EngineDLL", ENGINE_CLIENT_LIB);
		if (!g_EngineLib->Init(engine, "engine.log")) {
			return FALSE;
		}
	}
	else if (type == DLL_PROCESS_DETACH)
	{
		delete g_EngineLib;
	}

	return TRUE;
}

void *WINAPI _GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	auto res = GetProcAddress(hModule, lpProcName);
	for (auto &func : g_cldllapi_info)
	{
		if (Q_strcmp(func.name, lpProcName) != 0)
			continue;

		*(size_t *)(size_t(&gClientfuncs) + func.offset) = size_t(res);
	}

	if (!Q_strcmp(lpProcName, "Initialize")) {
		return &Initialize;
	}

	return res;
}

BOOL LoadSecureClient(char *pszDllName)
{
	return FALSE;
}
