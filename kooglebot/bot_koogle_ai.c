///////////////////////////////////////////////////////////////////////
//
//  kooglebot_ai.c -      This file contains all of the
//                        AI routines for the KOOGLE bot.
//
//
// NOTE: I went back and pulled out most of the brains from
//       a number of these functions. They can be expanded on
//       to provide a "higher" level of AI.
////////////////////////////////////////////////////////////////////////
#include "..\g_local.h"
#include "..\m_player.h"
#include "..\mq2_player.h"
#include "kooglebot.h"
///////////////////////////////////////////////////////////////////////
// Main Think function for bot
///////////////////////////////////////////////////////////////////////
void thug_talk(edict_t *self);
void thug_talk_think(edict_t *self);

void KOOGLEAI_Think(edict_t *self)
{
	usercmd_t	ucmd;

	// Set up client movement
	VectorCopy(self->client->ps.viewangles, self->s.angles);
	VectorSet(self->client->ps.pmove.delta_angles, 0, 0, 0);
	memset(&ucmd, 0, sizeof(ucmd));
	self->enemy = NULL;
	self->movetarget = NULL;

	// Force respawn
	if (self->deadflag)
	{
		self->client->buttons = 0;
		ucmd.buttons = BUTTON_ATTACK;
	}

	if (self->state == BOTSTATE_WANDER && self->wander_timeout < level.time)
		KOOGLEAI_PickLongRangeGoal(self); // pick a new long range goal

										   // Kill the bot if completely stuck somewhere
	if (VectorLength(self->velocity) > 37) //
		self->suicide_timeout = level.time + 10.0;

	if (self->suicide_timeout < level.time)
	{
		self->health = 0;
		meansOfDeath = MOD_RESTART; // added by Ghost 5/7/2016
		player_die(self, self, self, 1, vec3_origin, 0, 0);
	}

	// Find any short range goal
	KOOGLEAI_PickShortRangeGoal(self);

	// Look for enemies
	if (KOOGLEAI_FindEnemy(self))
	{
		KOOGLEAI_ChooseWeapon(self);
		KOOGLEMV_Attack(self, &ucmd);
	}
	else
	{
		// Execute the move, or wander
		if (self->state == BOTSTATE_WANDER)
			KOOGLEMV_Wander(self, &ucmd);
		else if (self->state == BOTSTATE_MOVE)
			KOOGLEMV_Move(self, &ucmd);
	}

	//debug_printf("State: %d\n",self->state);

	// set approximate ping
	ucmd.msec = 75 + floor(random() * 25) + 1;

	// show random ping values in scoreboard
	self->client->ping = ucmd.msec;

	// set bot's view angle
	ucmd.angles[PITCH] = ANGLE2SHORT(self->s.angles[PITCH]);
	ucmd.angles[YAW] = ANGLE2SHORT(self->s.angles[YAW]);
	ucmd.angles[ROLL] = ANGLE2SHORT(self->s.angles[ROLL]);

	// send command through id's code
	ClientThink(self, &ucmd);

	self->nextthink = level.time + FRAMETIME;
}

