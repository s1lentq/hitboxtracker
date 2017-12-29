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

float g_flStartScaleTime;
int g_iPrevRenderState;
qboolean g_iRenderStateChanged;

typedef struct
{
	vec3_t origin;
	vec3_t angles;

	vec3_t realangles;

	float animtime;
	float frame;
	int sequence;
	int gaitsequence;
	float framerate;

	qboolean m_fSequenceLoops;
	qboolean m_fSequenceFinished;

	byte controller[4];
	byte blending[2];
	latchedvars_t lv;

} client_anim_state_t;

static client_anim_state_t g_state;
static client_anim_state_t g_clientstate;

// The renderer object, created on the stack.
CGameStudioModelRenderer g_StudioRenderer;

CGameStudioModelRenderer::CGameStudioModelRenderer()
{
	// If you want to predict animations locally, set this to TRUE
	// NOTE:  The animation code is somewhat broken, but gives you a sense for how
	//  to do client side animation of the predicted player in a third person game.
	m_bLocal = false;
}

void CGameStudioModelRenderer::StudioSetupBones()
{
	double f;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	static vec4_t q[MAXSTUDIOBONES];
	float bonematrix[3][4];

	static float pos2[MAXSTUDIOBONES][3];
	static vec4_t q2[MAXSTUDIOBONES];
	static float pos3[MAXSTUDIOBONES][3];
	static vec4_t q3[MAXSTUDIOBONES];
	static float pos4[MAXSTUDIOBONES][3];
	static vec4_t q4[MAXSTUDIOBONES];

	// Use default bone setup for nonplayers
	if (!m_pCurrentEntity->player)
	{
		CStudioModelRenderer::StudioSetupBones();
		return;
	}

	// Bound sequence number
	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.sequence < 0)
	{
		m_pCurrentEntity->curstate.sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;
	panim = StudioGetAnim(m_pRenderModel, pseqdesc);

	if (m_pPlayerInfo)
	{
		int playerNum = m_pCurrentEntity->index - 1;
		if (m_nPlayerGaitSequences[playerNum] != ANIM_JUMP_SEQUENCE && m_pPlayerInfo->gaitsequence == ANIM_JUMP_SEQUENCE)
		{
			m_pPlayerInfo->gaitframe = 0;
		}

		m_nPlayerGaitSequences[playerNum] = m_pPlayerInfo->gaitsequence;
	}

	f = StudioEstimateFrame(pseqdesc);

	if (m_pPlayerInfo->gaitsequence == ANIM_WALK_SEQUENCE)
	{
		if (m_pCurrentEntity->curstate.blending[0] > 26)
		{
			m_pCurrentEntity->curstate.blending[0] -= 26;
		}
		else
		{
			m_pCurrentEntity->curstate.blending[0] = 0;
		}

		m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
	}

	// This game knows how to do three way blending
	if (pseqdesc->numblends != NUM_BLENDING)
	{
		StudioCalcRotations(pos, q, pseqdesc, panim, f);
	}
	else
	{
		float s = m_pCurrentEntity->curstate.blending[0];
		float t = m_pCurrentEntity->curstate.blending[1];

		// Blending is 0-127 == Left to Middle, 128 to 255 == Middle to right
		if (s <= 127.0f)
		{
			// Scale 0-127 blending up to 0-255
			s = (s * 2.0f);

			if (t <= 127.0f)
			{
				t = (t * 2.0f);

				StudioCalcRotations(pos, q, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 1);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 3);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
			else
			{
				t = 2.0f * (t - 127.0f);

				panim = LookupAnimation(pseqdesc, 3);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 6);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 7);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
		}
		else
		{
			// Scale 127-255 blending up to 0-255
			s = 2.0f * (s - 127.0f);

			if (t <= 127.0f)
			{
				t = (t * 2.0f);

				panim = LookupAnimation(pseqdesc, 1);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 2);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 5);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
			else
			{
				t = 2.0f * (t - 127.0f);

				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 5);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 7);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);

				panim = LookupAnimation(pseqdesc, 8);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
		}

		// Normalize interpolant
		s /= 255.0f;
		t /= 255.0f;

		// Spherically interpolate the bones
		StudioSlerpBones(q, pos, q2, pos2, s);
		StudioSlerpBones(q3, pos3, q4, pos4, s);
		StudioSlerpBones(q, pos, q3, pos3, t);
	}

	// Are we in the process of transitioning from one sequence to another.
	if (m_fDoInterp &&
		m_pCurrentEntity->latched.sequencetime &&
		(m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime) &&
		(m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq))
	{
		// blend from last sequence
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];

		float s = m_pCurrentEntity->latched.prevseqblending[0];
		float t = m_pCurrentEntity->latched.prevseqblending[1];

		// Point at previous sequenece
		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim(m_pRenderModel, pseqdesc);

		// Know how to do three way blends
		if (pseqdesc->numblends != 9)
		{
			// clip prevframe
			StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
		}
		else
		{
			// Blending is 0-127 == Left to Middle, 128 to 255 == Middle to right
			if (s <= 127.0f)
			{
				// Scale 0-127 blending up to 0-255
				s = (s * 2.0f);

				if (t <= 127.0f)
				{
					t = (t * 2.0f);

					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 1);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 3);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
				else
				{
					t = 2.0f * (t - 127.0f);

					panim = LookupAnimation(pseqdesc, 3);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 6);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 7);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
			}
			else
			{
				// Scale 127-255 blending up to 0-255
				s = 2.0f * (s - 127.0f);

				if (t <= 127.0f)
				{
					t = (t * 2.0f);

					panim = LookupAnimation(pseqdesc, 1);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 2);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 5);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
				else
				{
					t = 2.0f * (t - 127.0f);

					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 5);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 7);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

					panim = LookupAnimation(pseqdesc, 8);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
			}

			// Normalize interpolant
			s /= 255.0f;
			t /= 255.0f;

			// Spherically interpolate the bones
			StudioSlerpBones(q1b, pos1b, q2, pos2, s);
			StudioSlerpBones(q3, pos3, q4, pos4, s);
			StudioSlerpBones(q1b, pos1b, q3, pos3, t);
		}

		// Now blend last frame of previous sequence with current sequence.
		s = 1.0 - (m_clTime - m_pCurrentEntity->latched.sequencetime) / 0.2;
		StudioSlerpBones(q, pos, q1b, pos1b, s);
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	// Now convert quaternions and bone positions into matrices
	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	if (m_pPlayerInfo
		&& m_pCurrentEntity->curstate.sequence < ANIM_FIRST_DEATH_SEQUENCE
		&& m_pCurrentEntity->curstate.sequence != ANIM_SWIM_1
		&& m_pCurrentEntity->curstate.sequence != ANIM_SWIM_2)
	{
		bool bCopy = true;

		if (m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq || m_pPlayerInfo->gaitsequence < 0)
			m_pPlayerInfo->gaitsequence = 0;

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim(m_pRenderModel, pseqdesc);
		StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe);

		for (int i = 0; i < m_pStudioHeader->numbones; i++)
		{
			if (!Q_strcmp(pbones[i].name, "Bip01 Spine"))
				bCopy = false;
			else if (!Q_strcmp(pbones[pbones[i].parent].name, "Bip01 Pelvis"))
				bCopy = true;

			if (bCopy)
			{
				Q_memcpy(pos[i], pos2[i], sizeof(pos[i]));
				Q_memcpy(q[i], q2[i], sizeof(q[i]));
			}
		}
	}

	for (int i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1)
		{
			if (IEngineStudio.IsHardware())
			{
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_pbonetransform)[i]);
				MatrixCopy((*m_pbonetransform)[i], (*m_plighttransform)[i]);
			}
			else
			{
				ConcatTransforms((*m_paliastransform), bonematrix, (*m_pbonetransform)[i]);
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
			}

			// Apply client-side effects to the transformation matrix
			StudioFxTransform(m_pCurrentEntity, (*m_pbonetransform)[i]);
		}
		else
		{
			ConcatTransforms((*m_pbonetransform)[pbones[i].parent], bonematrix, (*m_pbonetransform)[i]);
			ConcatTransforms((*m_plighttransform)[pbones[i].parent], bonematrix, (*m_plighttransform)[i]);
		}
	}
}

