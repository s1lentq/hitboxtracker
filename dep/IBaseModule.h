#pragma once

#include <ISysModule.h>

class IBaseModule
{
public:
	virtual ~IBaseModule() {};

	virtual bool Init() = 0;
	virtual ISysModule *GetModule() const = 0;
};
