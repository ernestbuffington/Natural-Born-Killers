///////////////////////////////////////////////////////////////////////
//
//  kooglebot_spawn.c - This file contains all of the 
//                   spawing support routines for the KOOGLE bot.
//
///////////////////////////////////////////////////////////////////////
#include "..\g_local.h"
#include "..\m_player.h"
#include "..\mq2_player.h"
#include "kooglebot.h"
#include "..\voice_punk.h"
///////////////////////////////////////////////////////////////////////
// Had to add this function in this version for some reason.
// any globals are wiped out between level changes....so
// save the bots out to a file. 
//
// NOTE: There is still a bug when the bots are saved for
//       a dm game and then reloaded into a CTF game.
///////////////////////////////////////////////////////////////////////
void KOOGLESP_SaveBots()
{
	edict_t *bot;
	FILE *pOut;
	int i, count = 0;

	if ((pOut = fopen("nbk\\kooglebot\\bots.tmp", "wb")) == NULL)
		return; // bail

	// Get number of bots
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (bot->inuse && bot->is_bot)
			count++;
	}

	fwrite(&count, sizeof(int), 1, pOut); // Write number of bots

	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (bot->inuse && bot->is_bot)
			fwrite(bot->client->pers.userinfo, sizeof(char) * MAX_INFO_STRING, 1, pOut);
	}

	fclose(pOut);
}

///////////////////////////////////////////////////////////////////////
// Had to add this function in this version for some reason.
// any globals are wiped out between level changes....so
// load the bots from a file.
//
// Side effect/benifit are that the bots persist between games.
///////////////////////////////////////////////////////////////////////
void KOOGLESP_LoadBots()
{
	FILE *pIn;
	char userinfo[MAX_INFO_STRING];
	int i, count;

	if ((pIn = fopen("nbk\\kooglebot\\bots.tmp", "rb")) == NULL)
		return; // bail

	fread(&count, sizeof(int), 1, pIn);

	for (i = 0; i<count; i++)
	{
		fread(userinfo, sizeof(char) * MAX_INFO_STRING, 1, pIn);
		KOOGLESP_SpawnBot(NULL, NULL, NULL, userinfo);
	}

	fclose(pIn);
}

void Cmd_Join_f(edict_t *self, char *teamcmd);
///////////////////////////////////////////////////////////////////////
// Called by PutClient in Server to actually release the bot into the game
// Keep from killing each other when all spawned at once
///////////////////////////////////////////////////////////////////////
void KOOGLESP_HoldSpawn(edict_t *self)
{
	if (!KillBox(self))
	{	// couldn't spawn in?
	}

	gi.linkentity(self);

	self->think = KOOGLEAI_Think;
	self->nextthink = level.time + FRAMETIME;

	// send effect
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_LOGIN);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	
	//added koogle	
	if (teamplay->value != 0)
	{
		self->client->pers.team = TEAM_2;
	}
	else
		self->client->pers.team = 0;

	//added koogle	
	//	Cmd_Join_f(self,TEAM_2 );
	//added koogle
	//	else
	//		safe_bprintf (PRINT_MEDIUM, "%s entered the game\n", self->client->pers.netname);
}


