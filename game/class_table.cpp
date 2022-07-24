// Table of classes

#include "class_table.h"

#include <data_struct/hashtable.h>
#include <data_struct/hashdict.h>

#define LINK_CLASS_INFO(classID, flags) { static_cast<unsigned short>(flags & 0xFFFF), static_cast<unsigned short>(classID & 0xFFFF) }

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

const char *g_szClassName[] =
{
	// position of class name must match with the class id, need to improve it..
	"Unknown",

	"Player",

	"Respawn Point",

	// NPCs
	"Gordon Freeman",
	"Scientist",
	"Barney",
	"Otis",
	"Headcrab",
	"Baby Headcrab",
	"Zombie",
	"Zombie Soldier",
	"Bullsquid",
	"Houndeye",
	"Barnacle",
	"Vortigaunt",
	"Human Grunt",
	"Sniper",
	"Alient Grunt",
	"Tentacle",
	"Sentry",
	"Turret",
	"Leech",
	"G-Man",
	"Female Assassin",
	"Male Assassin",
	"Male Sniper Assassin",
	"Snark",
	"Chumtoad",
	"Alien Controller",
	"Ichtyosaur",
	"Gargantua",
	"Baby Gargantua",
	"Big Momma",
	"Osprey",
	"Destroyed Osprey",
	"Apache",
	"Nihilanth",
	"Boid",
	"H.E.V.",
	"Spore Ammo",
	"Human Grunt",
	"Torch | Human Grunt",
	"Medic | Human Grunt",
	"Gonome",
	"Pit Drone",
	"Shock Trooper",
	"Voltigore",
	"Baby Voltigore",
	"Pit Worm",
	"Geneworm",
	"Shock Rifle",
	"Mortar",
	"Heavy Grunt",
	"Robot Grunt",
	"Barnabus",
	"Skeleton",
	"Piranha",

	// Wouldn't add them tbh
	"Stukabat",
	"Kingpin",
	"Xen Commander",
	"Special Forces Grunt",
	"Barniel",
	"Archer",
	"Panthereye",
	"Fiona",
	"Twitcher",
	"Spitter",
	"Handcrab",
	"Ghost",
	"Screamer",
	"Devourer",
	"Wheelchair",
	"Face",
	"Hellhound",
	"Addiction",
	"Shark",

	// Items
	"H.E.V.",
	"Medkit",
	"Suit Battery",
	"Glock Ammo",
	".357 Ammo",
	"Shotgun Ammo",
	"UZI Ammo",
	"AR Ammo",
	"MP5 Ammo",
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
	"UZI",
	"9mm AR",
	"M16",
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
	"Long Jump",
	"Barney Vest",
	"Barney Helmet",
	"Suit",
	"Spore",
	"Crossbow Bolt",
	"RPG Rocket",
	"HVR Rocket",
	"Mortar Shell"
};

CHashTable<uint32_t, class_info_t> g_HashClassTable(511);
CHashDict<class_info_t, true, false> g_HashModelsTable(511);

CClassTable g_ClassTable;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