void CGameStudioModelRenderer::StudioEstimateGait(entity_state_t *pplayer)
{
	float dt;
	if (m_pPlayerSync)
	{
		dt = clamp(m_pPlayerSync->time - m_pPlayerSync->oldtime, 0.0f, 1.0f);
	}
	else
	{
		dt = clamp(m_clTime - m_clOldTime, 0.0, 1.0);
	}

	if (dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount)
	{
		m_flGaitMovement = 0;
		return;
	}

	vec3_t est_velocity;
	if (m_fGaitEstimation)
	{
		VectorSubtract(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity);
		VectorCopy(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin);
		m_flGaitMovement = Length(est_velocity);
		if (dt <= 0 || m_flGaitMovement / dt < 5)
		{
			m_flGaitMovement = 0;
			est_velocity[0] = 0;
			est_velocity[1] = 0;
		}
	}
	else
	{
		VectorCopy(pplayer->velocity, est_velocity);
		m_flGaitMovement = Length(est_velocity) * dt;
	}

	float flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
	if (m_pCurrentEntity->curstate.sequence > 100)
	{
		m_pPlayerInfo->gaityaw += flYaw;
		return;
	}

	if (est_velocity[1] == 0 && est_velocity[0] == 0)
	{
		float flYawDiff = flYaw - (int)(flYaw / 360) * 360;

		if (flYawDiff > 180)
			flYawDiff -= 360;

		if (flYawDiff < -180)
			flYawDiff += 360;

		flYaw = fmod(flYaw, 360.0);

		if (flYaw < -180)
			flYaw += 360;

		else if (flYaw > 180)
			flYaw -= 360;

		if (flYaw > -5.0 && flYaw < 5.0)
			m_pCurrentEntity->baseline.fuser1 = 0.05;

		if (flYaw < -90.0 || flYaw > 90.0)
			m_pCurrentEntity->baseline.fuser1 = 3.5;

		if (dt < 0.25)
			flYawDiff *= dt * m_pCurrentEntity->baseline.fuser1;
		else
			flYawDiff *= dt;

		if (abs(flYawDiff) < 0.1)
			flYawDiff = 0.0;

		m_pPlayerInfo->gaityaw += flYawDiff;
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - (int)(m_pPlayerInfo->gaityaw / 360) * 360;
		m_flGaitMovement = 0;
	}
	else
	{
		m_pPlayerInfo->gaityaw = (atan2(est_velocity[1], est_velocity[0]) * (180 / M_PI));

		if (m_pPlayerInfo->gaityaw > 180)
			m_pPlayerInfo->gaityaw = 180;

		if (m_pPlayerInfo->gaityaw < -180)
			m_pPlayerInfo->gaityaw = -180;
	}
}

