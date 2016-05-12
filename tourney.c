#include "g_local.h"

int	 vote_set[9];        // stores votes for next map

char admincode[16];		 // the admincode
char default_map[32];    // default settings
char default_teamplay[16];
char default_dmflags[16];
char default_password[16];
char default_timelimit[16];
char default_cashlimit[16];
char default_fraglimit[16];
char default_dm_realmode[16];
char custom_map_filename[32];  // stores where various files can be found
char ban_name_filename[32];
char ban_ip_filename[32];

int allow_map_voting;
int disable_admin_voting;
int scoreboard_first;
int fph_scoreboard;
int total_rank;          // used in calculating maps picks based on weight
int num_custom_maps;
int num_netnames;
int num_ips;

int fixed_gametype;
int enable_password;
char rconx_file[32];
//char server_url[64];
int num_rconx_pass;
int keep_admin_status;
int default_random_map;
int disable_anon_text;
int disable_curse;
//int enable_asc;
int unlimited_curse;
int enable_killerhealth;

MOTD_t	MOTD[20];

player_t playerlist[64];

ban_t	netname[100];
ban_t	ip[100];

ban_t	rconx_pass[100];

// Snap START - 1/1/2016
void getTeamTags();
// Snap END - 1/1/2016

//==============================================================
// Papa - This file contains all the functions that control the
//        modes a server may be in.
//===============================================================
void PublicSetup()  // returns the server into ffa mode and resets all the cvars (settings)
{
	edict_t		*self;
	int			i;

	level.modeset = FREEFORALL;
	gi.cvar_set("dmflags", default_dmflags);
	gi.cvar_set("teamplay", default_teamplay);
	gi.cvar_set("password", default_password);
	gi.cvar_set("timelimit", default_timelimit);
	gi.cvar_set("fraglimit", default_fraglimit);
	gi.cvar_set("cashlimit", default_cashlimit);
	gi.cvar_set("dm_realmode", default_dm_realmode);
	level.startframe = level.framenum;
	for_each_player(self, i)
	{
		
		if (self->client->chase_target)
		self->client->chase_target = NULL;
		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost
		self->flags &= ~FL_GODMODE;
		self->health = 0;
		meansOfDeath = MOD_RESTART;
		ClientBeginDeathmatch(self);
	}

	safe_bprintf(PRINT_HIGH, "The server is once again public.\n");
}

// Places the server in prematch mode
void MatchSetup() 
{
	edict_t		*self;
	int			i;

	level.modeset = MATCHSETUP;
	level.startframe = level.framenum;

	for_each_player(self, i)
	{
		if (self->client->pers.spectator == SPECTATING)
			continue;

		if (self->client->chase_target)
			self->client->chase_target = NULL;

		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost

		meansOfDeath = MOD_RESTART;
		self->client->pers.spectator = SPECTATING;
		self->flags &= ~FL_GODMODE;
		self->health = 0;
		ClientBeginDeathmatch(self);
	}

	safe_bprintf(PRINT_HIGH, "The server is now ready to setup a match.\n");
	safe_bprintf(PRINT_HIGH, "Players need to join the correct teams.\n");
}

int memalloced[3] = { 0,0,0 };

void ResetServer() // completely resets the server including map
{
	char command[64];

	gi.cvar_set("dmflags", default_dmflags);
	gi.cvar_set("teamplay", default_teamplay);
	gi.cvar_set("password", default_password);
	gi.cvar_set("timelimit", default_timelimit);
	gi.cvar_set("fraglimit", default_fraglimit);
	gi.cvar_set("cashlimit", default_cashlimit);
	gi.cvar_set("dm_realmode", default_dm_realmode);
	gi.cvar_set("cheats", "0");
	memalloced[1] = 0; memalloced[2] = 0;

	if (default_random_map && num_custom_maps)
		Com_sprintf(command, sizeof(command), "map \"%s\"\n", custom_list[rand() % num_custom_maps].custom_map);
	else
		Com_sprintf(command, sizeof(command), "map \"%s\"\n", default_map);
	gi.AddCommandString(command);
}

int team_startcash[2] = { 0,0 };