///////////////////////////////////////////////////////////////////////
// Evaluate the best long range goal and send the bot on
// its way. This is a good time waster, so use it sparingly.
// Do not call it for every think cycle.
///////////////////////////////////////////////////////////////////////
void KOOGLEAI_PickLongRangeGoal(edict_t *self)
{
	int i;
	int node;
	float weight, best_weight = 0.0;
	int current_node, goal_node;
	edict_t *goal_ent;
	float cost;
	edict_t *cash;

	// look for a target
	current_node = KOOGLEND_FindClosestReachableNode(self, BOTNODE_DENSITY, BOTNODE_ALL);

	self->current_node = current_node;

	if (current_node == -1)
	{
		self->state = BOTSTATE_WANDER;
		self->wander_timeout = level.time + 1.0;
		self->goal_node = -1;
		return;
	}

	///////////////////////////////////////////////////////
	// Items
	///////////////////////////////////////////////////////
	for (i = 0; i < num_items; i++)
	{
		if (item_table[i].ent == NULL || item_table[i].ent->solid == SOLID_NOT) // ignore items that are not there.
			continue;

		cost = KOOGLEND_FindCost(current_node, item_table[i].node);

		if (cost == INVALID) //failsafe
			continue;

		if (cost < 2) // ignore invalid and very short hops
			continue;

		weight = KOOGLEIT_ItemNeed(self, item_table[i].item);
		/*
		// If I am on team one and I have the flag for the other team....return it
		if(ctf->value && (item_table[i].item == ITEMLIST_FLAG2 || item_table[i].item == ITEMLIST_FLAG1) &&
		(self->client->resp.ctf_team == CTF_TEAM1 && self->client->pers.inventory[ITEMLIST_FLAG2] ||
		self->client->resp.ctf_team == CTF_TEAM2 && self->client->pers.inventory[ITEMLIST_FLAG1]))
		weight = 10.0;
		*/
		weight *= random(); // Allow random variations
		weight /= cost; // Check against cost of getting there

		if (weight > best_weight)
		{
			best_weight = weight;
			goal_node = item_table[i].node;
			goal_ent = item_table[i].ent;
		}

	}

	///////////////////////////////////////////////////////
	// Players
	///////////////////////////////////////////////////////
	// This should be its own function and is for now just
	// finds a player to set as the goal.
	

	for (i = 0; i < level.num_players; i++)
	{
		node = KOOGLEND_FindClosestReachableNode(players[i], BOTNODE_DENSITY, BOTNODE_ALL);

		// updated 5/2/2016 by Ghost - ignore people flagged as SOLID_NOT, NOTARGET, GOD, DEAD, SPECTATING or in CHASECAM
		if ((players[i] == NULL) || (players[i] == self)
			|| (players[i]->flags & FL_NOTARGET)
			|| (players[i]->flags & FL_GODMODE)
			|| (players[i]->flags & FL_CHASECAM)
			|| (players[i]->deadflag == DEAD_DEAD)
			|| (players[i]->client->pers.spectator == SPECTATING)
			|| (players[i]->solid == SOLID_NOT))
			continue;

		if (node != INVALID)
		{
			//debug
			//gi.dprintf("--------------------------------------------------\n");
			//gi.dprintf("%s selected kill target %s -> node %d\n", self->client->pers.netname, players[i]->client->pers.netname, node);
		}

		if (node == INVALID) //failsafe
			continue;

		cost = KOOGLEND_FindCost(current_node, node);

		if (cost == INVALID || cost < 3) // ignore invalid and very short hops
			continue;

		// really go hard after the other team players if it is not regular TEAM DEATHMATCH and they are holding money!
		if (teamplay->value && (teamplay->value != TM_GANGBANG) && (players[i]->client) && (players[i]->client->pers.team != self->client->pers.team))
		{
			if (players[i]->s.renderfx2 |= RF2_MONEYBAG)
				weight = 2.0;
			else
				weight = 0.3;
		}
		else // freeforall lets not go quite as hard after the players!
			weight = 0.3;

		// note: this is what we use to declare money already belongs to our team
		//if (cash->style != self->client->pers.team)

		/*
		// Player carrying the flag?
		if(teamplay->value && (players[i]->client->pers.inventory[ITEMLIST_FLAG2] || players[i]->client->pers.inventory[ITEMLIST_FLAG1]))
		weight = 2.0;
		else*/
		//weight = 0.3;

		weight *= random(); // Allow random variations
		weight /= cost; // Check against cost of getting there

		if (weight > best_weight)
		{
			best_weight = weight;
			goal_node = node;
			goal_ent = players[i];
		}
	}

	// If do not find a goal, go wandering to find a goal....
	if (best_weight == 0.0 || goal_node == INVALID)
	{
		self->goal_node = INVALID;
		self->state = BOTSTATE_WANDER;
		self->wander_timeout = level.time + 1.0;

		//if (debug_mode)
		//{
		//	gi.dprintf("-------------------------------------------------------------------------\n");
		//	gi.dprintf("%s is looking for LR goal, wandering.\n", self->client->pers.netname);
		//	gi.dprintf("-------------------------------------------------------------------------\n");
		//}
	

		return; // no path?
	}

	// OK, everything valid, let's start moving to our goal.
	self->state = BOTSTATE_MOVE;
	self->tries = 0; // Reset the count of how many times we tried this goal

	//if (goal_ent != NULL && debug_mode)
	if (goal_ent != NULL)
	{
		//debug
		//gi.dprintf("--------------------------------------------------\n");
		//gi.dprintf("%s selected a %s -> node %d (LR goal)\n", self->client->pers.netname, goal_ent->classname, goal_node);
	}
	KOOGLEND_SetGoal(self, goal_node);
}

