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

class CBasePlayer;

const float MAX_NORMAL_BATTERY    = 100.0f;
const float MAX_DIST_RELOAD_SOUND = 512.0f;

#define MAX_WEAPONS                 32

#define ITEM_FLAG_SELECTONEMPTY     1
#define ITEM_FLAG_NOAUTORELOAD      2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY 4
#define ITEM_FLAG_LIMITINWORLD      8
#define ITEM_FLAG_EXHAUSTIBLE       16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET          0x40

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP               -1

#define LOUD_GUN_VOLUME             1000
#define NORMAL_GUN_VOLUME           600
#define QUIET_GUN_VOLUME            200

#define BRIGHT_GUN_FLASH            512
#define NORMAL_GUN_FLASH            256
#define DIM_GUN_FLASH               128

#define BIG_EXPLOSION_VOLUME        2048
#define NORMAL_EXPLOSION_VOLUME     1024
#define SMALL_EXPLOSION_VOLUME      512

#define WEAPON_ACTIVITY_VOLUME      64

// spawn flags
#define SF_DETONATE                 BIT(0) // Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges

// custom enum
enum ArmorType
{
	ARMOR_NONE,     // No armor
	ARMOR_KEVLAR,   // Body vest only
	ARMOR_VESTHELM, // Vest and helmet
};

enum ArmouryItemPack
{
	ARMOURY_MP5NAVY,
	ARMOURY_TMP,
	ARMOURY_P90,
	ARMOURY_MAC10,
	ARMOURY_AK47,
	ARMOURY_SG552,
	ARMOURY_M4A1,
	ARMOURY_AUG,
	ARMOURY_SCOUT,
	ARMOURY_G3SG1,
	ARMOURY_AWP,
	ARMOURY_M3,
	ARMOURY_XM1014,
	ARMOURY_M249,
	ARMOURY_FLASHBANG,
	ARMOURY_HEGRENADE,
	ARMOURY_KEVLAR,
	ARMOURY_ASSAULT,
	ARMOURY_SMOKEGRENADE,
	ARMOURY_SHIELD,
	ARMOURY_FAMAS,
	ARMOURY_SG550,
	ARMOURY_GALIL,
	ARMOURY_UMP45,
	ARMOURY_GLOCK18,
	ARMOURY_USP,
	ARMOURY_ELITE,
	ARMOURY_FIVESEVEN,
	ARMOURY_P228,
	ARMOURY_DEAGLE,
};

struct ItemInfo
{
	int iSlot;
	int iPosition;
	const char *pszAmmo1;
	int iMaxAmmo1;
	const char *pszAmmo2;
	int iMaxAmmo2;
	const char *pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight;
};

struct AmmoInfo
{
	const char *pszName;
	int iId;
};

struct MULTIDAMAGE
{
	CBaseEntity *pEntity;
	float amount;
	int type;
};

#include "weapontype.h"
#include "items.h"

class CArmoury: public CBaseEntity {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual void Restart() = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
public:
	ArmouryItemPack m_iItem;
	int m_iCount;
	int m_iInitialCount;
	bool m_bAlreadyCounted;
};

// Smoke Grenade / HE grenade / Flashbang grenade / C4
class CGrenade: public CBaseMonster {
public:
	virtual void Spawn() = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual int ObjectCaps() = 0;
	virtual void Killed(entvars_t *pevAttacker, int iGib) = 0;
	virtual int BloodColor() = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual void BounceSound() = 0;
public:
	bool m_bStartDefuse;
	bool m_bIsC4;
	EntityHandle<CBasePlayer> m_pBombDefuser;
	float m_flDefuseCountDown;
	float m_flC4Blow;
	float m_flNextFreqInterval;
	float m_flNextBeep;
	float m_flNextFreq;
	char *m_sBeepName;
	float m_fAttenu;
	float m_flNextBlink;
	float m_fNextDefuse;
	bool m_bJustBlew;
	int m_iTeam;
	int m_iCurWave;
	edict_t *m_pentCurBombTarget;
	int m_SGSmoke;
	int m_angle;
	unsigned short m_usEvent;
	bool m_bLightSmoke;
	bool m_bDetonated;
	Vector m_vSmokeDetonate;
	int m_iBounceCount;
	BOOL m_fRegisteredSound;	// whether or not this grenade has issued its DANGER sound to the world sound list yet.
};

