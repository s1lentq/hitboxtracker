#include "precompiled.h"

const size_t CSysModule::BASE_OFFSET = 0x01D00000;
const module_handle_t CSysModule::INVALID_HANDLE = (module_handle_t)0;

CSysModule::CSysModule() : m_handle(INVALID_HANDLE), m_base(nullptr), m_size(0), m_free(true)
{
}

CSysModule::~CSysModule()
{
}

bool CSysModule::Init(const char *szModuleName, const char *pszFile)
{
	m_handle = GetModuleHandle(szModuleName);

	if (!m_handle) {
		return false;
	}

	MODULEINFO module_info;
	if (GetModuleInformation(GetCurrentProcess(), m_handle, &module_info, sizeof(module_info)))
	{
		m_base = (byteptr_t)module_info.lpBaseOfDll;
		m_size = module_info.SizeOfImage;
	}

	m_debug.SetFile(pszFile);
	return true;
}

const char *CSysModule::getFileName()
{
	static char szFileName[MAX_PATH] = "";
	if (m_handle == INVALID_HANDLE) {
		return "";
	}

	GetModuleFileName(m_handle, szFileName, sizeof(szFileName));
	return szFileName;
}

void CSysModule::Printf(const char *fmt, ...)
{
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	Q_vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	m_debug.Printf("%s", string);
}

void CSysModule::TraceLog(const char *fmt, ...)
{
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	Q_vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	m_debug.TraceLog("%s", string);
}

module_handle_t CSysModule::load(void *addr)
{
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(addr), &m_handle)) {
		return INVALID_HANDLE;
	}

	MEMORY_BASIC_INFORMATION mem;
	if (!VirtualQuery(m_handle, &mem, sizeof(mem)))
		return INVALID_HANDLE;

	if (mem.State != MEM_COMMIT)
		return INVALID_HANDLE;

	if (!mem.AllocationBase)
		return INVALID_HANDLE;

	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)mem.AllocationBase;
	IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS *)((uintptr_t)dos + (uintptr_t)dos->e_lfanew);

	if (pe->Signature != IMAGE_NT_SIGNATURE)
		return INVALID_HANDLE;

	m_free = false;
	m_base = (byteptr_t)mem.AllocationBase;
	m_size = (size_t)pe->OptionalHeader.SizeOfImage;

	return m_handle;
}

module_handle_t CSysModule::find(void *addr)
{
	module_handle_t hHandle = INVALID_HANDLE;
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(addr), &hHandle)) {
		return INVALID_HANDLE;
	}

	return hHandle;
}

module_handle_t CSysModule::load(const char *filepath)
{
	if (!m_handle) {
		m_handle = LoadLibrary(filepath);

		MODULEINFO module_info;
		if (GetModuleInformation(GetCurrentProcess(), m_handle, &module_info, sizeof(module_info))) {
			m_base = (byteptr_t)module_info.lpBaseOfDll;
			m_size = module_info.SizeOfImage;
		}
	}

	return m_handle;
}

bool CSysModule::unload()
{
	if (m_handle == INVALID_HANDLE) {
		return false;
	}

	bool ret = true;
	if (m_free) {
		ret = FreeLibrary(m_handle) != ERROR;
	}

	m_handle = INVALID_HANDLE;
	m_base = 0;
	m_size = 0;

	return ret;
}

bool CSysModule::is_opened() const
{
	return m_handle != INVALID_HANDLE;
}

byteptr_t CSysModule::find_pattern(byteptr_t pStart, size_t range, const char *pattern) const
{
	return m_pattern.find(pStart, range, (byteptr_t)pattern, Q_strlen(pattern));
}

byteptr_t CSysModule::find_pattern_back(byteptr_t pStart, size_t range, const char *pattern) const
{
	return m_pattern.find_back(pStart, range, (byteptr_t)pattern, Q_strlen(pattern));
}