void MatchStart()  // start the match
{
	int			i;
	edict_t		*self;

	level.player_num = 0;
	level.modeset = FINALCOUNT;
	level.startframe = level.framenum;
	safe_bprintf(PRINT_HIGH, "FINAL COUNTDOWN STARTED.  15 SECONDS TO MATCH.\n");
	team_cash[1] = team_startcash[0]; team_startcash[0] = 0;
	team_cash[2] = team_startcash[1]; team_startcash[1] = 0;
	UPDATESCORE

		G_ClearUp(NULL, FOFS(classname));

	for_each_player(self, i)
	{
		if (self->client->chase_target)
			self->client->chase_target = NULL;
		
		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost

		self->client->pers.bagcash = 0;
		self->client->resp.deposited = 0;
		self->client->resp.score = 0;
		self->client->pers.currentcash = 0;
		self->client->resp.acchit = self->client->resp.accshot = 0;
		memset(self->client->resp.fav, 0, 8 * sizeof(int));

		if (self->client->pers.spectator == SPECTATING)
			continue;

		meansOfDeath = MOD_RESTART;
		self->client->pers.spectator = SPECTATING;
		self->flags &= ~FL_GODMODE;
		self->health = 0;
		ClientBeginDeathmatch(self);
	}

	gi.WriteByte(svc_stufftext);
	gi.WriteString(va("play world/cypress%i.wav", 2 + (rand() % 4)));
	gi.multicast(vec3_origin, MULTICAST_ALL);
}

void SpawnPlayer() // Here I spawn players - 1 per server frame in hopes of reducing overflows
{
	edict_t		*self;
	edict_t     *ent;
	int			i;
	int			team1 = false;

	for (i = 0; i < maxclients->value; i++)
	{
		self = g_edicts + 1 + i;

		if ((!self->inuse) || (!self->is_bot))
			continue;

		if (self->client->chase_target)
			self->client->chase_target = NULL;

		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost

		self->client->pers.bagcash = 0;
		self->client->resp.deposited = 0;
		self->client->resp.score = 0;
		self->client->pers.currentcash = 0;
		self->client->resp.acchit = self->client->resp.accshot = 0;
		memset(self->client->resp.fav, 0, 8 * sizeof(int));

		self->flags &= ~FL_GODMODE;
		self->health = 0;
		meansOfDeath = MOD_RESTART;
		/* Koogle bots use player die as a pass through for respawn */
		player_die(self, self, self, 1, vec3_origin, 0, 0); // <- this has to be here or the server will crash!
		self->client->pers.spectator = PLAYING;
		self->client->resp.is_spawn = true;
		break;
	}

	//gi.dprintf("START: SpawnPlayer\n");
	for (i = 0; i < maxclients->value; i++)
	{
		self = g_edicts + 1 + i;
		ent = g_edicts + 1 + i;
		
		if( (ent->client->pers.admin > NOT_ADMIN ) || (!strcmp(ent->client->pers.netname, "console")) )
		{
			if (maxclients->value > 1)
			{
				while (!ent->client->chase_target)
				{
					ent->client->chasemode = FREE_CHASE;
					ent->client->chase_check = level.framenum;
					ChaseNext(ent);
				}
			}
			continue;
		}

		if ((!self->inuse) || (self->is_bot))
			continue;

		if (!self->client->resp.is_spawn)
		{
			if (!self->is_bot)
				gi.dprintf("Spawning Player %d ( %s ) (level.num_players = %i)\n", i + 1, self->client->pers.netname, level.num_players);

			if (self->client->chase_target)
				self->client->chase_target = NULL;

			// KOOGLEBOT_ADD 5/9/2016 by Ghost
			self->client->pers.playertobotindex = DONOTADDPLAYER;
			// KOOGLEBOT_END 5/9/2016 by Ghost

			self->flags &= ~FL_GODMODE;
			self->health = 0;
			meansOfDeath = MOD_RESTART;
			team1 = true;
			self->client->pers.spectator = PLAYING;

			ClientBeginDeathmatch(self);
			self->client->resp.is_spawn = true;
			break;
		}
	}
	if (!team1)
		level.is_spawn = true;

	//	gi.dprintf("IN Server: SpawnPlayer ( %s )\n",self->client->pers.netname);
}