// Items that the player has in their inventory that they can use
class CBasePlayerItem: public CBaseAnimating {
public:
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual void SetObjectCollisionBox() = 0;
	virtual CBaseEntity *Respawn() = 0;
	virtual int AddToPlayer(CBasePlayer *pPlayer) = 0;			// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate(CBasePlayerItem *pItem) = 0;		// return TRUE if you want your duplicate removed from world
	virtual int GetItemInfo(ItemInfo *p) = 0;					// returns 0 if struct not filled out
	virtual BOOL CanDeploy() = 0;
	virtual BOOL CanDrop() = 0;									// returns is deploy was successful
	virtual BOOL Deploy() = 0;
	virtual BOOL IsWeapon() = 0;
	virtual BOOL CanHolster() = 0;								// can this weapon be put away right now?
	virtual void Holster(int skiplocal = 0) = 0;
	virtual void UpdateItemInfo() = 0;
	virtual void ItemPreFrame() = 0;							// called each frame by the player PreThink
	virtual void ItemPostFrame() = 0;							// called each frame by the player PostThink
	virtual void Drop() = 0;
	virtual void Kill() = 0;
	virtual void AttachToPlayer(CBasePlayer *pPlayer) = 0;
	virtual int PrimaryAmmoIndex() = 0;
	virtual int SecondaryAmmoIndex() = 0;
	virtual int UpdateClientData(CBasePlayer *pPlayer) = 0;
	virtual CBasePlayerItem *GetWeaponPtr() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;								// return 0 to MAX_ITEMS_SLOTS, used in hud
public:
	CBasePlayer *m_pPlayer;
	CBasePlayerItem *m_pNext;
	int m_iId;							// WEAPON_???
};

// inventory items that
class CBasePlayerWeapon: public CBasePlayerItem {
public:
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddToPlayer(CBasePlayer *pPlayer) = 0;
	virtual int AddDuplicate(CBasePlayerItem *pItem) = 0;
	virtual BOOL CanDeploy() = 0;
	virtual BOOL IsWeapon() = 0;
	virtual void Holster(int skiplocal = 0) = 0;
	virtual void UpdateItemInfo() = 0;
	virtual void ItemPostFrame() = 0;
	virtual int PrimaryAmmoIndex() = 0;
	virtual int SecondaryAmmoIndex() = 0;
	virtual int UpdateClientData(CBasePlayer *pPlayer) = 0;
	virtual CBasePlayerItem *GetWeaponPtr() = 0;
	virtual int ExtractAmmo(CBasePlayerWeapon *pWeapon) = 0;
	virtual int ExtractClipAmmo(CBasePlayerWeapon *pWeapon) = 0;
	virtual int AddWeapon() = 0;
	virtual BOOL PlayEmptySound() = 0;
	virtual void ResetEmptySound() = 0;
	virtual void SendWeaponAnim(int iAnim, int skiplocal = 0) = 0;
	virtual BOOL IsUseable() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual void RetireWeapon() = 0;
	virtual BOOL ShouldWeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	BOOL IsPistol() { return (m_iId == WEAPON_USP || m_iId == WEAPON_GLOCK18 || m_iId == WEAPON_P228 || m_iId == WEAPON_DEAGLE || m_iId == WEAPON_ELITE || m_iId == WEAPON_FIVESEVEN); }

