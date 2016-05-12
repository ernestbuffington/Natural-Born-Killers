///////////////////////////////////////////////////////////////////////	
//
//  kooglebot.h - Main header file for KOOGLEBOT
// 
///////////////////////////////////////////////////////////////////////
#ifndef _KOOGLEBOT_H
#define _KOOGLEBOT_H

// Only 100 allowed for now (probably never be enough edicts for 'em
#define MAX_BOTS 100

#define TEAM_1 1
#define TEAM_2 2

// Platform states
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

// Maximum nodes
#define MAX_BOTNODES 1000

// Link types
#define INVALID -1

// Node types
#define BOTNODE_MOVE 0
#define BOTNODE_LADDER 1
#define BOTNODE_PLATFORM 2
#define BOTNODE_TELEPORTER 3
#define BOTNODE_ITEM 4
#define BOTNODE_WATER 5
#define BOTNODE_CROUCH 6
#define BOTNODE_JUMP 7
#define BOTNODE_ALL 99 // For selecting all nodes

// Density setting for nodes
#define BOTNODE_DENSITY 128

// Bot state types
#define BOTSTATE_STAND 0
#define BOTSTATE_MOVE 1
#define BOTSTATE_ATTACK 2
#define BOTSTATE_WANDER 3
#define BOTSTATE_FLEE 4

#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MOVE_FORWARD 2
#define MOVE_BACK 3

// KingPin Item defines 
#define ITEMLIST_NULL				0
#define ITEMLIST_ARMORHELMET		1
#define ITEMLIST_ARMORJACKET		2
#define ITEMLIST_ARMORLEGS			3
#define ITEMLIST_ARMORHELMETHEAVY	4
#define ITEMLIST_ARMORJACKETHEAVY	5
#define ITEMLIST_ARMORLEGSHEAVY		6
#define ITEMLIST_BLACKJACK          7
#define ITEMLIST_CROWBAR			8
#define ITEMLIST_PISTOL				9
#define ITEMLIST_SPISTOL			10
#define ITEMLIST_SHOTGUN2			11
#define ITEMLIST_TOMMYGUN			12
#define ITEMLIST_HEAVYMACHINEGUN	13
#define ITEMLIST_GRENADELAUNCHER	14
#define ITEMLIST_BAZOOKA			15
#define ITEMLIST_FLAMETHROWER		16
#define ITEMLIST_SHOTGUN_E			17
#define ITEMLIST_HEAVYMACHINEGUN_E	18
#define ITEMLIST_BAZOOKA_E			19
#define ITEMLIST_FLAMETHROWER_E		20
#define ITEMLIST_GRENADELAUNCHER_E	21
#define ITEMLIST_PISTOL_E			22
#define ITEMLIST_TOMMYGUN_E			23
#define ITEMLIST_GRENADES			24
#define ITEMLIST_SHELLS				25
#define ITEMLIST_BULLETS			26
#define ITEMLIST_ROCKETS			27
#define ITEMLIST_AMMO308			28
#define ITEMLIST_CYLINDER			29
#define ITEMLIST_FLAMETANK			30
#define ITEMLIST_COIL				31
#define ITEMLIST_LIZZYHEAD			32
#define ITEMLIST_CASHROLL			33
#define ITEMLIST_CASHBAGLARGE		34
#define ITEMLIST_CASHBAGSMALL		35
#define ITEMLIST_BATTERY			36
#define ITEMLIST_JETPACK			37
#define ITEMLIST_HEALTH_SMALL		38
#define ITEMLIST_HEALTH_LARGE		39
#define ITEMLIST_FLASHLIGHT			40
#define ITEMLIST_WATCH				41
#define ITEMLIST_WHISKEY			42
#define ITEMLIST_PACK				43
#define ITEMLIST_ADRENALINE			44
#define ITEMLIST_KEYFUSE			45
#define ITEMLIST_SAFEDOCS			46
#define ITEMLIST_VALVE				47
#define ITEMLIST_OILCAN				48
#define ITEMLIST_KEY1				49
#define ITEMLIST_KEY2				50
#define ITEMLIST_KEY3				51
#define ITEMLIST_KEY4				52
#define ITEMLIST_KEY5				53
#define ITEMLIST_KEY6				54
#define ITEMLIST_KEY7				55
#define ITEMLIST_KEY8				56
#define ITEMLIST_KEY9				57
#define ITEMLIST_KEY10				58
#define ITEMLIST_PISTOLMODS			59
#define ITEMLIST_BOT				60
#define ITEMLIST_PLAYER				61

// Item defines (got this list from FREDZ) Thanks buddy!
#define ITEMLIST_NULLINDEX			62

