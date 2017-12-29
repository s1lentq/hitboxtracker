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

int g_bIsCZ = -1;
int g_iRightHandValue;

// Global engine <-> studio model rendering code interface
engine_studio_api_t IEngineStudio;

int CStudioModelRenderer::m_boxpnt[6][4] =
{
	{ 0, 4, 6, 2 },
	{ 0, 1, 5, 4 },
	{ 0, 2, 3, 1 },
	{ 7, 5, 1, 3 },
	{ 7, 3, 2, 6 },
	{ 7, 6, 4, 5 },
};

vec3_t CStudioModelRenderer::m_hullcolor[8] =
{
	{  1.0,  1.0,  1.0 },
	{  1.0,  0.5,  0.5 },
	{  0.5,  1.0,  0.5 },
	{  1.0,  1.0,  0.5 },
	{  0.5,  0.5,  1.0 },
	{  1.0,  0.5,  1.0 },
	{  0.5,  1.0,  1.0 },
	{  1.0,  1.0,  1.0 },
};

void CStudioModelRenderer::Init()
{
	// Set up some variables shared with engine
	m_pCvarHiModels     = IEngineStudio.GetCvar("cl_himodels");
	m_pCvarDeveloper    = IEngineStudio.GetCvar("developer");
	m_pCvarDrawEntities = IEngineStudio.GetCvar("r_drawentities");
	m_pChromeSprite     = IEngineStudio.GetChromeSprite();
	m_pWhiteSprite      = IEngineStudio.Mod_ForName("sprites/white.spr", 1);

	IEngineStudio.GetModelCounters(&m_pStudioModelCount, &m_pModelsDrawn);

	// Get pointers to engine data structures
	m_pbonetransform  = (float (*)[MAXSTUDIOBONES][3][4])IEngineStudio.StudioGetBoneTransform();
	m_plighttransform = (float (*)[MAXSTUDIOBONES][3][4])IEngineStudio.StudioGetLightTransform();
	m_paliastransform = (float (*)[3][4])IEngineStudio.StudioGetAliasTransform();
	m_protationmatrix = (float (*)[3][4])IEngineStudio.StudioGetRotationMatrix();
}

CStudioModelRenderer::CStudioModelRenderer()
{
	m_fGaitEstimation   = TRUE;
	m_fDoInterp         = TRUE;

	m_pCurrentEntity    = nullptr;
	m_pCvarHiModels     = nullptr;
	m_pCvarDeveloper    = nullptr;
	m_pCvarDrawEntities = nullptr;
	m_pChromeSprite     = nullptr;
	m_pStudioModelCount = nullptr;
	m_pModelsDrawn      = nullptr;
	m_protationmatrix   = nullptr;
	m_paliastransform   = nullptr;
	m_pbonetransform    = nullptr;
	m_plighttransform   = nullptr;
	m_pStudioHeader     = nullptr;
	m_pBodyPart         = nullptr;
	m_pSubModel         = nullptr;
	m_pPlayerInfo       = nullptr;
	m_pRenderModel      = nullptr;
}

CStudioModelRenderer::~CStudioModelRenderer()
{
}

void CStudioModelRenderer::StudioCalcBoneAdj(float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen)
{
	int i, j;
	float value;
	mstudiobonecontroller_t *pbonecontroller;

	pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pStudioHeader + m_pStudioHeader->bonecontrollerindex);

	for (j = 0; j < m_pStudioHeader->numbonecontrollers; j++)
	{
		i = pbonecontroller[j].index;
		if (i >= STUDIO_MOUTH)
		{
			// mouth hardcoded at controller 4
			value = mouthopen / 64.0;
			if (value > 1.0)
				value = 1.0;

			value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			//Con_DPrintf("%d %f\n", mouthopen, value);
		}
		else
		{
			// check for 360% wrapping
			if (pbonecontroller[j].type & STUDIO_RLOOP)
			{
				if (abs(pcontroller1[i] - pcontroller2[i]) > 128)
				{
					int a, b;
					a = (pcontroller1[j] + 128) % 256;
					b = (pcontroller2[j] + 128) % 256;
					value = ((a * dadt) + (b * (1 - dadt)) - 128) * (360.0 / 256.0) + pbonecontroller[j].start;
				}
				else
				{
					value = ((pcontroller1[i] * dadt + (pcontroller2[i]) * (1.0 - dadt))) * (360.0 / 256.0) + pbonecontroller[j].start;
				}
			}
			else
			{
				value = (pcontroller1[i] * dadt + pcontroller2[i] * (1.0 - dadt)) / 255.0;
				value = clamp(value, 0.0f, 1.0f);
				value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			}

			//Con_DPrintf("%d %d %f : %f\n", m_pCurrentEntity->curstate.controller[j], m_pCurrentEntity->latched.prevcontroller[j], value, dadt);
		}

		switch (pbonecontroller[j].type & STUDIO_TYPES)
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			adj[j] = value * (M_PI / 180.0);
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			adj[j] = value;
			break;
		}
	}
}

void CStudioModelRenderer::StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q)
{
	int j, k;
	vec4_t q1, q2;
	vec3_t angle1, angle2;
	mstudioanimvalue_t *panimvalue;

	for (j = 0; j < 3; j++)
	{
		if (panim->offset[j + 3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j + 3]; // default;
		}
		else
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + panim->offset[j + 3]);

			// DEBUG
			k = frame;
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;

			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;

				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}

			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k + 1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k + 2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;

				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}

			angle1[j] = pbone->value[j + 3] + angle1[j] * pbone->scale[j + 3];
			angle2[j] = pbone->value[j + 3] + angle2[j] * pbone->scale[j + 3];
		}

		if (pbone->bonecontroller[j + 3] != -1)
		{
			angle1[j] += adj[pbone->bonecontroller[j + 3]];
			angle2[j] += adj[pbone->bonecontroller[j + 3]];
		}
	}

	if (!VectorCompare(angle1, angle2))
	{
		AngleQuaternion(angle1, q1);
		AngleQuaternion(angle2, q2);
		QuaternionSlerp(q1, q2, s, q);
	}
	else
	{
		AngleQuaternion(angle1, q);
	}
}

