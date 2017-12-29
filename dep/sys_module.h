#pragma once

#include <ISysModule.h>

#define ResolveFunc(addr, func)\
	(void *)((size_t)func - (size_t)addr - 5)

#pragma pack(push, 1)
struct jmp_t
{
	char _jmp;
	void *addr;
};
#pragma pack(pop)

constexpr size_t MAX_JUMP_SIZE = sizeof(jmp_t);

struct hook_t
{
	hook_t(int8_t opcode, void *handler)
	{
		m_addr = nullptr;
		m_handler = handler;

		m_jmpBytes._jmp = opcode;
		m_jmpBytes.addr = nullptr;

		memset(&m_originalBytes, 0, sizeof(m_originalBytes));
	}

	void SetFunc(byteptr_t addr)
	{
		if (m_handler)
		{
			m_addr = addr;
			m_jmpBytes.addr = ResolveFunc(addr, m_handler);
		}
	}

	void *m_addr;
	void *m_handler;
	size_t m_size;
	jmp_t m_jmpBytes;
	byte m_originalBytes[MAX_JUMP_SIZE];
};

class CSysModule: virtual public ISysModule
{
public:
	CSysModule();
	~CSysModule();

	bool Init(const char *szModuleName, const char *pszFile = nullptr);

	module_handle_t load(void *addr);
	module_handle_t load(const char *filename);
	bool unload();

	void Printf(const char *fmt, ...);
	void TraceLog(const char *fmt, ...);

	byteptr_t getsym(const char *name) const;
	module_handle_t gethandle() const;
	byteptr_t getbase() const;
	size_t getsize() const;
	bool is_opened() const;
	const char *getFileName();

	bool set(hook_t *hook);
	bool unset(hook_t *hook);
	bool memcpy(void *dst, void *src, size_t size);

	bool SetHook(byteptr_t addr, hook_t *hook);

	template <typename TMethod = void (*)()>
	size_t GetFunc(byteptr_t addr, TMethod *(*method) = nullptr);

	template <typename TMethod = void (*)(), typename T>
	size_t GetFunc(byteptr_t addr, TMethod *(T::*method) = nullptr);

	template <size_t size>
	byteptr_t find_pattern(const char (&pattern)[size]) const;

	template <size_t size>
	byteptr_t find_pattern(byteptr_t pStart, const char (&pattern)[size]) const;

	template <size_t size>
	byteptr_t find_pattern_back(byteptr_t pStart, const char (&pattern)[size]) const;

	byteptr_t find_pattern(byteptr_t pStart, size_t range, const char *pattern) const;
	byteptr_t find_pattern_back(byteptr_t pStart, size_t range, const char *pattern) const;
	byteptr_t find_string(const char *string, int8_t opcode = OP_ANY) const;

	void StartDumpBytes(byteptr_t pStart, size_t range);

	static module_handle_t find(void *addr);
	static const char *getloaderror();
	static const module_handle_t INVALID_HANDLE;
	static const size_t BASE_OFFSET;

private:
	module_handle_t m_handle;

protected:
	size_t dumpBytes(byteptr_t pStart, bool collect_nopBytes = true) const;

	class CPattern
	{
	public:
		byteptr_t find(byteptr_t pStart, size_t range, byteptr_t pattern, size_t len) const;
		byteptr_t find_ref(byteptr_t pStart, byteptr_t pEnd, uint32_t ref, int8_t opcode = OP_ANY, bool relative = false) const;

		byteptr_t find_back(byteptr_t pStart, size_t range, byteptr_t pattern, size_t len) const;
		byteptr_t find_ref_back(byteptr_t pStart, byteptr_t pBase, uint32_t ref, int8_t opcode = OP_ANY, bool relative = false) const;

		bool compare(byteptr_t pStart, const byteptr_t pattern, size_t len) const;
	};

	class CMemDebug
	{
	public:
		CMemDebug();

		void SetFile (const char *pszFileName);
		void TraceLog(const char *fmt, ...) const;
		void Printf  (const char *fmt, ...) const;

	protected:
		friend class CSysModule;
		char m_DebugFile[MAX_PATH];
	};

protected:
	CMemDebug       m_debug;
	CPattern        m_pattern;
	byteptr_t       m_base;
	size_t          m_size;
	bool            m_free; // m_handle should be released
};

template <size_t size>
byteptr_t CSysModule::find_pattern(byteptr_t pStart, const char (&pattern)[size]) const
{
	return m_pattern.find(pStart, m_size, (byteptr_t)pattern, size - 1);
}

template <size_t size>
byteptr_t CSysModule::find_pattern_back(byteptr_t pStart, const char (&pattern)[size]) const
{
	return m_pattern.find_back(pStart, m_size, (byteptr_t)pattern, size - 1);
}

template <size_t size>
byteptr_t CSysModule::find_pattern(const char (&pattern)[size]) const
{
	return m_pattern.find(m_base, m_size, (byteptr_t)pattern, size - 1);
}

template <typename TMethod>
size_t CSysModule::GetFunc(byteptr_t addr, TMethod *(*method))
{
	auto ptr = (size_t)addr + *(size_t *)(addr + 1) + 5;
	if (method)
	{
		*method = reinterpret_cast<TMethod (*)>(ptr);
	}

	return ptr;
}

template <typename TMethod, typename T>
size_t CSysModule::GetFunc(byteptr_t addr, TMethod *(T::*method))
{
	auto ptr = (size_t)addr + *(size_t *)(addr + 1) + 5;
	if (method)
	{
		*method = reinterpret_cast<TMethod (T::*)>(ptr);
	}

	return ptr;
}