	int m_iPlayEmptySound;
	int m_fFireOnEmpty;
	float m_flNextPrimaryAttack;	// soonest time ItemPostFrame will call PrimaryAttack
	float m_flNextSecondaryAttack;	// soonest time ItemPostFrame will call SecondaryAttack
	float m_flTimeWeaponIdle;		// soonest time ItemPostFrame will call WeaponIdle
	int m_iPrimaryAmmoType;			// "primary" ammo index into players m_rgAmmo[]
	int m_iSecondaryAmmoType;		// "secondary" ammo index into players m_rgAmmo[]
	int m_iClip;					// number of shots left in the primary weapon clip, -1 it not used
	int m_iClientClip;				// the last version of m_iClip sent to hud dll
	int m_iClientWeaponState;		// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int m_fInReload;				// Are we in the middle of a reload;
	int m_fInSpecialReload;			// Are we in the middle of a reload for the shotguns
	int m_iDefaultAmmo;				// how much ammo you get when you pick up this weapon as placed by a level designer.
	int m_iShellId;
	float m_fMaxSpeed;
	bool m_bDelayFire;
	int m_iDirection;
	bool m_bSecondarySilencerOn;
	float m_flAccuracy;
	float m_flLastFire;
	int m_iShotsFired;
	Vector m_vVecAiming;
	string_t model_name;
	float m_flGlock18Shoot;			// time to shoot the remaining bullets of the glock18 burst fire
	int m_iGlock18ShotsFired;		// used to keep track of the shots fired during the Glock18 burst fire mode.
	float m_flFamasShoot;
	int m_iFamasShotsFired;
	float m_fBurstSpread;
	int m_iWeaponState;
	float m_flNextReload;
	float m_flDecreaseShotsFired;
	unsigned short m_usFireGlock18;
	unsigned short m_usFireFamas;

	// hle time creep vars
	float m_flPrevPrimaryAttack;
	float m_flLastFireTime;
};

class CBasePlayerAmmo: public CBaseEntity {
public:
	virtual void Spawn() = 0;
	virtual BOOL AddAmmo(CBaseEntity *pOther) = 0;
	virtual CBaseEntity *Respawn() = 0;
};

class CWeaponBox: public CBaseEntity {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual int Save(CSave &save) = 0;
	virtual int Restore(CRestore &restore) = 0;
	virtual void SetObjectCollisionBox() = 0;
	virtual void Touch(CBaseEntity *pOther) = 0;
public:
	CBasePlayerItem *m_rgpPlayerItems[MAX_ITEM_TYPES];
	string_t m_rgiszAmmo[MAX_AMMO_SLOTS];
	int m_rgAmmo[MAX_AMMO_SLOTS];
	int m_cAmmoTypes;
	bool m_bIsBomb;
};


const float USP_MAX_SPEED     = 250.0f;
const float USP_DAMAGE        = 34.0f;
const float USP_DAMAGE_SIL    = 30.0f;
const float USP_RANGE_MODIFER = 0.79f;
const float USP_RELOAD_TIME   = 2.7f;

enum usp_e
{
	USP_IDLE,
	USP_SHOOT1,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOT_EMPTY,
	USP_RELOAD,
	USP_DRAW,
	USP_ATTACH_SILENCER,
	USP_UNSIL_IDLE,
	USP_UNSIL_SHOOT1,
	USP_UNSIL_SHOOT2,
	USP_UNSIL_SHOOT3,
	USP_UNSIL_SHOOT_EMPTY,
	USP_UNSIL_RELOAD,
	USP_UNSIL_DRAW,
	USP_DETACH_SILENCER,
};

enum usp_shield_e
{
	USP_SHIELD_IDLE,
	USP_SHIELD_SHOOT1,
	USP_SHIELD_SHOOT2,
	USP_SHIELD_SHOOT_EMPTY,
	USP_SHIELD_RELOAD,
	USP_SHIELD_DRAW,
	USP_SHIELD_UP_IDLE,
	USP_SHIELD_UP,
	USP_SHIELD_DOWN,
};

class CUSP: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float MP5N_MAX_SPEED     = 250.0f;
const float MP5N_DAMAGE        = 26.0f;
const float MP5N_RANGE_MODIFER = 0.84f;
const float MP5N_RELOAD_TIME   = 2.63f;

enum mp5n_e
{
	MP5N_IDLE1,
	MP5N_RELOAD,
	MP5N_DRAW,
	MP5N_SHOOT1,
	MP5N_SHOOT2,
	MP5N_SHOOT3,
};

class CMP5N: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float SG552_MAX_SPEED      = 235.0f;
const float SG552_MAX_SPEED_ZOOM = 200.0f;
const float SG552_DAMAGE         = 33.0f;
const float SG552_RANGE_MODIFER  = 0.955f;
const float SG552_RELOAD_TIME    = 3.0f;

enum sg552_e
{
	SG552_IDLE1,
	SG552_RELOAD,
	SG552_DRAW,
	SG552_SHOOT1,
	SG552_SHOOT2,
	SG552_SHOOT3,
};

class CSG552: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float AK47_MAX_SPEED     = 221.0f;
const float AK47_DAMAGE        = 36.0f;
const float AK47_RANGE_MODIFER = 0.98f;
const float AK47_RELOAD_TIME   = 2.45f;