void CStudioModelRenderer::StudioCalcBonePosition(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos)
{
	int j, k;
	mstudioanimvalue_t *panimvalue;

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;

		if (panim->offset[j] != 0)
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + panim->offset[j]);

			//if (i == 0 && j == 0)
			//	Con_DPrintf("%d  %d:%d  %f\n", frame, panimvalue->num.valid, panimvalue->num.total, s);

			// DEBUG
			k = frame;
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;

			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;

  				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}

			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k + 1].value * (1.0 - s) + s * panimvalue[k + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[k + 1].value * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				}
			}
		}

		if (pbone->bonecontroller[j] != -1 && adj)
		{
			pos[j] += adj[pbone->bonecontroller[j]];
		}
	}
}

void CStudioModelRenderer::StudioSlerpBones(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s)
{
	vec4_t q3;
	float s1;

	s = clamp(s, 0.0f, 1.0f);
	s1 = 1.0 - s;

	for (int i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionSlerp(q1[i], q2[i], s, q3);

		q1[i][0] = q3[0];
		q1[i][1] = q3[1];
		q1[i][2] = q3[2];
		q1[i][3] = q3[3];

		pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s;
		pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s;
		pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s;
	}
}

mstudioanim_t *CStudioModelRenderer::StudioGetAnim(model_t *psubmodel, mstudioseqdesc_t *pseqdesc)
{
	mstudioseqgroup_t *pseqgroup;
	cache_user_t *paSequences;

	pseqgroup = (mstudioseqgroup_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqgroupindex) + pseqdesc->seqgroup;

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t *)((byte *)m_pStudioHeader + pseqdesc->animindex);
	}

	paSequences = (cache_user_t *)psubmodel->submodels;

	if (!paSequences)
	{
		paSequences = (cache_user_t *)IEngineStudio.Mem_Calloc(MAXSTUDIOGROUPS, sizeof(cache_user_t)); // UNDONE: leak!
		psubmodel->submodels = (dmodel_t *)paSequences;
	}

	if (!IEngineStudio.Cache_Check((struct cache_user_s *)&(paSequences[pseqdesc->seqgroup])))
	{
		gEngfuncs.Con_DPrintf("loading %s\n", pseqgroup->name);
		IEngineStudio.LoadCacheFile(pseqgroup->name, (struct cache_user_s *)&paSequences[pseqdesc->seqgroup]);
	}

	return (mstudioanim_t *)((byte *)paSequences[pseqdesc->seqgroup].data + pseqdesc->animindex);
}

void CStudioModelRenderer::StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch)
{
	// calc up/down pointing
	*pBlend = (*pPitch * 3);
	if (*pBlend < pseqdesc->blendstart[0])
	{
		*pPitch -= pseqdesc->blendstart[0] / 3.0;
		*pBlend = 0;
	}
	else if (*pBlend > pseqdesc->blendend[0])
	{
		*pPitch -= pseqdesc->blendend[0] / 3.0;
		*pBlend = 255;
	}
	else
	{
		if (pseqdesc->blendend[0] - pseqdesc->blendstart[0] < 0.1) // catch qc error
			*pBlend = 127;
		else
			*pBlend = 255 * (*pBlend - pseqdesc->blendstart[0]) / (pseqdesc->blendend[0] - pseqdesc->blendstart[0]);
		*pPitch = 0;
	}
}

void CStudioModelRenderer::StudioSetUpTransform(int trivial_accept)
{
	vec3_t angles, modelpos;

	VectorCopy(m_pCurrentEntity->origin, modelpos);

	// TODO: should really be stored with the entity instead of being reconstructed
	// TODO: should use a look-up table
	// TODO: could cache lazily, stored in the entity
	angles[ROLL]  = m_pCurrentEntity->curstate.angles[ROLL];
	angles[PITCH] = m_pCurrentEntity->curstate.angles[PITCH];
	angles[YAW]   = m_pCurrentEntity->curstate.angles[YAW];

	if (m_pCurrentEntity->curstate.movetype != MOVETYPE_NONE)
	{
		VectorCopy(m_pCurrentEntity->angles, angles);
	}

	angles[PITCH] = -angles[PITCH];
	AngleMatrix(angles, (*m_protationmatrix));

	if (!IEngineStudio.IsHardware())
	{
		static float viewmatrix[3][4];

		VectorCopy(m_vRight, viewmatrix[0]);
		VectorCopy(m_vUp, viewmatrix[1]);
		VectorInverse(viewmatrix[1]);
		VectorCopy(m_vNormal, viewmatrix[2]);

		(*m_protationmatrix)[0][3] = modelpos[0] - m_vRenderOrigin[0];
		(*m_protationmatrix)[1][3] = modelpos[1] - m_vRenderOrigin[1];
		(*m_protationmatrix)[2][3] = modelpos[2] - m_vRenderOrigin[2];

		ConcatTransforms(viewmatrix, (*m_protationmatrix), (*m_paliastransform));

		// do the scaling up of x and y to screen coordinates as part of the transform
		// for the unclipped case (it would mess up clipping in the clipped case).
		// Also scale down z, so 1/z is scaled 31 bits for free, and scale down x and y
		// correspondingly so the projected x and y come out right
		// FIXME: make this work for clipped case too?
		if (trivial_accept)
		{
			for (int i = 0; i < 4; i++)
			{
				(*m_paliastransform)[0][i] *= m_fSoftwareXScale * (1.0 / (ZISCALE * 0x10000));
				(*m_paliastransform)[1][i] *= m_fSoftwareYScale * (1.0 / (ZISCALE * 0x10000));
				(*m_paliastransform)[2][i] *= 1.0 / (ZISCALE * 0x10000);

			}
		}
	}

	(*m_protationmatrix)[0][3] = modelpos[0];
	(*m_protationmatrix)[1][3] = modelpos[1];
	(*m_protationmatrix)[2][3] = modelpos[2];
}