void CGameStudioModelRenderer::StudioProcessGait(entity_state_t *pplayer)
{
	CalculateYawBlend(pplayer);
	CalculatePitchBlend(pplayer);

	float dt;
	if (m_pPlayerSync)
	{
		dt = clamp(m_pPlayerSync->time - m_pPlayerSync->oldtime, 0.0f, 1.0f);
	}
	else
	{
		dt = clamp(m_clTime - m_clOldTime, 0.0, 1.0);
	}

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + pplayer->gaitsequence;

	if (pseqdesc->linearmovement[0] > 0)
		m_pPlayerInfo->gaitframe += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	else
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt * m_pCurrentEntity->curstate.framerate;

	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)(m_pPlayerInfo->gaitframe / pseqdesc->numframes) * pseqdesc->numframes;

	if (m_pPlayerInfo->gaitframe < 0)
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

// For local player, in third person, we need to store real
// render data and then setup for with fake/client side animation data
void CGameStudioModelRenderer::SavePlayerState(entity_state_t *pplayer)
{
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if (!ent)
		return;

	client_anim_state_t *st = &g_state;

	st->angles     = ent->curstate.angles;
	st->origin     = ent->curstate.origin;
	st->realangles = ent->angles;

	st->sequence     = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime     = ent->curstate.animtime;
	st->frame        = ent->curstate.frame;
	st->framerate    = ent->curstate.framerate;

	Q_memcpy(st->blending, ent->curstate.blending, sizeof(st->blending));
	Q_memcpy(st->controller, ent->curstate.controller, sizeof(st->controller));

	st->lv = ent->latched;
}

