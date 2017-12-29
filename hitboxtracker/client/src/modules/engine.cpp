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

CEngine           *g_EngineLib;

svc_func_t        *g_pSVCfuncs;
cl_enginefunc_t   *g_pEngfuncs;
cldll_func_t      *g_pClientfuncs;

svc_func_t        gSVCfuncs;
cl_enginefunc_t   gEngfuncs;
cldll_func_t      gClientfuncs;
StudioLighting_t  pfnR_StudioLighting;

CEngine::CEngine()
{
	registry->Init();
}

CEngine::~CEngine()
{
	registry->Shutdown();
	gHUD.ShutDown();
}

hook_t CEngine::m_LoadSecureClient(OP_JUMP, LoadSecureClient);
bool CEngine::LoadSecureClient_Init()
{
	byteptr_t pos;
	if (!(pos = find_pattern("\x55\x8B\xEC\x8B\x45\x08\x6A\x01\x68"))) {
		pos = find_pattern("\x8B\x44\x24\x04\x6A\x00\x68");
	}

	return SetHook(pos, &m_LoadSecureClient);
}

bool CEngine::LoadInSecureClient_Init()
{
	auto pos = find_string("could not link client.dll function Initialize\n", OP_PUSH);
	if (!pos)
	{
		return false;
	}

	pos = find_pattern_back(pos, "\x8B\x2A\x2A\x2A\x2A\x2A\x68");
	if (!pos)
	{
		return false;
	}

	static auto refproc = &_GetProcAddress;

	// 8B 35 14 A1 E1 01   mov  esi, ds:GetProcAddress
	// ->
	// BE XX XX XX XX      mov  esi, _GetProcAddress
	// 90

	char bytes[6];
	bytes[0] = '\xBE';
	Q_memcpy(&bytes[1], &refproc, sizeof(refproc));
	bytes[5] = '\x90';

	return patch(pos, &bytes, sizeof(bytes));
}

bool CEngine::Init(const char *szModuleName, const char *pszFile)
{
	if (!CSysModule::Init(szModuleName, pszFile))
		return false;

	m_Software = !Q_stricmp(szModuleName, ENGINE_CLIENT_SOFT_LIB) ? true : false;

	if (!LoadSecureClient_Init() || !LoadInSecureClient_Init())
	{
		TraceLog("> %s: Not found ClientFuncs\n", __FUNCTION__);
		return false;
	}

	if (!(g_pClientfuncs = FindClientFuncs()))
	{
		TraceLog("> %s: Not found ClientFuncs\n", __FUNCTION__);
		return false;
	}

	if (!(g_pSVCfuncs = FindSVCFuncs()))
	{
		TraceLog("> %s: Not found SVCFuncs\n", __FUNCTION__);
		return false;
	}

	if (!(pg_pClientUserMsgs = FindClientUserMsgs()))
	{
		TraceLog("> %s: Not found UserMsg\n", __FUNCTION__);
		return false;
	}

	// Find R_ForceCVars and block
	if (!FindForceCVars())
	{
		TraceLog("> %s: Not found function R_ForceCVars\n", __FUNCTION__);
	}

	return true;
}

hook_t CEngine::m_ForceCVars(OP_JUMP, &R_ForceCVars);
bool CEngine::FindForceCVars()
{
	if (m_Software) {
		return true;
	}

	byteptr_t pos;
	if (!(pos = find_pattern("\x8B\x2A\x2A\x2A\x85\xC0\x0F\x2A\x2A\x2A\x2A\x2A\xD9\x2A\x2A\x2A\x2A\x2A\xD8"))) {
		pos = find_pattern("\x55\x8B\xEC\x8B\x45\x08\x85\xC0\x0F\x2A\x2A\x2A\x2A\x2A\xD9");
	}

	if (!pos) {
		return false;
	}

	return SetHook(pos, &m_ForceCVars);
}

svc_func_t *CEngine::FindSVCFuncs()
{
	auto pos = find_string("svc_bad");
	if (!pos) {
		return nullptr;
	}

	auto pSVC = (svc_func_t *)(pos - offsetof(svc_func_t, pszName) + 1);
	Q_memcpy(&gSVCfuncs, pSVC, sizeof(gSVCfuncs));
	return pSVC;
}

cl_enginefunc_t *CEngine::FindEngineFuncs()
{
	auto pos = find_string("ScreenFade", OP_PUSH);
	if (!pos) {
		return nullptr;
	}

	pos = find_pattern(pos + 1, 16, "\x68");
	if (!pos) {
		return nullptr;
	}

	auto pEngine = *(cl_enginefunc_t **)(pos + 1);
	Q_memcpy(&gEngfuncs, pEngine, sizeof(gEngfuncs));
	return pEngine;
}

cldll_func_t *CEngine::FindClientFuncs()
{
	auto pos = find_string("ScreenFade", OP_PUSH);
	if (!pos) {
		return nullptr;
	}

	pos = find_pattern(pos, 32, "\xFF\x2A\x2A\x2A\x2A\x2A\x68");
	if (!pos) {
		return nullptr;
	}

	auto pClient = *(cldll_func_t **)(pos + 2);
	Q_memcpy(&gClientfuncs, pClient, sizeof(gClientfuncs));
	return pClient;
}

UserMsg **CEngine::FindClientUserMsgs()
{
	auto pos = find_string("UserMsg: Not Present on Client %d\n", OP_PUSH);
	if (!pos) {
		return nullptr;
	}

	pos = find_pattern(pos - 32, 32, "\x8B\x2A\x2A\x2A\x2A\x2A\x85\xF6\x74\x0B");
	if (!pos) {
		return nullptr;
	}

	auto pClientUserMsgs = *(UserMsg ***)(pos + 2);
	return pClientUserMsgs;
}

bool CEngine::StudioLightingInit()
{
	auto pos = find_pattern("\x55\x8B\xEC\x51\xDB\x2A\x2A\x2A\x2A\x2A\x8A\x2A\x2A\xB8");
	if (!pos) {
		pos = find_pattern("\x51\xDB\x2A\x2A\x2A\x2A\x2A\x8A\x2A\x2A\x2A\xB8");
	}

	if (!pos) {
		TraceLog("> %s: Not found function StudioLightingInit #2\n", __FUNCTION__);
		return false;
	}

	pfnR_StudioLighting = StudioLighting_t(pos);
	return true;
}

void R_StudioLighting(float *lv, int bone, int flags, const vec_t *normal)
{
	if (!pfnR_StudioLighting) {
		*lv = 0.75f;
		return;
	}

	pfnR_StudioLighting(lv, bone, flags, normal);

	if (g_EngineLib->IsSoftware()) {
		*lv = *lv / (USHRT_MAX + 1);
	}
}
