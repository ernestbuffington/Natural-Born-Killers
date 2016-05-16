///////////////////////////////////////////////////////////////////////	
//
//  kooglebot_nodes.c -   This file contains all of the 
//                     pathing routines for the KOOGLE bot.
// 
///////////////////////////////////////////////////////////////////////
#include "..\g_local.h"
#include "kooglebot.h"

// flags
qboolean newmap = true;

// Total number of nodes that are items
int numitemnodes;

// Total number of nodes
int numnodes;

// For debugging paths
int show_path_from = -1;
int show_path_to = -1;

// array for node data
botnode_t nodes[MAX_BOTNODES];
short int path_table[MAX_BOTNODES][MAX_BOTNODES];

///////////////////////////////////////////////////////////////////////
// NODE INFORMATION FUNCTIONS
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Determine cost of moving from one node to another
///////////////////////////////////////////////////////////////////////
int KOOGLEND_FindCost(short int from,short int to)
{
	short int curnode; //fixed by hypo, use to just say int curnode; WAS CAUSING A CRASH
	int cost = 1; // Shortest possible is 1

				  // If we can not get there then return invalid
	if (path_table[from][to] == INVALID)
		return INVALID;

	// Otherwise check the path and return the cost
	curnode = path_table[from][to];

	// Find a path (linear time, very fast)
	while (curnode != to)
	{
		curnode = path_table[curnode][to];
		if (curnode == INVALID) // something has corrupted the path abort
			return INVALID;
		cost++;
	}

	return cost;
}