mstudioanim_t *CGameStudioModelRenderer::LookupAnimation(mstudioseqdesc_t *pseqdesc, int index)
{
	mstudioanim_t *panim = StudioGetAnim(m_pRenderModel, pseqdesc);
	if (index >= 0 && index <= (pseqdesc->numblends - 1))
	{
		panim += index * m_pStudioHeader->numbones;
	}

	return panim;
}

const char *CGameStudioModelRenderer::m_boneNames[] =
{
	"Bip01 Head",
	"Bip01 Pelvis",
	"Bip01 Spine1",
	"Bip01 Spine2",
	"Bip01 Spine3"
};

void CGameStudioModelRenderer::CachePlayerBoneIndices()
{
	if (m_isBoneCacheValid)
		return;

	mstudiobone_t *pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);
	for (int cacheIdx = 0; cacheIdx < ARRAYSIZE(m_boneNames); cacheIdx++)
	{
		for (int boneIdx = 0; boneIdx < m_pStudioHeader->numbones; boneIdx++)
		{
			if (!Q_strcmp(pbones[boneIdx].name, m_boneNames[cacheIdx]))
			{
				m_boneIndexCache[cacheIdx] = boneIdx;
				break;
			}
		}
	}

	m_isBoneCacheValid = true;
}

int CGameStudioModelRenderer::GetPlayerBoneIndex(BoneIndex whichBone)
{
	return m_boneIndexCache[whichBone];
}

bool CGameStudioModelRenderer::GetPlayerBoneWorldPosition(BoneIndex whichBone, Vector &pos)
{
	CachePlayerBoneIndices();

	int boneIdx = GetPlayerBoneIndex(whichBone);
	if (boneIdx < 0 || m_pStudioHeader->numbones <= boneIdx)
		return false;

	pos.x = m_rgCachedBoneTransform[boneIdx][0][3];
	pos.y = m_rgCachedBoneTransform[boneIdx][1][3];
	pos.z = m_rgCachedBoneTransform[boneIdx][2][3];

	return true;
}

void GetSequenceInfo(void *pmodel, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed)
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;
	if (!pstudiohdr)
		return;

	if (pev->sequence >= pstudiohdr->numseq || pev->sequence < 0)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = pseqdesc->linearmovement.Length();
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

int GetSequenceFlags(void *pmodel, client_anim_state_t *pev)
{
	studiohdr_t *pstudiohdr = (studiohdr_t *)pmodel;
	if (!pstudiohdr || pev->sequence >= pstudiohdr->numseq || pev->sequence < 0)
		return 0;

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;
	return pseqdesc->flags;
}

float StudioFrameAdvance(client_anim_state_t *st, float framerate, float flInterval)
{
	if (flInterval == 0.0)
	{
		flInterval = (gEngfuncs.GetClientTime() - st->animtime);
		if (flInterval <= 0.001)
		{
			st->animtime = gEngfuncs.GetClientTime();
			return 0.0;
		}
	}

	if (!st->animtime)
		flInterval = 0.0;

	st->frame += flInterval * framerate * st->framerate;
	st->animtime = gEngfuncs.GetClientTime();

	if (st->frame < 0.0 || st->frame >= 256.0)
	{
		if (st->m_fSequenceLoops)
			st->frame -= (int)(st->frame / 256.0) * 256.0;
		else
			st->frame = (st->frame < 0.0) ? 0 : 255;
		st->m_fSequenceFinished = TRUE;	// just in case it wasn't caught in GetEvents
	}

	return flInterval;
}