#define ITEMLIST_BODYARMOR			63
#define ITEMLIST_COMBATARMOR		64
#define ITEMLIST_JACKETARMOR		65
#define ITEMLIST_ARMORSHARD			66
#define ITEMLIST_POWERSCREEN		67
#define ITEMLIST_POWERSHIELD		68
#define ITEMLIST_GRAPPLE            69
#define ITEMLIST_BLASTER			70
#define ITEMLIST_SHOTGUN			71
#define ITEMLIST_SUPERSHOTGUN		72
#define ITEMLIST_MACHINEGUN			73
#define ITEMLIST_CHAINGUN			74
#define ITEMLIST_HYPERBLASTER		75
#define ITEMLIST_RAILGUN			76
#define ITEMLIST_BFG10K				77
#define ITEMLIST_CELLS				78
#define ITEMLIST_SLUGS				79
#define ITEMLIST_QUADDAMAGE			80
#define ITEMLIST_INVULNERABILITY	81
#define ITEMLIST_SILENCER			82
#define ITEMLIST_REBREATHER			83
#define ITEMLIST_ENVIRONMENTSUIT	84
#define ITEMLIST_ANCIENTHEAD		85
#define ITEMLIST_ADRENALINE2		86
#define ITEMLIST_BANDOLIER			87
#define ITEMLIST_AMMOPACK			88
#define ITEMLIST_DATACD				89
#define ITEMLIST_POWERCUBE			90
#define ITEMLIST_PYRAMIDKEY			91
#define ITEMLIST_DATASPINNER		92
#define ITEMLIST_SECURITYPASS		93
#define ITEMLIST_BLUEKEY			94
#define ITEMLIST_REDKEY				95
#define ITEMLIST_COMMANDERSHEAD		96
#define ITEMLIST_AIRSTRIKEMARKER	97
#define ITEMLIST_HEALTH				98
#define ITEMLIST_HEALTH_MEGA        99
#define ITEMLIST_FLAG1              100
#define ITEMLIST_FLAG2              101
#define ITEMLIST_RESISTANCETECH     102
#define ITEMLIST_STRENGTHTECH       103
#define ITEMLIST_HASTETECH          104
#define ITEMLIST_REGENERATIONTECH   105

// Node structure
typedef struct botnode_s
{
	vec3_t origin; // Using Id's representation
	int type;      // type of node
} botnode_t;

typedef struct item_table_s
{
	int item;
	float weight;
	edict_t *ent;
	int node;
} item_table_t;

//extern int num_players;
extern edict_t *players[MAX_CLIENTS];		// pointers to all players in the game

											// extern decs
extern botnode_t nodes[MAX_BOTNODES];
extern item_table_t item_table[MAX_EDICTS];
extern qboolean debug_mode;
extern int numnodes;
extern int num_items;

// id Function Protos Natural Born Killers need
void LookAtKiller(edict_t *self, edict_t *inflictor, edict_t *attacker);
void ClientObituary(edict_t *self, edict_t *inflictor, edict_t *attacker);
void TossClientWeapon(edict_t *self);
void ClientThink(edict_t *ent, usercmd_t *ucmd);
void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void CopyToBodyQue(edict_t *ent);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void Use_Plat(edict_t *ent, edict_t *other, edict_t *activator);

// bot_koogle_ai.c protos
void KOOGLEAI_Think(edict_t *self);
void KOOGLEAI_PickLongRangeGoal(edict_t *self);
void KOOGLEAI_PickShortRangeGoal(edict_t *self);
qboolean KOOGLEAI_FindEnemy(edict_t *self);
void KOOGLEAI_ChooseWeapon(edict_t *self);

// bot_koogle_cmds.c protos
qboolean KOOGLECM_Commands(edict_t *ent);
void KOOGLECM_Store();

// bot_koogle_items.c protos
void KOOGLEIT_PlayerAdded(edict_t *ent);
void KOOGLEIT_PlayerRemoved(edict_t *ent);
qboolean KOOGLEIT_IsVisible(edict_t *self, vec3_t goal);
qboolean KOOGLEIT_IsReachable(edict_t *self, vec3_t goal);
qboolean KOOGLEIT_ChangeWeapon(edict_t *ent, gitem_t *item);
qboolean KOOGLEIT_CanUseArmor(gitem_t *item, edict_t *other);
float KOOGLEIT_ItemNeed(edict_t *self, int item);
int KOOGLEIT_ClassnameToIndex(char *classname);
void KOOGLEIT_BuildItemNodeTable(qboolean rebuild);

// bot_koogle_movement.c protos
qboolean KOOGLEMV_SpecialMove(edict_t *self, usercmd_t *ucmd);
void KOOGLEMV_Move(edict_t *self, usercmd_t *ucmd);
void KOOGLEMV_Attack(edict_t *self, usercmd_t *ucmd);
void KOOGLEMV_Wander(edict_t *self, usercmd_t *ucmd);