///////////////////////////////////////////////////////////////////////
// Find a close node to the player within dist.
//
// Faster than looking for the closest node, but not very 
// accurate.
///////////////////////////////////////////////////////////////////////
int KOOGLEND_FindCloseReachableNode(edict_t *self, int range, int type)
{
	vec3_t v;
	int i;
	trace_t tr;
	float dist;

	range *= range;

	for (i = 0; i<numnodes; i++)
	{
		if (type == BOTNODE_ALL || type == nodes[i].type) // check node type
		{

			VectorSubtract(nodes[i].origin, self->s.origin, v); // subtract first

			dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

			if (dist < range) // square range instead of sqrt
			{
				// KOOGLEBOT_ADD added 5/13/2016 by Ghost 
				// Hack to keep bots from spinning and trying to find nodes through alpha bridges! 
				// make sure it is visible
				tr = gi.trace(self->s.origin, self->mins, self->maxs, nodes[i].origin, self, MASK_ALPHA);
				if (tr.fraction == 1.0)
					continue; 
				// KOOGLEBOT_END added 5/13/2016 by Ghost

				//trace = gi.trace (self->s.origin, vec3_origin, vec3_origin, nodes[i].origin, self, MASK_OPAQUE);
				tr = gi.trace(self->s.origin, self->mins, self->maxs, nodes[i].origin, self, MASK_OPAQUE);
				if (tr.fraction == 1.0)
					return i;
			}
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////
// Find the closest node to the player within a certain range
///////////////////////////////////////////////////////////////////////
int KOOGLEND_FindClosestReachableNode(edict_t *self, int range, int type)
{
	int i;
	float closest = 99999;
	float dist;
	int node = -1;
	vec3_t v;
	trace_t tr, zr;
	float rng;
	vec3_t maxs, mins;

	VectorCopy(self->mins, mins);
	VectorCopy(self->maxs, maxs);

	// For Ladders, do not worry so much about reachability
	if (type == BOTNODE_LADDER)
	{
		VectorCopy(vec3_origin, maxs);
		VectorCopy(vec3_origin, mins);
	}
	else
		mins[2] += 18; // Stepsize

	rng = (float)(range * range); // square range for distance comparison (eliminate sqrt)	

	for (i = 0; i<numnodes; i++)
	{
		if (type == BOTNODE_ALL || type == nodes[i].type) // check node type
		{
			VectorSubtract(nodes[i].origin, self->s.origin, v); // subtract first

			dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

			if (dist < closest && dist < rng)
			{
				// make sure it is visible
				zr = gi.trace(self->s.origin, mins, maxs, nodes[i].origin, self, MASK_ALPHA);
				tr = gi.trace(self->s.origin, mins, maxs, nodes[i].origin, self, MASK_OPAQUE);
				if ((tr.fraction == 1.0) || (zr.fraction == 1.0))
				{
					node = i;
					closest = dist;
				}
			}
		}
	}

	return node;
}

///////////////////////////////////////////////////////////////////////
// BOT NAVIGATION ROUTINES
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Set up the goal
///////////////////////////////////////////////////////////////////////
void KOOGLEND_SetGoal(edict_t *self, int goal_node)
{
	int node;

	self->goal_node = goal_node;
	node = KOOGLEND_FindClosestReachableNode(self, BOTNODE_DENSITY * 3, BOTNODE_ALL);

	if (node == -1)
		return;

	if (debug_mode)
		debug_printf("%s new start node selected %d\n", self->client->pers.netname, node);

	self->current_node = node;
	self->next_node = self->current_node; // make sure we get to the nearest node first
	self->node_timeout = 0;
}

///////////////////////////////////////////////////////////////////////
// Move closer to goal by pointing the bot to the next node
// that is closer to the goal
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEND_FollowPath(edict_t *self)
{
	vec3_t v;

	//////////////////////////////////////////
	//  Show the path (uncomment for debugging)
	//	show_path_from = self->current_node;
	//	show_path_to = self->goal_node;
	//	KOOGLEND_DrawPath();
	//////////////////////////////////////////

	// Try again?
	if (self->node_timeout++ > 30)
	{
		if (self->tries++ > 3)
			return false;
		else
			KOOGLEND_SetGoal(self, self->goal_node);
	}

	// Are we there yet?
	VectorSubtract(self->s.origin, nodes[self->next_node].origin, v);

	if (VectorLength(v) < 32)
	{
		// reset timeout
		self->node_timeout = 0;

		if (self->next_node == self->goal_node)
		{
			if (debug_mode)
				debug_printf("%s reached goal!\n", self->client->pers.netname);

			KOOGLEAI_PickLongRangeGoal(self); // Pick a new goal
		}
		else
		{
			self->current_node = self->next_node;
			self->next_node = path_table[self->current_node][self->goal_node];
		}
	}

	if (self->current_node == -1 || self->next_node == -1)
		return false;

	// Set bot's movement vector
	VectorSubtract(nodes[self->next_node].origin, self->s.origin, self->move_vector);

	return true;
}


///////////////////////////////////////////////////////////////////////
// MAPPING CODE
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Capture when the grappling hook has been fired for mapping purposes.
///////////////////////////////////////////////////////////////////////
void KOOGLEND_GrapFired(edict_t *self)
{
	/*	int closest_node;

	if(!self->owner)
	return; // should not be here

	// Check to see if the grapple is in pull mode
	if(self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
	{
	// Look for the closest node of type grapple
	closest_node = KOOGLEND_FindClosestReachableNode(self,BOTNODE_DENSITY,BOTNODE_GRAPPLE);
	if(closest_node == -1 ) // we need to drop a node
	{
	closest_node = KOOGLEND_AddNode(self,BOTNODE_GRAPPLE);

	// Add an edge
	KOOGLEND_UpdateNodeEdge(self->owner->last_node,closest_node);

	self->owner->last_node = closest_node;
	}
	else
	self->owner->last_node = closest_node; // zero out so other nodes will not be linked
	}*/
}


///////////////////////////////////////////////////////////////////////
// Check for adding ladder nodes
///////////////////////////////////////////////////////////////////////
qboolean KOOGLEND_CheckForLadder(edict_t *self)
{
	int closest_node;

	// If there is a ladder and we are moving up, see if we should add a ladder node
	if (gi.pointcontents(self->s.origin) & CONTENTS_LADDER && self->velocity[2] > 0)
	{
		//debug_printf("contents: %x\n",tr.contents);

		closest_node = KOOGLEND_FindClosestReachableNode(self, BOTNODE_DENSITY, BOTNODE_LADDER);
		if (closest_node == -1)
		{
			closest_node = KOOGLEND_AddNode(self, BOTNODE_LADDER);

			// Now add link
			KOOGLEND_UpdateNodeEdge(self->last_node, closest_node);

			// Set current to last
			self->last_node = closest_node;
		}
		else
		{
			KOOGLEND_UpdateNodeEdge(self->last_node, closest_node);
			self->last_node = closest_node; // set visited to last
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////
// This routine is called to hook in the pathing code and sets
// the current node if valid.
///////////////////////////////////////////////////////////////////////
void KOOGLEND_PathMap(edict_t *self)
{
	int closest_node;
	static float last_update = 0; // start off low
	vec3_t v;

	if (level.time < last_update)
		return;

	last_update = level.time + 0.15; // slow down updates a bit

									 // Special node drawing code for debugging
	if (show_path_to != -1)
		KOOGLEND_DrawPath();

	////////////////////////////////////////////////////////
	// Special check for ladder nodes
	///////////////////////////////////////////////////////
	if (KOOGLEND_CheckForLadder(self)) // check for ladder nodes
		return;

	// Not on ground, and not in the water, so bail
	if (!self->groundentity && !self->waterlevel)
		return;

	////////////////////////////////////////////////////////
	// Lava/Slime
	////////////////////////////////////////////////////////
	VectorCopy(self->s.origin, v);
	v[2] -= 18;
	if (gi.pointcontents(v) & (CONTENTS_LAVA | CONTENTS_SLIME))
		return; // no nodes in slime

	// added to stop spinning under alpha bridges - does not seem to werk
	//VectorCopy(self->s.origin, v);
	//v[2] -= 18;
	//if (gi.pointcontents(v) & (MASK_ALL))
	//	return; // no nodes in slime

	////////////////////////////////////////////////////////
	// Jumping
	////////////////////////////////////////////////////////
	if (self->is_jumping == true)
	{
		// See if there is a close by jump landing node (prevent adding too many)
		closest_node = KOOGLEND_FindClosestReachableNode(self, 24, BOTNODE_JUMP);

		if (closest_node == INVALID)
			closest_node = KOOGLEND_AddNode(self, BOTNODE_JUMP);

		// Now add link
		if (self->last_node != -1)
			KOOGLEND_UpdateNodeEdge(self->last_node, closest_node);
		//self->is_jumping = false;
		return;
	}

	/*
	////////////////////////////////////////////////////////////
	// Grapple
	// Do not add nodes during grapple, added elsewhere manually
	////////////////////////////////////////////////////////////
	if(ctf->value && self->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
	return;*/

	// Iterate through all nodes to make sure far enough apart
	closest_node = KOOGLEND_FindClosestReachableNode(self, BOTNODE_DENSITY, BOTNODE_ALL);

	////////////////////////////////////////////////////////
	// Special Check for Platforms
	////////////////////////////////////////////////////////
	if (self->groundentity && self->groundentity->use == Use_Plat)
	{
		if (closest_node == INVALID)
			return; // Do not want to do anything here.

					// Here we want to add links
		if (closest_node != self->last_node && self->last_node != INVALID)
			KOOGLEND_UpdateNodeEdge(self->last_node, closest_node);

		self->last_node = closest_node; // set visited to last
		return;
	}

	////////////////////////////////////////////////////////
	// Add Nodes as needed
	////////////////////////////////////////////////////////
	if (closest_node == INVALID)
	{
		// Add nodes in the water as needed
		if (self->waterlevel)
			closest_node = KOOGLEND_AddNode(self, BOTNODE_WATER);
		else
		{
			// KOOGLEBOT_ADD added 4/29/2016 by Ghost
			if (self->update_node == BOTNODE_MOVE)
			   closest_node = KOOGLEND_AddNode(self, BOTNODE_MOVE);
			// KOOGLEBOT_END added 4/29/2016 by Ghost
			else
			// KOOGLEBOT_ADD added 4/29/2016 by Ghost
			if (self->update_node == BOTNODE_LADDER)
				closest_node = KOOGLEND_AddNode(self, BOTNODE_LADDER); 
			// KOOGLEBOT_END added 4/29/2016 by Ghost
			else
			// KOOGLEBOT_ADD added 4/29/2016 by Ghost
			if (self->update_node == BOTNODE_CROUCH)
				closest_node = KOOGLEND_AddNode(self, BOTNODE_CROUCH); 
			// KOOGLEBOT_END added 4/29/2016 by Ghost
			else
			// KOOGLEBOT_ADD added 4/29/2016 by Ghost
			if (self->update_node == BOTNODE_JUMP)
				closest_node = KOOGLEND_AddNode(self, BOTNODE_JUMP); 
			// KOOGLEBOT_END added 4/29/2016 by Ghost
		}
		// Now add link
			if (self->last_node != -1)
				KOOGLEND_UpdateNodeEdge(self->last_node, closest_node);
	}
	else if (closest_node != self->last_node && self->last_node != INVALID)
		KOOGLEND_UpdateNodeEdge(self->last_node, closest_node);

	self->last_node = closest_node; // set visited to last

}

///////////////////////////////////////////////////////////////////////
// Init node array (set all to INVALID)
///////////////////////////////////////////////////////////////////////
void KOOGLEND_InitNodes(void)
{
	numnodes = 1;
	numitemnodes = 1;
	memset(nodes, 0, sizeof(botnode_t) * MAX_BOTNODES);
	memset(path_table, INVALID, sizeof(short int)*MAX_BOTNODES*MAX_BOTNODES);

}

///////////////////////////////////////////////////////////////////////
// Show the node for debugging (utility function)
///////////////////////////////////////////////////////////////////////
void KOOGLEND_ShowNode(int node)
{
	edict_t *ent;

	return; // commented out for now. uncomment to show nodes during debugging,
			// but too many will cause overflows. You have been warned.

	ent = G_Spawn();

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if (nodes[node].type == BOTNODE_MOVE)
		ent->s.renderfx = RF_SHELL_BLUE;
	else if (nodes[node].type == BOTNODE_WATER)
		ent->s.renderfx = RF_SHELL_RED;
	else
		ent->s.renderfx = RF_SHELL_GREEN; // action nodes

	ent->s.modelindex = gi.modelindex("models/items/ammo/grenades/medium/tris.md2");
	ent->owner = ent;
	ent->nextthink = level.time + 200000.0;
	ent->think = G_FreeEdict;
	ent->dmg = 0;

	VectorCopy(nodes[node].origin, ent->s.origin);
	gi.linkentity(ent);

}

///////////////////////////////////////////////////////////////////////
// Draws the current path (utility function)
///////////////////////////////////////////////////////////////////////
void KOOGLEND_DrawPath()
{
	int current_node, goal_node, next_node;

	current_node = show_path_from;
	goal_node = show_path_to;

	next_node = path_table[current_node][goal_node];

	// Now set up and display the path
	while (current_node != goal_node && current_node != -1)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(nodes[current_node].origin);
		gi.WritePosition(nodes[next_node].origin);
		gi.multicast(nodes[current_node].origin, MULTICAST_PVS);
		current_node = next_node;
		next_node = path_table[current_node][goal_node];
	}
}

///////////////////////////////////////////////////////////////////////
// Turns on showing of the path, set goal to -1 to 
// shut off. (utility function)
///////////////////////////////////////////////////////////////////////
void KOOGLEND_ShowPath(edict_t *self, int goal_node)
{
	show_path_from = KOOGLEND_FindClosestReachableNode(self, BOTNODE_DENSITY, BOTNODE_ALL);
	show_path_to = goal_node;
}

///////////////////////////////////////////////////////////////////////
// Add a node of type ?
///////////////////////////////////////////////////////////////////////
int KOOGLEND_AddNode(edict_t *self, int type)
{
	vec3_t v1, v2;

	// Block if we exceed maximum
	if (numnodes + 1 > MAX_BOTNODES)
		return false;

	// Set location
	VectorCopy(self->s.origin, nodes[numnodes].origin);

	// Set type
	nodes[numnodes].type = type;

	/////////////////////////////////////////////////////
	// ITEMS
	// Move the z location up just a bit.
	if (type == BOTNODE_ITEM)
	{
		nodes[numnodes].origin[2] += 16;
		numitemnodes++;
	}

	if (type == BOTNODE_CROUCH)
	{
		nodes[numnodes].origin[2] += 16;
		numitemnodes++;
	}


	if (type == BOTNODE_JUMP)
	{
		nodes[numnodes].origin[2] += 64;
		numitemnodes++;
	}

	// Teleporters
	if (type == BOTNODE_TELEPORTER)
	{
		// Up 32
		nodes[numnodes].origin[2] += 32;
	}

	if (type == BOTNODE_LADDER)
	{
		nodes[numnodes].type = BOTNODE_LADDER;

		if (debug_mode)
		{
			debug_printf("Node added %d type: Ladder\n", numnodes);
			KOOGLEND_ShowNode(numnodes);
		}

		numnodes++;
		return numnodes - 1; // return the node added

	}

	// For platforms drop two nodes one at top, one at bottom
	if (type == BOTNODE_PLATFORM)
	{
		VectorCopy(self->maxs, v1);
		VectorCopy(self->mins, v2);

		// To get the center
		nodes[numnodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
		nodes[numnodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
		nodes[numnodes].origin[2] = self->maxs[2];

		if (debug_mode)
			KOOGLEND_ShowNode(numnodes);

		numnodes++;

		nodes[numnodes].origin[0] = nodes[numnodes - 1].origin[0];
		nodes[numnodes].origin[1] = nodes[numnodes - 1].origin[1];
		nodes[numnodes].origin[2] = self->mins[2] + 64;

		nodes[numnodes].type = BOTNODE_PLATFORM;

		// Add a link
		KOOGLEND_UpdateNodeEdge(numnodes, numnodes - 1);

		if (debug_mode)
		{
			debug_printf("Node added %d type: Platform\n", numnodes);
			KOOGLEND_ShowNode(numnodes);
		}

		numnodes++;

		return numnodes - 1;
	}

	if (debug_mode)
	{
		if (nodes[numnodes].type == BOTNODE_MOVE)
			debug_printf("Node added %d type: Move\n", numnodes);
		else if (nodes[numnodes].type == BOTNODE_TELEPORTER)
			debug_printf("Node added %d type: Teleporter\n", numnodes);
		else if (nodes[numnodes].type == BOTNODE_ITEM)
			debug_printf("Node added %d type: Item\n", numnodes);
		else if (nodes[numnodes].type == BOTNODE_CROUCH)
			debug_printf("Node added %d type: crouch\n", numnodes);
		else if (nodes[numnodes].type == BOTNODE_JUMP)
			debug_printf("Node added %d type: jump\n", numnodes);
		else if (nodes[numnodes].type == BOTNODE_WATER)
			debug_printf("Node added %d type: Water\n", numnodes);
		/*		else if(nodes[numnodes].type == BOTNODE_GRAPPLE)
		debug_printf("Node added %d type: Grapple\n",numnodes);*/

		KOOGLEND_ShowNode(numnodes);
	}

	numnodes++;

	return numnodes - 1; // return the node added
}

///////////////////////////////////////////////////////////////////////
// Add/Update node connections (paths)
///////////////////////////////////////////////////////////////////////
void KOOGLEND_UpdateNodeEdge(int from, int to)
{
	int i;

	if (from == -1 || to == -1 || from == to)
		return; // safety

				// Add the link
	path_table[from][to] = to;

	// Now for the self-referencing part, linear time for each link added
	for (i = 0; i<numnodes; i++)
		if (path_table[i][from] != INVALID)
			if (i == to)
				path_table[i][to] = INVALID; // make sure we terminate
			else
				path_table[i][to] = path_table[i][from];

	if (debug_mode)
		debug_printf("Link %d -> %d\n", from, to);
}

///////////////////////////////////////////////////////////////////////
// Remove a node edge
///////////////////////////////////////////////////////////////////////
void KOOGLEND_RemoveNodeEdge(edict_t *self, int from, int to)
{
	int i;

	if (debug_mode)
		debug_printf("%s: Removing Edge %d -> %d\n", self->client->pers.netname, from, to);

	path_table[from][to] = INVALID; // set to invalid			

									// Make sure this gets updated in our path array
	for (i = 0; i<numnodes; i++)
		if (path_table[from][i] == to)
			path_table[from][i] = INVALID;
}

///////////////////////////////////////////////////////////////////////
// This function will resolve all paths that are incomplete
// usually called before saving to disk
///////////////////////////////////////////////////////////////////////
void KOOGLEND_ResolveAllPaths()
{
	int i, from, to;
	int num = 0;
	gi.dprintf("Resolving all paths...");
	for (from = 0; from<numnodes; from++)
	for (to = 0; to<numnodes; to++)
	{
	 // update unresolved paths
	 // Not equal to itself, not equal to -1 and equal to the last link
	  if (from != to && path_table[from][to] == to)
	  {
	   num++;
	   // Now for the self-referencing part linear time for each link added
	   for (i = 0; i<numnodes; i++)
	   if (path_table[i][from] != -1)
	   if (i == to)
	   path_table[i][to] = -1; // make sure we terminate
	   else
	   path_table[i][to] = path_table[i][from];
	  }
	}
	gi.dprintf("done (%d updated)\n", num);
}

///////////////////////////////////////////////////////////////////////
// Save to disk file
//
// Since my compression routines are one thing I did not want to
// release, I took out the compressed format option. Most levels will
// save out to a node file around 50-200k, so compression is not really
// a big deal.
///////////////////////////////////////////////////////////////////////
void KOOGLEND_SaveNodes()
{
	FILE *pOut;
	char filename[60];
	int i, j;
	int version = 1;
	// Resolve paths
	KOOGLEND_ResolveAllPaths();
	gi.dprintf("Saving node table...");
	strcpy(filename, MODDIR"\\"BOTDIR"\\");
	strcat(filename, level.mapname);
	strcat(filename, ".nav");
	if ((pOut = fopen(filename, "wb")) == NULL)
	return; // bail
	fwrite(&version, sizeof(int), 1, pOut); // write version
	fwrite(&numnodes, sizeof(int), 1, pOut); // write count
	fwrite(&num_items, sizeof(int), 1, pOut); // write facts count
	fwrite(nodes, sizeof(botnode_t), numnodes, pOut); // write nodes
	for (i = 0; i<numnodes; i++)
	for (j = 0; j<numnodes; j++)
	fwrite(&path_table[i][j], sizeof(short int), 1, pOut); // write count
	fwrite(item_table, sizeof(item_table_t), num_items, pOut); 		// write out the fact table
	fclose(pOut);
	gi.dprintf("done.\n");
}

///////////////////////////////////////////////////////////////////////
// Read from disk file
///////////////////////////////////////////////////////////////////////
void KOOGLEND_LoadNodes(void)
{
	FILE *pIn;
	int i, j;
	char filename[60];
	int version;

	strcpy(filename, MODDIR"\\"BOTDIR"\\");
	strcat(filename, level.mapname);
	strcat(filename, ".nav");

	if ((pIn = fopen(filename, "rb")) == NULL)
	{
		// Create item table
		//safe_bprintf(PRINT_MEDIUM, "[ No node file found, creating a new one!               ]");
		KOOGLEIT_BuildItemNodeTable(false);
		//safe_bprintf(PRINT_MEDIUM, " ...done!\n");
		return;
	}

	// determine version
	fread(&version, sizeof(int), 1, pIn); // read version

	if (version == 1)
	{
		//safe_bprintf(PRINT_MEDIUM, "[ Loading Node Table!");

		fread(&numnodes, sizeof(int), 1, pIn); // read count
		fread(&num_items, sizeof(int), 1, pIn); // read facts count

		fread(nodes, sizeof(botnode_t), numnodes, pIn);

		for (i = 0; i<numnodes; i++)
			for (j = 0; j<numnodes; j++)
				fread(&path_table[i][j], sizeof(short int), 1, pIn); // write count

		fread(item_table, sizeof(item_table_t), num_items, pIn);
		fclose(pIn);
	}
	else
	{
		// Create item table
		//safe_bprintf(PRINT_MEDIUM, "[ No node file found, creating a new one!");
		KOOGLEIT_BuildItemNodeTable(false);
		//safe_bprintf(PRINT_MEDIUM, "...done!\n");
		return; // bail
	}

	//safe_bprintf(PRINT_MEDIUM, "...done!\n");

	KOOGLEIT_BuildItemNodeTable(true);

}