///////////////////////////////////////////////////////////////////////
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
// THIS DOES NOT INCLUDE PLAYERS
///////////////////////////////////////////////////////////////////////
void KOOGLEAI_PickShortRangeGoal(edict_t *self)
{
	edict_t *target;
	float weight, best_weight = 0.0;
	edict_t *best;
	int index, rnd;

	// look for a target (should make more efficient later)
	//rnd = random() * 199;
	//target = findradius(NULL, self->s.origin, rnd);

	target = findradius(NULL, self->s.origin, 200);

	while (target)
	{
		if (target->classname == NULL)
			return;

		// Missle avoidance code
		// Set our movetarget to be the rocket or grenade fired at us.
		// modified by Ghost 5/2/2016 so that bots pay very close attention to money dropped!
		if (Q_stricmp(target->classname, "rocket") == 0
			|| Q_stricmp(target->classname, "grenade") == 0
			|| Q_stricmp(target->classname, "item_cashroll") == 0
			|| Q_stricmp(target->classname, "item_cashbagsmall") == 0
			|| Q_stricmp(target->classname, "item_cashbaglarge") == 0)
		{
			if (debug_mode)
				gi.dprintf("%s ALERT!\n", target->classname); // modified to print the exact ALERT 5/2/2106 by Ghost

			self->movetarget = target;
			return;
		}

		if (KOOGLEIT_IsReachable(self, target->s.origin))
		{
			if (infront(self, target))
			{
				index = KOOGLEIT_ClassnameToIndex(target->classname);
				weight = KOOGLEIT_ItemNeed(self, index);

				if (weight > best_weight)
				{
					best_weight = weight;
					best = target;
				}
			}
		}

		// next target
		target = findradius(target, self->s.origin, 200);
		//rnd = random() * 199;
		//target = findradius(target, self->s.origin, rnd);
	}

	if (best_weight)
	{
		self->movetarget = best;

		// this is item nodes only (not players) all short range goals includes weapons or ammo or armor so far as I can tell
		// the radius was modified to 1000 from 200 - old code made no sense at all LR or SR who gives a fuck!
		// bot selects a node and goes to it, why the fuck should it matter if it is close or far away! clearly it
		// explains why the bots look so fucking retarded most of the time!

		//debug is turned off while testing by Ghost 5/2/2106
		//if (debug_mode && self->goalentity != self->movetarget)

		if (debug_mode)
			if (self->goalentity != self->movetarget)
			{
				int goaltonode;
				goaltonode = KOOGLEIT_ClassnameToIndex(self->movetarget->classname);
				gi.dprintf("%s -> best_weight = %d for ", self->client->pers.netname, self->goalentity);
				gi.dprintf("%s SR Node %i\n", self->movetarget->classname, goaltonode);
			}
		self->goalentity = best;
	}
}