float CStudioModelRenderer::StudioEstimateInterpolant()
{
	float dadt = 1.0;
	if (m_fDoInterp && (m_pCurrentEntity->curstate.animtime >= m_pCurrentEntity->latched.prevanimtime + 0.01))
	{
		dadt = (m_clTime - m_pCurrentEntity->curstate.animtime) / 0.1;
		if (dadt > 2.0)
		{
			dadt = 2.0;
		}
	}

	return dadt;
}

void CStudioModelRenderer::StudioCalcRotations(float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f)
{
	int frame;
	mstudiobone_t *pbone;

	float s;
	float adj[MAXSTUDIOCONTROLLERS];
	float dadt;

	if (f > pseqdesc->numframes - 1)
	{
		f = 0;	// bah, fix this bug with changing sequences too fast
	}

	// BUG (somewhere else) but this code should validate this data.
	// This could cause a crash if the frame # is negative, so we'll go ahead
	//  and clamp it here
	else if (f < -0.01)
	{
		f = -0.01;
	}

	frame = (int)f;

	//Con_DPrintf("%d %.4f %.4f %.4f %.4f %d\n", m_pCurrentEntity->curstate.sequence, m_clTime, m_pCurrentEntity->animtime, m_pCurrentEntity->frame, f, frame);
	//Con_DPrintf("%f %f %f\n", m_pCurrentEntity->angles[ROLL], m_pCurrentEntity->angles[PITCH], m_pCurrentEntity->angles[YAW]);
	//Con_DPrintf("frame %d %d\n", frame1, frame2);

	dadt = StudioEstimateInterpolant();
	s = (f - frame);

	// add in programtic controllers
	pbone = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	StudioCalcBoneAdj(dadt, adj, m_pCurrentEntity->curstate.controller, m_pCurrentEntity->latched.prevcontroller, m_pCurrentEntity->mouth.mouthopen);

	for (int i = 0; i < m_pStudioHeader->numbones; i++, pbone++, panim++)
	{
		StudioCalcBoneQuaterion(frame, s, pbone, panim, adj, q[i]);
		StudioCalcBonePosition(frame, s, pbone, panim, adj, pos[i]);

		//if (0 && i == 0)
		//	Con_DPrintf("%d %d %d %d\n", m_pCurrentEntity->curstate.sequence, frame, j, k);
	}

	if (pseqdesc->motiontype & STUDIO_X)
	{
		pos[pseqdesc->motionbone][0] = 0.0;
	}
	if (pseqdesc->motiontype & STUDIO_Y)
	{
		pos[pseqdesc->motionbone][1] = 0.0;
	}
	if (pseqdesc->motiontype & STUDIO_Z)
	{
		pos[pseqdesc->motionbone][2] = 0.0;
	}

	s = 0 * ((1.0 - (f - (int)(f))) / (pseqdesc->numframes)) * m_pCurrentEntity->curstate.framerate;

	if (pseqdesc->motiontype & STUDIO_LX)
	{
		pos[pseqdesc->motionbone][0] += s * pseqdesc->linearmovement[0];
	}
	if (pseqdesc->motiontype & STUDIO_LY)
	{
		pos[pseqdesc->motionbone][1] += s * pseqdesc->linearmovement[1];
	}
	if (pseqdesc->motiontype & STUDIO_LZ)
	{
		pos[pseqdesc->motionbone][2] += s * pseqdesc->linearmovement[2];
	}
}

void CStudioModelRenderer::StudioFxTransform(cl_entity_t *ent, float transform[3][4])
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
		float scale = 1.0 + (m_clTime - ent->curstate.animtime) * 10.0;
		if (scale > 2)	// Don't blow up more than 200%
			scale = 2;

		transform[0][1] *= scale;
		transform[1][1] *= scale;
		transform[2][1] *= scale;
		break;
	}
	}
}

float CStudioModelRenderer::StudioEstimateFrame(mstudioseqdesc_t *pseqdesc)
{
	double dfdt, f;

	if (m_fDoInterp)
	{
		if (m_clTime < m_pCurrentEntity->curstate.animtime)
		{
			dfdt = 0;
		}
		else
		{
			dfdt = (m_clTime - m_pCurrentEntity->curstate.animtime) * m_pCurrentEntity->curstate.framerate * pseqdesc->fps;
		}
	}
	else
	{
		dfdt = 0;
	}

	if (pseqdesc->numframes <= 1)
	{
		f = 0;
	}
	else
	{
		f = (m_pCurrentEntity->curstate.frame * (pseqdesc->numframes - 1)) / 256.0;
	}

	f += dfdt;

	if (pseqdesc->flags & STUDIO_LOOPING)
	{
		if (pseqdesc->numframes > 1)
		{
			f -= (int)(f / (pseqdesc->numframes - 1)) *  (pseqdesc->numframes - 1);
		}

		if (f < 0)
		{
			f += (pseqdesc->numframes - 1);
		}
	}
	else
	{
		if (f >= pseqdesc->numframes - 1.001)
		{
			f = pseqdesc->numframes - 1.001;
		}

		if (f < 0.0)
		{
			f = 0.0;
		}
	}

	return f;
}

