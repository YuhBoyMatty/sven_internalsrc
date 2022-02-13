// Table of classes

#include "class_table.h"

#include "../data_struct/hashdict.h"
#include "../data_struct/hashtable.h"

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

const char *g_szClassName[] =
{
	// position of class name must match to the class id
	"Unknown",

	// NPCs
	"Player",
	"Scientist",
	"Barney",
	"Otis",
	"Headcrab",
	"Baby Headcrab",
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
	"Spore Ammo",
	"Spore",
	"Human Grunt",
	"Gonome",
	"Pit Drone",
	"Shock Trooper",
	"Voltigore",
	"Baby Voltigore",
	"Pit Worm",
	"Shock Rifle",
	"Heavy Grunt",
	"Robot Grunt",

	// Items
	"Medkit",
	"Suit Battery",
	"Glock Ammo",
	".357 Ammo",
	"Shotgun Ammo",
	"AR Ammo",
	"Crossbow Ammo",
	"Gauss Ammo",
	"RPG Ammo",
	"Sniper Rifle Ammo",
	"Machine Gun Ammo",
	"Crowbar",
	"Wrench",
	"Knife",
	"Barnacle Grapple",
	"Glock",
	".357",
	"Deagle",
	"Shotgun",
	"9mm AR",
	"Crossbow",
	"Gauss",
	"Gluon Gun",
	"RPG",
	"Hornet Gun",
	"Sniper Rifle",
	"Machine Gun",
	"Spore Launcher",
	"Displacer",
	"Minigun",
	"Snark Nest",
	"Grenade",
	"Satchel Charge",
	"AR Grenade",
	"Trip Mine",
	"Weapon Box",
	"Long Jump"
};

CHashTable<uint32_t, class_info_t> g_HashClassTable(127);
CHashDict<class_info_t> g_HashModelsTable(127);

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
		const char *pszSlashLastOccur = strrchr(pszModelName, '/');
		const char *pszModelNameSliced = pszModelName;

		if (pszSlashLastOccur)
			pszModelNameSliced = pszSlashLastOccur + 1;

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
// Initialize models on DLL load
//-----------------------------------------------------------------------------

CClassTable::CClassTable()
{
	// NPCs
	g_HashModelsTable.Insert("scientist.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("scientist2.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("cleansuit_scientist.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("scientist_rosenberg.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("wheelchair_sci.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("scigun.mdl", LinkClassInfo(CLASS_SCIENTIST, FL_CLASS_FRIEND));

	g_HashModelsTable.Insert("barney.mdl", LinkClassInfo(CLASS_BARNEY, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));
	g_HashModelsTable.Insert("otis.mdl", LinkClassInfo(CLASS_OTIS, FL_CLASS_FRIEND | FL_CLASS_DEAD_BODY));

	g_HashModelsTable.Insert("headcrab.mdl", LinkClassInfo(CLASS_HEADCRAB, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("baby_headcrab.mdl", LinkClassInfo(CLASS_BABY_HEADCRAB, FL_CLASS_ENEMY));

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

	g_HashModelsTable.Insert("spore_ammo.mdl", LinkClassInfo(CLASS_SPORE_AMMO, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("spore.mdl", LinkClassInfo(CLASS_SPORE, FL_CLASS_ENEMY));

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

	g_HashModelsTable.Insert("hwgrunt.mdl", LinkClassInfo(CLASS_HEAVY_GRUNT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("rgrunt.mdl", LinkClassInfo(CLASS_ROBOT_GRUNT, FL_CLASS_ENEMY));

	// Items
	g_HashModelsTable.Insert("w_medkit.mdl", LinkClassInfo(CLASS_ITEM_MEDKIT, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_battery.mdl", LinkClassInfo(CLASS_ITEM_BATTERY, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmclip.mdl", LinkClassInfo(CLASS_ITEM_GLOCK_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_357ammobox.mdl", LinkClassInfo(CLASS_ITEM_PYTHON_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_shotbox.mdl", LinkClassInfo(CLASS_ITEM_SHOTGUN_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmarclip.mdl", LinkClassInfo(CLASS_ITEM_MP5_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_crossbow_clip.mdl", LinkClassInfo(CLASS_ITEM_CROSSBOW_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_gaussammo.mdl", LinkClassInfo(CLASS_ITEM_GAUSS_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_rpgammo.mdl", LinkClassInfo(CLASS_ITEM_RPG_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_m40a1clip.mdl", LinkClassInfo(CLASS_ITEM_SNIPER_RIFLE_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_saw_clip.mdl", LinkClassInfo(CLASS_ITEM_MACHINEGUN_AMMO, FL_CLASS_ITEM));	
	g_HashModelsTable.Insert("w_crowbar.mdl", LinkClassInfo(CLASS_ITEM_CROWBAR, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_pipe_wrench.mdl", LinkClassInfo(CLASS_ITEM_WRENCH, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_knife.mdl", LinkClassInfo(CLASS_ITEM_KNIFE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_bgrap.mdl", LinkClassInfo(CLASS_ITEM_BARNACLE_GRAPPLE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmhandgun.mdl", LinkClassInfo(CLASS_ITEM_GLOCK, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_357.mdl", LinkClassInfo(CLASS_ITEM_PYTHON, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_desert_eagle.mdl", LinkClassInfo(CLASS_ITEM_DEAGLE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_shotgun.mdl", LinkClassInfo(CLASS_ITEM_SHOTGUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmAR.mdl", LinkClassInfo(CLASS_ITEM_MP5, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_crossbow.mdl", LinkClassInfo(CLASS_ITEM_CROSSBOW, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_gauss.mdl", LinkClassInfo(CLASS_ITEM_GAUSS, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_egon.mdl", LinkClassInfo(CLASS_ITEM_EGON, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_rpg.mdl", LinkClassInfo(CLASS_ITEM_RPG, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_hgun.mdl", LinkClassInfo(CLASS_ITEM_HORNET_GUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_m40a1.mdl", LinkClassInfo(CLASS_ITEM_SNIPER_RIFLE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_saw.mdl", LinkClassInfo(CLASS_ITEM_MACHINEGUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_spore_launcher.mdl", LinkClassInfo(CLASS_ITEM_SPORE_LAUNCHER, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_displacer.mdl", LinkClassInfo(CLASS_ITEM_DISPLACER, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_minigun.mdl", LinkClassInfo(CLASS_ITEM_MINIGUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_sqknest.mdl", LinkClassInfo(CLASS_ITEM_SNARK_NEST, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_grenade.mdl", LinkClassInfo(CLASS_ITEM_GRENADE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_satchel.mdl", LinkClassInfo(CLASS_ITEM_SATCHEL, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_argrenade.mdl", LinkClassInfo(CLASS_ITEM_ARGRENADE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_tripmine.mdl", LinkClassInfo(CLASS_ITEM_TRIPMINE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_weaponbox.mdl", LinkClassInfo(CLASS_ITEM_WEAPON_BOX, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_longjump.mdl", LinkClassInfo(CLASS_ITEM_LONGJUMP, FL_CLASS_ITEM));

	// World entites that have at least one hitbox
	g_HashModelsTable.Insert("tree.mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus.mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus(small).mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus(large).mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("rengine.mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("sat_globe.mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("w_pmedkit.mdl", LinkClassInfo(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
}