// Table of classes

#include "class_table.h"

#include "../data_struct/hashdict.h"
#include "../data_struct/hashtable.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

static bool HashTable_CompareFunc(const unsigned int a, const unsigned int b);
static unsigned int HashTable_HashFunc(const unsigned int key);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

const char *g_szClassName[] =
{
	// position of class name must match to the class id
	"Unknown",
	"Player",
	"Scientist",
	"Barney",
	"Otis",
	"Headcrab",
	"Zombie",
	"Bullsquid",
	"Houndeye",
	"Barnacle",
	"Vortigaunt",
	"Human Grunt",
	"Alient Grunt",
	"Tentacle",
	"Sentry",
	"Turret",
	"Leech",
	"G-Man",
	"Female Assassin",
	"Male Assassin",
	"Snark",
	"Chumtoad",
	"Alien Controller",
	"Ichtyosaur",
	"Gargantua",
	"Baby Gargantua",
	"Big Momma",
	"Osprey",
	"Apache",
	"Nihilanth",
	"H.E.V",
	"Xen Tree",
	"Xen Fungus",
	"Human Grunt",
	"Gonome",
	"Pit Drone",
	"Shock Trooper",
	"Voltigore",
	"Baby Voltigore",
	"Pit Worm",
	"Shock Rifle",
	"Spore Ammo",
	"Heavy Grunt",
	"Robot Grunt"
};

CHashTable<uint32_t, class_info_t> g_HashClassTable(32);
CHashDict<class_info_t> g_HashModelsTable(48);

CClassTable g_ClassTable;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

constexpr static class_info_t LinkClassInfo(eClassID classID, int flags)
{
	return { static_cast<unsigned short>(flags & 0xFFFF), static_cast<unsigned short>(classID & 0xFFFF) };
}

class_info_t GetEntityClassInfo(const char *pszModelName)
{
	auto pClassEntry = g_HashClassTable.Find((uint32_t)pszModelName);

	if (!pClassEntry)
	{
		size_t nModelLength = 0;

		const char *pszModelNameSliced = pszModelName;
		const char *pszModelNameIterator = pszModelName;

		while (*pszModelNameIterator)
		{
			++pszModelNameIterator;
			++nModelLength;

			if (*(pszModelNameIterator - 1) == '/')
			{
				nModelLength = 0;
				pszModelNameSliced = pszModelNameIterator;
			}
		}

		// Result: "model/hlclassic/scientist.mdl" -> "scientist.mdl"

		class_info_t *pClassInfo = g_HashModelsTable.Find(pszModelNameSliced);

		if (!pClassInfo)
		{
			g_HashClassTable.Insert((uint32_t)pszModelName, LinkClassInfo(CLASS_NONE, FL_CLASS_NEUTRAL));
			return { CLASS_NONE, FL_CLASS_NEUTRAL };
		}

		g_HashClassTable.Insert((uint32_t)pszModelName, *pClassInfo);
		return *pClassInfo;
	}

	return *pClassEntry;
}

//-----------------------------------------------------------------------------
// CClassTable constructor
//-----------------------------------------------------------------------------

CClassTable::CClassTable()
{
	g_HashModelsTable.Insert("scientist.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("cleansuit_scientist.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("scientist_rosenberg.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("wheelchair_sci.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_HashModelsTable.Insert("barney.mdl", LinkClassInfo(CLASS_BARNEY, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("otis.mdl", LinkClassInfo(CLASS_OTIS, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_HashModelsTable.Insert("headcrab.mdl", LinkClassInfo(CLASS_HEADCRAB, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("zombie.mdl", LinkClassInfo(CLASS_ZOMBIE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("zombie_soldier.mdl", LinkClassInfo(CLASS_ZOMBIE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("zombie_barney.mdl", LinkClassInfo(CLASS_ZOMBIE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("bullsquid.mdl", LinkClassInfo(CLASS_BULLSQUID, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("houndeye.mdl", LinkClassInfo(CLASS_HOUNDEYE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("barnacle.mdl", LinkClassInfo(CLASS_BARNACLE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("islave.mdl", LinkClassInfo(CLASS_VORTIGAUNT, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("hgrunt.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT, FL_CLASS_ENEMY | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("agrunt.mdl", LinkClassInfo(CLASS_ALIENT_GRUNT, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("tentacle2.mdl", LinkClassInfo(CLASS_TENTACLE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("tentacle3.mdl", LinkClassInfo(CLASS_TENTACLE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("sentry.mdl", LinkClassInfo(CLASS_SENTRY, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("turret.mdl", LinkClassInfo(CLASS_TURRET, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("miniturret.mdl", LinkClassInfo(CLASS_TURRET, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("leech.mdl", LinkClassInfo(CLASS_LEECH, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("gman.mdl", LinkClassInfo(CLASS_GMAN, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("hassassin.mdl", LinkClassInfo(CLASS_FEMALE_ASSASSIN, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("w_squeak.mdl", LinkClassInfo(CLASS_SNARK, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("chubby.mdl", LinkClassInfo(CLASS_CHUMTOAD, FL_CLASS_FRIEND));

	g_HashModelsTable.Insert("controller.mdl", LinkClassInfo(CLASS_ALIEN_CONTROLLER, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("icky.mdl", LinkClassInfo(CLASS_ICHTYOSAUR, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("garg.mdl", LinkClassInfo(CLASS_GARGANTUA, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("babygarg.mdl", LinkClassInfo(CLASS_BABY_GARGANTUA, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("big_mom.mdl", LinkClassInfo(CLASS_BIG_MOMMA, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("osprey.mdl", LinkClassInfo(CLASS_OSPREY, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("apache.mdl", LinkClassInfo(CLASS_APACHE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("blkop_apache.mdl", LinkClassInfo(CLASS_APACHE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("nihilanth.mdl", LinkClassInfo(CLASS_NIHILANTH, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("player.mdl", LinkClassInfo(CLASS_HEV, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("tree.mdl", LinkClassInfo(CLASS_XEN_TREE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus.mdl", LinkClassInfo(CLASS_XEN_FUNGUS, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus(small).mdl", LinkClassInfo(CLASS_XEN_FUNGUS, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus(large).mdl", LinkClassInfo(CLASS_XEN_FUNGUS, FL_CLASS_WORLD_ENTITY));

	g_HashModelsTable.Insert("hgrunt_opforf.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("hgrunt_torchf.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("hgrunt_medicf.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("hgrunt_opfor.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("hgrunt_torch.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("hgrunt_medic.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_HashModelsTable.Insert("massn.mdl", LinkClassInfo(CLASS_MALE_ASSASSIN, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("gonome.mdl", LinkClassInfo(CLASS_GONOME, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("pit_drone.mdl", LinkClassInfo(CLASS_PIT_DRONE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("strooper.mdl", LinkClassInfo(CLASS_SHOCK_TROOPER, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("voltigore.mdl", LinkClassInfo(CLASS_VOLTIGORE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("baby_voltigore.mdl", LinkClassInfo(CLASS_BABY_VOLTIGORE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("pit_worm_up.mdl", LinkClassInfo(CLASS_PIT_WORM, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("w_shock_rifle.mdl", LinkClassInfo(CLASS_SHOCK_RIFLE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("spore_ammo.mdl", LinkClassInfo(CLASS_SPORE_AMMO, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("hwgrunt.mdl", LinkClassInfo(CLASS_HEAVY_GRUNT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("rgrunt.mdl", LinkClassInfo(CLASS_ROBOT_GRUNT, FL_CLASS_ENEMY));
}