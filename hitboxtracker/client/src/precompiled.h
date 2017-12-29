#pragma once

//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstdio>
#include <cstddef>

#include <maintypes.h>
#include <basetypes.h>

#include <math.h>
#include <hltv.h>
#include <stdlib.h>
#include <IBaseModule.h>
#include <weapontype.h>

#include "maintypes.h"
#include "strtools.h"
#include "vector.h"
#include "const.h"
#include "edict.h"
#include "eiface.h"
#include "const.h"
#include "vmodes.h"
#include "usermsg.h"
#include "APIProxy.h"
#include "r_studioint.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "pm_info.h"
#include "cvardef.h"
#include "studio.h"
#include "event_args.h"
#include "event_flags.h"
#include "keydefs.h"
#include "kbutton.h"
#include "com_model.h"
#include "ref_params.h"
#include "studio_event.h"
#include "net_api.h"
#include "r_efx.h"
#include "parsemsg.h"
#include "event_api.h"
#include "screenfade.h"
#include "engine_launcher_api.h"
#include "entity_types.h"
#include "cdll_int.h"
#include "cdll_dll.h"
#include "client.h"

#include "main.h"
#include "hud.h"
#include "sys_module.h"
#include "cstrike_util.h"
#include "studio/GameStudioModelRenderer.h"

// Modules
#include "modules/engine.h"	// Handle hw.dll
#include "modules/client.h"	// Handle client.dll