///////////////////////////////////////////////////////////////////////
// Modified version of id's code
///////////////////////////////////////////////////////////////////////
void KOOGLESP_PutClientInServer(edict_t *bot, qboolean respawn, int team)
{
	vec3_t	mins = { -16, -16, -24 };
	vec3_t	maxs = { 16, 16, 32 };
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	char *s;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint(bot, spawn_origin, spawn_angles);

	index = bot - g_edicts - 1;
	client = bot->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING];

		resp = bot->client->resp;
		
		memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
		
		InitClientPersistant(client);
		
		ClientUserinfoChanged(bot, userinfo);
	}
	else
		memset(&resp, 0, sizeof(resp));

	bot->name_index = -1;

	// clear everything but the Persistant data
	saved = client->pers;

	memset(client, 0, sizeof(*client));

	client->pers = saved;

	if (client->pers.health <= 0)
	InitClientPersistant(client);

	client->resp = resp;

	//copy some data from the client to the entity
	FetchClientEntData(bot);

	// clear entity values
	bot->groundentity = NULL;
	bot->client = &game.clients[index];
	bot->takedamage = DAMAGE_AIM;
	bot->movetype = MOVETYPE_WALK;
	bot->viewheight = 40;
	bot->inuse = true;
	bot->classname = "bot";
	bot->mass = 200;
	bot->solid = SOLID_BBOX;
	bot->deadflag = DEAD_NO;
	bot->air_finished = level.time + 12;
	bot->clipmask = MASK_PLAYERSOLID;
	bot->pain = player_pain;
	bot->die = player_die;
	bot->waterlevel = 0;
	bot->watertype = 0;
	bot->flags &= ~FL_NO_KNOCKBACK;
	bot->svflags &= ~(SVF_DEADMONSTER | SVF_NOCLIENT);
	
	bot->client->pers.version = 121; //added for FREDZ kingpin.exe check by GHost 4/21/2016

	bot->is_jumping = false;

	/*	if(ctf->value)
	{
	client->resp.ctf_team = team;
	client->resp.ctf_state = CTF_BOTSTATE_START;
	s = Info_ValueForKey (client->pers.userinfo, "skin");
	CTFAssignSkin(bot, s);
	}*/
	//KP_ADD

	//added koogle	
	if(teamplay->value != 0)
	{
	   bot->client->pers.team = TEAM_2;
	}
	else
	   bot->client->pers.team = 0;
	//added koogle

	bot->s.renderfx2 = 0;
	bot->onfiretime = 0;
	bot->cast_info.aiflags |= AI_GOAL_RUN;	// make AI run towards us if in pursuit

	VectorCopy(mins, bot->mins);
	VectorCopy(maxs, bot->maxs);
	VectorClear(bot->velocity);

	bot->cast_info.standing_max_z = bot->maxs[2];
	bot->cast_info.scale = MODEL_SCALE;
	bot->s.scale = bot->cast_info.scale - 1.0;

	// clear playerstate values
	memset(&bot->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0] * 8;
	client->ps.pmove.origin[1] = spawn_origin[1] * 8;
	client->ps.pmove.origin[2] = spawn_origin[2] * 8;

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	// weapon mdx
	{
		int i;

		memset(&(client->ps.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);

		client->ps.num_parts++;

		if (client->pers.weapon)
			client->ps.model_parts[PART_HEAD].modelindex = gi.modelindex(client->pers.weapon->view_model);

		for (i = 0; i<MAX_MODELPART_OBJECTS; i++)
			client->ps.model_parts[PART_HEAD].skinnum[i] = 0; // will we have more than one skin???
	}

	if (client->pers.weapon)
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	bot->s.effects = 0;
	bot->s.skinnum = bot - g_edicts - 1;
	bot->s.modelindex = 255;		// will use the skin specified model
									// KINGPIN_X	bot->s.modelindex2 = 255;		// custom gun model
	bot->s.frame = 0;
	VectorCopy(spawn_origin, bot->s.origin);
	bot->s.origin[2] += 1;	// make sure off ground

	VectorCopy(bot->s.origin, bot->s.old_origin);

	// bikestuff
	bot->biketime = 0;
	bot->bikestate = 0;

	// Hovercars
	if (g_vehicle_test->value)
	{
		if (g_vehicle_test->value == 3)
			bot->s.modelindex = gi.modelindex("models/props/moto/moto.mdx");
		else
			bot->s.modelindex = gi.modelindex("models/vehicles/cars/viper/tris_test.md2");

		//ent->s.modelindex2 = 0;
		bot->s.skinnum = 0;
		bot->s.frame = 0;

		if ((int)g_vehicle_test->value == 1)
			bot->flags |= FL_HOVERCAR_GROUND;
		else if ((int)g_vehicle_test->value == 2)
			bot->flags |= FL_HOVERCAR;
		else if ((int)g_vehicle_test->value == 3)
			bot->flags |= FL_BIKE;
		else if ((int)g_vehicle_test->value == 4)
			bot->flags |= FL_CAR;
	}
	else if (dm_locational_damage->value)	// deathmatch, note models must exist on server for client's to use them, 
											// but if the server has a model a client doesn't that client will see the default male model
	{
		char	modeldir[MAX_QPATH]; //, *skins;
		int		len;
		int		did_slash;
		char	modelname[MAX_QPATH];
		//		int		skin;

		// NOTE: this is just here for collision detection, modelindex's aren't actually set
		bot->s.num_parts = 0;		// so the client's setup the model for viewing

		s = Info_ValueForKey(client->pers.userinfo, "skin");

		//skins = strstr( s, "/" ) + 1;

		// converts some characters to NULL's
		len = strlen(s);
		did_slash = 0;
		
		for (i = 0; i<len; i++)
		{
			if (s[i] == '/')
			{
				s[i] = '\0';
				did_slash = true;
			}
			else if (s[i] == ' ' && did_slash)
			{
				s[i] = '\0';
			}
		}

		if (strlen(s) > MAX_QPATH - 1)
			s[MAX_QPATH - 1] = '\0';

		strcpy(modeldir, s);

		if (strlen(modeldir) < 1)
			strcpy(modeldir, "male_thug"); //changed from thug

		memset(&(bot->s.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);

		bot->s.num_parts++;
		strcpy(modelname, "players/");
		strcat(modelname, modeldir);
		strcat(modelname, "/head.mdx");
		bot->s.model_parts[bot->s.num_parts - 1].modelindex = 255;
		gi.GetObjectBounds(modelname, &bot->s.model_parts[bot->s.num_parts - 1]);
		if (!bot->s.model_parts[bot->s.num_parts - 1].object_bounds[0])
			gi.GetObjectBounds("players/male_thug/head.mdx", &bot->s.model_parts[bot->s.num_parts - 1]);

		bot->s.num_parts++;
		strcpy(modelname, "players/");
		strcat(modelname, modeldir);
		strcat(modelname, "/legs.mdx");
		bot->s.model_parts[bot->s.num_parts - 1].modelindex = 255;
		gi.GetObjectBounds(modelname, &bot->s.model_parts[bot->s.num_parts - 1]);
		if (!bot->s.model_parts[bot->s.num_parts - 1].object_bounds[0])
			gi.GetObjectBounds("players/male_thug/legs.mdx", &bot->s.model_parts[bot->s.num_parts - 1]);

		bot->s.num_parts++;
		strcpy(modelname, "players/");
		strcat(modelname, modeldir);
		strcat(modelname, "/body.mdx");
		bot->s.model_parts[bot->s.num_parts - 1].modelindex = 255;
		gi.GetObjectBounds(modelname, &bot->s.model_parts[bot->s.num_parts - 1]);
		if (!bot->s.model_parts[bot->s.num_parts - 1].object_bounds[0])
			gi.GetObjectBounds("players/male_thug/body.mdx", &bot->s.model_parts[bot->s.num_parts - 1]);

		bot->s.num_parts++;
		bot->s.model_parts[PART_GUN].modelindex = 255;
	}
	else	// make sure we can see their weapon
	{
		memset(&(bot->s.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);
		bot->s.model_parts[PART_GUN].modelindex = 255;
		bot->s.num_parts = PART_GUN + 1;	// make sure old clients recieve the view weapon index
	}
	// set the delta angle
	for (i = 0; i<3; i++)
	client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	bot->s.angles[PITCH] = 0;
	bot->s.angles[YAW] = spawn_angles[YAW];
	bot->s.angles[ROLL] = 0;
	VectorCopy(bot->s.angles, client->ps.viewangles);
	VectorCopy(bot->s.angles, client->v_angle);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon(bot);

	bot->enemy = NULL;
	bot->movetarget = NULL;
	bot->state = BOTSTATE_MOVE;

	// Set the current node
	bot->current_node = KOOGLEND_FindClosestReachableNode(bot, BOTNODE_DENSITY, BOTNODE_ALL);
	bot->goal_node = bot->current_node;
	bot->next_node = bot->current_node;
	bot->next_move_time = level.time;
	bot->suicide_timeout = level.time + 15.0;

	// If we are not respawning hold off for up to three seconds before releasing into game
	if(!respawn)
	{
	  bot->think = KOOGLESP_HoldSpawn;
	  bot->nextthink = level.time + 0.1;
	  bot->nextthink = level.time + random()*3.0; // up to three seconds
	}
	else
	{
		if (!KillBox(bot))
		{	// couldn't spawn in?
		}

		gi.linkentity(bot);

		bot->think = KOOGLEAI_Think;
		bot->nextthink = level.time + FRAMETIME;
		
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (bot-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (bot->s.origin, MULTICAST_PVS);
	}

}

///////////////////////////////////////////////////////////////////////
// Respawn the bot
///////////////////////////////////////////////////////////////////////
void KOOGLESP_Respawn(edict_t *self)
{
	CopyToBodyQue(self);

	// locate ent at a spawn point
	if (teamplay->value != 0)
	{
		KOOGLESP_PutClientInServer(self, false, TEAM_2);
	}
	else
		KOOGLESP_PutClientInServer(self, false, 0);

	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;

	// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;
	self->client->respawn_time = level.time;
}

///////////////////////////////////////////////////////////////////////
// Find a free client spot
///////////////////////////////////////////////////////////////////////
edict_t *KOOGLESP_FindFreeClient(void)
{
	edict_t *bot;
	int	i;
	int max_count = 0;

	// This is for the naming of the bots
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (bot->count > max_count)
			max_count = bot->count;
	}

	// Check for free spot
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}

	bot->count = max_count + 1; // Will become bot name...

	if (bot->inuse)
		bot = NULL;

	return bot;
}

///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
void KOOGLESP_SetName(edict_t *bot, char *name, char *skin, char *team)
{
	float rnd;
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];

	// Set the name for the bot.
	if (strlen(name) != 0)
	strcpy(bot_name, name);

	// skin
	if (strlen(skin) == 0)
	{
		if (level.num_botloads == 0)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Lucky");
			sprintf(bot_skin, "male_thug/002 006 007");
		}
		if (level.num_botloads == 1)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Bugsy");
			sprintf(bot_skin, "male_thug/005 007 009");
		}
		if (level.num_botloads == 2)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "DonVito");
			sprintf(bot_skin, "male_thug/017 008 013");
		}
		if (level.num_botloads == 3)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "FrankCastello");
			sprintf(bot_skin, "male_thug/041 009 014");
		}
		if (level.num_botloads == 4)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "SantoJr.");
			sprintf(bot_skin, "male_thug/049 010 016");
		}
		if (level.num_botloads == 5)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Clyde");
			sprintf(bot_skin, "male_thug/065 011 032");
		}
		if (level.num_botloads == 6)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "MadMozo");
			sprintf(bot_skin, "male_thug/071 013 033");
		}
		if (level.num_botloads == 7)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "MG Kelle");
			sprintf(bot_skin, "male_thug/109 019 041");
		}
		if (level.num_botloads == 8)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "ICEMAN");
			sprintf(bot_skin, "male_thug/123 020 041");
		}
		if (level.num_botloads == 9)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Victor G");
			sprintf(bot_skin, "male_thug/501 300 300");
		}
		if (level.num_botloads == 10)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Big T");
			sprintf(bot_skin, "male_thug/504 301 301");
		}
		if (level.num_botloads == 11)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Big E");
			sprintf(bot_skin, "male_thug/511 132 070");
		}
		if (level.num_botloads == 12)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Leroy Brown");
			sprintf(bot_skin, "male_thug/700 124 070");
		}
		if (level.num_botloads == 13)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Jimmi H");
			sprintf(bot_skin, "male_runt/142 140 140");
		}
		if (level.num_botloads == 14)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Shorty");
			sprintf(bot_skin, "male_runt/141 141 046");
		}
		if (level.num_botloads == 15)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Frosty");
			sprintf(bot_skin, "male_runt/140 141 046");
		}
		if (level.num_botloads == 16)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "El Chapo");
			sprintf(bot_skin, "male_runt/134 132 132");
		}
		if (level.num_botloads == 17)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Pablo Escobar");
			sprintf(bot_skin, "male_runt/133 132 132");
		}
		if (level.num_botloads == 18)
		{
			if (strlen(name) == 0)
			strcpy(bot_name, "Frank Lucas");
			sprintf(bot_skin, "male_runt/132 132 132");
		}
	}
	else
		strcpy(bot_skin, skin);

	if (teamplay->value != 0)
		sprintf(bot_skin, "male_thug/888 888 888");

	if (Q_stricmp(name, "Jesus") == 0)
		sprintf(bot_skin, "male_runt/021 017 010");
	
	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	//KOOGLESP_SaveBots(); // make sure to save the bots
}