void SpawnPlayers()  // Same idea but 1 player per team
{
	edict_t		*self;
	int			i;
	int			team1, team2;

	team1 = false;
	team2 = false;

	for (i = 0; i < maxclients->value && (!team1 || !team2); i++)
	{
		self = g_edicts + 1 + i;
		if (!self->inuse)
			continue;
		if ((self->client->pers.team == 1) && (!team1) && (!self->client->resp.is_spawn))
		{
			if (self->client->chase_target)
				self->client->chase_target = NULL;

			// KOOGLEBOT_ADD 5/9/2016 by Ghost
			self->client->pers.playertobotindex = DONOTADDPLAYER;
			// KOOGLEBOT_END 5/9/2016 by Ghost

			self->flags &= ~FL_GODMODE;
			self->health = 0;
			meansOfDeath = MOD_RESTART;
			team1 = true;
			self->client->pers.spectator = PLAYING;
			ClientBeginDeathmatch(self);
			self->client->resp.is_spawn = true;
		}
		if ((self->client->pers.team == 2) && (!team2) && (!self->client->resp.is_spawn))
		{
			if (self->client->chase_target)
				self->client->chase_target = NULL;

			// KOOGLEBOT_ADD 5/9/2016 by Ghost
			self->client->pers.playertobotindex = DONOTADDPLAYER;
			// KOOGLEBOT_END 5/9/2016 by Ghost

			self->flags &= ~FL_GODMODE;
			self->health = 0;
			meansOfDeath = MOD_RESTART;
			team2 = true;
			self->client->pers.spectator = PLAYING;
			ClientBeginDeathmatch(self);
			self->client->resp.is_spawn = true;
		}
	}
	if ((!team1) && (!team2))
		level.is_spawn = true;
}

void Start_Match() // Starts the match
{
	edict_t		*self;
	int			i;

	level.startframe = level.framenum;
	level.modeset = STARTINGMATCH;
	level.is_spawn = false;
	for_each_player(self, i)
	{
		safe_centerprintf(self, "The Match has begun.\n");
		self->client->resp.is_spawn = false;
		self->client->resp.enterframe = level.framenum;
		self->client->chase_target = NULL;
		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost
	}

	gi.WriteByte(svc_stufftext);
	gi.WriteString("play world/pawnbuzz_out.wav");
	gi.multicast(vec3_origin, MULTICAST_ALL);
}

void Start_Pub() // Starts a pub
{
	edict_t		*self;
	int			i;

	level.startframe = level.framenum;
	level.modeset = STARTINGPUB;
	level.is_spawn = false;
	for_each_player(self, i)
	{
		safe_centerprintf(self, "Let the Fun Begin!\n");
		self->client->resp.is_spawn = false;
		self->client->resp.enterframe = level.framenum;
		self->client->chase_target = NULL;
		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost
	}

	gi.WriteByte(svc_stufftext);
	gi.WriteString("play world/pawnbuzz_out.wav");
	//gi.WriteString("play actors/male/popeye/postfind3.wav");
	
	gi.multicast(vec3_origin, MULTICAST_ALL);
}

void SetupMapVote() // at the end of a level - starts the vote for the next map
{
	edict_t *self;
	int i;

	level.modeset = ENDMATCHVOTING;
	level.startframe = level.framenum;

	for_each_player(self, i)
	{
		HideWeapon(self);
		if (self->client->flashlight) self->client->flashlight = false;

		if (self->client->chase_target)
			self->client->chase_target = NULL;

		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost

		self->movetype = MOVETYPE_NOCLIP;
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
		self->client->pers.spectator = SPECTATING;
	}

	gi.WriteByte(svc_stufftext);
	gi.WriteString(va("play world/cypress%i.wav", 2 + (rand() % 4)));
	gi.multicast(vec3_origin, MULTICAST_ALL);
}

void MatchEnd() // end of the match
{
	edict_t *self;
	int i;

	level.modeset = MATCHSETUP;
	level.startframe = level.framenum;
	for_each_player(self, i)
	{
		HideWeapon(self);
		if (self->client->flashlight) self->client->flashlight = false;
		safe_centerprintf(self, "The Match has ended!");

		if (self->client->chase_target)
			self->client->chase_target = NULL;

		// KOOGLEBOT_ADD 5/9/2016 by Ghost
		self->client->pers.playertobotindex = DONOTADDPLAYER;
		// KOOGLEBOT_END 5/9/2016 by Ghost

		self->flags &= ~FL_GODMODE;
		self->health = 0;
		meansOfDeath = MOD_RESTART;
		ClientBeginDeathmatch(self);
	}
	gi.WriteByte(svc_stufftext);
	gi.WriteString(va("play world/cypress%i.wav", 2 + (rand() % 4)));
	gi.multicast(vec3_origin, MULTICAST_ALL);
}

