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

#ifndef _WIN32
typedef int HKEY;
#endif

// Exposes registry interface to rest of launcher
class CRegistry: public IRegistry
{
public:
	CRegistry();
	virtual ~CRegistry();

	void Init();
	void Shutdown();

	int ReadInt(const char *key, int defaultValue = 0);
	void WriteInt(const char *key, int value);

	const char *ReadString(const char *key, const char *defaultValue = nullptr);
	void WriteString(const char *key, const char *value);

private:
	bool m_bValid;
	HKEY m_hKey;
};

// Expose to launcher
static CRegistry g_Registry;
IRegistry *registry = &g_Registry;

CRegistry::CRegistry()
{
	// Assume failure
	m_bValid = false;
	m_hKey = 0;
}

CRegistry::~CRegistry()
{
}

// Purpose: Read integer from registry
// Input  : *key -
//			defaultValue -
// Output : int
int CRegistry::ReadInt(const char *key, int defaultValue)
{
#ifdef _WIN32
	LONG lResult; // Registry function result code
	DWORD dwType; // Type of key
	DWORD dwSize; // Size of element data

	int value;

	if (!m_bValid)
	{
		return defaultValue;
	}

	dwSize = sizeof(DWORD);

	lResult = RegQueryValueEx(
		m_hKey,				// handle to key
		key,				// value name
		0,					// reserved
		&dwType,			// type buffer
		(LPBYTE)&value,		// data buffer
		&dwSize);			// size of data buffer

	if (lResult != ERROR_SUCCESS)  // Failure
		return defaultValue;

	if (dwType != REG_DWORD)
		return defaultValue;

	return value;
#else
	return defaultValue;
#endif
}

// Purpose: Save integer to registry
// Input  : *key -
//			value -
void CRegistry::WriteInt(const char *key, int value)
{
#ifdef _WIN32
	// Size of element data
	DWORD dwSize;

	if (!m_bValid)
	{
		return;
	}

	dwSize = sizeof(DWORD);

	RegSetValueEx(
		m_hKey,			// handle to key
		key,			// value name
		0,				// reserved
		REG_DWORD,		// type buffer
		(LPBYTE)&value,	// data buffer
		dwSize);		// size of data buffer
#endif
}

// Purpose: Read string value from registry
// Input  : *key -
//			*defaultValue -
// Output : const char
const char *CRegistry::ReadString(const char *key, const char *defaultValue)
{
#ifdef _WIN32
	LONG lResult;	// Type of key
	DWORD dwType;	// Size of element data
	DWORD dwSize = 512;

	static char value[512];

	value[0] = 0;

	if (!m_bValid)
	{
		return defaultValue;
	}

	lResult = RegQueryValueEx(
		m_hKey,						// handle to key
		key,						// value name
		0,							// reserved
		&dwType,					// type buffer
		(unsigned char *)value,		// data buffer
		&dwSize);					// size of data buffer

	if (lResult != ERROR_SUCCESS)
	{
		return defaultValue;
	}

	if (dwType != REG_SZ)
	{
		return defaultValue;
	}

	return value;
#else
	return defaultValue;
#endif
}

// Purpose: Save string to registry
// Input  : *key -
//			*value -
void CRegistry::WriteString(const char *key, const char *value)
{
#ifdef _WIN32
	DWORD dwSize;	// Size of element data

	if (!m_bValid)
	{
		return;
	}

	dwSize = strlen(value) + 1;

	RegSetValueEx(
		m_hKey,			// handle to key
		key,			// value name
		0,				// reserved
		REG_SZ,			// type buffer
		(LPBYTE)value,	// data buffer
		dwSize);		// size of data buffer
#endif
}

// FIXME:  SHould be "steam"
static char *GetPlatformName()
{
	return "Half-Life";
}

// Purpose: Open default launcher key based on game directory
void CRegistry::Init()
{
#ifdef _WIN32
	LONG lResult;			// Registry function result code
	DWORD dwDisposition;	// Type of key opening event

	char szModelKey[1024];
	wsprintf(szModelKey, "Software\\Valve\\%s\\Settings\\", GetPlatformName());

	lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,			// handle of open key
		szModelKey,					// address of name of subkey to open
		0,							// DWORD ulOptions,	  // reserved
		NULL,						// Type of value
		REG_OPTION_NON_VOLATILE,	// Store permanently in reg.
		KEY_ALL_ACCESS,				// REGSAM samDesired, // security access mask
		NULL,
		&m_hKey,					// Key we are creating
		&dwDisposition);			// Type of creation

	if (lResult != ERROR_SUCCESS)
	{
		m_bValid = false;
		return;
	}

	// Success
	m_bValid = true;
#endif
}

void CRegistry::Shutdown()
{
#ifdef _WIN32
	if (!m_bValid)
		return;

	// Make invalid
	m_bValid = false;
	RegCloseKey(m_hKey);
#endif
}
