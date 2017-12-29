#pragma once

#define PSAPI_VERSION 1
#include <psapi.h>

using module_handle_t = HINSTANCE;
using byteptr_t = uint8_t *;

enum OpCodes : int8_t
{
	OP_ANY  = '\x2A',
	OP_PUSH = '\x68',
	OP_JUMP = '\xE9',
	OP_CALL = '\xE8',
};

class ISysModule
{
public:
	virtual ~ISysModule() {}
	virtual bool Init(const char *szModuleName, const char *pszFile = nullptr) = 0;

	virtual module_handle_t load(void *addr) = 0;
	virtual module_handle_t load(const char *filename) = 0;

	virtual void Printf(const char *fmt, ...) = 0;
	virtual void TraceLog(const char *fmt, ...) = 0;

	virtual byteptr_t getsym(const char *name) const = 0;
	virtual module_handle_t gethandle() const = 0;
	virtual byteptr_t getbase() const = 0;
	virtual size_t getsize() const = 0;
	virtual bool is_opened() const = 0;

	virtual byteptr_t find_string(const char *string, int8_t opcode = OP_ANY) const = 0;
	virtual byteptr_t find_pattern(byteptr_t pStart, size_t range, const char *pattern) const = 0;
};