// Called to set up local player's animation values
void CGameStudioModelRenderer::SetupClientAnimation(entity_state_t *pplayer)
{
	static double oldtime;
	double curtime, dt;

	client_anim_state_t *st;
	float fr, gs;

	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if (!ent)
		return;

	curtime = gEngfuncs.GetClientTime();
	dt = clamp(curtime - oldtime, 0.0, 1.0);

	oldtime = curtime;
	st = &g_clientstate;

	st->framerate = 1.0;

	int oldseq = st->sequence;
	CounterStrike_GetSequence(&st->sequence, &st->gaitsequence);
	CounterStrike_GetOrientation((float *)&st->origin, (float *)&st->angles);
	st->realangles = st->angles;

	if (st->sequence != oldseq)
	{
		st->frame = 0.0;
		st->lv.prevsequence = oldseq;
		st->lv.sequencetime = st->animtime;

		Q_memcpy(st->lv.prevseqblending, st->blending, sizeof(st->lv.prevseqblending));
		Q_memcpy(st->lv.prevcontroller, st->controller, sizeof(st->lv.prevcontroller));
	}

	studiohdr_t *pmodel = (studiohdr_t *)IEngineStudio.Mod_Extradata(ent->model);

	GetSequenceInfo(pmodel, st, &fr, &gs);
	st->m_fSequenceLoops = ((GetSequenceFlags(pmodel, st) & STUDIO_LOOPING) != 0) ? TRUE : FALSE;
	StudioFrameAdvance(st, fr, dt);

	// gEngfuncs.Con_Printf("gs %i frame %f\n", st->gaitsequence, st->frame);

	ent->angles             = st->realangles;
	ent->curstate.angles    = st->angles;
	ent->curstate.origin    = st->origin;
	ent->curstate.sequence  = st->sequence;
	pplayer->gaitsequence   = st->gaitsequence;
	ent->curstate.animtime  = st->animtime;
	ent->curstate.frame     = st->frame;
	ent->curstate.framerate = st->framerate;

	Q_memcpy(ent->curstate.blending, st->blending, sizeof(ent->curstate.blending));
	Q_memcpy(ent->curstate.controller, st->controller, sizeof(ent->curstate.controller));

	ent->latched = st->lv;
}

// Called to restore original player state information
void CGameStudioModelRenderer::RestorePlayerState(entity_state_t *pplayer)
{
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if (!ent)
		return;

	client_anim_state_t *st = &g_clientstate;

	st->angles       = ent->curstate.angles;
	st->origin       = ent->curstate.origin;
	st->realangles   = ent->angles;
	st->sequence     = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime     = ent->curstate.animtime;
	st->frame        = ent->curstate.frame;
	st->framerate    = ent->curstate.framerate;

	Q_memcpy(st->blending, ent->curstate.blending, sizeof(st->blending));
	Q_memcpy(st->controller, ent->curstate.controller, sizeof(st->controller));

	st->lv = ent->latched;

	st = &g_state;

	ent->curstate.angles = st->angles;
	ent->curstate.origin = st->origin;
	ent->angles          = st->realangles;

	ent->curstate.sequence  = st->sequence;
	pplayer->gaitsequence   = st->gaitsequence;
	ent->curstate.animtime  = st->animtime;
	ent->curstate.frame     = st->frame;
	ent->curstate.framerate = st->framerate;

	Q_memcpy(ent->curstate.blending, st->blending, sizeof(ent->curstate.blending));
	Q_memcpy(ent->curstate.controller, st->controller, sizeof(ent->curstate.controller));

	ent->latched = st->lv;
}

int CGameStudioModelRenderer::StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	bool isLocalPlayer = false;

	// Set up for client?
	if (m_bLocal && IEngineStudio.GetCurrentEntity() == gEngfuncs.GetLocalPlayer())
	{
		isLocalPlayer = true;
	}

	if (isLocalPlayer)
	{
		SavePlayerState(pplayer);		// Store original data
		SetupClientAnimation(pplayer);	// Copy in client side animation data
	}

	// Call real draw function
	int iret = _StudioDrawPlayer(flags, pplayer);
	if (iret && m_pCvarDrawEntities->value == 6)
	{
		m_pPlayerSync = &g_PlayerSyncInfo[pplayer->number];
		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

		// save
		player_info_t saveplayer = *m_pPlayerInfo;
		cl_entity_t saveent = *m_pCurrentEntity;

		float gaitmovement = m_flGaitMovement;
		m_flGaitMovement = m_pPlayerSync->gaitmovement;

		VectorCopy(m_pPlayerSync->prevgaitorigin, m_pPlayerInfo->prevgaitorigin);

		m_pPlayerInfo->gaitsequence = m_pPlayerSync->gaitsequence;
		m_pPlayerInfo->gaityaw      = m_pPlayerSync->gaityaw;
		m_pPlayerInfo->gaitframe    = m_pPlayerSync->gaitframe;
		m_pPlayerInfo = nullptr;

		_StudioDrawPlayer(flags, pplayer);

		// restore
		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);
		*m_pPlayerInfo = saveplayer;
		*m_pCurrentEntity = saveent;
		m_flGaitMovement = gaitmovement;
		m_pPlayerInfo = nullptr;
		m_pPlayerSync = nullptr;
	}

	// Restore for client?
	if (isLocalPlayer)
	{
		// Restore the original data for the player
		RestorePlayerState(pplayer);
	}

	return iret;
}