enum ak47_e
{
	AK47_IDLE1,
	AK47_RELOAD,
	AK47_DRAW,
	AK47_SHOOT1,
	AK47_SHOOT2,
	AK47_SHOOT3,
};

class CAK47: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float AUG_MAX_SPEED     = 240.0f;
const float AUG_DAMAGE        = 32.0f;
const float AUG_RANGE_MODIFER = 0.96f;
const float AUG_RELOAD_TIME   = 3.3f;

enum aug_e
{
	AUG_IDLE1,
	AUG_RELOAD,
	AUG_DRAW,
	AUG_SHOOT1,
	AUG_SHOOT2,
	AUG_SHOOT3,
};

class CAUG: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float AWP_MAX_SPEED      = 210.0f;
const float AWP_MAX_SPEED_ZOOM = 150.0f;
const float AWP_DAMAGE         = 115.0f;
const float AWP_RANGE_MODIFER  = 0.99f;
const float AWP_RELOAD_TIME    = 2.5f;

enum awp_e
{
	AWP_IDLE,
	AWP_SHOOT,
	AWP_SHOOT2,
	AWP_SHOOT3,
	AWP_RELOAD,
	AWP_DRAW,
};

class CAWP: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


// for usermsg BombDrop
#define BOMB_FLAG_DROPPED	0 // if the bomb was dropped due to voluntary dropping or death/disconnect
#define BOMB_FLAG_PLANTED	1 // if the bomb has been planted will also trigger the round timer to hide will also show where the dropped bomb on the Terrorist team's radar.

const float C4_MAX_AMMO       = 1.0f;
const float C4_MAX_SPEED      = 250.0f;
const float C4_ARMING_ON_TIME = 3.0f;

enum c4_e
{
	C4_IDLE1,
	C4_DRAW,
	C4_DROP,
	C4_ARM,
};

class CC4: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual void Holster(int skiplocal) = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	bool m_bStartedArming;
	bool m_bBombPlacedAnimation;
	float m_fArmedTime;
	bool m_bHasShield;
};


const float DEAGLE_MAX_SPEED     = 250.0f;
const float DEAGLE_DAMAGE        = 54.0f;
const float DEAGLE_RANGE_MODIFER = 0.81f;
const float DEAGLE_RELOAD_TIME   = 2.2f;

enum deagle_e
{
	DEAGLE_IDLE1,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,
	DEAGLE_DRAW,
};

class CDEAGLE: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float FLASHBANG_MAX_SPEED        = 250.0f;
const float FLASHBANG_MAX_SPEED_SHIELD = 180.0f;

enum flashbang_e
{
	FLASHBANG_IDLE,
	FLASHBANG_PULLPIN,
	FLASHBANG_THROW,
	FLASHBANG_DRAW,
};

class CFlashbang: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL CanDeploy() = 0;
	virtual BOOL CanDrop() = 0;
	virtual BOOL Deploy() = 0;
	virtual void Holster(int skiplocal) = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
};


const float G3SG1_MAX_SPEED      = 210.0f;
const float G3SG1_MAX_SPEED_ZOOM = 150.0f;
const float G3SG1_DAMAGE         = 80.0f;
const float G3SG1_RANGE_MODIFER  = 0.98f;
const float G3SG1_RELOAD_TIME    = 3.5f;

enum g3sg1_e
{
	G3SG1_IDLE,
	G3SG1_SHOOT,
	G3SG1_SHOOT2,
	G3SG1_RELOAD,
	G3SG1_DRAW,
};

class CG3SG1: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float GLOCK18_MAX_SPEED     = 250.0f;
const float GLOCK18_DAMAGE        = 25.0f;
const float GLOCK18_RANGE_MODIFER = 0.75f;
const float GLOCK18_RELOAD_TIME   = 2.2f;

enum glock18_e
{
	GLOCK18_IDLE1,
	GLOCK18_IDLE2,
	GLOCK18_IDLE3,
	GLOCK18_SHOOT,
	GLOCK18_SHOOT2,
	GLOCK18_SHOOT3,
	GLOCK18_SHOOT_EMPTY,
	GLOCK18_RELOAD,
	GLOCK18_DRAW,
	GLOCK18_HOLSTER,
	GLOCK18_ADD_SILENCER,
	GLOCK18_DRAW2,
	GLOCK18_RELOAD2,
};