void CStudioModelRenderer::StudioSetupBones()
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

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.sequence < 0)
	{
		m_pCurrentEntity->curstate.sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	// always want new gait sequences to start on frame zero
/*	if (m_pPlayerInfo)
	{
		int playerNum = m_pCurrentEntity->index - 1;

		// new jump gaitsequence?  start from frame zero
		if (m_nPlayerGaitSequences[playerNum] != m_pPlayerInfo->gaitsequence)
		{
			//m_pPlayerInfo->gaitframe = 0.0;
			gEngfuncs.Con_Printf("Setting gaitframe to 0\n");
		}

		m_nPlayerGaitSequences[playerNum] = m_pPlayerInfo->gaitsequence;
		//gEngfuncs.Con_Printf("index: %d     gaitsequence: %d\n",playerNum, m_pPlayerInfo->gaitsequence);
	}
*/
	f = StudioEstimateFrame(pseqdesc);

	if (m_pCurrentEntity->latched.prevframe > f)
	{
		//Con_DPrintf("%f %f\n", m_pCurrentEntity->prevframe, f);
	}

	panim = StudioGetAnim(m_pRenderModel, pseqdesc);
	StudioCalcRotations(pos, q, pseqdesc, panim, (int)f); // TODO: CHECK ME cast from float to int and to float

	if (pseqdesc->numblends > 1)
	{
		float s;
		float dadt;

		panim += m_pStudioHeader->numbones;
		StudioCalcRotations(pos2, q2, pseqdesc, panim, f);

		dadt = StudioEstimateInterpolant();
		s = (m_pCurrentEntity->curstate.blending[0] * dadt + m_pCurrentEntity->latched.prevblending[0] * (1.0 - dadt)) / 255.0;

		StudioSlerpBones(q, pos, q2, pos2, s);

		if (pseqdesc->numblends == 4)
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations(pos3, q3, pseqdesc, panim, f);

			panim += m_pStudioHeader->numbones;
			StudioCalcRotations(pos4, q4, pseqdesc, panim, f);

			s = (m_pCurrentEntity->curstate.blending[0] * dadt + m_pCurrentEntity->latched.prevblending[0] * (1.0 - dadt)) / 255.0;
			StudioSlerpBones(q3, pos3, q4, pos4, s);

			s = (m_pCurrentEntity->curstate.blending[1] * dadt + m_pCurrentEntity->latched.prevblending[1] * (1.0 - dadt)) / 255.0;
			StudioSlerpBones(q, pos, q3, pos3, s);
		}
	}

	if (m_fDoInterp &&
		m_pCurrentEntity->latched.sequencetime &&
		(m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime) &&
		(m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq))
	{
		// blend from last sequence
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s;

		//if (m_pCurrentEntity->latched.prevsequence >= m_pStudioHeader->numseq)
		//{
		//	m_pCurrentEntity->latched.prevsequence = 0;
		//}

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim(m_pRenderModel, pseqdesc);

		// clip prevframe
		StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

		if (pseqdesc->numblends > 1)
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

			s = (m_pCurrentEntity->latched.prevseqblending[0]) / 255.0;
			StudioSlerpBones(q1b, pos1b, q2, pos2, s);

			if (pseqdesc->numblends == 4)
			{
				panim += m_pStudioHeader->numbones;
				StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

				panim += m_pStudioHeader->numbones;
				StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);

				s = (m_pCurrentEntity->latched.prevseqblending[0]) / 255.0;
				StudioSlerpBones(q3, pos3, q4, pos4, s);

				s = (m_pCurrentEntity->latched.prevseqblending[1]) / 255.0;
				StudioSlerpBones(q1b, pos1b, q3, pos3, s);
			}
		}

		s = 1.0 - (m_clTime - m_pCurrentEntity->latched.sequencetime) / 0.2;
		StudioSlerpBones(q, pos, q1b, pos1b, s);
	}
	else
	{
		//Con_DPrintf("prevframe = %4.2f\n", f);
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	// bounds checking
	//if (m_pPlayerInfo)
	//{
	//	if (m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq)
	//	{
	//		m_pPlayerInfo->gaitsequence = 0;
	//	}
	//}

	// calc gait animation
	if (m_pPlayerInfo && m_pPlayerInfo->gaitsequence != 0)
	{
		//if (m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq)
		//{
		//	m_pPlayerInfo->gaitsequence = 0;
		//}

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim(m_pRenderModel, pseqdesc);
		StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe);

		for (int i = 0; i < m_pStudioHeader->numbones; i++)
		{
			if (!Q_strcmp(pbones[i].name, "Bip01 Spine"))
				break;

			Q_memcpy(pos[i], pos2[i], sizeof(pos[i]));
			Q_memcpy(q[i], q2[i], sizeof(q[i]));
		}
	}

	cl_entity_t *pViewModel = gEngfuncs.GetViewModel();
	bool bIsViewModel = (pViewModel && pViewModel == m_pCurrentEntity) ? true : false;

	for (int i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1)
		{
			if ((cl_righthand && cl_righthand->value && bIsViewModel && IEngineStudio.IsHardware() && !g_bHoldingShield)
				|| (g_bIsCZ && bIsViewModel && IEngineStudio.IsHardware() && g_bHoldingShield))
			{
				bonematrix[1][0] = bonematrix[1][0] * -1.0;
				bonematrix[1][1] = bonematrix[1][1] * -1.0;
				bonematrix[1][2] = bonematrix[1][2] * -1.0;
				bonematrix[1][3] = bonematrix[1][3] * -1.0;
			}

			if (IEngineStudio.IsHardware())
			{
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_pbonetransform)[i]);

				// MatrixCopy should be faster...
				//ConcatTransforms((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
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

void CStudioModelRenderer::StudioSaveBones()
{
	mstudiobone_t *pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	m_nCachedBones = m_pStudioHeader->numbones;

	for (int i = 0; i < m_pStudioHeader->numbones; i++)
	{
		Q_strlcpy(m_nCachedBoneNames[i], pbones[i].name);
		MatrixCopy((*m_pbonetransform)[i], m_rgCachedBoneTransform[i]);
		MatrixCopy((*m_plighttransform)[i], m_rgCachedLightTransform[i]);
	}
}

