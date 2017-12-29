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

#include "studio_util.h"
#include "studio/StudioModelRenderer.h"

#define NUM_BLENDING                9

#define ANIM_WALK_SEQUENCE          3
#define ANIM_JUMP_SEQUENCE          6
#define ANIM_SWIM_1                 8
#define ANIM_SWIM_2                 9
#define ANIM_FIRST_DEATH_SEQUENCE   101

enum BoneIndex
{
	BONE_HEAD = 0,
	BONE_PELVIS,
	BONE_SPINE1,
	BONE_SPINE2,
	BONE_SPINE3,

	BONE_MAX,
};

class CGameStudioModelRenderer: public CStudioModelRenderer
{
public:
	CGameStudioModelRenderer();

	// Set up model bone positions
	virtual void StudioSetupBones();

	// Estimate gait frame for player
	virtual void StudioEstimateGait(entity_state_t *pplayer);

	// Process movement of player
	virtual void StudioProcessGait(entity_state_t *pplayer);

	// Player drawing code
	virtual int StudioDrawPlayer(int flags, entity_state_t *pplayer);
	virtual int _StudioDrawPlayer(int flags, entity_state_t *pplayer);

	// Apply special effects to transform matrix
	virtual void StudioFxTransform(cl_entity_t *ent, float transform[3][4]);

	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch);
	virtual void CalculateYawBlend(entity_state_t *pplayer);
	virtual void CalculatePitchBlend(entity_state_t *pplayer);

private:
	// For local player, in third person, we need to store real render
	// data and then setup for with fake/client side animation data
	void SavePlayerState(entity_state_t *pplayer);
	void SetupClientAnimation(entity_state_t *pplayer);	// Called to set up local player's animation values
	void RestorePlayerState(entity_state_t *pplayer);	// Called to restore original player state information

	bool GetPlayerBoneWorldPosition(BoneIndex whichBone, Vector &pos);
	mstudioanim_t *LookupAnimation(mstudioseqdesc_t *pseqdesc, int index);
	void CachePlayerBoneIndices();
	int GetPlayerBoneIndex(BoneIndex whichBone);

private:
	static const char *m_boneNames[];

	// Private data
	int m_nPlayerGaitSequences[MAX_CLIENTS];
	bool m_bLocal;
	int m_boneIndexCache[BONE_MAX];
	bool m_isBoneCacheValid;
};

extern CGameStudioModelRenderer g_StudioRenderer;
