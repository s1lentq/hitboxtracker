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

enum ModelType_e
{
	CS_DEFAULT,
	CS_LEET,
	CS_GIGN,
	CS_VIP,
	CS_GSG9,
	CS_GUERILLA,
	CS_ARCTIC,
	CS_SAS,
	CS_TERROR,
	CS_URBAN,
	CS_SPETSNAZ,
	CS_MILITIA,
};

bool IsValidCTModelIndex(int modelType);
bool IsValidTModelIndex(int modelType);

void CounterStrike_GetSequence(int *seq, int *gaitseq);
void CounterStrike_SetSequence(int seq, int gaitseq);
void CounterStrike_SetOrientation(Vector *p_o, Vector *p_a);
void CounterStrike_GetOrientation(float *o, float *a);

extern const char *sPlayerModelFiles[];