///////////////////////////////////////////////////////////////////////
// Spawn the bot
///////////////////////////////////////////////////////////////////////
void KOOGLESP_SpawnBot(char *team, char *name, char *skin, char *userinfo)
{
	edict_t	*bot;

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	// To allow bots to respawn
	if (userinfo == NULL)
		KOOGLESP_SetName(bot, name, skin, team);
	else
		ClientConnect(bot, userinfo);

	G_InitEdict(bot);

	InitClientResp(bot->client);
	
	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// locate ent at a spawn point
	if(teamplay->value != 0)
	{
	  if (team != NULL && strcmp(team,"1")==0)
	  KOOGLESP_PutClientInServer (bot,false, TEAM_1);
	  else
	  KOOGLESP_PutClientInServer (bot,false, TEAM_2);
	}
	else
	KOOGLESP_PutClientInServer(bot, false, 0);

	// make sure all view stuff is valid
	ClientEndServerFrame(bot);

	KOOGLEIT_PlayerAdded(bot); // let the world know we added another

	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal

}

///////////////////////////////////////////////////////////////////////
// Remove a bot by name or all bots
///////////////////////////////////////////////////////////////////////
void KOOGLESP_RemoveBot(char *name)
{
	int i;
	qboolean freed = false;
	edict_t *bot;

	for (i = 0; i<maxclients->value; i++)
	{
		bot = g_edicts + i + 1;
		if (bot->inuse)
		{
			if (bot->is_bot && (strcmp(bot->client->pers.netname, name) == 0 || strcmp(name, "all") == 0))
			{
				bot->health = 0;
				player_die(bot, bot, bot, 1, vec3_origin, 0, 0);
				// don't even bother waiting for death frames
				bot->deadflag = DEAD_DEAD;
				bot->inuse = false;
				freed = true;
				KOOGLEIT_PlayerRemoved(bot);
				safe_bprintf(PRINT_MEDIUM, "%s has left the building folks...\n", bot->client->pers.netname);
			}
		}
	}

	if (!freed)
		safe_bprintf(PRINT_MEDIUM, "%s isn't there, you must have typed the shit wrong again..\n", name);

	//KOOGLESP_SaveBots(); // Save them again
}