void CStudioModelRenderer::StudioMergeBones(model_t *psubmodel)
{
	int i, j;
	double f;
	int do_hunt = true;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	float bonematrix[3][4];
	static vec4_t q[MAXSTUDIOBONES];

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq || m_pCurrentEntity->curstate.sequence < 0)
	{
		m_pCurrentEntity->curstate.sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	f = StudioEstimateFrame(pseqdesc);

	if (m_pCurrentEntity->latched.prevframe > f)
	{
		// Con_DPrintf("%f %f\n", m_pCurrentEntity->prevframe, f);
	}

	panim = StudioGetAnim(psubmodel, pseqdesc);
	StudioCalcRotations(pos, q, pseqdesc, panim, f);

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		for (j = 0; j < m_nCachedBones; j++)
		{
			if (Q_stricmp(pbones[i].name, m_nCachedBoneNames[j]) == 0)
			{
				MatrixCopy(m_rgCachedBoneTransform[j], (*m_pbonetransform)[i]);
				MatrixCopy(m_rgCachedLightTransform[j], (*m_plighttransform)[i]);
				break;
			}
		}

		if (j >= m_nCachedBones)
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

					// MatrixCopy should be faster...
					//ConcatTransforms((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
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
}

int CStudioModelRenderer::StudioDrawModel(int flags)
{
	cl_entity_t *pViewModel = gEngfuncs.GetViewModel();
	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();

	bool changedRighthand = false;
	bool bIsViewModel = (pViewModel && pViewModel == m_pCurrentEntity) ? true : false;

	if (g_bIsCZ == -1)
	{
		g_bIsCZ = gHUD.IsGame("czero") ? 1 : 0;
	}

	if (flags & STUDIO_RENDER)
	{
		if (g_bIsCZ)
		{
			if (g_bHoldingShield && bIsViewModel)
			{
				g_iRightHandValue = cl_righthand->value;

				cl_righthand->value = 1;
				changedRighthand = true;
			}
		}
		else
		{
			if (g_bHoldingKnife && bIsViewModel)
			{
				g_iRightHandValue = cl_righthand->value;
				cl_righthand->value = g_iRightHandValue ? 0 : 1;
				changedRighthand = true;
			}
		}
	}

	if (gHUD.m_iFOV < 90)
	{
		if (Q_strstr(m_pCurrentEntity->model->name, "v_awp")
			|| Q_strstr(m_pCurrentEntity->model->name, "v_scout")
			|| Q_strstr(m_pCurrentEntity->model->name, "v_g3sg1")
			|| Q_strstr(m_pCurrentEntity->model->name, "v_sg550"))
		{
			return 0;
		}
	}

	IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	if (m_pCurrentEntity->curstate.renderfx == kRenderFxDeadPlayer)
	{
		if (m_pCurrentEntity->curstate.renderamt <= 0 || m_pCurrentEntity->curstate.renderamt > gEngfuncs.GetMaxClients())
			return 0;

		// get copy of player
		entity_state_t deadplayer = *(IEngineStudio.GetPlayerState(m_pCurrentEntity->curstate.renderamt - 1)); // cl.frames[cl.parsecount & CL_UPDATE_MASK].playerstate[m_pCurrentEntity->curstate.renderamt - 1];

		// clear weapon, movement state
		deadplayer.number = m_pCurrentEntity->curstate.renderamt;
		deadplayer.weaponmodel = 0;
		deadplayer.gaitsequence = 0;
		deadplayer.movetype = MOVETYPE_NONE;

		VectorCopy(m_pCurrentEntity->curstate.angles, deadplayer.angles);
		VectorCopy(m_pCurrentEntity->curstate.origin, deadplayer.origin);

		qboolean save_interp = m_fDoInterp;
		m_fDoInterp = FALSE;

		// draw as though it were a player
		int result = StudioDrawPlayer(flags, &deadplayer);

		m_fDoInterp = save_interp;
		return result;
	}

	m_pRenderModel = m_pCurrentEntity->model;
	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);
	IEngineStudio.StudioSetHeader(m_pStudioHeader);
	IEngineStudio.SetRenderModel(m_pRenderModel);

	StudioSetUpTransform(0);

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

	if (m_pCurrentEntity->curstate.movetype == MOVETYPE_FOLLOW)
	{
		StudioMergeBones(m_pRenderModel);
	}
	else
	{
		StudioSetupBones();
	}

	StudioSaveBones();

	if (flags & STUDIO_EVENTS)
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
		vec3_t dir;
		alight_t lighting;

		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
		IEngineStudio.StudioEntityLight(&lighting);
		IEngineStudio.StudioSetupLighting(&lighting); // model and frame independant

		// get remap colors
		m_nTopColor    = (m_pCurrentEntity->curstate.colormap & 0xFF);
		m_nBottomColor = (m_pCurrentEntity->curstate.colormap & 0xFF00) >> 8;

		IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);
		StudioRenderModel();

		if (cl_shadows->value > 0 && (Q_strstr(m_pCurrentEntity->model->name, "hostage") || Q_strstr(m_pCurrentEntity->model->name, "scientist")))
		{
			StudioDrawShadow(m_pCurrentEntity->origin, 12);
		}
	}

	if (changedRighthand)
	{
		cl_righthand->value = (float)g_iRightHandValue;
	}

	return 1;
}

void CStudioModelRenderer::StudioEstimateGait(entity_state_t *pplayer)
{
	float dt = clamp(m_clTime - m_clOldTime, 0.0, 1.0);
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

	if (est_velocity[1] == 0 && est_velocity[0] == 0)
	{
		float flYawDiff = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;

		if (flYawDiff > 180)
			flYawDiff -= 360;

		if (flYawDiff < -180)
			flYawDiff += 360;

		if (dt < 0.25)
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_pPlayerInfo->gaityaw += flYawDiff;
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - (int)(m_pPlayerInfo->gaityaw / 360) * 360;
		m_flGaitMovement = 0;
	}
	else
	{
		m_pPlayerInfo->gaityaw = (atan2(est_velocity[1], est_velocity[0]) * 180 / M_PI);

		if (m_pPlayerInfo->gaityaw > 180)
			m_pPlayerInfo->gaityaw = 180;

		if (m_pPlayerInfo->gaityaw < -180)
			m_pPlayerInfo->gaityaw = -180;
	}
}