enum glock18_shield_e
{
	GLOCK18_SHIELD_IDLE1,
	GLOCK18_SHIELD_SHOOT,
	GLOCK18_SHIELD_SHOOT2,
	GLOCK18_SHIELD_SHOOT_EMPTY,
	GLOCK18_SHIELD_RELOAD,
	GLOCK18_SHIELD_DRAW,
	GLOCK18_SHIELD_IDLE,
	GLOCK18_SHIELD_UP,
	GLOCK18_SHIELD_DOWN,
};

class CGLOCK18: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
public:
	int m_iShell;
	bool m_bBurstFire;
};


const float HEGRENADE_MAX_SPEED        = 250.0f;
const float HEGRENADE_MAX_SPEED_SHIELD = 180.0f;

enum hegrenade_e
{
	HEGRENADE_IDLE,
	HEGRENADE_PULLPIN,
	HEGRENADE_THROW,
	HEGRENADE_DRAW,
};

class CHEGrenade: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL CanDeploy() = 0;
	virtual BOOL CanDrop() = 0;
	virtual BOOL Deploy() = 0;
	virtual void Holster(int skiplocal) = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	unsigned short m_usCreate;
};


const float KNIFE_BODYHIT_VOLUME   = 128.0f;
const float KNIFE_WALLHIT_VOLUME   = 512.0f;
const float KNIFE_MAX_SPEED        = 250.0f;
const float KNIFE_MAX_SPEED_SHIELD = 180.0f;

enum knife_e
{
	KNIFE_IDLE,
	KNIFE_ATTACK1HIT,
	KNIFE_ATTACK2HIT,
	KNIFE_DRAW,
	KNIFE_STABHIT,
	KNIFE_STABMISS,
	KNIFE_MIDATTACK1HIT,
	KNIFE_MIDATTACK2HIT,
};

enum knife_shield_e
{
	KNIFE_SHIELD_IDLE,
	KNIFE_SHIELD_SLASH,
	KNIFE_SHIELD_ATTACKHIT,
	KNIFE_SHIELD_DRAW,
	KNIFE_SHIELD_UPIDLE,
	KNIFE_SHIELD_UP,
	KNIFE_SHIELD_DOWN,
};

class CKnife: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL CanDrop() = 0;
	virtual BOOL Deploy() = 0;
	virtual void Holster(int skiplocal) = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual void WeaponIdle() = 0;
public:
	TraceResult m_trHit;
	unsigned short m_usKnife;
};


const float M249_MAX_SPEED     = 220.0f;
const float M249_DAMAGE        = 32.0f;
const float M249_RANGE_MODIFER = 0.97f;
const float M249_RELOAD_TIME   = 4.7f;

enum m249_e
{
	M249_IDLE1,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_RELOAD,
	M249_DRAW,
};

class CM249: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float M3_MAX_SPEED   = 230.0f;
const Vector M3_CONE_VECTOR = Vector(0.0675, 0.0675, 0.0); // special shotgun spreads

enum m3_e
{
	M3_IDLE,
	M3_FIRE1,
	M3_FIRE2,
	M3_RELOAD,
	M3_PUMP,
	M3_START_RELOAD,
	M3_DRAW,
	M3_HOLSTER,
};

class CM3: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	float m_flPumpTime;
	unsigned short m_usFire;
};


const float M4A1_MAX_SPEED         = 230.0f;
const float M4A1_DAMAGE            = 32.0f;
const float M4A1_DAMAGE_SIL        = 33.0f;
const float M4A1_RANGE_MODIFER     = 0.97f;
const float M4A1_RANGE_MODIFER_SIL = 0.95f;
const float M4A1_RELOAD_TIME       = 3.05f;

enum m4a1_e
{
	M4A1_IDLE,
	M4A1_SHOOT1,
	M4A1_SHOOT2,
	M4A1_SHOOT3,
	M4A1_RELOAD,
	M4A1_DRAW,
	M4A1_ATTACH_SILENCER,
	M4A1_UNSIL_IDLE,
	M4A1_UNSIL_SHOOT1,
	M4A1_UNSIL_SHOOT2,
	M4A1_UNSIL_SHOOT3,
	M4A1_UNSIL_RELOAD,
	M4A1_UNSIL_DRAW,
	M4A1_DETACH_SILENCER,
};