byteptr_t CSysModule::find_string(const char *string, int8_t opcode) const
{
	auto ptr = m_pattern.find(m_base, m_size, (byteptr_t)string, Q_strlen(string) + 1);
	if (!ptr) {
		return nullptr;
	}

	return m_pattern.find_ref(m_base, m_base + m_size - 5, (uint32_t)ptr, opcode);
}

byteptr_t CSysModule::getsym(const char *name) const
{
	return m_handle ? (byteptr_t)GetProcAddress(m_handle, name) : nullptr;
}

module_handle_t CSysModule::gethandle() const
{
	return m_handle;
}

byteptr_t CSysModule::getbase() const
{
	return m_base;
}

size_t CSysModule::getsize() const
{
	return m_size;
}

const char *CSysModule::getloaderror()
{
#ifdef _WIN32
	static char buf[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, sizeof(buf) - 1, nullptr);
	return buf;
#else
	return dlerror();
#endif
}

bool CSysModule::CPattern::compare(byteptr_t pStart, const byteptr_t pattern, size_t len) const
{
	for (auto c = pattern, pattern_end = pattern + len; c < pattern_end; c++, pStart++)
	{
		if (*c == *pStart || *c == OP_ANY) {
			continue;
		}

		return false;
	}

	return true;
}

byteptr_t CSysModule::CPattern::find(byteptr_t pStart, size_t range, const byteptr_t pattern, size_t len) const
{
	for (auto c = pStart + range - len; pStart < c; pStart++) {
		if (compare(pStart, pattern, len))
			return pStart;
	}

	return nullptr;
}

byteptr_t CSysModule::CPattern::find_back(byteptr_t pStart, size_t range, const byteptr_t pattern, size_t len) const
{
	for (auto c = pStart - range - len; pStart > c; pStart--) {
		if (compare(pStart, pattern, len))
			return pStart;
	}

	return nullptr;
}

byteptr_t CSysModule::CPattern::find_ref(byteptr_t pStart, byteptr_t pEnd, uint32_t ref, int8_t opcode, bool relative) const
{
	for (; pStart < pEnd; pStart++)
	{
		if (opcode != OP_ANY && *pStart != opcode) {
			continue;
		}

		if (relative)
		{
			if ((uint32_t)pStart + 5 + *(uint32_t *)(pStart + 1) == ref)
				return pStart;
		}
		else
		{
			if (*(uint32_t *)(pStart + 1) == ref)
				return pStart;
		}
	}

	return nullptr;
}

byteptr_t CSysModule::CPattern::find_ref_back(byteptr_t pStart, byteptr_t pBase, uint32_t ref, int8_t opcode, bool relative) const
{
	for (; pStart > pBase; pStart--)
	{
		if (opcode != OP_ANY && *pStart != opcode) {
			continue;
		}

		if (relative)
		{
			if ((uint32_t)pStart + 5 + *(uint32_t *)(pStart + 1) == ref)
				return pStart;
		}
		else
		{
			if (*(uint32_t *)(pStart + 1) == ref)
				return pStart;
		}
	}

	return nullptr;
}

bool CSysModule::SetHook(byteptr_t addr, hook_t *hook)
{
	hook->SetFunc(addr);

	// copy original bytes
	::memcpy(hook->m_originalBytes, addr, sizeof(hook->m_originalBytes));

	// make patch
	return memcpy(addr, &hook->m_jmpBytes, sizeof(hook->m_jmpBytes));
}

bool CSysModule::set(hook_t *hook)
{
	if (!hook->m_addr)
		return false;

	return memcpy(hook->m_addr, &hook->m_jmpBytes, hook->m_size);
}

bool CSysModule::unset(hook_t *hook)
{
	if (!hook->m_addr)
		return false;

	return memcpy(hook->m_addr, hook->m_originalBytes, hook->m_size);
}

bool CSysModule::memcpy(void *dst, void *src, size_t size)
{
	static HANDLE process = 0;

	DWORD OldProtection = 0;
	DWORD NewProtection = PAGE_EXECUTE_READWRITE;

	if (!process)
		process = GetCurrentProcess();

	auto res = false;
	//FlushInstructionCache(process, dst, size);
	if (VirtualProtect(dst, size, NewProtection, &OldProtection))
	{
		::memcpy(dst, src, size);
		res = !!VirtualProtect(dst, size, OldProtection, &NewProtection);
	}

	FlushInstructionCache(process, dst, size);
	return res;
}