// bot_koogle_nodes.c protos
int KOOGLEND_FindCost(short int from, short int to);
int KOOGLEND_FindCloseReachableNode(edict_t *self, int dist, int type);
int KOOGLEND_FindClosestReachableNode(edict_t *self, int range, int type);
void KOOGLEND_SetGoal(edict_t *self, int goal_node);
qboolean KOOGLEND_FollowPath(edict_t *self);
void KOOGLEND_GrapFired(edict_t *self);
qboolean KOOGLEND_CheckForLadder(edict_t *self);
void KOOGLEND_PathMap(edict_t *self);
void KOOGLEND_InitNodes(void);
void KOOGLEND_ShowNode(int node);
void KOOGLEND_DrawPath();
void KOOGLEND_ShowPath(edict_t *self, int goal_node);
int KOOGLEND_AddNode(edict_t *self, int type);
void KOOGLEND_UpdateNodeEdge(int from, int to);
void KOOGLEND_RemoveNodeEdge(edict_t *self, int from, int to);
void KOOGLEND_ResolveAllPaths();
void KOOGLEND_SaveNodes();
void KOOGLEND_LoadNodes();

// bot_koogle_spawn.c protos
void KOOGLESP_SaveBots();
void KOOGLESP_LoadBots();
void KOOGLESP_HoldSpawn(edict_t *self);
void KOOGLESP_PutClientInServer(edict_t *bot, qboolean respawn, int team);
void KOOGLESP_Respawn(edict_t *self);
edict_t *KOOGLESP_FindFreeClient(void);
void KOOGLESP_SetName(edict_t *bot, char *name, char *skin, char *team);
void KOOGLESP_SpawnBot(char *team, char *name, char *skin, char *userinfo);
void KOOGLESP_RemoveBot(char *name);
void safe_cprintf(edict_t *ent, int printlevel, char *fmt, ...);
void safe_centerprintf(edict_t *ent, char *fmt, ...);
void safe_bprintf(int printlevel, char *fmt, ...);
void debug_printf(char *fmt, ...);
void KOOGLESP_SpawnHomer(edict_t *bot, char *name, char *skin, char *team); // Homer Bot with extras for RedKneck
void KOOGLESP_SpawnChastity(edict_t *bot, char *name, char *skin, char *team); // Chastity Bot No Extras
void KOOGLESP_SpawnPurgator(edict_t *bot, char *name, char *skin, char *team); // not used until I find the model or remake it (i lost this model, too bad that nobody gives a shit to help me find it!)
void KOOGLESP_SpawnMarvin(edict_t *bot, char *name, char *skin, char *team); // Marvin a Toon Bot
void KOOGLESP_SpawnPoop(edict_t *bot, char *name, char *skin, char *team); // Poop used to turn cheaters into a turd (model is male_nogule and is misssing) we lost it!
void KOOGLESP_SpawnPit(edict_t * bot, char * name, char * skin, char * team); // PitBullet bot in memory of an old hypocrit friend! (a loser in my book for being disrespectful to Vita)
void KOOGLESP_SetBotNameBandit(edict_t * bot, char * name, char * skin, char * team); // Bandit bot in honor and memory of an old friend! (who stole from me but I forgive him)
void KOOGLESP_SetBotNameDeeg(edict_t * bot, char * name, char * skin, char * team); // Deeg bot in honor and memory of an old friend! (never fucked me over)
void KOOGLESP_SetBotNameShorty(edict_t * bot, char * name, char * skin, char * team); // Shorty bot
void KOOGLESP_SetBotNameBeerHomer(edict_t * bot, char * name, char * skin, char * team); //Homer in Beer Gear
void KOOGLESP_SetBotNameChastitySandy(edict_t * bot, char * name, char * skin, char * team); //Chastity Bot with the name Sandy
void KOOGLESP_SetBotNameTina(edict_t * bot, char * name, char * skin, char * team); // Bot named Tina
void KOOGLESP_SetBotNameEddie(edict_t * bot, char * name, char * skin, char * team); // Bot named Eddie
void KOOGLESP_SetBotNameDrFreak(edict_t * bot, char * name, char * skin, char * team); // Freak Bot
void KOOGLESP_SetBotNameJack(edict_t * bot, char * name, char * skin, char * team); // Jack in the box Bot
void KOOGLESP_SetBotNameMelissaCrakhor(edict_t * bot, char * name, char * skin, char * team); // Named after my whore x-girl friend!
void KOOGLESP_SetBotNameFionaFlapsLadyDeath(edict_t * bot, char * name, char * skin, char * team); // Bot named after a nickname Ron Vogel gave somone when we were teenagers!
void KOOGLESP_SetBotNameSharonWhoreHill(edict_t * bot, char * name, char * skin, char * team); // Sharon WhoreHill bot named after my s-thief girlfriend!
void KOOGLESP_SetBotNameLynnHopper(edict_t * bot, char * name, char * skin, char * team); // bot created in memory of my whore x-wife
void KOOGLESP_SetBotNameDeadLock(edict_t * bot, char * name, char * skin, char * team); // bot created in memory 1999
void KOOGLESP_SetBotNameKrafty(edict_t * bot, char * name, char * skin, char * team); // bot created in memory of 1999
void KOOGLESP_SetBotNameGhost(edict_t * bot, char * name, char * skin, char * team); // The Ghost bot
void KOOGLESP_SpawnGoblin(edict_t * bot, char * name, char * skin, char * team); // Booger Eater
#endif