bool WeaponHasAttachments(entity_state_t *pplayer)
{
	if (!pplayer)
		return false;

	model_t *pweaponmodel = IEngineStudio.GetModelByIndex(pplayer->weaponmodel);
	studiohdr_t *modelheader = (studiohdr_t *)IEngineStudio.Mod_Extradata(pweaponmodel);

	return (modelheader->numattachments != 0);
}

int CGameStudioModelRenderer::_StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();

	IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	m_nPlayerIndex = pplayer->number - 1;

	if (m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients())
		return 0;

	if (cl_minmodels && cl_minmodels->value)
	{
		if (g_PlayerExtraInfo[pplayer->number].teamnumber == TEAM_CT)
		{
			int modelindex = (cl_min_t && IsValidCTModelIndex(cl_min_t->value)) ? cl_min_t->value : CS_LEET;
			m_pRenderModel = gEngfuncs.CL_LoadModel(sPlayerModelFiles[modelindex], nullptr);
		}
		else if (g_PlayerExtraInfo[pplayer->number].teamnumber == TEAM_CT)
		{
			if (g_PlayerExtraInfo[pplayer->number].vip)
			{
				m_pRenderModel = gEngfuncs.CL_LoadModel(sPlayerModelFiles[CS_VIP], nullptr);
			}
			else
			{
				int modelindex = (cl_min_ct && IsValidCTModelIndex(cl_min_ct->value)) ? cl_min_ct->value : CS_GIGN;
				m_pRenderModel = gEngfuncs.CL_LoadModel(sPlayerModelFiles[modelindex], nullptr);
			}
		}
	}
	else
	{
		m_pRenderModel = IEngineStudio.SetupPlayerModel(m_nPlayerIndex);
	}

	if (!m_pRenderModel)
		return 0;

	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);
	IEngineStudio.StudioSetHeader(m_pStudioHeader);
	IEngineStudio.SetRenderModel(m_pRenderModel);

	// Check sequence index of bounds
	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.sequence < 0)
		m_pCurrentEntity->curstate.sequence = 0;

	if (pplayer->sequence >= m_pStudioHeader->numseq || pplayer->sequence < 0)
		pplayer->sequence = 0;

	if (m_pCurrentEntity->curstate.gaitsequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.gaitsequence < 0)
		m_pCurrentEntity->curstate.gaitsequence = 0;

	if (pplayer->gaitsequence >= m_pStudioHeader->numseq || pplayer->gaitsequence < 0)
		pplayer->gaitsequence = 0;

	if (pplayer->gaitsequence)
	{
		vec3_t orig_angles;
		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

		VectorCopy(m_pCurrentEntity->angles, orig_angles);

		StudioProcessGait(pplayer);

		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo = nullptr;

		StudioSetUpTransform(0);
		VectorCopy(orig_angles, m_pCurrentEntity->angles);
	}
	else
	{
		m_pCurrentEntity->curstate.controller[0] = 127;
		m_pCurrentEntity->curstate.controller[1] = 127;
		m_pCurrentEntity->curstate.controller[2] = 127;
		m_pCurrentEntity->curstate.controller[3] = 127;

		m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
		m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
		m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
		m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];

		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);
		CalculatePitchBlend(pplayer);
		CalculateYawBlend(pplayer);
		m_pPlayerInfo->gaitsequence = 0;

		StudioSetUpTransform(0);
	}

	if (flags & STUDIO_RENDER)
	{
		// see if the bounding box lets us trivially reject, also sets
		if (!IEngineStudio.StudioCheckBBox())
			return 0;

		(*m_pModelsDrawn)++;
		(*m_pStudioModelCount)++; // render data cache cookie

		if (m_pStudioHeader->numbodyparts == 0)
			return 1;
	}

	m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);
	StudioSetupBones();
	StudioSaveBones();

	m_pPlayerInfo->renderframe = m_nFrameCount;
	m_pPlayerInfo = nullptr;

	if ((flags & STUDIO_EVENTS) && (!(flags & STUDIO_RENDER) || !pplayer->weaponmodel || !WeaponHasAttachments(pplayer)))
	{
		StudioCalcAttachments();
		IEngineStudio.StudioClientEvents();

		// copy attachments into global entity array
		if (m_pCurrentEntity->index > 0)
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(m_pCurrentEntity->index);

			Q_memcpy(ent->attachment, m_pCurrentEntity->attachment, sizeof(ent->attachment));
		}
	}

	if (flags & STUDIO_RENDER)
	{
		//if (m_pCvarHiModels->value && m_pRenderModel != m_pCurrentEntity->model)
		//{
		//	// show highest resolution multiplayer model
		//	m_pCurrentEntity->curstate.body = 255;
		//}

		//if (!(m_pCvarDeveloper->value == 0 && gEngfuncs.GetMaxClients() == 1) && (m_pRenderModel == m_pCurrentEntity->model))
		//{
		//	m_pCurrentEntity->curstate.body = 1; // force helmet
		//}

		vec3_t dir;
		alight_t lighting;
		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
		IEngineStudio.StudioEntityLight(&lighting);
		IEngineStudio.StudioSetupLighting(&lighting); // model and frame independant

		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

		// get remap colors
		m_nTopColor = m_pPlayerInfo->topcolor;
		if (m_nTopColor < 0)
			m_nTopColor = 0;

		if (m_nTopColor > 360)
			m_nTopColor = 360;

		m_nBottomColor = m_pPlayerInfo->bottomcolor;
		if (m_nBottomColor < 0)
			m_nBottomColor = 0;

		if (m_nBottomColor > 360)
			m_nBottomColor = 360;

		IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);

		StudioRenderModel();
		m_pPlayerInfo = nullptr;

		if (pplayer->weaponmodel)
		{
			studiohdr_t *saveheader = m_pStudioHeader;
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = IEngineStudio.GetModelByIndex(pplayer->weaponmodel);

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(pweaponmodel);
			IEngineStudio.StudioSetHeader(m_pStudioHeader);

			StudioMergeBones(pweaponmodel);
			IEngineStudio.StudioSetupLighting(&lighting);

			// no draw weaponmodel only the player
			if (!m_pPlayerSync)
			{
				StudioRenderModel();
			}

			StudioCalcAttachments();

			if (m_pCurrentEntity->index > 0)
			{
				Q_memcpy(saveent.attachment, m_pCurrentEntity->attachment, m_pStudioHeader->numattachments * sizeof(vec3_t));
			}

			*m_pCurrentEntity = saveent;
			m_pStudioHeader = saveheader;

			IEngineStudio.StudioSetHeader(m_pStudioHeader);

			if (flags & STUDIO_EVENTS)
			{
				IEngineStudio.StudioClientEvents();
			}
		}

		if (cl_shadows && cl_shadows->value)
		{
			Vector start;
			if (GetPlayerBoneWorldPosition(BONE_PELVIS, start))
			{
				StudioDrawShadow(start, 20);
			}
		}
	}

	return 1;
}

