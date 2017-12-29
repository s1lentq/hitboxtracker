/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#pragma once

#define SF_PLAT_TOGGLE BIT(0) // The lift is no more automatically called from top and activated by stepping on it.
                              // It required trigger to do so.

class CBasePlatTrain: public CBaseToggle {
public:
	virtual void Precache() = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual int ObjectCaps() = 0;

	// This is done to fix spawn flag collisions between this class and a derived class
	virtual BOOL IsTogglePlat() = 0;
public:
	byte m_bMoveSnd;
	byte m_bStopSnd;
	float m_volume;
};

class CFuncPlat: public CBasePlatTrain {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual void Blocked(CBaseEntity *pOther) = 0;
	virtual void GoUp() = 0;
	virtual void GoDown() = 0;
	virtual void HitTop() = 0;
	virtual void HitBottom() = 0;
};

class CPlatTrigger: public CBaseEntity {
public:
	virtual int ObjectCaps() = 0;
	virtual void Touch(CBaseEntity *pOther) = 0;
public:
	CFuncPlat *m_pPlatform;
};

class CFuncPlatRot: public CFuncPlat {
public:
	virtual void Spawn() = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual void GoUp() = 0;
	virtual void GoDown() = 0;
	virtual void HitTop() = 0;
	virtual void HitBottom() = 0;
public:
	Vector m_end;
	Vector m_start;
};

#define SF_TRAIN_WAIT_RETRIGGER BIT(0)
#define SF_TRAIN_START_ON       BIT(2) // Train is initially moving
#define SF_TRAIN_PASSABLE       BIT(3) // Train is not solid -- used to make water trains

class CFuncTrain: public CBasePlatTrain {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual void Restart() = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual void Activate() = 0;
	virtual void OverrideReset() = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual void Blocked(CBaseEntity *pOther) = 0;
public:
	Vector m_vStartPosition;
	entvars_t *m_pevFirstTarget;
	entvars_t *m_pevCurrentTarget;
	int m_sounds;
	BOOL m_activated;
};

// This class defines the volume of space that the player must stand in to control the train
class CFuncTrainControls: public CBaseEntity {
public:
	virtual void Spawn() = 0;
	virtual int ObjectCaps() = 0;
};

#define SF_TRACK_ACTIVATETRAIN BIT(0)
#define SF_TRACK_RELINK        BIT(1)
#define SF_TRACK_ROTMOVE       BIT(2)
#define SF_TRACK_STARTBOTTOM   BIT(3)
#define SF_TRACK_DONT_MOVE     BIT(4)

enum TRAIN_CODE { TRAIN_SAFE, TRAIN_BLOCKING, TRAIN_FOLLOWING };

// This entity is a rotating/moving platform that will carry a train to a new track.
// It must be larger in X-Y planar area than the train, since it must contain the
// train within these dimensions in order to operate when the train is near it.
class CFuncTrackChange: public CFuncPlatRot {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual void OverrideReset() = 0;
	virtual void Touch(CBaseEntity *pOther) = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual BOOL IsTogglePlat() = 0;
	virtual void GoUp() = 0;
	virtual void GoDown() = 0;
	virtual void HitTop() = 0;
	virtual void HitBottom() = 0;
	virtual void UpdateAutoTargets(int toggleState) = 0;

public:
	void DisableUse() { m_use = 0; }
	void EnableUse() { m_use = 1; }

	int UseEnabled() const { return m_use; }

public:
	static TYPEDESCRIPTION IMPL(m_SaveData)[9];

	CPathTrack *m_trackTop;
	CPathTrack *m_trackBottom;
	CFuncTrackTrain *m_train;

	int m_trackTopName;
	int m_trackBottomName;
	int m_trainName;

	TRAIN_CODE m_code;
	int m_targetState;
	int m_use;
};

class CFuncTrackAuto: public CFuncTrackChange {
public:
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual void UpdateAutoTargets(int toggleState) = 0;
};

// pev->speed is the travel speed
// pev->health is current health
// pev->max_health is the amount to reset to each time it starts

#define SF_GUNTARGET_START_ON BIT(0)

class CGunTarget: public CBaseMonster {
public:
	virtual void Spawn() = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual int ObjectCaps() = 0;
	virtual void Activate() = 0;
	virtual int Classify() = 0;
	virtual BOOL TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual int BloodColor() = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual Vector BodyTarget(const Vector &posSrc) = 0;
private:
	BOOL m_on;
};