void CStudioModelRenderer::StudioProcessGait(entity_state_t *pplayer)
{
	mstudioseqdesc_t *pseqdesc;
	float dt;
	int iBlend;
	float flYaw; // view direction relative to movement

	//if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
	//{
	//	m_pCurrentEntity->curstate.sequence = 0;
	//}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	StudioPlayerBlend(pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH]);

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[0] = iBlend;
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	//Con_DPrintf("%f %d\n", m_pCurrentEntity->angles[PITCH], m_pCurrentEntity->blending[0]);

	dt = clamp(m_clTime - m_clOldTime, 0.0, 1.0);
	StudioEstimateGait(pplayer);

	//Con_DPrintf("%f %f\n", m_pCurrentEntity->angles[YAW], m_pPlayerInfo->gaityaw);

	// calc side to side turning
	flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
	flYaw = flYaw - (int)(flYaw / 360) * 360;

	if (flYaw < -180)
		flYaw = flYaw + 360;

	if (flYaw > 180)
		flYaw = flYaw - 360;

	if (flYaw > 120)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw - 180;
	}
	else if (flYaw < -120)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw + 180;
	}

	// adjust torso
	m_pCurrentEntity->curstate.controller[0] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->curstate.controller[1] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->curstate.controller[2] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);
	m_pCurrentEntity->curstate.controller[3] = ((flYaw / 4.0) + 30) / (60.0 / 255.0);

	m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
	m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
	m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
	m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];

	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;
	if (m_pCurrentEntity->angles[YAW] < -0)
		m_pCurrentEntity->angles[YAW] += 360;

	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];

	if (pplayer->gaitsequence >= m_pStudioHeader->numseq)
	{
		pplayer->gaitsequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + pplayer->gaitsequence;

	// calc gait frame
	if (pseqdesc->linearmovement[0] > 0)
	{
		m_pPlayerInfo->gaitframe += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	}
	else
	{
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt;
	}

	// do modulo
	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)(m_pPlayerInfo->gaitframe / pseqdesc->numframes) * pseqdesc->numframes;

	if (m_pPlayerInfo->gaitframe < 0)
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

int CStudioModelRenderer::m_iShadowSprite;
void CStudioModelRenderer::StudioSetShadowSprite(int iSprite)
{
	m_iShadowSprite = iSprite;
}

void CStudioModelRenderer::StudioDrawShadow(Vector &origin, float scale)
{
	Vector p1, p2, p3, p4;
	pmtrace_t pmtrace;

	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(origin, origin - Vector(0, 0, 150), PM_STUDIO_BOX | PM_WORLD_ONLY, -1, &pmtrace);

	if (pmtrace.startsolid)
		return;

	if (pmtrace.fraction >= 1.0f)
		return;

	pmtrace.plane.normal = pmtrace.plane.normal.Normalize();

	if (pmtrace.plane.normal.z <= 0.7f)
		return;

	pmtrace.plane.normal = pmtrace.plane.normal * scale * (1.0f - pmtrace.fraction);

	// add 2.0f to Z, for avoid Z-fighting
	p1.x = pmtrace.endpos.x - pmtrace.plane.normal.z;
	p1.y = pmtrace.endpos.y + pmtrace.plane.normal.z;
	p1.z = pmtrace.endpos.z + 1.0f + pmtrace.plane.normal.x - pmtrace.plane.normal.y;

	p2.x = pmtrace.endpos.x + pmtrace.plane.normal.z;
	p2.y = pmtrace.endpos.y + pmtrace.plane.normal.z;
	p2.z = pmtrace.endpos.z + 1.0f - pmtrace.plane.normal.x - pmtrace.plane.normal.y;

	p3.x = pmtrace.endpos.x + pmtrace.plane.normal.z;
	p3.y = pmtrace.endpos.y - pmtrace.plane.normal.z;
	p3.z = pmtrace.endpos.z + 1.0f - pmtrace.plane.normal.x + pmtrace.plane.normal.y;

	p4.x = pmtrace.endpos.x - pmtrace.plane.normal.z;
	p4.y = pmtrace.endpos.y - pmtrace.plane.normal.z;
	p4.z = pmtrace.endpos.z + 1.0f + pmtrace.plane.normal.x + pmtrace.plane.normal.y;

	IEngineStudio.StudioRenderShadow(m_iShadowSprite, p1, p2, p3, p4);
}

int CStudioModelRenderer::StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();

	IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	m_nPlayerIndex = pplayer->number - 1;

	if (m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients())
		return 0;

	m_pRenderModel = IEngineStudio.SetupPlayerModel(m_nPlayerIndex);
	if (!m_pRenderModel)
		return 0;

	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);
	IEngineStudio.StudioSetHeader(m_pStudioHeader);
	IEngineStudio.SetRenderModel(m_pRenderModel);

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

	if (flags & STUDIO_EVENTS)
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
		if (m_pCvarHiModels->value && m_pRenderModel != m_pCurrentEntity->model)
		{
			// show highest resolution multiplayer model
			m_pCurrentEntity->curstate.body = 255;
		}

		if (!(m_pCvarDeveloper->value == 0 && gEngfuncs.GetMaxClients() == 1) && (m_pRenderModel == m_pCurrentEntity->model))
		{
			m_pCurrentEntity->curstate.body = 1; // force helmet
		}

		vec3_t dir;
		alight_t lighting;
		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
		IEngineStudio.StudioEntityLight(&lighting);
		IEngineStudio.StudioSetupLighting(&lighting); // model and frame independant

		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

		// get remap colors
		m_nTopColor    = m_pPlayerInfo->topcolor;
		m_nBottomColor = m_pPlayerInfo->bottomcolor;

		// bounds check
		if (m_nTopColor < 0)
			m_nTopColor = 0;

		if (m_nTopColor > 360)
			m_nTopColor = 360;

		if (m_nBottomColor < 0)
			m_nBottomColor = 0;

		if (m_nBottomColor > 360)
			m_nBottomColor = 360;

		IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);

		StudioRenderModel();
		m_pPlayerInfo = nullptr;

		if (pplayer->weaponmodel)
		{
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = IEngineStudio.GetModelByIndex(pplayer->weaponmodel);

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(pweaponmodel);
			IEngineStudio.StudioSetHeader(m_pStudioHeader);

			StudioMergeBones(pweaponmodel);
			IEngineStudio.StudioSetupLighting(&lighting);

			StudioRenderModel();
			StudioCalcAttachments();

			*m_pCurrentEntity = saveent;
		}
	}

	return 1;
}