void KOOGLESP_SpawnHomer(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "RedNeckHomer");
	sprintf(bot_skin, "male_homer/001 IRA RBT");
	sprintf(bot_extras, "1110");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!


	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}


void KOOGLESP_SpawnChastity(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Chastity");
	sprintf(bot_skin, "female_chastity/003 007 007");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SpawnPurgator(edict_t *bot, char *name, char *skin, char *team) // not used until I fin the model or remake it
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "PurgaWhore");
	sprintf(bot_skin, "qfemale_purgator/015 015 015");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SpawnMarvin(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Marvin");
	sprintf(bot_skin, "male_marvin/002 001 001"); //marvin
	sprintf(bot_extras, "2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SpawnPoop(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;


	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	// name
	if (level.num_botloads == 1)
		sprintf(bot_name, "Nugget");
	if (level.num_botloads == 2)
		sprintf(bot_name, "SuperLog");
	if (level.num_botloads == 3)
		sprintf(bot_name, "LongLoaf");
	if (level.num_botloads == 4)
		sprintf(bot_name, "Swirly");
	if (level.num_botloads == 5)
		sprintf(bot_name, "TidyTurd");
	if (level.num_botloads == 6)
		sprintf(bot_name, "TipsyTurd");

	sprintf(bot_skin, "male_nogule/026 026 026");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SpawnPit(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	// name
	sprintf(bot_name, "PitBullet");
	sprintf(bot_skin, "male_thug/PB2 PB2 969"); //pitbullet

												// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameBandit(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Ban|)it");
	sprintf(bot_skin, "male_thug/030 bm2 bm1");
	sprintf(bot_extras, "0102");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameDeeg(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;


	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "D33G");
	sprintf(bot_skin, "male_thug/5pr zdz z2z");
	sprintf(bot_extras, "2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameFreak(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;


	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "FrEaK");
	sprintf(bot_skin, "male_thug/512 gf3 0zb");
	sprintf(bot_extras, "0102");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameShorty(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	// name
	sprintf(bot_name, "Shorty");
	sprintf(bot_skin, "male_runt/032 017 006");
	sprintf(bot_extras, "0100");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameBeerHomer(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "BeerHomer");
	sprintf(bot_skin, "male_homer/001 001 001");
	sprintf(bot_extras, "2110");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameChastitySandy(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;


	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	// name
	sprintf(bot_name, "Sandy");
	sprintf(bot_skin, "female_chastity/001 001 001");
	//  sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameTina(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Tina");
	sprintf(bot_skin, "female_chick/601 300 300");
	//  sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameEddie(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	// name
	sprintf(bot_name, "Eddie");
	sprintf(bot_skin, "male_blade/002 002 002");
	sprintf(bot_extras, "0000");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameDrFreak(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Dr.Freak");
	sprintf(bot_skin, "male_drfreak/blu blu blu");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameJack(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Jack");
	sprintf(bot_skin, "male_jack/001 001 002");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameMelissaCrakhor(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "Melissa");
	sprintf(bot_skin, "female_crakhor/022 022 022");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameFionaFlapsLadyDeath(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "FionaFlaps");
	sprintf(bot_skin, "female_ladydeath/002 002 002");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameSharonWhoreHill(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "WhoreHill");
	sprintf(bot_skin, "female_crafty/ctr 004 004");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameLynnHopper(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "LynnHopper");
	sprintf(bot_skin, "female_chick/001 001 001");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameDeadLock(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "DeadLock");
	sprintf(bot_skin, "female_deadlock/001 001 001");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);

	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SetBotNameKrafty(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	// name
	sprintf(bot_name, "Krafty");
	sprintf(bot_skin, "male_krafty/001 001 001");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal

}

void KOOGLESP_SetBotNameGhost(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "GH0ST");
	sprintf(bot_skin, "male_thug/006 TG1 969");
	sprintf(bot_extras, "0102");
	// initialise userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

void KOOGLESP_SpawnGoblin(edict_t *bot, char *name, char *skin, char *team)
{
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	//	char bot_extras[MAX_INFO_STRING];

	bot = KOOGLESP_FindFreeClient();

	if (!bot)
	{
		safe_bprintf(PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	if (level.num_botloads == 0)
		level.num_botloads = 1;
	else
		level.num_botloads++;

	// Set the name for the bot.
	sprintf(bot_name, "BoogerEater");
	sprintf(bot_skin, "qmale_goblin/001 001 001");
	//    sprintf(bot_extras,"2012");

	// initialize userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	//  Info_SetValueForKey (userinfo, "extras", bot_extras);
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect(bot, userinfo);

	G_InitEdict(bot);
	InitClientResp(bot->client);
	
	KOOGLESP_PutClientInServer(bot, false, 0);
	// make sure all view stuff is valid 
	ClientEndServerFrame(bot);
	KOOGLEIT_PlayerAdded(bot); // let the world know we added another
	KOOGLEAI_PickLongRangeGoal(bot); // pick a new goal
}