void CGameStudioModelRenderer::StudioFxTransform(cl_entity_t *ent, float transform[3][4])
{
	switch (ent->curstate.renderfx)
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
	{
		if (gEngfuncs.pfnRandomLong(0, 49) == 0)
		{
			int axis = gEngfuncs.pfnRandomLong(0, 1);
			if (axis == 1) // Choose between x & z
				axis = 2;

			VectorScale(transform[axis], gEngfuncs.pfnRandomFloat(1, 1.484), transform[axis]);
		}
		else if (gEngfuncs.pfnRandomLong(0, 49) == 0)
		{
			float offset;
			int axis = gEngfuncs.pfnRandomLong(0, 1);
			if (axis == 1) // Choose between x & z
				axis = 2;

			offset = gEngfuncs.pfnRandomFloat(-10, 10);
			transform[gEngfuncs.pfnRandomLong(0, 2)][3] += offset;
		}
		break;
	}
	case kRenderFxExplode:
	{
		if (g_iRenderStateChanged)
		{
			g_flStartScaleTime = m_clTime;
			g_iRenderStateChanged = FALSE;
		}

		// Make the Model continue to shrink
		float flTimeDelta = m_clTime - g_flStartScaleTime;
		if (flTimeDelta > 0)
		{
			float flScale = 0.001;
			// Goes almost all away
			if (flTimeDelta <= 2.0)
				flScale = 1.0 - (flTimeDelta / 2.0);

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
					transform[i][j] *= flScale;
			}
		}
		break;
	}
	}
}

