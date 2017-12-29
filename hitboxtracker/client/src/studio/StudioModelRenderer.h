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

class CStudioModelRenderer
{
public:
	// Construction/Destruction
	CStudioModelRenderer();
	virtual ~CStudioModelRenderer();

	// Initialization
	virtual void Init();

public:
	// Public Interfaces
	virtual int StudioDrawModel(int flags);
	virtual int StudioDrawPlayer(int flags, struct entity_state_s *pplayer);

public:
	// Local interfaces
	void StudioSetShadowSprite(int iSprite);

	// Look up animation data for sequence
	virtual mstudioanim_t *StudioGetAnim(model_t *psubmodel, mstudioseqdesc_t *pseqdesc);

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpTransform(int trivial_accept);

	// Set up model bone positions
	virtual void StudioSetupBones();

	// Find final attachment points
	virtual void StudioCalcAttachments();

	// Save bone matrices and names
	virtual void StudioSaveBones();

	// Merge cached bones with current bones for model
	virtual void StudioMergeBones(model_t *psubmodel);

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant();

	// Determine current frame for rendering
	virtual float StudioEstimateFrame(mstudioseqdesc_t *pseqdesc);

	// Apply special effects to transform matrix
	virtual void StudioFxTransform(cl_entity_t *ent, float transform[3][4]);

	// Spherical interpolation of bones
	virtual void StudioSlerpBones(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s);

	// Compute bone adjustments(bone controllers)
	virtual void StudioCalcBoneAdj(float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen);

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q);

	// Get bone positions
	virtual void StudioCalcBonePosition(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos);

	// Compute rotations
	virtual void StudioCalcRotations(float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f);

	// Send bones and verts to renderer
	virtual void StudioRenderModel();

	// Finalize rendering
	virtual void StudioRenderFinal();

	// GL&D3D vs. Software renderer finishing functions
	virtual void StudioRenderFinal_Software();
	virtual void StudioRenderFinal_Hardware();

	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch);

	// Estimate gait frame for player
	virtual void StudioEstimateGait(entity_state_t *pplayer);

	// Process movement of player
	virtual void StudioProcessGait(entity_state_t *pplayer);

	void StudioDrawShadow(Vector &origin, float scale);

	void StudioDrawBones();
	void StudioDrawHulls();
	void StudioDrawAbsBBox();

public:
	static int m_iShadowSprite;
	static int m_boxpnt[6][4];
	static vec3_t m_hullcolor[8];

	double   m_clTime;			// Client clock
	double   m_clOldTime;		// Old Client clock
	qboolean m_fDoInterp;		// Do interpolation?
	qboolean m_fGaitEstimation;	// Do gait estimation?
	int      m_nFrameCount;		// Current render frame #

	// Cvars that studio model code needs to reference
	cvar_t *m_pCvarHiModels;				// Use high quality models?
	cvar_t *m_pCvarDeveloper;				// Developer debug output desired?
	cvar_t *m_pCvarDrawEntities;			// Draw entities bone hit boxes, etc?

	cl_entity_t   *m_pCurrentEntity;		// The entity which we are currently rendering.
	model_t       *m_pRenderModel;			// The model for the entity being rendered
	player_info_t *m_pPlayerInfo;			// Player info for current player, if drawing a player
	player_sync_t *m_pPlayerSync;			// Player synchronized info from the server for current player, if drawing a player

	int            m_nPlayerIndex;			// The index of the player being drawn
	float          m_flGaitMovement;		// The player's gait movement

	studiohdr_t        *m_pStudioHeader;	// Pointer to header block for studio model data
	mstudiobodyparts_t *m_pBodyPart;		// Pointers to current body part and submodel
	mstudiomodel_t     *m_pSubModel;

	// Palette substition for top and bottom of model
	int m_nTopColor;
	int m_nBottomColor;

	model_t *m_pChromeSprite; // Sprite model used for drawing studio model chrome
	model_t *m_pWhiteSprite;  // Sprite model used for drawing studio model hulls

	// Caching
	int  m_nCachedBones;							// Number of bones in bone cache
	char m_nCachedBoneNames[MAXSTUDIOBONES][32];	// Names of cached bones

	// Cached bone & light transformation matrices
	float m_rgCachedBoneTransform [MAXSTUDIOBONES][3][4];
	float m_rgCachedLightTransform[MAXSTUDIOBONES][3][4];

	// Software renderer scale factors
	float m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	float m_vUp[3];
	float m_vRight[3];
	float m_vNormal[3];

	float m_vRenderOrigin[3];

	// Model render counters(from engine)
	int *m_pStudioModelCount;
	int *m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float (*m_protationmatrix)[3][4]; // Model to world transformation
	float (*m_paliastransform)[3][4]; // Model to view transformation

	// Concatenated bone and light transforms
	float (*m_pbonetransform) [MAXSTUDIOBONES][3][4];
	float (*m_plighttransform)[MAXSTUDIOBONES][3][4];
};

extern engine_studio_api_t IEngineStudio;
