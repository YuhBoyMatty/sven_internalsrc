// Table of classes

#pragma once

// 16 flags only
#define FL_CLASS_FRIEND (1 << 0)
#define FL_CLASS_ENEMY (1 << 1)
#define FL_CLASS_NEUTRAL (1 << 2)
#define FL_CLASS_WORLD_ENTITY (1 << 3)
#define FL_CLASS_DEAD_BODY (1 << 4)
#define FL_CLASS_UNDEFINED_6 (1 << 5)
#define FL_CLASS_UNDEFINED_7 (1 << 6)
#define FL_CLASS_UNDEFINED_8 (1 << 7)
#define FL_CLASS_UNDEFINED_9 (1 << 8)
#define FL_CLASS_UNDEFINED_10 (1 << 9)
#define FL_CLASS_UNDEFINED_11 (1 << 10)
#define FL_CLASS_UNDEFINED_12 (1 << 11)
#define FL_CLASS_UNDEFINED_13 (1 << 12)
#define FL_CLASS_UNDEFINED_14 (1 << 13)
#define FL_CLASS_UNDEFINED_15 (1 << 14)
#define FL_CLASS_UNDEFINED_16 (1 << 15)

enum eClassID
{
	CLASS_NONE = 0,
	CLASS_PLAYER,
	CLASS_SCIENTIST,
	CLASS_BARNEY,
	CLASS_OTIS,
	CLASS_HEADCRAB,
	CLASS_ZOMBIE,
	CLASS_BULLSQUID,
	CLASS_HOUNDEYE,
	CLASS_BARNACLE,
	CLASS_VORTIGAUNT,
	CLASS_HUMAN_GRUNT,
	CLASS_ALIENT_GRUNT,
	CLASS_TENTACLE,
	CLASS_SENTRY,
	CLASS_TURRET,
	CLASS_LEECH,
	CLASS_GMAN,
	CLASS_FEMALE_ASSASSIN,
	CLASS_MALE_ASSASSIN,
	CLASS_SNARK,
	CLASS_CHUMTOAD,
	CLASS_ALIEN_CONTROLLER,
	CLASS_ICHTYOSAUR,
	CLASS_GARGANTUA,
	CLASS_BABY_GARGANTUA,
	CLASS_BIG_MOMMA,
	CLASS_OSPREY,
	CLASS_APACHE,
	CLASS_NIHILANTH,
	CLASS_HEV,
	CLASS_XEN_TREE,
	CLASS_XEN_FUNGUS,
	CLASS_HUMAN_GRUNT_OPFOR,
	CLASS_GONOME,
	CLASS_PIT_DRONE,
	CLASS_SHOCK_TROOPER,
	CLASS_VOLTIGORE,
	CLASS_BABY_VOLTIGORE,
	CLASS_PIT_WORM,
	CLASS_SHOCK_RIFLE,
	CLASS_SPORE_AMMO,
	CLASS_HEAVY_GRUNT,
	CLASS_ROBOT_GRUNT
};

typedef struct class_info_s
{
	unsigned short flags;
	unsigned short id;
} class_info_t;

inline const char *GetEntityClassname(class_info_t &classInfo)
{
	extern const char *g_szClassName[];
	return g_szClassName[classInfo.id];
}

inline bool IsEntityClassFriend(class_info_t &classInfo)
{
	return classInfo.flags & FL_CLASS_FRIEND;
}

inline bool IsEntityClassEnemy(class_info_t &classInfo)
{
	return classInfo.flags & FL_CLASS_ENEMY;
}

inline bool IsEntityClassNeutral(class_info_t &classInfo)
{
	return classInfo.flags & FL_CLASS_NEUTRAL;
}

inline bool IsEntityClassDeadbody(class_info_t &classInfo, int iSolid)
{
	return iSolid == 2 && classInfo.flags & FL_CLASS_DEAD_BODY;
}

inline bool IsEntityClassTrash(class_info_t &classInfo)
{
	return classInfo.flags & FL_CLASS_WORLD_ENTITY;
}

class_info_t GetEntityClassInfo(const char *pszModelName);

void InitClassTable();