void CheckAllPlayersSpawned() // when starting a match this function is called until all the players are in the game
{
	if (teamplay->value)
		SpawnPlayers();
	else
		SpawnPlayer();

	if ((level.is_spawn) && (level.modeset == STARTINGMATCH))
		level.modeset = MATCH;
	if ((level.is_spawn) && (level.modeset == STARTINGPUB))
		level.modeset = TEAMPLAY;
}

void CheckIdleMatchSetup() // restart the server if its empty in matchsetup mode
{
	int		count = 0;
	int		i;
	edict_t	*doot;

	for_each_player(doot, i)
		count++;

	if (count == 0)
		ResetServer();
}

void CheckStartMatch() // 15 countdown before matches
{
	if (level.framenum >= level.startframe + 145)
	{
		Start_Match();
		return;
	}

	if ((level.framenum % 10 == 0) && (level.framenum > level.startframe + 49))
		safe_bprintf(PRINT_HIGH, "The Match will start in %d seconds!\n", (140 - (level.framenum - level.startframe)) / 10);
}

void CheckStartPub() // 35 second countdown before server starts
{
	static char current_map[55];
	edict_t	*doot;
	int i;

	for_each_player(doot, i)
	{

		if (!doot->client->chase_target)
			continue;

		if (doot->client->chase_target)
			doot->client->chase_target = NULL;

		break;
	}


	//int i;

	//level.motdline = 1;
	//level.num_seconds = (level.framenum - level.startframe) / 10;
	//level.num_players = 1;

	switch (level.framenum)
	{
		case 40:
			break;
		case 50:
			break;
		case 60:
			break;
		case 70:
			break;
		case 80:
			break;
		case 90:
			break;
		case 100:
			break;
		case 110:
			break;
		case 120:
			break;
		case 130:
			break;
		case 140:
			break;
		case 150:
			break;
		case 160:
			break;
		case 170:
			break;
		case 180:
			break;
		case 190:
			break;
		case 200:
			break;
		case 210:
			break;
		case 220:
			break;
		case 230:
			break;
		case 240:
			gi.WriteByte(svc_stufftext);
			gi.WriteString("play bm_mapstart/10_0.wav"); // 10 second count down before everyone spawns into the game!
			gi.multicast(vec3_origin, MULTICAST_ALL);
			break;
		case 250:
			safe_bprintf(PRINT_HIGH, "Fight in 10 seconds!\n");
			break;
		case 260:
			safe_bprintf(PRINT_HIGH, "Fight in 9 seconds!\n");
			break;
		case 270:
			safe_bprintf(PRINT_HIGH, "Fight in 8 seconds!\n");
			break;
		case 280:
			safe_bprintf(PRINT_HIGH, "Fight in 7 seconds!\n");
			break;
		case 290:
			safe_bprintf(PRINT_HIGH, "Fight in 6 seconds!\n");
			break;
		case 300:
			safe_bprintf(PRINT_HIGH, "Fight in 5 seconds!\n");
			break;
		case 310:
			safe_bprintf(PRINT_HIGH, "Fight in 4 seconds!\n");
			break;
		case 320:
			safe_bprintf(PRINT_HIGH, "Fight in 3 seconds!\n");
			break;
		case 330:
			safe_bprintf(PRINT_HIGH, "Fight in 2 seconds!\n");
			break;
		case 340:
			safe_bprintf(PRINT_HIGH, "Fight in 1 seconds!\n"); // here matches the timer display game entrance!
			if (strcmp(level.mapname, current_map))
			{
				KOOGLEND_InitNodes();
				strcpy(current_map, level.mapname);
				KOOGLEND_LoadNodes();
				KOOGLESP_LoadBots();
			}
			else
				KOOGLESP_LoadBots();
			Start_Pub();
			return;
			break;
	}
}