// Prints assembler code in the runtime like IDA Pro
// but not all opcodes can correctly to handle
void CSysModule::StartDumpBytes(byteptr_t pStart, size_t range)
{
	m_debug.TraceLog("		-> START DUMP: (%p)\n", pStart);

	size_t nPos;
	for (nPos = 0; nPos < range; nPos++) {
		pStart += dumpBytes(pStart, true);
	}

	m_debug.TraceLog("		-> END DUMP: (%p), pos: (%d)\n", pStart, nPos);
}

size_t CSysModule::dumpBytes(byteptr_t pStart, bool collect_nopBytes) const
{
	static bool prevNopBytes = false;
	byteptr_t pBytes = pStart;

	// print address
	if (!prevNopBytes || pBytes[0] != 0x90)
	{
		m_debug.TraceLog("%s0%X: ", prevNopBytes ? "\n" : "", (pStart - m_base + BASE_OFFSET));
		prevNopBytes = false;
	}

	int iNumBytesOnLines = 1;
	switch (pBytes[0])
	{
	case 0x68: // push
		iNumBytesOnLines = 5;
		break;

	case 0xE8: // call
	case 0xE9: // jmp
	case 0xEB: // jmp
	case 0xB8: // mov
	case 0xBF: // mov
	case 0xA2: // mov
		iNumBytesOnLines = 5;
		break;

	case 0x8D: // lea
	case 0x81: // cmp
	case 0xD8: // fcomp
	case 0xD9: // fld
	case 0x0F: // jnz
		iNumBytesOnLines = 6;
		break;

	case 0xC6: // mov
		iNumBytesOnLines = 7;
		break;

	case 0x75: // jnz
	case 0x72: // jb
	case 0x84: // test
	case 0xDF: // fnstsw
	case 0x7A: // jp
		iNumBytesOnLines = 2;
		break;

	case 0x8B: // mov
	{
		if (pBytes[1] == 0x5D)
		{
			iNumBytesOnLines = 3;
			break;
		}
		else if (pBytes[1] == 0x35)
		{
			iNumBytesOnLines = 6;
			break;
		}

		iNumBytesOnLines = 2;
		break;
	}
	case 0x33: // xor
	case 0x83: // add
	case 0xF6: // test
		iNumBytesOnLines = 3;
		break;

	case 0x90:
		if (!collect_nopBytes)
		{
			iNumBytesOnLines = 1;
			break;
		}

		m_debug.TraceLog(" 90");
		prevNopBytes = true;
		return 1;

	default:
		iNumBytesOnLines = 1;
		break;
	}

	for (int i = 0; i < iNumBytesOnLines; i++)
	{
		//m_pattern.find(nullptr, 0, nullptr, 1);

		m_debug.Printf("");

		m_debug.TraceLog(" %s%X", (pBytes[i] <= 0x0A) ? "0" : "", pBytes[i]);
	}

	if (!prevNopBytes)
	{
		m_debug.TraceLog("\n");
	}

	return iNumBytesOnLines;
}

CSysModule::CMemDebug::CMemDebug()
{
	m_DebugFile[0] = '\0';
}

void CSysModule::CMemDebug::SetFile(const char *pszFileName)
{
	Q_strlcpy(m_DebugFile, pszFileName);
}

void CSysModule::CMemDebug::TraceLog(const char *fmt, ...) const
{
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	Q_vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	if (m_DebugFile[0] == '\0') {
		Printf(string);
		return;
	}

	FILE *fp = fopen(m_DebugFile, "a+");
	if (!fp) {
		return;
	}

	fprintf(fp, "%s", string);
	fclose(fp);
}

void CSysModule::CMemDebug::Printf(const char *fmt, ...) const
{
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	Q_vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	printf("%s", string);
}
