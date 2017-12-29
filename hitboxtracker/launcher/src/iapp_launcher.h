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

#if defined (_WIN32)
	#define LAUNCHER_NAME "hl.exe"
#else
	#define LAUNCHER_NAME "hl_linux"
#endif // _WIN32

// Interface to launcher app
class IAppLauncher {
public:
	virtual bool Init(const char *commandline) = 0;
	virtual bool Init(int argc, const char *argv[]) = 0;

	virtual void ShutDown() = 0;
	virtual bool FreeEngine() = 0;

	virtual CSysModule *LoadEngineModule(const char *modulename) = 0;
	virtual CSysModule *LoadFilesystemModule(const char *modulename) = 0;

	virtual bool OnInitVideoMode() = 0;
	virtual bool OnVideoModeFailed() = 0;

	virtual void SetEngineDLL(const char **ppEngineDLL) = 0;
	virtual bool GetExecutableName(char *dest, size_t len) = 0;
	virtual char *GetBaseDir() = 0;
};

IAppLauncher *AppLauncher();