const int MAX_NUM_ATTACHMENTS = 4;

void CStudioModelRenderer::StudioCalcAttachments()
{
	if (m_pStudioHeader->numattachments > MAX_NUM_ATTACHMENTS)
	{
		gEngfuncs.Con_DPrintf("Too many attachments on %s\n", m_pCurrentEntity->model->name);
		exit(-1);
	}

	// calculate attachment points
	mstudioattachment_t *pattachment = (mstudioattachment_t *)((byte *)m_pStudioHeader + m_pStudioHeader->attachmentindex);
	for (int i = 0; i < m_pStudioHeader->numattachments; i++)
	{
		VectorTransform(pattachment[i].org, (*m_plighttransform)[pattachment[i].bone],  m_pCurrentEntity->attachment[i]);
	}
}

void CStudioModelRenderer::StudioRenderModel()
{
	IEngineStudio.SetChromeOrigin();
	IEngineStudio.SetForceFaceFlags(0);

	if (m_pCurrentEntity->curstate.renderfx == kRenderFxGlowShell)
	{
		m_pCurrentEntity->curstate.renderfx = kRenderFxNone;
		StudioRenderFinal();

		if (!IEngineStudio.IsHardware())
		{
			gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		}

		IEngineStudio.SetForceFaceFlags(STUDIO_NF_CHROME);

		gEngfuncs.pTriAPI->SpriteTexture(m_pChromeSprite, 0);
		m_pCurrentEntity->curstate.renderfx = kRenderFxGlowShell;

		StudioRenderFinal();

		if (!IEngineStudio.IsHardware())
		{
			gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
		}
	}
	else
	{
		StudioRenderFinal();
	}
}

void CStudioModelRenderer::StudioDrawBones()
{
	int i, j, k;
	float lv;
	vec3_t tmp;
	vec3_t p[8];
	vec3_t up, right, forward;
	vec3_t a1;
	mstudiobone_t *pbones;

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	gEngfuncs.pTriAPI->SpriteTexture(m_pWhiteSprite, 0);

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		if (pbones[i].parent == -1)
			continue;

		k = pbones[i].parent;

		a1[0] = a1[1] = a1[2] = 1.0f;
		up[0] = (*m_plighttransform)[i][0][3] - (*m_plighttransform)[k][0][3];
		up[1] = (*m_plighttransform)[i][1][3] - (*m_plighttransform)[k][1][3];
		up[2] = (*m_plighttransform)[i][2][3] - (*m_plighttransform)[k][2][3];

		if (up[0] > up[1])
		{
			if (up[0] > up[2])
				a1[0] = 0.0f;
			else
				a1[2] = 0.0f;
		}
		else
		{
			if (up[1] > up[2])
				a1[1] = 0.0f;
			else
				a1[2] = 0.0f;
		}

		CrossProduct(up, a1, right);
		VectorNormalize(right);
		CrossProduct(up, right, forward);
		VectorNormalize(forward);
		VectorScale(right, 2.0f, right);
		VectorScale(forward, 2.0f, forward);

		for (j = 0; j < 8; j++)
		{
			p[j][0] = (*m_plighttransform)[k][0][3];
			p[j][1] = (*m_plighttransform)[k][1][3];
			p[j][2] = (*m_plighttransform)[k][2][3];

			if (j & 1)
			{
				VectorSubtract(p[j], right, p[j]);
			}
			else
			{
				VectorAdd(p[j], right, p[j]);
			}

			if (j & 2)
			{
				VectorSubtract(p[j], forward, p[j]);
			}
			else
			{
				VectorAdd(p[j], forward, p[j]);
			}

			if (j & 4)
			{
			}
			else
			{
				VectorAdd(p[j], up, p[j]);
			}
		}

		VectorNormalize(up);
		VectorNormalize(right);
		VectorNormalize(forward);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->Color4f(1.0f, 1.0f, 1.0f, 1.0f);
		gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);

		for (j = 0; j < 6; j++)
		{
			switch (j)
			{
			case 0:	VectorCopy (right, tmp);       break;
			case 1:	VectorCopy (forward, tmp);     break;
			case 2:	VectorCopy (up, tmp);          break;
			case 3:	VectorScale(right, -1, tmp);   break;
			case 4:	VectorScale(forward, -1, tmp); break;
			case 5:	VectorScale(up, -1, tmp);      break;
			}

			R_StudioLighting(&lv, -1, 0, tmp);

			gEngfuncs.pTriAPI->Brightness(lv);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][0]]);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][1]]);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][2]]);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][3]]);
		}

		gEngfuncs.pTriAPI->End();
	}
}