///////////////////////////////////////////////////////////////////////
// Scan for enemy (simplifed for now to just pick any visible enemy)
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEAI_FindEnemy(edict_t *self)
{
	int i;

	for (i = 0; i <= level.num_players; i++)
	{
		// updated 5/2/2016 by Ghost - ignore people flagged as SOLID_NOT, NOTARGET, GOD, DEAD, SPECTATING or in CHASECAM
		if ((players[i] == NULL) || (players[i] == self)
			|| (players[i]->flags & FL_NOTARGET)
			|| (players[i]->flags & FL_GODMODE)
			|| (players[i]->flags & FL_CHASECAM)
			|| (players[i]->deadflag == DEAD_DEAD)
			|| (players[i]->client->pers.spectator == SPECTATING)
			|| (players[i]->solid == SOLID_NOT))
			continue;

		if (teamplay->value != 0 && self->client->pers.team == players[i]->client->pers.team)
			continue;

		/* this is an alternate version that tells bots not to pick an enemy if they are not right in front of the bot! */
		/* if (!players[i]->deadflag && visible(self, players[i]) && infront(self, players[i]) && gi.inPVS(self->s.origin, players[i]->s.origin)) */

		if (!players[i]->deadflag && visible(self, players[i]) && gi.inPVS(self->s.origin, players[i]->s.origin))
		{
			self->enemy = players[i];
			// gi.sound(self, CHAN_AUTO, gi.soundindex("actors/male/profanity/level1/cuss1-7.wav"), 1, ATTN_NORM, 0);
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
// Hold fire with RL/BFG?
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEAI_CheckShot(edict_t *self)
{
	trace_t tr;

	tr = gi.trace(self->s.origin, tv(-8, -8, -8), tv(8, 8, 8), self->enemy->s.origin, self, MASK_OPAQUE);

	// Blocked, do not shoot
	if (tr.fraction != 1.0)
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////
// Choose the best weapon for bot (simplified)
///////////////////////////////////////////////////////////////////////
void KOOGLEAI_ChooseWeapon(edict_t *self)
{
	float range;
	vec3_t v;

	// if no enemy, then what are we doing here?
	if (!self->enemy)
		return;


	if (KOOGLEIT_ChangeWeapon(self, FindItem("shotgun")))
		return;

	// Base selection on distance.
	VectorSubtract(self->s.origin, self->enemy->s.origin, v);
	range = VectorLength(v);

	// Longer range
	if (range > 300)
	{
		//	// choose BFG if enough ammo
		//	if (self->client->pers.inventory[ITEMLIST_CELLS] > 50)
		//		if (KOOGLEAI_CheckShot(self) && KOOGLEIT_ChangeWeapon(self, FindItem("railgun")))
		//			return;

		if (KOOGLEAI_CheckShot(self) && KOOGLEIT_ChangeWeapon(self, FindItem("Bazooka")))
			return;
	}

	// Only use GL in certain ranges and only on targets at or below our level
	if (range > 100 && range < 500 && self->enemy->s.origin[2] - 20 < self->s.origin[2])
		if (KOOGLEIT_ChangeWeapon(self, FindItem("grenade launcher")))
			return;

	//if (KOOGLEIT_ChangeWeapon(self, FindItem("hyperblaster")))
	//	return;

	// Only use HMG when ammo > 30
	if (self->client->pers.inventory[ITEMLIST_AMMO308] >= 30)
		if (KOOGLEIT_ChangeWeapon(self, FindItem("Heavy machinegun")))
			return;

	// Only use CG when ammo > 50
	//if (self->client->pers.inventory[ITEMLIST_BULLETS] >= 50)
	//	if (KOOGLEIT_ChangeWeapon(self, FindItem("chaingun")))
	//		return;
	// always favor the HMG
//	if (KOOGLEIT_ChangeWeapon(self, FindItem("Heavy Machinegun")))
//		return;

	if (KOOGLEIT_ChangeWeapon(self, FindItem("Tommygun")))
		return;

	//if (KOOGLEIT_ChangeWeapon(self, FindItem("machinegun")))
	//	return;

	//if (KOOGLEIT_ChangeWeapon(self, FindItem("super shotgun")))
	//	return;

	//if (KOOGLEIT_ChangeWeapon(self, FindItem("bfg10k")))
	//	return;

	if (KOOGLEIT_ChangeWeapon(self, FindItem("Pistol")))
		return;

	if (KOOGLEIT_ChangeWeapon(self, FindItem("Crowbar")))
		return;

	if (KOOGLEIT_ChangeWeapon(self, FindItem("Pipe")))
		return;

	return;
	/////////////////////////////////////////////////////////////////////////////////////
	//	// if no enemy, then what are we doing here?
	//	if(!self->enemy)
	//		return;
	//
	//	// always favor the railgun
	//	if(KOOGLEIT_ChangeWeapon(self,FindItem("Bazooka")))
	//		return;
	//
	//	// Base selection on distance.
	//	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	//	range = VectorLength(v);
	//
	//	// Longer range
	//	if(range > 300)
	//	{
	/*//		// choose BFG if enough ammo
	//		if(self->client->pers.inventory[ITEMLIST_CELLS] > 50)
	//			if(KOOGLEAI_CheckShot(self) && KOOGLEIT_ChangeWeapon(self, FindItem("bfg10k")))
	//				return;*/
	//
	//		if(KOOGLEAI_CheckShot(self) && KOOGLEIT_ChangeWeapon(self,FindItem("Bazooka")))
	//			return;
	//	}
	//
	//	// Only use GL in certain ranges and only on targets at or below our level
	//	if(range > 100 && range < 500 && self->enemy->s.origin[2] - 20 < self->s.origin[2])
	//		if(KOOGLEIT_ChangeWeapon(self,FindItem("Grenade Launcher")))
	//			return;
	//
	//	// Only use HMG when ammo > 30
	//	if(self->client->pers.inventory[ITEMLIST_BULLETS] >= 30)
	//		if(KOOGLEIT_ChangeWeapon(self,FindItem("Heavy machinegun")))
	//			return;

	//	if(KOOGLEIT_ChangeWeapon(self,FindItem("Tommygun")))
	//		return;

	//	// Only use FT at short range and only on targets at or below our level
	//	if(range < 128 && self->enemy->s.origin[2] - 20 < self->s.origin[2])
	//		if(KOOGLEIT_ChangeWeapon(self,FindItem("Grenade Launcher")))
	//			return;
	//
	//	if(KOOGLEIT_ChangeWeapon(self,FindItem("Shotgun")))
	//		return;
	//
	//	if(KOOGLEIT_ChangeWeapon(self,FindItem("Pistol")))
	// 	   return;

	//	if(KOOGLEIT_ChangeWeapon(self,FindItem("Crowbar")))
	// 	   return;

	//	if(KOOGLEIT_ChangeWeapon(self,FindItem("Pipe")))
	//  	   return;

	//	return;
}