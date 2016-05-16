///////////////////////////////////////////////////////////////////////
//
//  kooglebot_items.c - This file contains all of the 
//                   item handling routines for the 
//                   KOOGLE bot, including fact table support
//           
///////////////////////////////////////////////////////////////////////
#include "..\g_local.h"
#include "kooglebot.h"

//int	num_players = 0;
int num_items = 0;
item_table_t item_table[MAX_EDICTS];
edict_t *players[MAX_CLIENTS];		// pointers to all players in the game

///////////////////////////////////////////////////////////////////////
// Add the player to our list
///////////////////////////////////////////////////////////////////////
void KOOGLEIT_PlayerAdded(edict_t *ent)
{
	players[level.num_players++] = ent;
}
///////////////////////////////////////////////////////////////////////
// Remove player from list
///////////////////////////////////////////////////////////////////////
void KOOGLEIT_PlayerRemoved(edict_t *ent)
{
	int i;
	int pos;

	// watch for 0 players
	if (level.num_players == 0)
		return;

	// special case for only one player
	if (level.num_players == 1)
	{
		level.num_players = 0;
		return;
	}

	// Find the player
	for (i = 0; i<level.num_players; i++)
		if (ent == players[i])
			pos = i;

	// decrement
	for (i = pos; i<level.num_players - 1; i++)
		players[i] = players[i + 1];

	level.num_players--;
}
///////////////////////////////////////////////////////////////////////
// Can we get there?
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEIT_IsReachable(edict_t *self, vec3_t goal)
{
	trace_t trace;
	vec3_t v;

	vec3_t	spot1;
	vec3_t	spot2;

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;

	VectorCopy(goal, spot2);

	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_ALPHA);

	if (trace.fraction == 1.0)
		return false;

	VectorCopy(self->mins, v);

	// Stepsize
	v[2] += 18;

	trace = gi.trace(self->s.origin, v, self->maxs, goal, self, MASK_OPAQUE);

	// Yes we can see it
	if (trace.fraction == 1.0)
		return true;
	else
		return false;

}
///////////////////////////////////////////////////////////////////////
// Visiblilty check 
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEIT_IsMaskAlpha(edict_t *self, vec3_t goal)
{
	trace_t trace;
	trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, goal, self, MASK_ALPHA);

	if (trace.fraction == 1.0)
		return true;
	else
		return false;
}

qboolean KOOGLEIT_IsVisible(edict_t *self, vec3_t goal)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;

	VectorCopy(goal, spot2);

	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_ALPHA);

	if (trace.fraction == 1.0)
		return false;

	trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, goal, self, MASK_OPAQUE);

	// Yes we can see it
	if (trace.fraction == 1.0)
		return true;
	else
		return false;

}
///////////////////////////////////////////////////////////////////////
//  Weapon changing support
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEIT_ChangeWeapon(edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return true;

	// Has not picked up weapon yet
	if (!ent->client->pers.inventory[ITEM_INDEX(item)])
		return false;

	// Do we have ammo for it?
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (!ent->client->pers.inventory[ammo_index] && !g_select_empty->value)
			return false;
	}

	// Change to this weapon
	ent->client->newweapon = item;

	return true;
}

extern gitem_armor_t jacketarmor_info;
extern gitem_armor_t combatarmor_info;
extern gitem_armor_t bodyarmor_info;

