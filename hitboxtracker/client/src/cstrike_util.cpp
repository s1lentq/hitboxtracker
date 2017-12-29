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

int g_runfuncs;
int g_rseq, g_gaitseq;
Vector g_clorg, g_clang;

const char *sPlayerModelFiles[] =
{
	"models/player.mdl",
	"models/player/leet/leet.mdl",
	"models/player/gign/gign.mdl",
	"models/player/vip/vip.mdl",
	"models/player/gsg9/gsg9.mdl",
	"models/player/guerilla/guerilla.mdl",
	"models/player/arctic/arctic.mdl",
	"models/player/sas/sas.mdl",
	"models/player/terror/terror.mdl",
	"models/player/urban/urban.mdl",
	"models/player/spetsnaz/spetsnaz.mdl", // CZ
	"models/player/militia/militia.mdl"    // CZ
};

void HUD_PostRunCmd(local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_runfuncs = runfuncs;
	gClientfuncs.pPostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	if (to->client.fov != g_lastFOV)
	{
		g_lastFOV = to->client.fov;
	}

	HUD_WeaponsPostThink(from, to, cmd, time, random_seed);

	if (runfuncs)
	{
		g_gaitseq = to->playerstate.gaitsequence;
		g_rseq = to->playerstate.sequence;

		g_clorg = to->playerstate.origin;
		g_clang = cmd->viewangles;
	}
}

void CounterStrike_GetSequence(int *seq, int *gaitseq)
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

void CounterStrike_SetSequence(int seq, int gaitseq)
{
	g_rseq = seq;
	g_gaitseq = gaitseq;
}

void CounterStrike_SetOrientation(Vector *p_o, Vector *p_a)
{
	g_clorg = *p_o;
	g_clang = *p_a;
}

void CounterStrike_GetOrientation(float *o, float *a)
{
	*(Vector *)o = g_clorg;
	*(Vector *)a = g_clang;
}

bool IsValidCTModelIndex(int modelType)
{
	switch (static_cast<ModelType_e>(modelType))
	{
	case CS_GIGN:
	case CS_GSG9:
	case CS_SAS:
	case CS_URBAN:
	case CS_SPETSNAZ:
		return true;
	default:
		break;
	}

	return false;
}

bool IsValidTModelIndex(int modelType)
{
	switch (static_cast<ModelType_e>(modelType))
	{
	case CS_LEET:
	case CS_GUERILLA:
	case CS_ARCTIC:
	case CS_TERROR:
	case CS_MILITIA:
		return true;
	default:
		break;
	}

	return false;
}