void CheckEndMatch() // check if time,frag,cash limits have been reached in a match
{
	int			i;
	int		count = 0;
	edict_t	*doot;

	for_each_player(doot, i)
	{
		if (!doot->client->chase_target)
			continue;

		if (doot->client->chase_target)
			doot->client->chase_target = NULL;
		
		break;
	}

	// Snap START - 1/1/2016
	if (level.framenum % 100 == 0 && !level.manual_tagset)
	{
		getTeamTags();
	}
	// Snap END - 1/1/2016

	for_each_player(doot, i)
		count++;

	//if (count == 0)
	if (count == level.num_botloads)
		ResetServer();

	if ((int)fraglimit->value && (int)teamplay->value == 4) 
	{
		if (team_cash[1] >= (int)fraglimit->value || team_cash[2] >= (int)fraglimit->value) 
		{
			safe_bprintf(PRINT_HIGH, "Fraglimit hit.\n");
			MatchEnd();
			return;
		}
	}

	if ((int)cashlimit->value)
	{
		if ((team_cash[1] >= (int)cashlimit->value) || (team_cash[2] >= (int)cashlimit->value))
		{
			safe_bprintf(PRINT_HIGH, "Cashlimit hit.\n");
			MatchEnd();
			return;
		}
	}

	if ((int)timelimit->value)
	{
		if (level.framenum > (level.startframe + ((int)timelimit->value) * 600 - 1))
		{
			safe_bprintf(PRINT_HIGH, "Timelimit hit.\n");
			MatchEnd();
			return;
		}

		if (((level.framenum - level.startframe) % 10 == 0) && (level.framenum > (level.startframe + (((int)timelimit->value * 600) - 155))))
		{
			safe_bprintf(PRINT_HIGH, "The Match will end in  %d seconds\n", (((int)timelimit->value * 600) + level.startframe - level.framenum) / 10);
			return;
		}

		if (((level.framenum - level.startframe) % 600 == 0) && (level.framenum > (level.startframe + (((int)timelimit->value * 600) - 3000))))
		{
			safe_bprintf(PRINT_HIGH, "The Match will end in  %d minutes\n", (((int)timelimit->value * 600) + level.startframe - level.framenum) / 600);
			return;
		}

		if ((level.framenum - level.startframe) % 3000 == 0)
			safe_bprintf(PRINT_HIGH, "The Match will end in  %d minutes\n", (((int)timelimit->value * 600) + level.startframe - level.framenum) / 600);
	}
}

// Fixed keep admin status after map vote - 5/6/2016 Fixed/added by Ghost
void CheckEndVoteTime() // check the timelimit for voting next level/start next map
{
	int	i, count[9];
	edict_t *player;
	char command[64];
	int wining_map;

	char buf[1024];
	edict_t *ent;

	edict_t	*doot;

	for_each_player(doot, i)
	{

		if (!doot->client->chase_target)
			continue;

		if (doot->client->chase_target)
			doot->client->chase_target = NULL;

		break;
	}


	if (level.framenum == (level.startframe + 10))
	{
		for_each_player(player, i)
		{
			if (scoreboard_first)
				player->client->showscores = SCOREBOARD;
			else
				player->client->showscores = SCORE_MAP_VOTE;
			DeathmatchScoreboard(player);
		}
	}

	if (level.framenum > (level.startframe + 300))
	{
		memset(&count, 0, sizeof(count));
		for_each_player(player, i)
		{
			count[player->vote]++;
		}
		wining_map = 1;
		for (i = 2; i < 9; i++)
		{
			custom_list[vote_set[i]].rank += count[i];
			if (count[i] > count[wining_map])
				wining_map = i;
		}
		i = write_map_file();
		if (i != OK)
			gi.dprintf("Error writing custom map file!\n");

		buf[0] = 0;

		for (i = 0; i < maxclients->value; i++)
		{
			ent = g_edicts + 1 + i;
			if (!ent->inuse) continue;
			if (ent->client->pers.rconx[0])
				Info_SetValueForKey(buf, ent->client->pers.ip, ent->client->pers.rconx);
		}
		gi.cvar_set("rconx", buf);

		if (keep_admin_status)
		{
			for (i = 0; i < maxclients->value; i++)
			{
				ent = g_edicts + 1 + i;
				if (!ent->inuse)
					continue;

				if (ent->client->pers.admin > NOT_ADMIN)
				{
					gi.cvar_set("modadmin", ent->client->pers.ip);

					if (ent->client->pers.admin == ELECTED)
						gi.cvar_set("admintype", "1");
					else if (ent->client->pers.admin == ADMIN)
						gi.cvar_set("admintype", "2");
					break;
				}
			}

			if (i == maxclients->value)
			{
				gi.cvar_set("modadmin", "");
				gi.cvar_set("admintype", "");
			}
		}

		Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", custom_list[vote_set[wining_map]].custom_map);
		gi.AddCommandString(command);
	}
}

void CheckVote() // check the timelimit for an admin vote
{
	if (level.framenum > (level.voteframe + 1200))
	{
		switch (level.voteset)
		{
		case VOTE_ON_ADMIN:
			safe_bprintf(PRINT_HIGH, "The request for admin has failed!\n");
			break;
		}

		level.voteset = NO_VOTES;
	}
}

