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

IFileSystem *g_pFileSystem;

enum {
	ENGINE_RESULT_NONE,
	ENGINE_RESULT_RESTART,
	ENGINE_RESULT_UNSUPPORTEDVIDEO,
};

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
#define hInstance nullptr
int main(int argc, char *argv[])
#endif
{
	static char szNewCommandParams[2048];

#ifdef _WIN32
	if (!AppLauncher()->Init(lpCmdLine))
		return 0;
#else
	if (!AppLauncher()->Init(argc, argv))
		return 0;
#endif

	bool bRunningSteam = CommandLine()->CheckParm("-steam") ? true : false;

	// calculate the details of our launch
	//char exename[256];
	//AppLauncher()->GetExecutableName(exename, sizeof(exename));

	// strip out the exe name
	//char *mod = Q_strrchr(exename, CORRECT_PATH_SEPARATOR) + 1;
	//if (Q_stricmp(mod, LAUNCHER_NAME) != 0 && !CommandLine()->CheckParm("-game"))
	//{
	//	mod[Q_strlen(mod) - 4] = '\0';
	//	CommandLine()->AppendParm("-game", mod);
	//}

	CommandLine()->AppendParm("-game", "cstrike");

#ifdef _WIN32
	_unlink("mssv29.asi");
	_unlink("mssv12.asi");
	_unlink("mp3dec.asi");
	_unlink("opengl32.dll");
#endif

	if (!AppLauncher()->OnInitVideoMode())
		return 0;

	bool restart = true;
	const char *enginedll;

	while (restart)
	{
		CSysModule *filesystemModule = AppLauncher()->LoadFilesystemModule(STDIO_FILESYSTEM_LIB);
		if (!filesystemModule) {
			break;
		}

		// Get FileSystem interface
		CreateInterfaceFn filesystemFactoryFn = Sys_GetFactory(filesystemModule);
		g_pFileSystem = (IFileSystem *)filesystemFactoryFn(FILESYSTEM_INTERFACE_VERSION, nullptr);
		g_pFileSystem->Mount();
		g_pFileSystem->AddSearchPath(AppLauncher()->GetBaseDir(), "ROOT");

		szNewCommandParams[0] = '\0';
		AppLauncher()->SetEngineDLL(&enginedll);

		int engineResult = ENGINE_RESULT_NONE;
		CSysModule *engineModule = AppLauncher()->LoadEngineModule(enginedll);
		if (!engineModule) {
			break;
		}

		CreateInterfaceFn engineFactory = Sys_GetFactory(engineModule);
		if (engineFactory)
		{
			IEngineAPI *engineAPI = (IEngineAPI *)engineFactory(VENGINE_LAUNCHER_API_VERSION, nullptr);
			if (engineAPI)
			{
				engineResult = engineAPI->Run(hInstance, AppLauncher()->GetBaseDir(), (char *)CommandLine()->GetCmdLine(), szNewCommandParams, Sys_GetFactoryThis(), filesystemFactoryFn);
			}
		}

		Sys_UnloadModule(engineModule);

		switch (engineResult)
		{
		case ENGINE_RESULT_NONE:
			restart = false;
			break;
		case ENGINE_RESULT_RESTART:
		{
			if (!AppLauncher()->FreeEngine()) {
				break;
			}

			restart = true;
			break;
		}
		case ENGINE_RESULT_UNSUPPORTEDVIDEO:
			restart = AppLauncher()->OnVideoModeFailed();
			break;
		}

		// Remove any overrides in case settings changed
		CommandLine()->RemoveParm("-sw");
		CommandLine()->RemoveParm("-startwindowed");
		CommandLine()->RemoveParm("-windowed");
		CommandLine()->RemoveParm("-window");
		CommandLine()->RemoveParm("-full");
		CommandLine()->RemoveParm("-fullscreen");
		CommandLine()->RemoveParm("-soft");
		CommandLine()->RemoveParm("-software");
		CommandLine()->RemoveParm("-gl");
		CommandLine()->RemoveParm("-d3d");
		CommandLine()->RemoveParm("-w");
		CommandLine()->RemoveParm("-width");
		CommandLine()->RemoveParm("-h");
		CommandLine()->RemoveParm("-height");
		CommandLine()->RemoveParm("+connect");
		CommandLine()->SetParm   ("-novid", 0);

		if (Q_strstr(szNewCommandParams, "-game")) {
			CommandLine()->RemoveParm("-game");
		}

		if (Q_strstr(szNewCommandParams, "+load")) {
			CommandLine()->RemoveParm("+load");
		}

		CommandLine()->AppendParm(szNewCommandParams, nullptr);
		g_pFileSystem->Unmount();
		Sys_UnloadModule(filesystemModule);
	}

	AppLauncher()->ShutDown();
	return 0;
}