class_info_t GetEntityClassInfo(const char *pszModelName)
{
	auto pClassEntry = g_HashClassTable.Find((uint32_t)pszModelName);

	if ( !pClassEntry )
	{
		const char *pszSlashLastOccur = strrchr(pszModelName, '/');
		const char *pszModelNameSliced = pszModelName;

		if ( pszSlashLastOccur )
			pszModelNameSliced = pszSlashLastOccur + 1;

		// Result: "model/hlclassic/scientist.mdl" -> "scientist.mdl"

		class_info_t *pClassInfo = g_HashModelsTable.Find(pszModelNameSliced);

		if ( !pClassInfo )
		{
			g_HashClassTable.Insert((uint32_t)pszModelName, LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_NEUTRAL));
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
	g_HashModelsTable.Insert("lambda.mdl", LINK_CLASS_INFO(CLASS_OBJECT_CP, FL_CLASS_NEUTRAL));

	// NPCs
	g_HashModelsTable.Insert("gordon_scientist.mdl", LINK_CLASS_INFO(CLASS_NPC_GORDON_FREEMAN, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("scientist.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("scientist2.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("cleansuit_scientist.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("scientist_rosenberg.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("wheelchair_sci.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("civ_scientist.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("civ_paper_scientist.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("console_civ_scientist.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("sc2sci.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("scigun.mdl", LINK_CLASS_INFO(CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND));

	g_HashModelsTable.Insert("hgruntf.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("sc2grunt.mdl", LINK_CLASS_INFO(CLASS_NPC_ALIEN_GRUNT, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("agruntf.mdl", LINK_CLASS_INFO(CLASS_NPC_ALIEN_GRUNT, FL_CLASS_FRIEND));

	g_HashModelsTable.Insert("barney.mdl", LINK_CLASS_INFO(CLASS_NPC_BARNEY, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("intro_barney.mdl", LINK_CLASS_INFO(CLASS_NPC_BARNEY, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("otis.mdl", LINK_CLASS_INFO(CLASS_NPC_OTIS, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("intro_otis.mdl", LINK_CLASS_INFO(CLASS_NPC_OTIS, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("hungerbarney.mdl", LINK_CLASS_INFO(CLASS_NPC_OTIS, FL_CLASS_ENEMY | FL_CLASS_CORPSE));

	g_HashModelsTable.Insert("headcrab.mdl", LINK_CLASS_INFO(CLASS_NPC_HEADCRAB, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("baby_headcrab.mdl", LINK_CLASS_INFO(CLASS_NPC_BABY_HEADCRAB, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("hungercrab.mdl", LINK_CLASS_INFO(CLASS_NPC_BABY_HEADCRAB, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("otisf.mdl", LINK_CLASS_INFO(CLASS_NPC_OTIS, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("hungerotis.mdl", LINK_CLASS_INFO(CLASS_NPC_OTIS, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("zombie.mdl", LINK_CLASS_INFO(CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("zombie_soldier.mdl", LINK_CLASS_INFO(CLASS_NPC_ZOMBIE_SOLDIER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("zombie_barney.mdl", LINK_CLASS_INFO(CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("hungerzombie.mdl", LINK_CLASS_INFO(CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("bullsquid.mdl", LINK_CLASS_INFO(CLASS_NPC_BULLSQUID, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("houndeye.mdl", LINK_CLASS_INFO(CLASS_NPC_HOUNDEYE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("barnacle.mdl", LINK_CLASS_INFO(CLASS_NPC_BARNACLE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("islave.mdl", LINK_CLASS_INFO(CLASS_NPC_VORTIGAUNT, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("hgrunt.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT, FL_CLASS_ENEMY | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("hgrunt_sniper.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_SNIPER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("agrunt.mdl", LINK_CLASS_INFO(CLASS_NPC_ALIEN_GRUNT, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("tentacle2.mdl", LINK_CLASS_INFO(CLASS_NPC_TENTACLE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("tentacle3.mdl", LINK_CLASS_INFO(CLASS_NPC_TENTACLE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("sentry.mdl", LINK_CLASS_INFO(CLASS_NPC_SENTRY, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("turret.mdl", LINK_CLASS_INFO(CLASS_NPC_TURRET, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("miniturret.mdl", LINK_CLASS_INFO(CLASS_NPC_TURRET, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("leech.mdl", LINK_CLASS_INFO(CLASS_NPC_LEECH, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("gman.mdl", LINK_CLASS_INFO(CLASS_NPC_GMAN, FL_CLASS_NEUTRAL));

	g_HashModelsTable.Insert("hassassin.mdl", LINK_CLASS_INFO(CLASS_NPC_FEMALE_ASSASSIN, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("w_squeak.mdl", LINK_CLASS_INFO(CLASS_NPC_SNARK, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("chubby.mdl", LINK_CLASS_INFO(CLASS_NPC_CHUMTOAD, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("chumtoad.mdl", LINK_CLASS_INFO(CLASS_NPC_CHUMTOAD, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("piranha.mdl", LINK_CLASS_INFO(CLASS_NPC_PIRANHA, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("zombierat.mdl", LINK_CLASS_INFO(CLASS_NPC_SNARK, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("controller.mdl", LINK_CLASS_INFO(CLASS_NPC_ALIEN_CONTROLLER, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("icky.mdl", LINK_CLASS_INFO(CLASS_NPC_ICHTYOSAUR, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("garg.mdl", LINK_CLASS_INFO(CLASS_NPC_GARGANTUA, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("babygarg.mdl", LINK_CLASS_INFO(CLASS_NPC_BABY_GARGANTUA, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("big_mom.mdl", LINK_CLASS_INFO(CLASS_NPC_BIG_MOMMA, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("osprey.mdl", LINK_CLASS_INFO(CLASS_NPC_OSPREY, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("dead_osprey.mdl", LINK_CLASS_INFO(CLASS_NPC_DESTROYED_OSPREY, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("apache.mdl", LINK_CLASS_INFO(CLASS_NPC_APACHE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("blkop_apache.mdl", LINK_CLASS_INFO(CLASS_NPC_APACHE, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("nihilanth.mdl", LINK_CLASS_INFO(CLASS_NPC_NIHILANTH, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("aflock.mdl", LINK_CLASS_INFO(CLASS_NPC_BOID, FL_CLASS_NEUTRAL));

	g_HashModelsTable.Insert("player.mdl", LINK_CLASS_INFO(CLASS_NPC_HEV, FL_CLASS_WORLD_ENTITY));

	g_HashModelsTable.Insert("spore_ammo.mdl", LINK_CLASS_INFO(CLASS_NPC_SPORE_AMMO, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("hgrunt_opforf.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("hgrunt_opfor.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("hgrunt_torchf.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_OPFOR_TORCH, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("hgrunt_torch.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_OPFOR_TORCH, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("hgrunt_medicf.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_OPFOR_MEDIC, FL_CLASS_FRIEND | FL_CLASS_CORPSE));
	g_HashModelsTable.Insert("hgrunt_medic.mdl", LINK_CLASS_INFO(CLASS_NPC_HUMAN_GRUNT_OPFOR_MEDIC, FL_CLASS_FRIEND | FL_CLASS_CORPSE));

	g_HashModelsTable.Insert("massn.mdl", LINK_CLASS_INFO(CLASS_NPC_MALE_ASSASSIN, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("massnf.mdl", LINK_CLASS_INFO(CLASS_NPC_MALE_SNIPER_ASSASSIN, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("gonome.mdl", LINK_CLASS_INFO(CLASS_NPC_GONOME, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("pit_drone.mdl", LINK_CLASS_INFO(CLASS_NPC_PIT_DRONE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("strooper.mdl", LINK_CLASS_INFO(CLASS_NPC_SHOCK_TROOPER, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("voltigore.mdl", LINK_CLASS_INFO(CLASS_NPC_VOLTIGORE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("baby_voltigore.mdl", LINK_CLASS_INFO(CLASS_NPC_BABY_VOLTIGORE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("pit_worm_up.mdl", LINK_CLASS_INFO(CLASS_NPC_PIT_WORM, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("geneworm.mdl", LINK_CLASS_INFO(CLASS_NPC_GENEWORM, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("w_shock_rifle.mdl", LINK_CLASS_INFO(CLASS_NPC_SHOCK_RIFLE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("mortar.mdl", LINK_CLASS_INFO(CLASS_NPC_MORTAR, FL_CLASS_ENEMY));

	g_HashModelsTable.Insert("hwgrunt.mdl", LINK_CLASS_INFO(CLASS_NPC_HEAVY_GRUNT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("rgrunt.mdl", LINK_CLASS_INFO(CLASS_NPC_ROBOT_GRUNT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("barnabus.mdl", LINK_CLASS_INFO(CLASS_NPC_BARNABUS, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("skeleton.mdl", LINK_CLASS_INFO(CLASS_NPC_SKELETON, FL_CLASS_ENEMY));

	// Wouldn't add them tbh
	g_HashModelsTable.Insert("stukabat.mdl", LINK_CLASS_INFO(CLASS_NPC_STUKABAT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("kingpin.mdl", LINK_CLASS_INFO(CLASS_NPC_KINGPIN, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("tor.mdl", LINK_CLASS_INFO(CLASS_NPC_TOR, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("spforce.mdl", LINK_CLASS_INFO(CLASS_NPC_SPECFOR_GRUNT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("barniel.mdl", LINK_CLASS_INFO(CLASS_NPC_BARNIEL, FL_CLASS_FRIEND));
	g_HashModelsTable.Insert("archer.mdl", LINK_CLASS_INFO(CLASS_NPC_ARCHER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("sslave.mdl", LINK_CLASS_INFO(CLASS_NPC_VORTIGAUNT, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("panther.mdl", LINK_CLASS_INFO(CLASS_NPC_PANTHEREYE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("hassassinf.mdl", LINK_CLASS_INFO(CLASS_NPC_FEMALE_ASSASSIN, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("fiona.mdl", LINK_CLASS_INFO(CLASS_NPC_FIONA, FL_CLASS_NEUTRAL));
	g_HashModelsTable.Insert("twitcher.mdl", LINK_CLASS_INFO(CLASS_NPC_TWITCHER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("twitcher2.mdl", LINK_CLASS_INFO(CLASS_NPC_TWITCHER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("twitcher3.mdl", LINK_CLASS_INFO(CLASS_NPC_TWITCHER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("twitcher4.mdl", LINK_CLASS_INFO(CLASS_NPC_TWITCHER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("spitter.mdl", LINK_CLASS_INFO(CLASS_NPC_SPITTER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("handcrab.mdl", LINK_CLASS_INFO(CLASS_NPC_HANDCRAB, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("ghost.mdl", LINK_CLASS_INFO(CLASS_NPC_GHOST, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("screamer.mdl", LINK_CLASS_INFO(CLASS_NPC_SCREAMER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("devourer.mdl", LINK_CLASS_INFO(CLASS_NPC_DEVOURER, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("wheelchair_new.mdl", LINK_CLASS_INFO(CLASS_NPC_WHEELCHAIR, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("face_new.mdl", LINK_CLASS_INFO(CLASS_NPC_FACE, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("hellhound.mdl", LINK_CLASS_INFO(CLASS_NPC_HELLHOUND, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("davidbad_cutscene.mdl", LINK_CLASS_INFO(CLASS_NPC_ADDICTION, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("davidbad_noaxe.mdl", LINK_CLASS_INFO(CLASS_NPC_ADDICTION, FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("shark.mdl", LINK_CLASS_INFO(CLASS_NPC_SHARK, FL_CLASS_ENEMY));

	// Items
	g_HashModelsTable.Insert("w_suit.mdl", LINK_CLASS_INFO(CLASS_ITEM_HEV, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_medkit.mdl", LINK_CLASS_INFO(CLASS_ITEM_MEDKIT, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_battery.mdl", LINK_CLASS_INFO(CLASS_ITEM_BATTERY, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmclip.mdl", LINK_CLASS_INFO(CLASS_ITEM_GLOCK_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_357ammobox.mdl", LINK_CLASS_INFO(CLASS_ITEM_PYTHON_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_shotbox.mdl", LINK_CLASS_INFO(CLASS_ITEM_SHOTGUN_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_uzi_clip.mdl", LINK_CLASS_INFO(CLASS_ITEM_UZI_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmarclip.mdl", LINK_CLASS_INFO(CLASS_ITEM_MP5_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_mp5_clip.mdl", LINK_CLASS_INFO(CLASS_ITEM_MP5_AMMO2, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_crossbow_clip.mdl", LINK_CLASS_INFO(CLASS_ITEM_CROSSBOW_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_gaussammo.mdl", LINK_CLASS_INFO(CLASS_ITEM_GAUSS_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_rpgammo.mdl", LINK_CLASS_INFO(CLASS_ITEM_RPG_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_m40a1clip.mdl", LINK_CLASS_INFO(CLASS_ITEM_SNIPER_RIFLE_AMMO, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_saw_clip.mdl", LINK_CLASS_INFO(CLASS_ITEM_MACHINEGUN_AMMO, FL_CLASS_ITEM));	
	g_HashModelsTable.Insert("w_crowbar.mdl", LINK_CLASS_INFO(CLASS_ITEM_CROWBAR, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_pipe_wrench.mdl", LINK_CLASS_INFO(CLASS_ITEM_WRENCH, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_knife.mdl", LINK_CLASS_INFO(CLASS_ITEM_KNIFE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_bgrap.mdl", LINK_CLASS_INFO(CLASS_ITEM_BARNACLE_GRAPPLE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmhandgun.mdl", LINK_CLASS_INFO(CLASS_ITEM_GLOCK, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_357.mdl", LINK_CLASS_INFO(CLASS_ITEM_PYTHON, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_desert_eagle.mdl", LINK_CLASS_INFO(CLASS_ITEM_DEAGLE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_shotgun.mdl", LINK_CLASS_INFO(CLASS_ITEM_SHOTGUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_uzi.mdl", LINK_CLASS_INFO(CLASS_ITEM_UZI, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_9mmAR.mdl", LINK_CLASS_INFO(CLASS_ITEM_MP5, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_m16.mdl", LINK_CLASS_INFO(CLASS_ITEM_M16, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_crossbow.mdl", LINK_CLASS_INFO(CLASS_ITEM_CROSSBOW, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_gauss.mdl", LINK_CLASS_INFO(CLASS_ITEM_GAUSS, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_egon.mdl", LINK_CLASS_INFO(CLASS_ITEM_EGON, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_rpg.mdl", LINK_CLASS_INFO(CLASS_ITEM_RPG, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_hgun.mdl", LINK_CLASS_INFO(CLASS_ITEM_HORNET_GUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_m40a1.mdl", LINK_CLASS_INFO(CLASS_ITEM_SNIPER_RIFLE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_saw.mdl", LINK_CLASS_INFO(CLASS_ITEM_MACHINEGUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_spore_launcher.mdl", LINK_CLASS_INFO(CLASS_ITEM_SPORE_LAUNCHER, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_displacer.mdl", LINK_CLASS_INFO(CLASS_ITEM_DISPLACER, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_minigun.mdl", LINK_CLASS_INFO(CLASS_ITEM_MINIGUN, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_sqknest.mdl", LINK_CLASS_INFO(CLASS_ITEM_SNARK_NEST, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_grenade.mdl", LINK_CLASS_INFO(CLASS_ITEM_GRENADE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_satchel.mdl", LINK_CLASS_INFO(CLASS_ITEM_SATCHEL, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_argrenade.mdl", LINK_CLASS_INFO(CLASS_ITEM_ARGRENADE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_tripmine.mdl", LINK_CLASS_INFO(CLASS_ITEM_TRIPMINE, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_weaponbox.mdl", LINK_CLASS_INFO(CLASS_ITEM_WEAPON_BOX, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("w_longjump.mdl", LINK_CLASS_INFO(CLASS_ITEM_LONGJUMP, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("barney_vset.mdl", LINK_CLASS_INFO(CLASS_ITEM_BARNEY_VEST, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("barney_helmet.mdl", LINK_CLASS_INFO(CLASS_ITEM_BARNEY_HELMET, FL_CLASS_ITEM));
	g_HashModelsTable.Insert("suit2.mdl", LINK_CLASS_INFO(CLASS_ITEM_SUIT, FL_CLASS_ITEM));

	g_HashModelsTable.Insert("spore.mdl", LINK_CLASS_INFO(CLASS_ITEM_SPORE, FL_CLASS_ITEM | FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("grenade.mdl", LINK_CLASS_INFO(CLASS_ITEM_GRENADE, FL_CLASS_ITEM | FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("crossbow_bolt.mdl", LINK_CLASS_INFO(CLASS_ITEM_CROSSBOW_BOLT, FL_CLASS_ITEM | FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("rpgrocket.mdl", LINK_CLASS_INFO(CLASS_ITEM_RPG_ROCKET, FL_CLASS_ITEM | FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("HVR.mdl", LINK_CLASS_INFO(CLASS_ITEM_HVR_ROCKET, FL_CLASS_ITEM | FL_CLASS_ENEMY));
	g_HashModelsTable.Insert("mortarshell.mdl", LINK_CLASS_INFO(CLASS_ITEM_MORTAR_SHELL, FL_CLASS_ITEM | FL_CLASS_ENEMY));

	// World entites that have at least one hitbox
	g_HashModelsTable.Insert("hornet.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("tree.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("protozoa.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus(small).mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("fungus(large).mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("hair.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("rengine.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("sat_globe.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("w_pmedkit.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("roach.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("forklift.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("loader.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("construction.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("dead_barney.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("dead_scientist.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("ball.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("can.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("w_crossbow_clip.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("pit_drone_spike.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("crashed_osprey.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("baby_strooper.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("filecabinet.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("light.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("pit_drone_gibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("vgibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("med_crategibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("tool_box.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("nuke_case.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("strooper_gibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("concrete_gibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("catwalkgibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("garbagegibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("arc_xer_tree1.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("chair.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("tool_box_sm.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("tech_crategibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("glassgibsw.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("woodgibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("w_flashlight.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("rip.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("base.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("metalplategibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("hgibs.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("shell.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("stretcher.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY));
	g_HashModelsTable.Insert("holo.mdl", LINK_CLASS_INFO(CLASS_NONE, FL_CLASS_WORLD_ENTITY)); // holo-.. Horo?

	// w_chainammo.mdl
}