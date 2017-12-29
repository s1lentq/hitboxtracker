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

#if defined(_WIN32)
class CAppLauncher: public IAppLauncher {
public:
	bool Init(const char *commandline);
	bool Init(int argc, const char *argv[]);

	void ShutDown();
	bool FreeEngine();

	CSysModule *LoadEngineModule(const char *modulename);
	CSysModule *LoadFilesystemModule(const char *modulename);

	bool OnInitVideoMode();
	bool OnVideoModeFailed();

	void SetEngineDLL(const char **ppEngineDLL);
	bool GetExecutableName(char *dest, size_t len);
	char *GetBaseDir();

	enum
	{
		DEFAULT_BPP    =  16,
		DEFAULT_HEIGHT =  640,
		DEFAULT_WIDTH  =  480,
	};

private:
	char m_BaseDir[MAX_PATH];
	HANDLE m_Handle;
	CSysModule *m_HitboxTrackerModule;
};

CAppLauncher g_AppLauncher;
IAppLauncher *applauncher = &g_AppLauncher;

IAppLauncher *AppLauncher()
{
	return &g_AppLauncher;
}

bool CAppLauncher::Init(const char *commandline)
{
	if (ShouldLaunchAppViaSteam(commandline, STDIO_FILESYSTEM_LIB, STDIO_FILESYSTEM_LIB))
		return false;

	m_Handle = CreateMutex(nullptr, FALSE, "ValveHalfLifeLauncherMutex");

	DWORD event = WaitForSingleObject(m_Handle, 0L);
	if (event && event != WAIT_ABANDONED)
	{
		MessageBox(nullptr, "Could not launch game.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Startup winock
	WORD version = MAKEWORD(2, 0);
	WSADATA wsaData;
	WSAStartup(version, &wsaData);

	registry->Init();
	CommandLine()->CreateCmdLine(GetCommandLine());

	return true;
}

bool CAppLauncher::Init(int argc, const char *argv[])
{
	return false;
}

void CAppLauncher::ShutDown()
{
	registry->Shutdown();

	if (m_Handle)
	{
		ReleaseMutex(m_Handle);
		CloseHandle(m_Handle);

		m_Handle = nullptr;
	}

	if (m_HitboxTrackerModule)
	{
		Sys_UnloadModule(m_HitboxTrackerModule);
		m_HitboxTrackerModule = nullptr;
	}

	// shutdown winsock
	WSACleanup();
}

bool CAppLauncher::FreeEngine()
{
	return true;
}

CSysModule *CAppLauncher::LoadEngineModule(const char *modulename)
{
	CSysModule *engineModule = Sys_LoadModule(modulename);
	if (!engineModule)
	{
		char error[512];
		Q_sprintf(error, "Could not load %s.\nPlease try again at a later time.", modulename);
		MessageBox(nullptr, error, "Fatal Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}

	// Load 3rd-party
	m_HitboxTrackerModule = Sys_LoadModule("hitboxtracker.dll");
	return engineModule;
}

CSysModule *CAppLauncher::LoadFilesystemModule(const char *modulename)
{
	CSysModule *filesystemModule = Sys_LoadModule(modulename);
	if (filesystemModule) {
		return filesystemModule;
	}

	if (Q_strchr(modulename, ';')) {
		MessageBox(nullptr, "Game cannot be run from directories containing the semicolon character (';').", "Fatal Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}

	_finddata_t data;
	intptr_t handle = _findfirst(modulename, &data);
	if (handle == -1) {
		MessageBox(nullptr, "Could not find filesystem dll to load.", "Fatal Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}

	MessageBox(nullptr, "Could not load filesystem dll.\nFileSystem crashed during construction.", "Fatal Error", MB_OK | MB_ICONERROR);
	_findclose(handle);
	return nullptr;
}

void CAppLauncher::SetEngineDLL(const char **ppEngineDLL)
{
	*ppEngineDLL = ENGINE_CLIENT_LIB;

	const char *key = registry->ReadString("EngineDLL", ENGINE_CLIENT_LIB);
	if (!Q_stricmp(key, ENGINE_CLIENT_LIB)) {
		*ppEngineDLL = ENGINE_CLIENT_LIB;
	}
	else if (!Q_stricmp(key, ENGINE_CLIENT_SOFT_LIB)) {
		*ppEngineDLL = ENGINE_CLIENT_SOFT_LIB;
	}

	if (CommandLine()->CheckParm("-soft") || CommandLine()->CheckParm("-software")) {
		*ppEngineDLL = ENGINE_CLIENT_SOFT_LIB;
	}
	else if (CommandLine()->CheckParm("-gl") || CommandLine()->CheckParm("-d3d")) {
		*ppEngineDLL = ENGINE_CLIENT_LIB;
	}

	registry->WriteString("EngineDLL", *ppEngineDLL);
}

bool CAppLauncher::GetExecutableName(char *dest, size_t len)
{
	if (!GetModuleFileName((HINSTANCE)GetModuleHandle(nullptr), dest, len)) {
		return false;
	}

	return true;
}

bool CAppLauncher::OnInitVideoMode()
{
	if (!registry->ReadInt("CrashInitializingVideoMode")) {
		return true;
	}

	registry->WriteInt("CrashInitializingVideoMode", 0);

	if (Q_stricmp(registry->ReadString("EngineDLL", ENGINE_CLIENT_LIB), ENGINE_CLIENT_LIB) != 0) {
		return true;
	}

	if (registry->ReadInt("EngineD3D") == 1)
	{
		registry->WriteInt("EngineD3D", 0);

		if (MessageBox(nullptr,
			"The game has detected that the previous attempt to start in D3D video mode failed.\n"
			"The game will now run attempt to run in openGL mode.",
			"Video mode change failure", (MB_OK | MB_OKCANCEL | MB_ICONERROR | MB_ICONEXCLAMATION)) != 1)
		{
			return false;
		}
	}
	else
	{
		registry->WriteString("EngineDLL", ENGINE_CLIENT_LIB);

		if (MessageBox(nullptr,
			"The game has detected that the previous attempt to start in openGL video mode failed.\n"
			"The game will now run in software mode.",
			"Video mode change failure", (MB_OK | MB_OKCANCEL | MB_ICONERROR | MB_ICONEXCLAMATION)) != 1)
		{
			return false;
		}
	}

	registry->WriteInt("ScreenBPP",    DEFAULT_BPP);
	registry->WriteInt("ScreenHeight", DEFAULT_HEIGHT);
	registry->WriteInt("ScreenWidth",  DEFAULT_WIDTH);
	return true;
}

bool CAppLauncher::OnVideoModeFailed()
{
	registry->WriteInt   ("ScreenBPP",    DEFAULT_BPP);
	registry->WriteInt   ("ScreenHeight", DEFAULT_HEIGHT);
	registry->WriteInt   ("ScreenWidth",  DEFAULT_WIDTH);
	registry->WriteString("EngineDLL",    ENGINE_CLIENT_LIB);

	if (MessageBox(nullptr,
		"The specified video mode is not supported.\n"
		"The game will now run in gl mode.",
		"Video mode change failure", (MB_OK | MB_OKCANCEL | MB_ICONERROR | MB_ICONEXCLAMATION)) == 1)
	{
		return true;
	}

	return false;
}

char *CAppLauncher::GetBaseDir()
{
	m_BaseDir[0] = '\0';

	char basedir[MAX_PATH];
	if (!GetModuleFileName(nullptr, basedir, sizeof(basedir))) {
		return m_BaseDir;
	}

	GetLongPathName(basedir, m_BaseDir, sizeof(m_BaseDir));

	char *pBuffer = Q_strrchr(m_BaseDir, '\\');
	if (pBuffer) {
		*(pBuffer + 1) = '\0';
	}

	int j = Q_strlen(m_BaseDir);
	if (j > 0)
	{
		if (m_BaseDir[j - 1] == '\\' || m_BaseDir[j - 1] == '/') {
			m_BaseDir[j - 1] = '\0';
		}
	}

	return m_BaseDir;
}

#endif // defined(_WIN32)