class CM4A1: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float MAC10_MAX_SPEED     = 250.0f;
const float MAC10_DAMAGE        = 29.0f;
const float MAC10_RANGE_MODIFER = 0.82f;
const float MAC10_RELOAD_TIME   = 3.15f;

enum mac10_e
{
	MAC10_IDLE1,
	MAC10_RELOAD,
	MAC10_DRAW,
	MAC10_SHOOT1,
	MAC10_SHOOT2,
	MAC10_SHOOT3,
};

class CMAC10: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float P228_MAX_SPEED     = 250.0f;
const float P228_DAMAGE        = 32.0f;
const float P228_RANGE_MODIFER = 0.8f;
const float P228_RELOAD_TIME   = 2.7f;

enum p228_e
{
	P228_IDLE,
	P228_SHOOT1,
	P228_SHOOT2,
	P228_SHOOT3,
	P228_SHOOT_EMPTY,
	P228_RELOAD,
	P228_DRAW,
};

enum p228_shield_e
{
	P228_SHIELD_IDLE,
	P228_SHIELD_SHOOT1,
	P228_SHIELD_SHOOT2,
	P228_SHIELD_SHOOT_EMPTY,
	P228_SHIELD_RELOAD,
	P228_SHIELD_DRAW,
	P228_SHIELD_IDLE_UP,
	P228_SHIELD_UP,
	P228_SHIELD_DOWN,
};

class CP228: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float P90_MAX_SPEED     = 245.0f;
const float P90_DAMAGE        = 21.0f;
const float P90_RANGE_MODIFER = 0.885f;
const float P90_RELOAD_TIME   = 3.4f;

enum p90_e
{
	P90_IDLE1,
	P90_RELOAD,
	P90_DRAW,
	P90_SHOOT1,
	P90_SHOOT2,
	P90_SHOOT3,
};

class CP90: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float SCOUT_MAX_SPEED      = 260.0f;
const float SCOUT_MAX_SPEED_ZOOM = 220.0f;
const float SCOUT_DAMAGE         = 75.0f;
const float SCOUT_RANGE_MODIFER  = 0.98f;
const float SCOUT_RELOAD_TIME    = 2.0f;

enum scout_e
{
	SCOUT_IDLE,
	SCOUT_SHOOT,
	SCOUT_SHOOT2,
	SCOUT_RELOAD,
	SCOUT_DRAW,
};

class CSCOUT: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float SMOKEGRENADE_MAX_SPEED        = 250.0f;
const float SMOKEGRENADE_MAX_SPEED_SHIELD = 180.0f;

enum smokegrenade_e
{
	SMOKEGRENADE_IDLE,
	SMOKEGRENADE_PINPULL,
	SMOKEGRENADE_THROW,
	SMOKEGRENADE_DRAW,
};

class CSmokeGrenade: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL CanDeploy() = 0;
	virtual BOOL CanDrop() = 0;
	virtual BOOL Deploy() = 0;
	virtual void Holster(int skiplocal) = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	unsigned short m_usCreate;
};


const float TMP_MAX_SPEED     = 250.0f;
const float TMP_DAMAGE        = 20.0f;
const float TMP_RANGE_MODIFER = 0.85f;
const float TMP_RELOAD_TIME   = 2.12f;

enum tmp_e
{
	TMP_IDLE1,
	TMP_RELOAD,
	TMP_DRAW,
	TMP_SHOOT1,
	TMP_SHOOT2,
	TMP_SHOOT3,
};

class CTMP: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float XM1014_MAX_SPEED   = 240.0f;
const Vector XM1014_CONE_VECTOR = Vector(0.0725, 0.0725, 0.0); // special shotgun spreads

enum xm1014_e
{
	XM1014_IDLE,
	XM1014_FIRE1,
	XM1014_FIRE2,
	XM1014_RELOAD,
	XM1014_PUMP,
	XM1014_START_RELOAD,
	XM1014_DRAW,
};

class CXM1014: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	float m_flPumpTime;
	unsigned short m_usFire;
};


const float ELITE_MAX_SPEED     = 250.0f;
const float ELITE_RELOAD_TIME   = 4.5f;
const float ELITE_DAMAGE        = 36.0f;
const float ELITE_RANGE_MODIFER = 0.75f;