int	CheckNameBan(char *name)
{
	char n[64];
	int i;

	strcpy(n, name);
	for (i = 0; i < strlen(n); i++) n[i] = tolower(n[i]);
	for (i = 0; i < num_netnames; i++)
	{
		if (strstr(n, netname[i].value))
			return true;
	}

	return false;
}

int	CheckPlayerBan(char *userinfo) // added by FREDZ using here Monkey Mod 1.40d otherwise crash - 5/6/2016
{
	char	*value;
	int		i, j;
	int		isSame;

	value = Info_ValueForKey(userinfo, "name");

	if (CheckNameBan(value))
		return true;

	value = Info_ValueForKey(userinfo, "ip");

	for (i = 0; i < num_ips; i++)
	{
		isSame = true;

		j = 0;

		while ((isSame) && (value[j] != '\0') && (value[j] != ':'))
		{
			if (ip[i].value[j] != value[j])
				isSame = false;

			j++;
		}

		if (isSame)
			return true;
	}

	return false;
}

/////////////////////////////////////////////////////
// snap - team tags
// Snap START - 1/1/2016
void setTeamName(int team, char *name) // tical's original code :D
{
	if (!name || !*name) return;

	if (strlen(name) < 16 && name[0] != ' ')
	{
		if (memalloced[team])
			free(team_names[team]);

		team_names[team] = (char *)malloc(16);

		if (team_names[team] != NULL)
		{
			memalloced[team] = 1;
			sprintf(team_names[team], "%s", name);
		}
	}
}
// Snap END - 1/1/2016
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
// Snap START - 1/1/2016
void getTeamTags()
{
	int i;
	edict_t *doot;
	char names[2][64][16];
	int namesLen[2] = { 0, 0 };
	char teamTag[2][12];
	int teamTagsFound[2] = { FALSE, FALSE };

	for_each_player(doot, i)
	{
		int team = doot->client->pers.team;
		if (team && namesLen[team - 1] < 64)
		{
			strcpy(names[team - 1][namesLen[team - 1]++], doot->client->pers.netname);
		}
	}

	for (i = 0; i < 2; i++)
	{
		int	j;
		for (j = 0; j < namesLen[i] && teamTagsFound[i] == FALSE; j++)
		{
			int	k;
			for (k = 0; k < namesLen[i] && j != k && teamTagsFound[i] == FALSE; k++)
			{
				char theTag[12];
				int	theTagNum = 0;
				int	y = 0;
				char s = names[i][j][y];

				while (s != '\0' && theTagNum == 0)
				{
					int	z = 0;
					char t = names[i][k][z];
					while (t != '\0')
					{
						if (s == t && s != ' ')
						{ // we have a matched char
							int	posY = y + 1;
							int	posZ = z + 1;
							char ss = names[i][j][posY];
							char tt = names[i][k][posZ];

							while (ss != '\0' && tt != '\0' && ss == tt && theTagNum < 11) {
								if (theTagNum == 0)
								{ // we have two consecutive matches, this is a tag
									theTag[theTagNum++] = s;
									theTag[theTagNum++] = ss;
								}
								else
								{
									theTag[theTagNum++] = ss;
								}
								ss = names[i][j][++posY];
								tt = names[i][k][++posZ];
							}
						}
						t = names[i][k][++z];
					}
					s = names[i][j][++y];
				}
				if (theTagNum > 0)
				{
					int	e;
					float howmany = 0.0;
					float percentage;
					theTag[theTagNum] = '\0';

					for (e = 0; e < namesLen[i]; e++)
					{
						if (strstr(names[i][e], theTag) != NULL)
						{
							howmany += 1.0;
						}
					}
					percentage = howmany / (float)namesLen[i] * 100.0;
					if (percentage > 75.0)
					{
						strcpy(teamTag[i], theTag);
						teamTagsFound[i] = TRUE;
					}
				}
			}
		}
	}

	for (i = 0; i < 2; i++)
	{
		if (teamTagsFound[i] == TRUE)
		{
			setTeamName(i + 1, teamTag[i]);
		}
		else if (i == 0)
		{
			setTeamName(i + 1, "Dragons");
		}
		else if (i == 1)
		{
			setTeamName(i + 1, "Nikki's Boyz");
		}
	}
}
// Snap END - 1/1/2016
//////////////////////////////////////////////////////////////////