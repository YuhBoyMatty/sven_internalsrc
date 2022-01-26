// Table of classes

#include "class_table.h"

#include "../utils/hash_table.h"

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

CHashTable<31, class_info_t> g_classTable;
CHashTableString<48, class_info_t, false> g_modelsTable;

constexpr static class_info_t LinkClassInfo(eClassID classID, int flags)
{
	return { static_cast<unsigned short>(flags & 0xFFFF), static_cast<unsigned short>(classID & 0xFFFF) };
}

class_info_t GetEntityClassInfo(const char *pszModelName)
{
	auto pClassEntry = g_classTable.GetEntry((uint32_t)pszModelName);

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

		auto pModelEntry = g_modelsTable.GetEntry(pszModelNameSliced, nModelLength);

		if (!pModelEntry)
		{
			g_classTable.AddEntry((uint32_t)pszModelName, LinkClassInfo(CLASS_NONE, FL_CLASS_NEUTRAL));
			return { CLASS_NONE, FL_CLASS_NEUTRAL };
		}

		g_classTable.AddEntry((uint32_t)pszModelName, pModelEntry->value);
		return pModelEntry->value;
	}

	return pClassEntry->value;
}

void InitClassTable()
{
	g_modelsTable.AddEntry("scientist.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("cleansuit_scientist.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("scientist_rosenberg.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("wheelchair_sci.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_modelsTable.AddEntry("barney.mdl", LinkClassInfo(CLASS_BARNEY, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("otis.mdl", LinkClassInfo(CLASS_OTIS, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_modelsTable.AddEntry("headcrab.mdl", LinkClassInfo(CLASS_HEADCRAB, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("zombie.mdl", LinkClassInfo(CLASS_ZOMBIE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("zombie_soldier.mdl", LinkClassInfo(CLASS_ZOMBIE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("zombie_barney.mdl", LinkClassInfo(CLASS_ZOMBIE, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("bullsquid.mdl", LinkClassInfo(CLASS_BULLSQUID, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("houndeye.mdl", LinkClassInfo(CLASS_HOUNDEYE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("barnacle.mdl", LinkClassInfo(CLASS_BARNACLE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("islave.mdl", LinkClassInfo(CLASS_VORTIGAUNT, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("hgrunt.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT, FL_CLASS_ENEMY | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("agrunt.mdl", LinkClassInfo(CLASS_ALIENT_GRUNT, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("tentacle2.mdl", LinkClassInfo(CLASS_TENTACLE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("tentacle3.mdl", LinkClassInfo(CLASS_TENTACLE, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("sentry.mdl", LinkClassInfo(CLASS_SENTRY, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("turret.mdl", LinkClassInfo(CLASS_TURRET, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("miniturret.mdl", LinkClassInfo(CLASS_TURRET, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("leech.mdl", LinkClassInfo(CLASS_LEECH, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("gman.mdl", LinkClassInfo(CLASS_GMAN, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("hassassin.mdl", LinkClassInfo(CLASS_FEMALE_ASSASSIN, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("w_squeak.mdl", LinkClassInfo(CLASS_SNARK, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("chubby.mdl", LinkClassInfo(CLASS_CHUMTOAD, FL_CLASS_FRIEND));

	g_modelsTable.AddEntry("controller.mdl", LinkClassInfo(CLASS_ALIEN_CONTROLLER, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("icky.mdl", LinkClassInfo(CLASS_ICHTYOSAUR, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("garg.mdl", LinkClassInfo(CLASS_GARGANTUA, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("babygarg.mdl", LinkClassInfo(CLASS_BABY_GARGANTUA, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("big_mom.mdl", LinkClassInfo(CLASS_BIG_MOMMA, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("osprey.mdl", LinkClassInfo(CLASS_OSPREY, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("apache.mdl", LinkClassInfo(CLASS_APACHE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("blkop_apache.mdl", LinkClassInfo(CLASS_APACHE, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("nihilanth.mdl", LinkClassInfo(CLASS_NIHILANTH, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("player.mdl", LinkClassInfo(CLASS_HEV, FL_CLASS_WORLD_ENTITY));
	g_modelsTable.AddEntry("tree.mdl", LinkClassInfo(CLASS_XEN_TREE, FL_CLASS_WORLD_ENTITY));
	g_modelsTable.AddEntry("fungus.mdl", LinkClassInfo(CLASS_XEN_FUNGUS, FL_CLASS_WORLD_ENTITY));
	g_modelsTable.AddEntry("fungus(small).mdl", LinkClassInfo(CLASS_XEN_FUNGUS, FL_CLASS_WORLD_ENTITY));
	g_modelsTable.AddEntry("fungus(large).mdl", LinkClassInfo(CLASS_XEN_FUNGUS, FL_CLASS_WORLD_ENTITY));

	g_modelsTable.AddEntry("hgrunt_opforf.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("hgrunt_torchf.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("hgrunt_medicf.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("hgrunt_opfor.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("hgrunt_torch.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_modelsTable.AddEntry("hgrunt_medic.mdl", LinkClassInfo(CLASS_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_modelsTable.AddEntry("massn.mdl", LinkClassInfo(CLASS_MALE_ASSASSIN, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("gonome.mdl", LinkClassInfo(CLASS_GONOME, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("pit_drone.mdl", LinkClassInfo(CLASS_PIT_DRONE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("strooper.mdl", LinkClassInfo(CLASS_SHOCK_TROOPER, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("voltigore.mdl", LinkClassInfo(CLASS_VOLTIGORE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("baby_voltigore.mdl", LinkClassInfo(CLASS_BABY_VOLTIGORE, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("pit_worm_up.mdl", LinkClassInfo(CLASS_PIT_WORM, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("w_shock_rifle.mdl", LinkClassInfo(CLASS_SHOCK_RIFLE, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("spore_ammo.mdl", LinkClassInfo(CLASS_SPORE_AMMO, FL_CLASS_ENEMY));

	g_modelsTable.AddEntry("hwgrunt.mdl", LinkClassInfo(CLASS_HEAVY_GRUNT, FL_CLASS_ENEMY));
	g_modelsTable.AddEntry("rgrunt.mdl", LinkClassInfo(CLASS_ROBOT_GRUNT, FL_CLASS_ENEMY));
}