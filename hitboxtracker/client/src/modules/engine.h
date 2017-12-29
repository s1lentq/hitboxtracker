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

#include "net.h"

class CEngine: virtual public CSysModule
{
public:
	CEngine();
	~CEngine();

	bool Init(const char *szModuleName, const char *pszFile);
	bool StudioLightingInit();
	bool IsSoftware() const { return m_Software; }

protected:
	bool m_Software;

	bool FindForceCVars();
	bool LoadSecureClient_Init();
	bool LoadInSecureClient_Init();

	svc_func_t      *FindSVCFuncs();
	cl_enginefunc_t *FindEngineFuncs();
	cldll_func_t    *FindClientFuncs();
	UserMsg        **FindClientUserMsgs();

	static hook_t m_ForceCVars;
	static hook_t m_GetProcAddress;
	static hook_t m_LoadSecureClient;
};

extern CEngine          *g_EngineLib;
extern svc_func_t       *g_pSVCfuncs;
extern cl_enginefunc_t  *g_pEngfuncs;
extern cldll_func_t     *g_pClientfuncs;

extern svc_func_t        gSVCfuncs;
extern cl_enginefunc_t   gEngfuncs;
extern cldll_func_t      gClientfuncs;

#define TRACE_LOG(fmt, ...)\
	g_EngineLib->TraceLog("> %s: " fmt, __FUNCTION__, __VA_ARGS__)

#define TRACE_LOG2(fmt, ...)\
	g_EngineLib->TraceLog(fmt, __VA_ARGS__)

using StudioLighting_t = void (*)(float *lv, int bone, int flags, const vec_t *normal);
void R_StudioLighting(float *lv, int bone, int flags, const vec_t *normal);