enum elite_e
{
	ELITE_IDLE,
	ELITE_IDLE_LEFTEMPTY,
	ELITE_SHOOTLEFT1,
	ELITE_SHOOTLEFT2,
	ELITE_SHOOTLEFT3,
	ELITE_SHOOTLEFT4,
	ELITE_SHOOTLEFT5,
	ELITE_SHOOTLEFTLAST,
	ELITE_SHOOTRIGHT1,
	ELITE_SHOOTRIGHT2,
	ELITE_SHOOTRIGHT3,
	ELITE_SHOOTRIGHT4,
	ELITE_SHOOTRIGHT5,
	ELITE_SHOOTRIGHTLAST,
	ELITE_RELOAD,
	ELITE_DRAW,
};

class CELITE: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
public:
	int m_iShell;
	unsigned short m_usFire_LEFT;
	unsigned short m_usFire_RIGHT;
};


const float FIVESEVEN_MAX_SPEED     = 250.0f;
const float FIVESEVEN_DAMAGE        = 20.0f;
const float FIVESEVEN_RANGE_MODIFER = 0.885f;
const float FIVESEVEN_RELOAD_TIME   = 2.7f;

enum fiveseven_e
{
	FIVESEVEN_IDLE,
	FIVESEVEN_SHOOT1,
	FIVESEVEN_SHOOT2,
	FIVESEVEN_SHOOT_EMPTY,
	FIVESEVEN_RELOAD,
	FIVESEVEN_DRAW,
};

class CFiveSeven: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
	virtual BOOL IsPistol() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float UMP45_MAX_SPEED     = 250.0f;
const float UMP45_DAMAGE        = 30.0f;
const float UMP45_RANGE_MODIFER = 0.82f;
const float UMP45_RELOAD_TIME   = 3.5f;

enum ump45_e
{
	UMP45_IDLE1,
	UMP45_RELOAD,
	UMP45_DRAW,
	UMP45_SHOOT1,
	UMP45_SHOOT2,
	UMP45_SHOOT3,
};

class CUMP45: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float SG550_MAX_SPEED      = 210.0f;
const float SG550_MAX_SPEED_ZOOM = 150.0f;
const float SG550_DAMAGE         = 70.0f;
const float SG550_RANGE_MODIFER  = 0.98f;
const float SG550_RELOAD_TIME    = 3.35f;

enum sg550_e
{
	SG550_IDLE,
	SG550_SHOOT,
	SG550_SHOOT2,
	SG550_RELOAD,
	SG550_DRAW,
};

class CSG550: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	unsigned short m_usFire;
};


const float GALIL_MAX_SPEED     = 240.0f;
const float GALIL_DAMAGE        = 30.0f;
const float GALIL_RANGE_MODIFER = 0.98f;
const float GALIL_RELOAD_TIME   = 2.45f;

enum galil_e
{
	GALIL_IDLE1,
	GALIL_RELOAD,
	GALIL_DRAW,
	GALIL_SHOOT1,
	GALIL_SHOOT2,
	GALIL_SHOOT3,
};

class CGalil: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
	unsigned short m_usFire;
};


const float FAMAS_MAX_SPEED     = 240.0f;
const float FAMAS_RELOAD_TIME   = 3.3f;
const float FAMAS_DAMAGE        = 30.0f;
const float FAMAS_DAMAGE_BURST  = 34.0f;
const float FAMAS_RANGE_MODIFER = 0.96f;

enum famas_e
{
	FAMAS_IDLE1,
	FAMAS_RELOAD,
	FAMAS_DRAW,
	FAMAS_SHOOT1,
	FAMAS_SHOOT2,
	FAMAS_SHOOT3,
};

class CFamas: public CBasePlayerWeapon {
public:
	virtual void Spawn() = 0;
	virtual void Precache() = 0;
	virtual int GetItemInfo(ItemInfo *p) = 0;
	virtual BOOL Deploy() = 0;
	virtual float GetMaxSpeed() = 0;
	virtual int iItemSlot() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void Reload() = 0;
	virtual void WeaponIdle() = 0;
	virtual BOOL UseDecrement() = 0;
public:
	int m_iShell;
	int m_iShellOn;
};