void CGameStudioModelRenderer::StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch)
{
	float range = 45.0;

	*pBlend = (*pPitch * 3);

	if (*pBlend <= -range)
		*pBlend = 255;
	else if (*pBlend >= range)
		*pBlend = 0;
	else
		*pBlend = 255 * (range - *pBlend) / (2 * range);

	*pPitch = 0;
}

void CGameStudioModelRenderer::CalculatePitchBlend(entity_state_t *pplayer)
{
	int iBlend;
	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	StudioPlayerBlend(pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH]);

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[1] = iBlend;
	m_pCurrentEntity->latched.prevblending[1] = m_pCurrentEntity->curstate.blending[1];
	m_pCurrentEntity->latched.prevseqblending[1] = m_pCurrentEntity->curstate.blending[1];
}

void CGameStudioModelRenderer::CalculateYawBlend(entity_state_t *pplayer)
{
	StudioEstimateGait(pplayer);

	// calc side to side turning
	float flYaw = fmod(m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw, 360.0f);

	if (flYaw < -180)
		flYaw += 360;
	else if (flYaw > 180)
		flYaw -= 360;

	if (m_flGaitMovement)
	{
		float maxyaw = 120.0;
		if (flYaw > maxyaw)
		{
			flYaw -= 180;
			m_pPlayerInfo->gaityaw -= 180;
			m_flGaitMovement = -m_flGaitMovement;
		}
		else if (flYaw < -maxyaw)
		{
			flYaw += 180;
			m_pPlayerInfo->gaityaw += 180;
			m_flGaitMovement = -m_flGaitMovement;
		}
	}

	float blend_yaw = (flYaw / 90.0) * 128.0 + 127.0;
	blend_yaw = clamp(blend_yaw, 0.0f, 255.0f);
	blend_yaw = 255.0 - blend_yaw;

	//engine->GetModule()->TraceLog("> %s: client yaw: %f, server yaw: (%f)\n", __FUNCTION__, blend_yaw, m_info[m_pCurrentEntity->index].yaw);

	m_pCurrentEntity->curstate.blending[0] = (int)(blend_yaw);
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;

	if (m_pCurrentEntity->angles[YAW] < 0)
		m_pCurrentEntity->angles[YAW] += 360;

	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];
}

// Hooks to class implementation
int R_StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	return g_StudioRenderer.StudioDrawPlayer(flags, pplayer);
}

int R_StudioDrawModel(int flags)
{
	return g_StudioRenderer.StudioDrawModel(flags);
}

void R_StudioInit()
{
	g_StudioRenderer.Init();
}

// The simple drawing interface we'll pass back to the engine
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

// Export this function for the engine to use the studio renderer class to render objects.
int HUD_GetStudioModelInterface(int version, r_studio_interface_t **ppinterface, engine_studio_api_t *pstudio)
{
	if (version != STUDIO_INTERFACE_VERSION)
		return 0;

	gClientfuncs.pStudioInterface(version, ppinterface, pstudio);

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	memcpy(&IEngineStudio, pstudio, sizeof(IEngineStudio));

	// Initialize local variables, etc.
	R_StudioInit();

	if (!g_EngineLib->StudioLightingInit())
	{
		g_EngineLib->TraceLog("> %s: not found R_StudioLighting\n", __FUNCTION__);
	}

	// Success
	return 1;
}