///////////////////////////////////////////////////////////////////////
// Check if we can use the armor
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEIT_CanUseArmor(gitem_t *item, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)item->info;

	old_armor_index = ArmorIndex(other);

	// handle armor shards specially
	if (item->tag == ARMOR_SHARD)
		return true;

	// get info on old armor
	if (old_armor_index == ITEM_INDEX(FindItem("Jacket Armor")))
		oldinfo = &jacketarmor_info;
	else if (old_armor_index == ITEM_INDEX(FindItem("Combat Armor")))
		oldinfo = &combatarmor_info;
	else // (old_armor_index == body_armor_index)
		oldinfo = &bodyarmor_info;

	if (newinfo->normal_protection <= oldinfo->normal_protection)
	{
		// calc new armor values
		salvage = newinfo->normal_protection / oldinfo->normal_protection;
		salvagecount = salvage * newinfo->base_count;
		newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

		if (newcount > oldinfo->max_count)
			newcount = oldinfo->max_count;

		// if we're already maxed out then we don't need the new armor
		if (other->client->pers.inventory[old_armor_index] >= newcount)
			return false;

	}

	return true;
}
///////////////////////////////////////////////////////////////////////
// Determines the NEED for an item
//
// This function can be modified to support new items to pick up
// Any other logic that needs to be added for custom decision making
// can be added here. For now it is very simple.
///////////////////////////////////////////////////////////////////////
float KOOGLEIT_ItemNeed(edict_t *self, int item)
{

	// Make sure item is at least close to being valid
	if (item < 0 || item > 100)
		return 0.0;

	switch (item)
	{
	case ITEMLIST_CASHROLL:
	case ITEMLIST_CASHBAGLARGE:
	case ITEMLIST_CASHBAGSMALL:
		return 3.0;
		// Health
	case ITEMLIST_HEALTH_SMALL:
	case ITEMLIST_HEALTH_LARGE:
	case ITEMLIST_HEALTH_MEGA:
	case ITEMLIST_ADRENALINE:
		if (self->health >= 100)
			return -1;
			else
			return 9.0;
	case ITEMLIST_COIL:
	case ITEMLIST_LIZZYHEAD:
	case ITEMLIST_QUADDAMAGE:
	case ITEMLIST_INVULNERABILITY:
	case ITEMLIST_SILENCER:
	case ITEMLIST_REBREATHER:
	case ITEMLIST_ENVIRONMENTSUIT:
	case ITEMLIST_BANDOLIER:
	case ITEMLIST_BATTERY:
	case ITEMLIST_JETPACK:
		return -1;
	case ITEMLIST_AMMOPACK:
	case ITEMLIST_BLACKJACK:
	case ITEMLIST_CROWBAR:
	case ITEMLIST_PISTOL:
	case ITEMLIST_SPISTOL:
	case ITEMLIST_SHOTGUN:
	case ITEMLIST_TOMMYGUN:
	case ITEMLIST_HEAVYMACHINEGUN:
	case ITEMLIST_GRENADELAUNCHER:
	case ITEMLIST_BAZOOKA:
	case ITEMLIST_FLAMETHROWER:
	case ITEMLIST_SHOTGUN_E:
	case ITEMLIST_HEAVYMACHINEGUN_E:
	case ITEMLIST_BAZOOKA_E:
	case ITEMLIST_FLAMETHROWER_E:
	case ITEMLIST_GRENADELAUNCHER_E:
	case ITEMLIST_PISTOL_E:
	case ITEMLIST_TOMMYGUN_E:
		// Weapons
	case ITEMLIST_RAILGUN:
	case ITEMLIST_MACHINEGUN:
	case ITEMLIST_CHAINGUN:
	case ITEMLIST_SUPERSHOTGUN:
	case ITEMLIST_BFG10K:
	case ITEMLIST_HYPERBLASTER:
		if (!self->client->pers.inventory[item])
			return 3.0;
		else
			return -1;
	case ITEMLIST_SLUGS:
		if (self->client->pers.inventory[ITEMLIST_SLUGS] < self->client->pers.max_slugs)
			return 0.3;
		else
			return -1;
	case ITEMLIST_CELLS:
		if (self->client->pers.inventory[ITEMLIST_CELLS] <	self->client->pers.max_cells)
			return 0.3;
		else
			return -1;
	case ITEMLIST_BODYARMOR:
		if (KOOGLEIT_CanUseArmor(FindItem("Body Armor"), self))
			return 0.6;
		else
			return -1;
	case ITEMLIST_COMBATARMOR:
		if (KOOGLEIT_CanUseArmor(FindItem("Combat Armor"), self))
			return 0.6;
		else
			return -1;
	case ITEMLIST_JACKETARMOR:
		if (KOOGLEIT_CanUseArmor(FindItem("Jacket Armor"), self))
			return 0.6;
		else
			return -1;
	case ITEMLIST_POWERSCREEN:
	case ITEMLIST_POWERSHIELD:
		return 0.5;
		// Ammo
	case ITEMLIST_SHELLS:
		if (self->client->pers.inventory[ITEMLIST_SHELLS] < self->client->pers.max_shells)
			return 0.3;
		else
			return -1;
	case ITEMLIST_CYLINDER:
	case ITEMLIST_BULLETS:
		if (self->client->pers.inventory[ITEMLIST_BULLETS] < self->client->pers.max_bullets)
			return 0.3;
		else
			return -1;
	case ITEMLIST_ROCKETS:
		if (self->client->pers.inventory[ITEMLIST_ROCKETS] < self->client->pers.max_rockets)
			return 0.3;
		else
			return -1;
	case ITEMLIST_GRENADES:
		if (self->client->pers.inventory[ITEMLIST_GRENADES] < self->client->pers.max_grenades)
			return 0.3;
		else
			return -1;
	case ITEMLIST_AMMO308:
		if (self->client->pers.inventory[ITEMLIST_AMMO308] < self->client->pers.max_slugs)
			return 0.3;
		else
			return -1;
	// Integrate	
	case ITEMLIST_FLAMETANK:
	case ITEMLIST_ARMORHELMET:
	case ITEMLIST_ARMORJACKET:
	case ITEMLIST_ARMORLEGS:
	case ITEMLIST_ARMORHELMETHEAVY:
	case ITEMLIST_ARMORJACKETHEAVY:
	case ITEMLIST_ARMORLEGSHEAVY:
		return 0.3;
	// Integrate
	case ITEMLIST_FLASHLIGHT:
	case ITEMLIST_WATCH:
	case ITEMLIST_WHISKEY:
	case ITEMLIST_PACK:
	case ITEMLIST_KEYFUSE:
	case ITEMLIST_SAFEDOCS:
	case ITEMLIST_VALVE:
	case ITEMLIST_OILCAN:
	case ITEMLIST_KEY1:
	case ITEMLIST_KEY2:
	case ITEMLIST_KEY3:
	case ITEMLIST_KEY4:
	case ITEMLIST_KEY5:
	case ITEMLIST_KEY6:
	case ITEMLIST_KEY7:
	case ITEMLIST_KEY8:
	case ITEMLIST_KEY9:
	case ITEMLIST_KEY10:
		return -1;

	case ITEMLIST_PISTOLMODS:
		return 0.3;
	default:
		return 0.0;
	}
}
///////////////////////////////////////////////////////////////////////
// Convert a classname to its index value
//
// I prefer to use integers/defines for simplicity sake. This routine
// can lead to some slowdowns I guess, but makes the rest of the code
// easier to deal with.
///////////////////////////////////////////////////////////////////////
int KOOGLEIT_ClassnameToIndex(char *classname)
{
	if (strcmp(classname, "item_armor_body") == 0)
		return ITEMLIST_BODYARMOR;

	if (strcmp(classname, "item_armor_combat") == 0)
		return ITEMLIST_COMBATARMOR;

	if (strcmp(classname, "item_armor_jacket") == 0)
		return ITEMLIST_JACKETARMOR;

	if (strcmp(classname, "item_armor_shard") == 0)
		return ITEMLIST_ARMORSHARD;

	if (strcmp(classname, "item_power_screen") == 0)
		return ITEMLIST_POWERSCREEN;

	if (strcmp(classname, "item_power_shield") == 0)
		return ITEMLIST_POWERSHIELD;

	if(strcmp(classname,"weapon_grapple")==0)
		return ITEMLIST_GRAPPLE;

	if(strcmp(classname,"weapon_blaster")==0)
		return ITEMLIST_BLASTER;

	if (strcmp(classname, "weapon_supershotgun") == 0)
		return ITEMLIST_SUPERSHOTGUN;

	if (strcmp(classname, "weapon_machinegun") == 0)
		return ITEMLIST_MACHINEGUN;

	if (strcmp(classname, "weapon_chaingun") == 0)
		return ITEMLIST_CHAINGUN;

	if (strcmp(classname, "weapon_chaingun") == 0)
		return ITEMLIST_CHAINGUN;

	if (strcmp(classname, "weapon_hyperblaster") == 0)
		return ITEMLIST_HYPERBLASTER;

	if (strcmp(classname, "weapon_railgun") == 0)
		return ITEMLIST_RAILGUN;

	if (strcmp(classname, "weapon_bfg") == 0)
		return ITEMLIST_BFG10K;

	if (strcmp(classname, "ammo_cells") == 0)
		return ITEMLIST_CELLS;

	if (strcmp(classname, "ammo_slugs") == 0)
		return ITEMLIST_SLUGS;

	if (strcmp(classname, "item_quad") == 0)
		return ITEMLIST_QUADDAMAGE;

	if (strcmp(classname, "item_invunerability") == 0)
		return ITEMLIST_INVULNERABILITY;

	if (strcmp(classname, "item_silencer") == 0)
		return ITEMLIST_SILENCER;

	if (strcmp(classname, "item_rebreather") == 0)
		return ITEMLIST_REBREATHER;

	if (strcmp(classname, "item_enviornmentsuit") == 0)
		return ITEMLIST_ENVIRONMENTSUIT;

	if (strcmp(classname, "item_ancienthead") == 0)
		return ITEMLIST_ANCIENTHEAD;

	if (strcmp(classname, "item_adrenaline") == 0)
		return ITEMLIST_ADRENALINE;

	if (strcmp(classname, "item_bandolier") == 0)
		return ITEMLIST_BANDOLIER;

	if (strcmp(classname, "item_pack") == 0)
		return ITEMLIST_AMMOPACK;

	if (strcmp(classname, "item_datacd") == 0)
		return ITEMLIST_DATACD;

	if (strcmp(classname, "item_powercube") == 0)
		return ITEMLIST_POWERCUBE;

	if (strcmp(classname, "item_pyramidkey") == 0)
		return ITEMLIST_PYRAMIDKEY;

	if (strcmp(classname, "item_dataspinner") == 0)
		return ITEMLIST_DATASPINNER;

	if (strcmp(classname, "item_securitypass") == 0)
		return ITEMLIST_SECURITYPASS;

	if (strcmp(classname, "item_bluekey") == 0)
		return ITEMLIST_BLUEKEY;

	if (strcmp(classname, "item_redkey") == 0)
		return ITEMLIST_REDKEY;

	if (strcmp(classname, "item_commandershead") == 0)
		return ITEMLIST_COMMANDERSHEAD;

	if (strcmp(classname, "item_airstrikemarker") == 0)
		return ITEMLIST_AIRSTRIKEMARKER;

	if (strcmp(classname, "item_health") == 0) 
		return ITEMLIST_HEALTH;

	if (strcmp(classname, "item_health_mega") == 0)
		return ITEMLIST_HEALTH_MEGA;

	if (strcmp(classname, "item_armor_helmet") == 0)
		return ITEMLIST_ARMORHELMET;

	if (strcmp(classname, "item_armor_jacket") == 0)
		return ITEMLIST_ARMORJACKET;

	if (strcmp(classname, "item_armor_legs") == 0)
		return ITEMLIST_ARMORLEGS;

	if (strcmp(classname, "item_armor_helmet_heavy") == 0)
		return ITEMLIST_ARMORHELMETHEAVY;

	if (strcmp(classname, "item_armor_jacket_heavy") == 0)
		return ITEMLIST_ARMORJACKETHEAVY;

	if (strcmp(classname, "item_armor_legs_heavy") == 0)
		return ITEMLIST_ARMORLEGSHEAVY;

	if (strcmp(classname, "weapon_blackjack") == 0)
		return ITEMLIST_BLACKJACK;

	if (strcmp(classname, "weapon_crowbar") == 0)
		return ITEMLIST_CROWBAR;

	if (strcmp(classname, "weapon_pistol") == 0)
		return ITEMLIST_PISTOL;

	if (strcmp(classname, "weapon_spistol") == 0)
		return ITEMLIST_SPISTOL;

	if (strcmp(classname, "weapon_shotgun") == 0)
		return ITEMLIST_SHOTGUN;

	if (strcmp(classname, "weapon_tommygun") == 0)
		return ITEMLIST_TOMMYGUN;

	if (strcmp(classname, "weapon_heavymachinegun") == 0)
		return ITEMLIST_HEAVYMACHINEGUN;

	if (strcmp(classname, "weapon_grenadelauncher") == 0)
		return ITEMLIST_GRENADELAUNCHER;

	if (strcmp(classname, "weapon_bazooka") == 0)
		return ITEMLIST_BAZOOKA;

	if (strcmp(classname, "weapon_flamethrower") == 0)
		return ITEMLIST_FLAMETHROWER;

	if (strcmp(classname, "ammo_grenades") == 0)
		return ITEMLIST_GRENADES;

	if (strcmp(classname, "weapon_shotgun_e") == 0)
		return ITEMLIST_SHOTGUN_E;

	if (strcmp(classname, "weapon_tommygun_e") == 0)
		return ITEMLIST_TOMMYGUN_E;

	if (strcmp(classname, "weapon_heavymachinegun_e") == 0)
		return ITEMLIST_HEAVYMACHINEGUN_E;

	if (strcmp(classname, "weapon_grenadelauncher_e") == 0)
		return ITEMLIST_GRENADELAUNCHER_E;

	if (strcmp(classname, "weapon_bazooka_e") == 0)
		return ITEMLIST_BAZOOKA_E;

	if (strcmp(classname, "weapon_flamethrower_e") == 0)
		return ITEMLIST_FLAMETHROWER_E;

	if (strcmp(classname, "weapon_pistol_e") == 0)
		return ITEMLIST_PISTOL_E;

	if (strcmp(classname, "ammo_shells") == 0)
		return ITEMLIST_SHELLS;

	if (strcmp(classname, "ammo_bullets") == 0)
		return ITEMLIST_BULLETS;

	if (strcmp(classname, "ammo_rockets") == 0)
		return ITEMLIST_ROCKETS;

	if (strcmp(classname, "ammo_308") == 0)
		return ITEMLIST_AMMO308;

	if (strcmp(classname, "ammo_cylinder") == 0)
		return ITEMLIST_CYLINDER;

	if (strcmp(classname, "ammo_flametank") == 0)
		return ITEMLIST_FLAMETANK;

	if (strcmp(classname, "item_coil") == 0)
		return ITEMLIST_COIL;

	if (strcmp(classname, "item_lizzyhead") == 0)
		return ITEMLIST_LIZZYHEAD;

	if (strcmp(classname, "item_cashroll") == 0)
		return ITEMLIST_CASHROLL;

	if (strcmp(classname, "item_cashbaglarge") == 0)
		return ITEMLIST_CASHBAGLARGE;

	if (strcmp(classname, "item_cashbagsmall") == 0)
		return ITEMLIST_CASHBAGSMALL;

	if (strcmp(classname, "item_battery") == 0)
		return ITEMLIST_BATTERY;

	if (strcmp(classname, "item_jetpack") == 0)
		return ITEMLIST_JETPACK;

	if (strcmp(classname, "item_health_sm") == 0)
		return ITEMLIST_HEALTH_SMALL;

	if (strcmp(classname, "item_health_lg") == 0)
		return ITEMLIST_HEALTH_LARGE;

	if (strcmp(classname, "item_flashlight") == 0)
		return ITEMLIST_FLASHLIGHT;

	if (strcmp(classname, "item_watch") == 0)
		return ITEMLIST_WATCH;

	if (strcmp(classname, "item_whiskey") == 0)
		return ITEMLIST_WHISKEY;

	if (strcmp(classname, "item_pack") == 0)
		return ITEMLIST_PACK;

	if (strcmp(classname, "item_adrenaline") == 0)
		return ITEMLIST_ADRENALINE;

	if (strcmp(classname, "key_fuse") == 0)
		return ITEMLIST_KEYFUSE;

	if (strcmp(classname, "item_safedocs") == 0)
		return ITEMLIST_SAFEDOCS;

	if (strcmp(classname, "item_valve") == 0)
		return ITEMLIST_VALVE;

	if (strcmp(classname, "item_oilcan") == 0)
		return ITEMLIST_OILCAN;

	if (strcmp(classname, "key_key1") == 0)
		return ITEMLIST_KEY1;

	if (strcmp(classname, "key_key2") == 0)
		return ITEMLIST_KEY2;

	if (strcmp(classname, "key_key3") == 0)
		return ITEMLIST_KEY3;

	if (strcmp(classname, "key_key4") == 0)
		return ITEMLIST_KEY4;

	if (strcmp(classname, "key_key5") == 0)
		return ITEMLIST_KEY5;

	if (strcmp(classname, "key_key6") == 0)
		return ITEMLIST_KEY6;

	if (strcmp(classname, "key_key7") == 0)
		return ITEMLIST_KEY7;

	if (strcmp(classname, "key_key8") == 0)
		return ITEMLIST_KEY8;

	if (strcmp(classname, "key_key9") == 0)
		return ITEMLIST_KEY9;

	if (strcmp(classname, "key_key10") == 0)
		return ITEMLIST_KEY10;

	if (strcmp(classname, "item_pistol_mods") == 0)
		return ITEMLIST_PISTOLMODS;
	
	if(strcmp(classname,"item_flag_team1")==0)
	return ITEMLIST_FLAG1;

	if(strcmp(classname,"item_flag_team2")==0)
	return ITEMLIST_FLAG2;

	if(strcmp(classname,"item_tech1")==0)
	return ITEMLIST_RESISTANCETECH;

	if(strcmp(classname,"item_tech2")==0)
	return ITEMLIST_STRENGTHTECH;

	if(strcmp(classname,"item_tech3")==0)
	return ITEMLIST_HASTETECH;

	if(strcmp(classname,"item_tech4")==0)
	return ITEMLIST_REGENERATIONTECH;
	
	return INVALID;
}
///////////////////////////////////////////////////////////////////////
// Only called once per level, when saved will not be called again
//
// Downside of the routine is that items can not move about. If the level
// has been saved before and reloaded, it could cause a problem if there
// are items that spawn at random locations.
//
// #define DEBUG // uncomment to write out items to a file.
///////////////////////////////////////////////////////////////////////
void KOOGLEIT_BuildItemNodeTable(qboolean rebuild)
{
	edict_t *items;
	int i, item_index;
	vec3_t v, v1, v2;

#ifdef DEBUG
	FILE *pOut; // for testing
	if ((pOut = fopen("items.txt", "wt")) == NULL)
		return;
#endif

	num_items = 0;

	// Add game items
	for (items = g_edicts; items < &g_edicts[globals.num_edicts]; items++)
	{
		// filter out crap
		if (items->solid == SOLID_NOT)
			continue;

		if (strcmp(items->classname, "func_wall") == 0)
			continue;

		if (strcmp(items->classname, "noclass") == 0)
			continue;

		if (!items->classname)
			continue;

		/////////////////////////////////////////////////////////////////
		// Items
		/////////////////////////////////////////////////////////////////
		item_index = KOOGLEIT_ClassnameToIndex(items->classname);

		////////////////////////////////////////////////////////////////
		// SPECIAL NAV NODE DROPPING CODE
		////////////////////////////////////////////////////////////////
		// Special node dropping for platforms
		if (strcmp(items->classname, "func_plat") == 0)
		{
			if (!rebuild)
				KOOGLEND_AddNode(items, BOTNODE_PLATFORM);
			item_index = 99; // to allow to pass the item index test
		}

		// Special node dropping for teleporters
		if (strcmp(items->classname, "misc_teleporter_dest") == 0 || strcmp(items->classname, "misc_teleporter") == 0 || strcmp(items->classname, "misc_teleporter2") == 0 || strcmp(items->classname, "misc_teleporter3") == 0 || strcmp(items->classname, "misc_teleporter4") == 0 || strcmp(items->classname, "misc_teleporter5") == 0 || strcmp(items->classname, "misc_teleporter6") == 0 || strcmp(items->classname, "misc_teleporter7") == 0 || strcmp(items->classname, "misc_teleporter8") == 0 || strcmp(items->classname, "misc_teleporter9") == 0 || strcmp(items->classname, "misc_teleporter10") == 0)
		{
			if (!rebuild)
				KOOGLEND_AddNode(items, BOTNODE_TELEPORTER);
			item_index = 99;
		}

#ifdef DEBUG
		if (item_index == INVALID)
			fprintf(pOut, "Rejected item: %s node: %d pos: %f %f %f\n", items->classname, item_table[num_items].node, items->s.origin[0], items->s.origin[1], items->s.origin[2]);
		else
			fprintf(pOut, "item: %s node: %d pos: %f %f %f\n", items->classname, item_table[num_items].node, items->s.origin[0], items->s.origin[1], items->s.origin[2]);
#endif		

		if (item_index == INVALID)
			continue;

		// add a pointer to the item entity
		item_table[num_items].ent = items;
		item_table[num_items].item = item_index;

		// If new, add nodes for items
		if (!rebuild)
		{
			// Add a new node at the item's location.
			item_table[num_items].node = KOOGLEND_AddNode(items, BOTNODE_ITEM);
			num_items++;
		}
		else // Now if rebuilding, just relink ent structures 
		{
			// Find stored location
			for (i = 0; i<numnodes; i++)
			{
				if (nodes[i].type == BOTNODE_ITEM ||
					nodes[i].type == BOTNODE_PLATFORM ||
					nodes[i].type == BOTNODE_CROUCH ||
					nodes[i].type == BOTNODE_JUMP ||
					nodes[i].type == BOTNODE_LADDER ||
					nodes[i].type == BOTNODE_TELEPORTER) // valid types
				{
					VectorCopy(items->s.origin, v);

					// Add 16 to item type nodes
					if (nodes[i].type == BOTNODE_ITEM)
						v[2] += 16;

					if (nodes[i].type == BOTNODE_CROUCH)
						v[2] += 24; // was 16


					if (nodes[i].type == BOTNODE_JUMP)
						v[2] += 64; // step size 64

					if (nodes[i].type == BOTNODE_LADDER)
						v[2] += 18; // step size 18

					// Add 32 to teleporter
					if (nodes[i].type == BOTNODE_TELEPORTER)
						v[2] += 32;

					if (nodes[i].type == BOTNODE_PLATFORM)
					{
						VectorCopy(items->maxs, v1);
						VectorCopy(items->mins, v2);

						// To get the center
						v[0] = (v1[0] - v2[0]) / 2 + v2[0];
						v[1] = (v1[1] - v2[1]) / 2 + v2[1];
						v[2] = items->mins[2] + 64;
					}

					if (v[0] == nodes[i].origin[0] &&
						v[1] == nodes[i].origin[1] &&
						v[2] == nodes[i].origin[2])
					{
						// found a match now link to facts
						item_table[num_items].node = i;

#ifdef DEBUG
						fprintf(pOut, "Relink item: %s node: %d pos: %f %f %f\n", items->classname, item_table[num_items].node, items->s.origin[0], items->s.origin[1], items->s.origin[2]);
#endif							
						num_items++;
					}
				}
			}
		}


	}

#ifdef DEBUG
	fclose(pOut);
#endif

}