void CStudioModelRenderer::StudioDrawHulls()
{
	int i, j;
	float lv;
	vec3_t tmp;
	vec3_t p[8];
	vec3_t pos;

	mstudiobbox_t *pbbox = (mstudiobbox_t *)((char *)m_pStudioHeader + m_pStudioHeader->hitboxindex);

	gEngfuncs.pTriAPI->SpriteTexture(m_pWhiteSprite, 0);

	for (i = 0; i < m_pStudioHeader->numhitboxes; i++)
	{
		// skip shield
		if (i == 20)
			continue;

		for (j = 0; j < 8; j++)
		{
			tmp[0] = (j & 1) ? pbbox[i].bbmin[0] : pbbox[i].bbmax[0];
			tmp[1] = (j & 2) ? pbbox[i].bbmin[1] : pbbox[i].bbmax[1];
			tmp[2] = (j & 4) ? pbbox[i].bbmin[2] : pbbox[i].bbmax[2];

			VectorTransform(tmp, (*m_plighttransform)[pbbox[i].bone], p[j]);
		}

		j = (pbbox[i].group % 8);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->Color4f(m_hullcolor[j][0], m_hullcolor[j][1], m_hullcolor[j][2], 1.0f);
		gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);

		for (j = 0; j < ARRAYSIZE(m_boxpnt); j++)
		{
			tmp[0] = tmp[1] = tmp[2] = 0;
			tmp[j % 3] = (j < 3) ? 1.0 : -1.0;

			R_StudioLighting(&lv, pbbox[i].bone, 0, tmp);

			gEngfuncs.pTriAPI->Brightness(lv);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][0]]);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][1]]);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][2]]);
			gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][3]]);
		}

		gEngfuncs.pTriAPI->End();
	}
}

void CStudioModelRenderer::StudioDrawAbsBBox()
{
	int j;
	float lv;
	vec3_t tmp;
	vec3_t p[8];
	mstudioseqdesc_t *pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
	gEngfuncs.pTriAPI->SpriteTexture(m_pWhiteSprite, 0);

	for (j = 0; j < ARRAYSIZE(p); j++)
	{
		p[j][0] = ((j & 1) ? pseqdesc->bbmin[0] : pseqdesc->bbmax[0]) + m_pCurrentEntity->origin[0];
		p[j][1] = ((j & 2) ? pseqdesc->bbmin[1] : pseqdesc->bbmax[1]) + m_pCurrentEntity->origin[1];
		p[j][2] = ((j & 4) ? pseqdesc->bbmin[2] : pseqdesc->bbmax[2]) + m_pCurrentEntity->origin[2];
	}

	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->Color4f(0.5f, 0.5f, 1.0f, 1.0f);
	//gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);

	for (j = 0; j < ARRAYSIZE(m_boxpnt); j++)
	{
		tmp[0] = tmp[1] = tmp[2] = 0;
		tmp[j % 3] = (j < 3) ? 1.0 : -1.0;

		R_StudioLighting(&lv, -1, 0, tmp);

		gEngfuncs.pTriAPI->Brightness(lv);
		gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][0]]);
		gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][1]]);
		gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][2]]);
		gEngfuncs.pTriAPI->Vertex3fv(p[m_boxpnt[j][3]]);
	}

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
}

void CStudioModelRenderer::StudioRenderFinal_Software()
{
	// Note, rendermode set here has effect in SW
	IEngineStudio.SetupRenderer(kRenderNormal);

	if (m_pCvarDrawEntities->value == 2)
	{
		//IEngineStudio.StudioDrawBones();
		StudioDrawBones(); // IEngineStudio.StudioDrawBones();
	}
	else if (m_pCvarDrawEntities->value == 3)
	{
		StudioDrawHulls(); // IEngineStudio.StudioDrawHulls();
	}
	else
	{
		if (m_pPlayerSync)
		{
			if (m_pCurrentEntity->player && m_pCvarDrawEntities->value == 6)
			{
				gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
				StudioDrawHulls();
				gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
			}
		}
		else
		{
			// Draw mesh model
			for (int i = 0; i < m_pStudioHeader->numbodyparts; i++)
			{
				IEngineStudio.StudioSetupModel(i, (void **)&m_pBodyPart, (void **)&m_pSubModel);
				IEngineStudio.StudioDrawPoints();
			}
		}
	}

	if (m_pCvarDrawEntities->value == 4)
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		StudioDrawHulls(); // IEngineStudio.StudioDrawHulls();
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	}

	if (m_pCvarDrawEntities->value == 5)
	{
		StudioDrawAbsBBox(); // IEngineStudio.StudioDrawAbsBBox();
	}

	IEngineStudio.RestoreRenderer();
}

void CStudioModelRenderer::StudioRenderFinal_Hardware()
{
	int rendermode = IEngineStudio.GetForceFaceFlags() ? kRenderTransAdd : m_pCurrentEntity->curstate.rendermode;
	IEngineStudio.SetupRenderer(rendermode);

	if (m_pCvarDrawEntities->value == 2)
	{
		StudioDrawBones(); // IEngineStudio.StudioDrawBones();
	}
	else if (m_pCvarDrawEntities->value == 3)
	{
		StudioDrawHulls(); // IEngineStudio.StudioDrawHulls();
	}
	else
	{
		if (m_pPlayerSync)
		{
			if (m_pCurrentEntity->player && m_pCvarDrawEntities->value == 6)
			{
				gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
				StudioDrawHulls();
				gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
			}
		}
		else
		{
			// Draw mesh model
			for (int i = 0; i < m_pStudioHeader->numbodyparts; i++)
			{
				IEngineStudio.StudioSetupModel(i, (void **)&m_pBodyPart, (void **)&m_pSubModel);

				if (m_fDoInterp)
				{
					// interpolation messes up bounding boxes.
					m_pCurrentEntity->trivial_accept = 0;
				}

				IEngineStudio.GL_SetRenderMode(rendermode);
				IEngineStudio.StudioDrawPoints();
				IEngineStudio.GL_StudioDrawShadow();
			}
		}
	}

	if (m_pCvarDrawEntities->value == 4)
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		StudioDrawHulls(); // IEngineStudio.StudioDrawHulls();
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	}

	if (m_pCvarDrawEntities->value == 5)
	{
		StudioDrawAbsBBox(); // IEngineStudio.StudioDrawAbsBBox();
	}

	IEngineStudio.RestoreRenderer();
}

void CStudioModelRenderer::StudioRenderFinal()
{
	if (IEngineStudio.IsHardware())
	{
		StudioRenderFinal_Hardware();
	}
	else
	{
		StudioRenderFinal_Software();
	}
}
