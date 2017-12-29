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

#if !defined(_WIN32)
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
	bool GetExecutableName(char *out, int len);
	char *GetBaseDir();

private:
	char m_BaseDir[MAX_PATH];
	char m_ExeName[MAX_PATH];
};

CAppLauncher g_AppLauncher;
IAppLauncher *applauncher = &g_AppLauncher;

IAppLauncher *AppLauncher()
{
	return applauncher;
}

bool CAppLauncher::Init(const char *commandline)
{
	const char en_US[] = "en_US.UTF-8";

	setenv("LC_ALL", en_US, 1);
	setlocale(LC_ALL, en_US);

	const char *pCurrentLocale = setlocale(LC_ALL, nullptr);
	if (Q_stricmp(pCurrentLocale, en_US) != 0)
	{
		fprintf(stderr, "WARNING: setlocale('%s') failed, using locale:'%s'. International characters may not work.\n", en_US, pCurrentLocale);
	}

	Q_snprintf(m_ExeName, sizeof(m_ExeName), "%s", argv[0]);
	Q_snprintf(m_BaseDir, sizeof(m_BaseDir), "%s", argv[0]);

	// strip out the exe name
	char *slash = Q_strrchr(m_BaseDir, CORRECT_PATH_SEPARATOR);
	if (slash)
	{
		*slash = '\0';
	}

	return true;
}

bool CAppLauncher::Init(int argc, const char *argv[])
{
	CommandLine()->CreateCmdLine(argc, argv);
	return true;
}

void CAppLauncher::ShutDown()
{
	;
}

bool CAppLauncher::FreeEngine()
{
	int maxref = 0;
	void *engine = dlopen(ENGINE_CLIENT_LIB, RTLD_NOLOAD);
	while (engine && maxref < 9)
	{
		maxref++;
		dlclose(engine);
		engine = dlopen(ENGINE_CLIENT_LIB, RTLD_NOLOAD);
	}

	// failed to release engine
	if (engine) {
		fprintf(stderr, "Failed to unload engine on exit (%s).", ENGINE_CLIENT_LIB);
		return false;
	}

	return true;
}

CSysModule *CAppLauncher::LoadEngineModule(const char *modulename)
{
	CSysModule *engineModule = Sys_LoadModule(modulename);
	if (engineModule) {
		return engineModule;
	}

	fprintf(stderr, "Could not load %s.\nPlease try again at a later time.", modulename);
	return nullptr;
}

CSysModule *CAppLauncher::LoadFilesystemModule(const char *modulename)
{
	CSysModule *filesystemModule = Sys_LoadModule(modulename);
	if (filesystemModule) {
		return filesystemModule;
	}

	const char message[] = "Failed to load filesystem library, exiting...\n";
	fwrite(message, sizeof(char), sizeof(message) - 1, stderr);
	return nullptr;
}

void CAppLauncher::SetEngineDLL(const char **ppEngineDLL)
{
	*ppEngineDLL = ENGINE_CLIENT_LIB;
}

bool CAppLauncher::GetExecutableName(char *dest, size_t len)
{
	Q_strnlcpy(out, m_ExeName, len);
	return true;
}

bool CAppLauncher::OnInitVideoMode()
{
	true;
}

bool CAppLauncher::OnVideoModeFailed()
{
	const char message[] = "The specified video mode is not supported.\nThe game will now exit.\n";
	fwrite(message, sizeof(char), sizeof(message) - 1, stderr);
	return false;
}

char *CAppLauncher::GetBaseDir()
{
	return m_BaseDir;
}

#endif // !defined(_WIN32)
