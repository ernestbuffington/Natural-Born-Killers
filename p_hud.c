#include "g_local.h"

/* SHOW HEADER MESSAGE */
#define HEADERMESSAGE\
	""GAMEVERSION,\
	"Build Version : "MODVERSION,\
	"Build Date : "__DATE__,\
	"http://www.kingpin.info",

/* SHOW GAMEMODE MESSAGE */
#define GAMEMODEMESSAGE\
	if ((int)teamplay->value == 0) {\
		if (dm_realmode->value)\
			Com_sprintf (temp, sizeof(temp), "This server is running Realmode Deathmatch");\
		else\
			Com_sprintf (temp, sizeof(temp), "This server is running Standard Deathmatch");\
	} else if ((int)teamplay->value == 1) {\
		if (dm_realmode->value)\
			Com_sprintf (temp, sizeof(temp), "This server is running Realmode Bagman");\
		else\
			Com_sprintf (temp, sizeof(temp), "This server is running Bagman");\
	} else if ((int)teamplay->value == 4) {\
		if (dm_realmode->value)\
			Com_sprintf (temp, sizeof(temp), "This server is running Realmode Team Deathmatch");\
		else\
			Com_sprintf (temp, sizeof(temp), "This server is running Team Deathmatch");\
	}

/* SHOW CHASE CAM MESSAGE */
#define SHOWCHASENAME\
	if (ent->client->chase_target && ent->client->chase_target->client)	{\
		ent->client->update_chase = false;\
		if(ent->client->chasemode == FREE_CHASE)\
		Com_sprintf(entry, sizeof(entry), "xm -180 yb -68 dmstr 773 \"Chasing %s in Freelook Mode\" xm -260 yb -40 dmstr 552 \".. [ and ] cycles, JUMP changes mode, ACTIVATE quits ..\" ",\
			ent->client->chase_target->client->pers.netname);\
		else if(ent->client->chasemode == EYECAM_CHASE)\
		Com_sprintf(entry, sizeof(entry), "xm -180 yb -68 dmstr 773 \"Chasing %s in Eyecam Mode\" xm -260 yb -40 dmstr 552 \".. [ and ] cycles, JUMP changes mode, ACTIVATE quits ..\" ",\
			ent->client->chase_target->client->pers.netname);\
		else if(ent->client->chasemode == LOCKED_CHASE)\
		Com_sprintf(entry, sizeof(entry), "xm -170 yb -68 dmstr 773 \"Chasing %s in Locked Mode\" xm -260 yb -40 dmstr 552 \".. [ and ] cycles, JUMP changes mode, ACTIVATE quits ..\" ",\
			ent->client->chase_target->client->pers.netname);\
		j = strlen(entry);\
		strcpy (string + stringlength, entry);\
		stringlength += j;\
	}

/* SHOW CHASE MESSAGE */
#define CHASEMESSAGE\
	if (!ent->client->chase_target) {\
		Com_sprintf (entry, sizeof(entry),\
			"xm %i yb -50 dmstr 773 \"%s\" ",\
			-5*23, "[hit ACTIVATE to chase]" );\
		j = strlen(entry);\
		strcpy (string + stringlength, entry);\
		stringlength += j;\
	}

/* FAVORITE WEAPON INDEX */
static char *weapnames[8]={" Pipe",
						   " Pist",
						   " Shot",
						   "Tommy",
						   "  HMG",
						   "   GL",
						   "   RL",
						   "Flame"};

/* MOVE CLIENT TO INTERMISSION */
void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
	ent->client->showscores = SCOREBOARD;
	
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;
	ent->client->quadfire_framenum = 0;
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout
	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboard (ent);
	}
}

/* BEGIN INTERMISSION */
void BeginIntermission (edict_t *targ, char *changenext)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		
		if (client->health <= 0)
			respawn(client);
	}

	if(level.changemap!=NULL)
	free(level.changemap);

	level.intermissiontime = level.time;
	level.changemap = (char *)malloc(MAX_QPATH);
	
	//level.changemap = targ->map;
	strcpy (level.changemap, changenext);
	//safe_bprintf (PRINT_HIGH, "2 %s\n", level.changemap);

	//safe_bprintf (PRINT_HIGH, "Next map will be: %s,%s.\n", level.changemap, targ->map);
	if (strstr(level.changemap, "*"))
	{
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_FLASHLIGHT)
						continue;

					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}

				client->episode_flags = client->client->pers.episode_flags = 0;
				client->client->pers.friends = 0;
				level.helpchange = 0;
			}
		}

		// Dan: this should eliminate the need to press a key to exit the end of episode
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}

/* SPECTATOR SCOREBOARD MESSAGE - dmstr xxx is the rgb color of the text */
void SpectatorScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		yofs=0;
	edict_t	*player;
	char	name[32];
	char	*header = "NAME             ping"; 
	char	*tag;
	char	*selectheader[] =
		{
			"Spectators",
			NULL
		};

	string[0] = 0;
	stringlength = 0;

	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);

		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}

	yofs += 20;

	Com_sprintf (entry, sizeof(entry), "xv 27 yv %i dmstr 663 \"%s\" ", yofs + (int)(-60.0+-3.5*14), header );

	j = strlen(entry);

	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;

	for (i=0 ; i<maxclients->value ; i++)
	{
		player = g_edicts + 1 + i;
		
		if (!player->inuse)
			continue;
		
		if (player->client->pers.spectator == SPECTATING)
		{
			if ((teamplay->value) && (player->client->pers.team != 0))
				continue;
		
			k =0;
			
			strcpy(name,"                ");
			
			while (player->client->pers.netname[k] != '\0')
			{
				name[k] = player->client->pers.netname[k];
				k++;
			}

			if (player->client->pers.rconx[0])
				tag = "096"; //green
			else if (player->client->pers.admin>NOT_ADMIN)
				tag = "900"; //red
			else
				tag = "999";	// fullbright

			Com_sprintf (entry, sizeof(entry), "xv 27 yv %i dmstr %s \"%s %4i\" ",yofs + (int)(-60.0+-3.5*14),
				tag, name, player->client->ping);
			
			j = strlen(entry);
			
			if (stringlength + j < 1024)
			{
				strcpy (string + stringlength, entry);
				stringlength += j;
			}
			
			yofs += 20;
		}
	}

	if ((level.modeset != MATCHSETUP) && (level.modeset != FINALCOUNT) && (level.modeset != FREEFORALL)
		&& ent->client->pers.spectator == SPECTATING) {
		SHOWCHASENAME
		CHASEMESSAGE
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* VOTE FOR A MAP SCOREBOARD MESSAGE */
void VoteMapScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	temp[32];
	char	string[1400];
	int		stringlength;
	int		i, j;
	int		yofs;
	int		count[9];
	int		num_vote_set;
	edict_t *player;
	char	*choice = "Your choices are :";
	char	*endmes = "[Hit your Scoreboard key (f1) for the scoreboard]";
	char	*selectheader[] =
		{
			"The level has Ended. Please Vote your choice",
			"for the next map. Hit the corresponding number",
			"(weapon key) or use [ and ] to place your vote.",
			NULL
		};
	char	*basechoice[] =
		{
			"1 (Pipe)     ",
			"2 (Pistol)   ",
			"3 (Shotgun)  ",
			"4 (Tommygun) ",
			"5 (HMG)      ",
			"6 (GL)       ",
			"7 (Bazooka)  ",
			"8 (FlameThr) ",
			NULL
		};

	string[0] = 0;
	stringlength = 0;

	yofs = 20;

	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 752 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);

		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}

	if (num_custom_maps < 8 )
		num_vote_set = num_custom_maps;
	else
		num_vote_set = 8;

	memset (&count, 0, sizeof(count));

	for_each_player(player,i)
	{
		count[player->vote]++;
	}

	if (ent->vote == 0)
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 999 \"-->      %d players have not voted.\" ",
				-5*40, yofs + (int)(-60.0+-3.5*14), count[0]);
	else
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 777 \"         %d players have not voted\" ",
				-5*40, yofs + (int)(-60.0+-3.5*14), count[0]);

	j = strlen(entry);

	if (stringlength + j < 1024)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	yofs += 30;

	Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 999 \"%s\" ",
		-5*32, yofs + (int)(-60.0+-3.5*14), choice );

	j = strlen(entry);

	if (stringlength + j < 1024)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	yofs += 30;

	for (i=1; i < (num_vote_set+1); i++)
	{
		if (count[i] == 1)
			Com_sprintf (temp, sizeof(temp), "1 Vote  -");
		else
			Com_sprintf (temp, sizeof(temp), "%d Votes -",count[i]);


		if (ent->vote == i)
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 999 \"--> %s %s %s\" ",
					-5*40, yofs + (int)(-60.0+-3.5*14), basechoice[i-1],temp,custom_list[vote_set[i]].custom_map);
		else
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 777 \"    %s %s %s\" ",
					-5*40, yofs + (int)(-60.0+-3.5*14), basechoice[i-1],temp,custom_list[vote_set[i]].custom_map);

		j = strlen(entry);

		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		yofs += 20;
	}

	yofs += 15;
	
	Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 772 \"%s\" ",
		-5*strlen(endmes), yofs + (int)(-60.0+-3.5*14), endmes );

	j = strlen(entry);
	
	if (stringlength + j < 1024)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	
	//if client has voted for a map, print this maps levelshot
	if(ent->vote>0)
	{
		yofs += 20;
		//print the pcx levelshot on screen
		//name of pcx must be same as bsp -- pcx files must be in pics dir
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i picn %s ",
			-5*20, yofs + (int)(-60.0+-3.5*14), custom_list[vote_set[ent->vote]].custom_map);
		
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}
		//tical
	}
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* MOTD SCOREBOARD MESSAGE */
void MOTDScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	char	temp[128];
	int		stringlength;
	int		i, j;
	int		yofs;
	int			found;
	edict_t		*player;
	char		*seperator = "==================================";
	char	*selectheader[] =
		{
			HEADERMESSAGE
			NULL
		};

	string[0] = 0;
	stringlength = 0;

	yofs = 80 - game.num_MOTD_lines * 10;

	if (yofs < 0 )
		yofs = 0;

	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 752 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}

	yofs += 10;

	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 772 \"%s\" ",
		-5*strlen(seperator), yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);

	strcpy (string + stringlength, entry);

	stringlength += j;

	yofs += 30;

	for (i=0; i<game.num_MOTD_lines; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 842 \"%s\" ",
			-5*strlen(MOTD[i].textline), yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		j = strlen(entry);

		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		yofs += 20;
	}

	yofs += 10;

	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 772 \"%s\" ",
		-5*strlen(seperator), yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);

	if (stringlength + j < 1024)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	
	yofs += 30;

		GAMEMODEMESSAGE

		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
			-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
		
		yofs += 20;
		
		j = strlen(entry);
		
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		if (level.modeset == FREEFORALL)
			Com_sprintf (temp, sizeof(temp), "The Game will start soon.");
		else if (level.modeset == MATCH)
			Com_sprintf (temp, sizeof(temp), "in Match Mode. (Please don't join)");
		else if (level.modeset == MATCHSETUP)
			Com_sprintf (temp, sizeof(temp), "in Match Setup Mode.");
		else if (level.modeset == FINALCOUNT)
			Com_sprintf (temp, sizeof(temp), "and is in the Final Countdown before a Match.");
		else if (level.modeset == TEAMPLAY)
			Com_sprintf (temp, sizeof(temp), "in Public Mode, so please join in.");

		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
			-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
		
		yofs += 20;
		
		j = strlen(entry);
		
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		if (admincode[0] || !disable_admin_voting) 
		{
			i=0;
		
			found = FALSE;
			
			while ((!found) && (i<maxclients->value)) 
			{
				player = g_edicts + 1 + i;
				
				i++;
				
				if (!player->inuse)
					continue;
				
				if (player->client->pers.admin > NOT_ADMIN)
					found = TRUE;
			}

			if (found)
				Com_sprintf (temp, sizeof(temp), "Your admin is %s",player->client->pers.netname);
			else
				Com_sprintf (temp, sizeof(temp), "No one currently has admin");

			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
				-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
			
			j = strlen(entry);
			
			if (stringlength + j < 1024)
			{
				strcpy (string + stringlength, entry);
				stringlength += j;
			}
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}

/* REJOIN SCORBOARD MESSAGE */
void RejoinScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j;
	int		yofs;
	char	*seperator = "==================================";
	char	*selectheader[] =
		{
			HEADERMESSAGE
			NULL
		};

	char	*rejoinheader[] =
		{
			"You were just playing on this server.",
			"Would you like to continue where you left off?",
			"Select the corresponding number.",
			NULL
		};
	char	*choices[] =
		{
			"1 - Yes",
			"2 - No",
			NULL
		};


	string[0] = 0;
	stringlength = 0;
	
	yofs = 0;

	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 752 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}
	
	yofs += 10;
	
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 772 \"%s\" ",
		-5*strlen(seperator), yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);
	
	strcpy (string + stringlength, entry);
	stringlength += j;
	
	yofs += 30;

	for (i=0; rejoinheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 842 \"%s\" ",
			-5*strlen(rejoinheader[i]), yofs + (int)(-60.0+-3.5*14), rejoinheader[i] );

		j = strlen(entry);
		
		if (stringlength + j > 1024)
			break;
		
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}
	
	yofs += 30;
	
	for (i=0; choices[i]; i++)
	{
		if (ent->vote == i)
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 999 \"--> %s\" ",
					-5*40, yofs + (int)(-60.0+-3.5*14), choices[i]);
		else
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 777 \"    %s\" ",
					-5*40, yofs + (int)(-60.0+-3.5*14), choices[i]);

		j = strlen(entry);
		
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		yofs += 20;

	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* GrabDaLootScoreboardMessage */
void GrabDaLootScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total, real_total;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
	int		team;
	char *header;
	char	*headerb = "NAME         ping hits depst ";
	char	*headerd = "NAME         ping hits deaths";
	char	*headera = "NAME         ping  acc   fav ";
	char	*headers = "NAME         ping stol   fav ";
	char	nfill[64];
	int		yofs=0, avping = 0, tp = 0, tc = 0;
	int		tmax;
	int		teamcount[3];
	char	*selectheader[] =
		{
			"You are a Spectator!",
			"Valid teams are:",
			NULL
		};

	if (ent->client->showscores==SCOREBOARD)
		header=teamplay->value==4?headerd:headerb;
	else
		header=teamplay->value==4?headera:headers;

	x = (-1*strlen(header) - 2) * 10;	// 10 pixels per char

	string[0] = 0;
	stringlength = 0;
	
	if (level.modeset == ENDMATCHVOTING)
	{
		Com_sprintf(entry, sizeof(entry), "xm -230 yb -40 dmstr 552 \"[Hit your scoreboard key (f1) for the vote menu]\" ");
			
		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	SHOWCHASENAME

	if (!ent->client->showscores)
		goto skipscores;
		
	if (!ent->client->pers.team)
	{
		if (ent->client->showscores==SCOREBOARD) 
		{
		  // count players per team
		  memset( teamcount, 0, sizeof(int) * 3 );

		  for (team=1; team<=2; team++)
		  {
			  for (i=0 ; i<game.maxclients ; i++)
			  {
				cl_ent = g_edicts + 1 + i;
		
				if (!cl_ent->inuse)
					continue;

				if (game.clients[i].pers.team != team)
					continue;

				teamcount[team]++;
			  }
		  }

		// print the team selection header
		for (i=0; selectheader[i]; i++)
		{
			Com_sprintf (entry, sizeof(entry),
				"xm %i yv %i dmstr 999 \"%s\" ",
				-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

			j = strlen(entry);
			
			strcpy (string + stringlength, entry);
			stringlength += j;

			yofs += 30;
		}
		// show team counts
		for (team=1; team<=2; team++)
		{
			strcpy( nfill, team_names[team] );
			
			if (strlen(nfill) > 14)
				nfill[14] = '\0';

			if (strlen(team_names[team]) < 14)
			{
				for (k=0; k<14-strlen(team_names[team]); k++)
					strcat( nfill, " " );
			}

			Com_sprintf (entry, sizeof(entry),
				"xm %i yv %i dmstr 999 \"%i - %s (%i plyrs)\" ",
				-15*10, yofs + (int)(-60.0+-3.5*14), team, nfill, teamcount[team] );

			j = strlen(entry);
			
			strcpy (string + stringlength, entry);
			stringlength += j;

			yofs += 20;
		}
	}
		yofs += 15;

		CHASEMESSAGE
  }

	tmax=(1024-226-stringlength)/((ent->client->pers.version>=114 && ent->client->showscores==SCOREBOARD)?52:92);

	for (team=1; team<=2; team++)
	{
		// Team header
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i tscore %i xm %i dmstr 677 \"%s\" teampic %i ",
			x+14*10, yofs + (int)(-60.0+-3.5*14), team, x, team_names[team], team );

		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;

		// sort the clients by score
		total = 0;

		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;

			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team != team)
				continue;

			if (teamplay->value==4) 
				score = (game.clients[i].resp.score<<8)-game.clients[i].resp.deposited;
			else
				score = (game.clients[i].resp.deposited<<4)+game.clients[i].resp.score;

			for (j=0 ; j<total ; j++)
			{
				if (score > sortedscores[j])
					break;
			}

			for (k=total ; k>j ; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}

			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		real_total = total;

		if (total>tmax) total=tmax-1;

		// header
		Com_sprintf (entry, sizeof(entry),
			"yv %i dmstr 663 \"%s\" ",
			yofs + (int)(-60.0+-1.5*14), header );

		j = strlen(entry);

		strcpy (string + stringlength, entry);
		stringlength += j;

		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

			y = 32 + 32 * i;

			if (i < 2)
				y -= 16;

			if (cl_ent == ent)
				tag = "990";
			else if (cl_ent->client->pers.rconx[0])
				tag = "096";
			else if (cl_ent->client->pers.admin>NOT_ADMIN)
				tag = "900"; // red
			else
				tag = "999"; // white fullbright

			if (ent->client->showscores==SCOREBOARD) 
			{
				if (ent->client->pers.version >= 114)
				{
					Com_sprintf (entry, sizeof(entry),
					"yv %i ds %s %i %i %i %i ",
					yofs + -60+i*16, tag, sorted[i], cl->ping, cl->resp.score, cl->resp.deposited);
				}
				else
				{
					strcpy( nfill, cl->pers.netname );
					if (strlen(nfill) > 13)
						nfill[13] = '\0';
					
					if (strlen(cl->pers.netname) < 13)
					{
						for (k=0; k<13-strlen(cl->pers.netname); k++)
							strcat( nfill, " " );
					}
					
					Com_sprintf (entry, sizeof(entry),
						"yv %i dmstr %s \"%s%4i %4i %5i\" ",
						yofs + -60+i*16, tag, nfill, cl->ping, cl->resp.score, cl->resp.deposited);
				}
			} 
			else 
			{
				int fc=0;
				char *fn="  -";
			
				strcpy( nfill, cl->pers.netname );
				
				if (strlen(nfill) > 13)
					nfill[13] = '\0';
				
				if (strlen(cl->pers.netname) < 13)
				{
					for (k=0; k<13-strlen(cl->pers.netname); k++)
						strcat( nfill, " " );
				}
				
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%s%4i %4i  ",
					yofs + -60+i*16, tag, nfill, cl->ping,
					teamplay->value==4?(cl->resp.accshot?cl->resp.acchit*1000/cl->resp.accshot:0):cl->resp.acchit);
				
				for (j=0;j<8;j++) 
				{
					if (cl->resp.fav[j]>fc) 
					{
						fc=cl->resp.fav[j];
						fn=weapnames[j];
					}
				}
				
				strcat(entry,fn);
				strcat(entry,"\"");
			}

			tp += cl->ping;
			tc++;

			j = strlen(entry);
			
			if (stringlength + j > 1024)
				break;
			
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		if (real_total > 1 && ent->client->showscores==SCOREBOARD) //used to be total instead of 1
		{	// show the nuber of undisplayed players

			avping = tp/tc;
			
			Com_sprintf (entry, sizeof(entry),
				"yv %i dmstr 777 \"(%i players, %i avg)\" ",
				yofs + -60+i*16 + 6, real_total, avping );

			j = strlen(entry);
			
			if (stringlength + j > 1024)
				break;
			
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		x = -8;
		avping = tp = tc = 0;
	}

skipscores:
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* MATCH SETUP SCORBOARD MESSAGE */
void MatchSetupScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	char	temp[64];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total, real_total;
	int		x, y;
	int		found;
	gclient_t	*cl;
	edict_t		*cl_ent, *player;
	char	*tag;
	int		team;
	char *header;
	char	*headerb = "NAME         ping hits depst ";
	char	*headerd = "NAME         ping hits deaths";
	char	*headera = "NAME         ping  acc   fav ";
	char	*headers = "NAME         ping stol   fav ";
	char	nfill[64];
	int		yofs=10;
	int		teamcount[3];
	int		tmax;
	char	*selectheader[] =
		{
			"Valid teams are:",
			NULL
		};

	if (ent->client->showscores==SCOREBOARD)
		header=teamplay->value==4?headerd:headerb;
	else
		header=teamplay->value==4?headera:headers;

	x = (-1*strlen(header) - 2) * 10;	// 10 pixels per char

	string[0] = 0;
	stringlength = 0;
	
	if (!ent->client->showscores)
		goto skipscores;
		
  if (ent->client->showscores==SCOREBOARD) 
  {
	GAMEMODEMESSAGE

	Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
		-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
	
	yofs += 20;
	
	j = strlen(entry);
	
	strcpy (string + stringlength, entry);
	stringlength += j;
		
	if (level.modeset == MATCHSETUP)
		Com_sprintf (temp, sizeof(temp), "in Match Setup Mode.");
	else if (level.modeset == FINALCOUNT)
		Com_sprintf (temp, sizeof(temp), "and is in the Final Countdown before a Match.");
	else if (level.modeset == FREEFORALL)
		Com_sprintf (temp, sizeof(temp), "The Game will start soon.");

	Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
		-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
	
	yofs += 20;
	
	j = strlen(entry);
	
	strcpy (string + stringlength, entry);
	stringlength += j;

	i=0;
	
	found = FALSE;
	
	while ((!found) && (i<maxclients->value)) 
	{
		player = g_edicts + 1 + i;
		
		i++;
		
		if (!player->inuse)
			continue;
		
		if (player->client->pers.admin > NOT_ADMIN)
			found = TRUE;
	}

	if (found)
		Com_sprintf (temp, sizeof(temp), "Your admin is %s",player->client->pers.netname);
	else
		Com_sprintf (temp, sizeof(temp), "No one currently has admin");

	Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
		-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
	
	yofs += 30;
	
	j = strlen(entry);
	
	strcpy (string + stringlength, entry);
	stringlength += j;

	// count players per team
	memset( teamcount, 0, sizeof(int) * 3 );

	for (team=1; team<=2; team++)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
		
			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team != team)
				continue;

			teamcount[team]++;
		}
	}

	// print the team selection header
	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 30;
	}

	// show team counts
	for (team=1; team<=2; team++)
	{
		strcpy( nfill, team_names[team] );
		
		if (strlen(nfill) > 14)
			nfill[14] = '\0';

		if (strlen(team_names[team]) < 14)
		{
			for (k=0; k<14-strlen(team_names[team]); k++)
				strcat( nfill, " " );
		}

		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%i - %s (%i plyrs)\" ",
			-15*10, yofs + (int)(-60.0+-3.5*14), team, nfill, teamcount[team] );

		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}
  }

	yofs += 15;
	
	tmax=(1024-220-stringlength)/((ent->client->pers.version>=114 && ent->client->showscores==SCOREBOARD)?52:92);

	for (team=1; team<=2; team++)
	{
		// Team header
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i tscore %i xm %i dmstr 677 \"%s\" teampic %i ",
			x+14*10, yofs + (int)(-60.0+-3.5*14), team, x, team_names[team], team );

		j = strlen(entry);

		strcpy (string + stringlength, entry);
		stringlength += j;

		// sort the clients by score
		total = 0;

		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
		
			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team != team)
				continue;

			if (teamplay->value==4) 
				score = (game.clients[i].resp.score<<8)-game.clients[i].resp.deposited;
			else
				score = (game.clients[i].resp.deposited<<4)+game.clients[i].resp.score;

			for (j=0 ; j<total ; j++)
			{
				if (score > sortedscores[j])
					break;
			}

			for (k=total ; k>j ; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}

			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		real_total = total;

		if (total>tmax) total=tmax-1;

		// header
		Com_sprintf (entry, sizeof(entry),
			"yv %i dmstr 663 \"%s\" ",
			yofs + (int)(-60.0+-1.5*14), header );

		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;

		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

			y = 32 + 32 * i;

			if (i < 2)
			y -= 16;

			if (cl_ent == ent)
				tag = "990";
			else if (cl_ent->client->pers.rconx[0])
				tag = "096";
			else if (cl_ent->client->pers.admin>NOT_ADMIN)
				tag = "900"; // red
			else
				tag = "999"; // white fullbright

			if (ent->client->showscores==SCOREBOARD) 
			{
				if (ent->client->pers.version >= 114)
				{
					Com_sprintf (entry, sizeof(entry),
					"yv %i ds %s %i %i %i %i ",
					yofs + -60+i*16, tag, sorted[i], cl->ping, cl->resp.score, cl->resp.deposited);
				}
				else
				{
					strcpy( nfill, cl->pers.netname );
		
					if (strlen(nfill) > 13)
						nfill[13] = '\0';
					
					if (strlen(cl->pers.netname) < 13)
					{
						for (k=0; k<13-strlen(cl->pers.netname); k++)
							strcat( nfill, " " );
					}
					
					Com_sprintf (entry, sizeof(entry),
						"yv %i dmstr %s \"%s%4i %4i %5i \" ",
						yofs + -60+i*16, tag, nfill, cl->ping, cl->resp.score, cl->resp.deposited);
				}
			} 
			else 
			{
				int fc=0;
				char *fn="  -";
				
				strcpy( nfill, cl->pers.netname );
			
				if (strlen(nfill) > 13)
					nfill[13] = '\0';
				
				if (strlen(cl->pers.netname) < 13)
				{
					for (k=0; k<13-strlen(cl->pers.netname); k++)
						strcat( nfill, " " );
				}
				
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%s%4i %4i  ",
					yofs + -60+i*16, tag, nfill, cl->ping,
					teamplay->value==4?(cl->resp.accshot?cl->resp.acchit*1000/cl->resp.accshot:0):cl->resp.acchit);

				for (j=0;j<8;j++) 
				{
					if (cl->resp.fav[j]>fc) 
					{
						fc=cl->resp.fav[j];
						fn=weapnames[j];
					}
				}

				strcat(entry,fn);
				strcat(entry,"\"");
			}

			j = strlen(entry);
			
			if (stringlength + j > 1024)
				break;
			
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		if (real_total > total)
		{	// show the nuber of undisplayed players

			Com_sprintf (entry, sizeof(entry),
				"yv %i dmstr 777 \"(%i players)\" ",
				yofs + -60+i*16 + 6, real_total );

			j = strlen(entry);
			
			if (stringlength + j > 1024)
				break;
			
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		x = -8;
	}

skipscores:

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* DeathmatchScoreboardMessage */
void DeathmatchScoreboardMessage (edict_t *ent)
{
	char	temp[64];
	char	entry[1024];
	char	string[1400]="";
	int		stringlength=0;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total, realtotal;
	int		y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
	int		tmax;

	SHOWCHASENAME

	if (!ent->client->showscores)
		goto skipscores;
		
	if (ent->client->pers.spectator == SPECTATING)
		CHASEMESSAGE

	// sort the clients by score
	total = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if ((cl_ent->client->pers.spectator == SPECTATING) && (level.modeset != ENDMATCHVOTING))
			continue;

		if (fph_scoreboard)
			score = game.clients[i].resp.score * 36000 / (level.framenum - game.clients[i].resp.enterframe);
		else
		{
			score = game.clients[i].resp.score;

			if (dm_realmode->value && !teamplay->value)
				score -= game.clients[i].resp.deposited;
		}
		
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	realtotal = total;

	Com_sprintf (temp, sizeof(temp), "current map : %s",level.mapname);
	Com_sprintf (entry, sizeof(entry), "xm %i yt 5 dmstr 752 \"%s\" ", -5*strlen(temp), temp);

	j = strlen(entry);
	
	strcpy (string + stringlength, entry);
	stringlength += j;

	// Send the current leader ..only on intermission
	// cause gl_ztrick messes things up when player can move the mouse
	if (total > 0/* && level.intermissiontime*/)
	{
		Com_sprintf (entry, sizeof(entry),
			"leader %i ",
			sorted[0] );

		j = strlen(entry);
		
		strcpy (string + stringlength, entry);
		stringlength += j;
	} 

	// header
	if (ent->client->pers.version >= 114)
	{
		if (ent->client->showscores==SCOREBOARD) 
		{
			 if (dm_realmode->value && !teamplay->value)
			 {  // show deaths
				if (fph_scoreboard)
				  Com_sprintf (entry, sizeof(entry),
					  "xr %i yv %i dmstr 442 \"NAME         ping hits  fph\" ",
					  -36*10 - 10, -60+-2*14 );
				else
				  Com_sprintf (entry, sizeof(entry),
					  "xr %i yv %i dmstr 442 \"NAME         ping time  hits\" ",
					  -36*10 - 10, -60+-2*14 );
			}
			else // normal
			{
				if (fph_scoreboard)
				  Com_sprintf (entry, sizeof(entry),
					  "xr %i yv %i dmstr 442 \"NAME         ping hits   fph\" ",
					  -36*10 - 10, -60+-2*14 );
				else
				  Com_sprintf (entry, sizeof(entry),
					  "xr %i yv %i dmstr 442 \"NAME         ping time  hits\" ",
					  -36*10 - 10, -60+-2*14 );
			}
		} 
		else
			Com_sprintf (entry, sizeof(entry),
				"xr %i yv %i dmstr 442 \"NAME       deaths  acc   fav\"",
				-36*10 - 10, -60+-2*14 );
	}
	else
	{
		if (ent->client->showscores==SCOREBOARD) 
		{
		   if (dm_realmode->value && !teamplay->value)
		   {		// show deaths
			Com_sprintf (entry, sizeof(entry),
				"xr %i yv %i dmstr 442 \"            NAME   ping  hits  score\" ",
				-36*10 - 10, -60+-2*14 );
		   }
		   else	// normal
		   {
			Com_sprintf (entry, sizeof(entry),
				"xr %i yv %i dmstr 442 \"            NAME   ping  time   hits\" ",
				-36*10 - 10, -60+-2*14 );
		   }
		}  
		else
			Com_sprintf (entry, sizeof(entry),
				"xr %i yv %i dmstr 442 \"            NAME   ping   acc   fav\"",
				-36*10 - 10, -60+-2*14 );
	}

	j = strlen(entry);
	
	strcpy (string + stringlength, entry);
	stringlength += j;

	tmax=(1024-stringlength)/((ent->client->pers.version>=114 && ent->client->showscores==SCOREBOARD)?52:92);
	
	if (total>tmax) total=tmax-1;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		y = 32 + 32 * i;

		if (i < 2)
		y -= 16;

		if (cl_ent == ent)
			tag = "990";
		else if (cl_ent->client->pers.rconx[0])
			tag = "096";
		else if (cl_ent->client->pers.admin>NOT_ADMIN)
			 tag = "900"; // red
		else
			tag = "999"; // white fullbright

		if (ent->client->showscores==SCOREBOARD) 
		{
		  if (ent->client->pers.version >= 114)
		  {
			if (dm_realmode->value && !teamplay->value)
			{	// show deaths
				Com_sprintf (entry, sizeof(entry),
					"yv %i ds %s %i %i %i %i ",
					-60+i*16, tag, sorted[i], cl->ping, (level.framenum - cl->resp.enterframe)/600, cl->resp.score );
			}
			else
			{
				if (fph_scoreboard)
					Com_sprintf (entry, sizeof(entry),
						"yv %i ds %s %i %i %i %i ",
						-60+i*16, tag, sorted[i], cl->ping, cl->resp.score, cl->resp.score * 36000 / (level.framenum - cl->resp.enterframe));
				else
					Com_sprintf (entry, sizeof(entry),
						"yv %i ds %s %i %i %i %i ",
						-60+i*16, tag, sorted[i], cl->ping, (level.framenum - cl->resp.enterframe)/600, cl->resp.score );
			}
		}
		else
		{
			if (dm_realmode->value && !teamplay->value)
			{	// show deaths
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%16s   %4i   %3i   %4i\" ",
					-60+i*16, tag, cl->pers.netname, cl->ping, cl->resp.score, sortedscores[i] );
			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%16s   %4i   %3i   %4i\" ",
					-60+i*16, tag, cl->pers.netname, cl->ping, (level.framenum - cl->resp.enterframe)/600, cl->resp.score * 36000 / (level.framenum - cl->resp.enterframe) );
			}
		}
	} 
	else 
	{
			int fc=0;
			char *fn="   -";
	
			if (ent->client->pers.version >= 114) 
			{
				char	nfill[64];
				strcpy( nfill, cl->pers.netname );
			
				if (strlen(nfill) > 13)
					nfill[13] = '\0';
				
				if (strlen(cl->pers.netname) < 13)
				{
					for (k=0; k<13-strlen(cl->pers.netname); k++)
						strcat( nfill, " " );
				}
				
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%s%4i %4i ",
					-60+i*16, tag, nfill, cl->resp.deposited, cl->resp.accshot?cl->resp.acchit*1000/cl->resp.accshot:0);
			} 
			else
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%16s   %4i  %4i ",
					-60+i*16, tag, cl->pers.netname, cl->resp.deposited, cl->resp.accshot?cl->resp.acchit*1000/cl->resp.accshot:0);

			for (j=0;j<8;j++) 
			{
				if (cl->resp.fav[j]>fc) 
				{
					fc=cl->resp.fav[j];
					fn=weapnames[j];
				}
			} 
			
			strcat(entry,fn);
			strcat(entry,"\"");
		}

		j = strlen(entry);
		
		if (stringlength + j > 1024)
			break;
		
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	if ((stringlength + 30 < 1024) && realtotal > total)
	{
		Com_sprintf (entry, sizeof(entry),
			"yv %i dmstr 999 \" ( %i total players )\" ",
			-60+total*16, realtotal );

		j = strlen(entry);
		
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}
	}

skipscores:
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* DeathmatchScoreboard - - Here is where you determine what scoreboard to display */
void DeathmatchScoreboard (edict_t *ent)
{
	// KOOGLEBOT_ADD
	if (ent->is_bot)
		return;
	// KOOGLEBOT_END

	if (ent->client->showscores == SCORE_MOTD)
		MOTDScoreboardMessage (ent);
	else if (ent->client->showscores == SCORE_REJOIN)
		RejoinScoreboardMessage (ent);
	else if (ent->client->showscores == SPECTATORS)
		SpectatorScoreboardMessage (ent);
	else if (ent->client->showscores == SCORE_MAP_VOTE)
		VoteMapScoreboardMessage(ent);
	else {
		if (teamplay->value)
			if ((level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT) || (level.modeset == FREEFORALL))
				MatchSetupScoreboardMessage (ent);
			else
				GrabDaLootScoreboardMessage (ent);
		else	
			DeathmatchScoreboardMessage (ent);	
	}

	if (level.intermissiontime)
		gi.unicast (ent, true);
	else
		gi.unicast (ent, false);
}

/* NO SCOREBOARD MESSAGE */
void NoScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		j;

	string[0] = 0;
	stringlength = 0;

	if ((level.modeset != MATCHSETUP) && (level.modeset != FINALCOUNT)//SNAP
		&& ent->client->pers.spectator == SPECTATING) {
		SHOWCHASENAME
		CHASEMESSAGE
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/* DEATH MATCH SCOREBOARD NEW */
void DeathmatchScoreboardNew (edict_t *ent)
{
	// KOOGLEBOT_ADD
	if (ent->is_bot)
		return;
	// KOOGLEBOT_END

	switch(ent->client->showscores)
	{
	case SCORE_MOTD:
		MOTDScoreboardMessage (ent);
		break;
	case SCORE_REJOIN:
		RejoinScoreboardMessage (ent);
		break;
	case SPECTATORS:
		SpectatorScoreboardMessage (ent);
		break;
	case SCORE_MAP_VOTE:
		VoteMapScoreboardMessage(ent);
		break;
	case NO_SCOREBOARD:
		NoScoreboardMessage(ent);
		break;
	default:
		{
			if (teamplay->value)
				if ((level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT) || (level.modeset == FREEFORALL))
					MatchSetupScoreboardMessage (ent);
				else
					GrabDaLootScoreboardMessage (ent);
				else	
					DeathmatchScoreboardMessage (ent);	
		}    
		break;
	}

	if (level.intermissiontime)
		gi.unicast (ent, true);
	else
		gi.unicast (ent, false);
}

/* HelpComputer - Draw help computer. */
void HelpComputer (edict_t *ent, int page)
{
	char	string[1024];

	EP_PlayerLog (ent, page);

	level.helpchange = 0;
	
	Com_sprintf (string, sizeof(string),
		"picnote \"%s\" "
		" \"%s\" ",		
		game.helpmessage1,
		game.helpmessage2);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

#if 0
/* HelpComputer - Draw help computer. */
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 string2 \"Help screen not yet implemented\" "	);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
#endif

/* Cmd_Help_f - Display the current help message. */
void Cmd_Help_f (edict_t *ent, int page)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{

		// KOOGLEBOT_ADD
		if (ent->is_bot)
			return;
		// KOOGLEBOT_END

		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = NO_SCOREBOARD;

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged) && !(page))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;

	HelpComputer (ent, page);
}

/* Cmd_Score_f - This is the start of the scoreboard command, this sets the showscores value */
void Cmd_Score_f(edict_t *ent)
{
	int		i, found;
	edict_t	*dood;

	// KOOGLEBOT_ADD
	if (ent->is_bot)
		return;
	// KOOGLEBOT_END

	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores == SCOREBOARD)
		ent->client->showscores = SCOREBOARD2;
	else
		if (level.modeset == ENDMATCHVOTING)
			if (ent->client->showscores == SCORE_MAP_VOTE)
				ent->client->showscores = SCOREBOARD;
			else
				ent->client->showscores = SCORE_MAP_VOTE;

		else if (ent->client->showscores == SCORE_MOTD)
			ent->client->showscores = SCOREBOARD;
		else if (ent->client->showscores == SCOREBOARD2)
		{
			found = false;
			for_each_player(dood, i)
			{
				if (dood->client->pers.spectator == SPECTATING)
					found = true;
			}
			if (found)
				ent->client->showscores = SPECTATORS;
			else
				ent->client->showscores = NO_SCOREBOARD;
		}
		else if (ent->client->showscores == SPECTATORS)
		{
			if (level.intermissiontime)
				ent->client->showscores = SCOREBOARD;
			else
				ent->client->showscores = NO_SCOREBOARD;
		}
		else
			ent->client->showscores = SCOREBOARD;

	DeathmatchScoreboard(ent);
}

/* G_SetStats - Display the use stats. */
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	// if chasecam, show stats of player we are following
	if (ent->client->chase_target && ent->client->chase_target->client)
	{
		memcpy( ent->client->ps.stats, ent->client->chase_target->client->ps.stats, sizeof( ent->client->ps.stats ) );
		ent->client->ps.stats[STAT_LAYOUTS] = true;

		if (deathmatch->value) 
		{
			if (level.intermissiontime || ent->client->showscores)
				ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		} 
		else 
		{
			if (ent->client->showscores || ent->client->showhelp)
				ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		}
		
		// make sure they don't gain world ranking stats
		ent->client->ps.stats[STAT_FRAGS] = 0;
		ent->client->ps.stats[STAT_DEPOSITED] = 0;
		
		return;
	}
	
	// health
	{	
		int     index1, index2, index3;
		
		item = FindItem ("Cash");
		index1 = ITEM_INDEX (item);
		
		item = FindItem ("Large Cash Bag");
		index2 = ITEM_INDEX (item);
		
		item = FindItem ("Small Cash Bag");
		index3 = ITEM_INDEX (item);
		
		if (!((ent->client->ps.stats[STAT_PICKUP_STRING] == CS_ITEMS+index1) ||
			  (ent->client->ps.stats[STAT_PICKUP_STRING] == CS_ITEMS+index2) ||
			  (ent->client->ps.stats[STAT_PICKUP_STRING] == CS_ITEMS+index3)))
			ent->client->ps.stats[STAT_CASH_PICKUP] = 0;

		if (ent->client->invincible_framenum>level.framenum && ((ent->client->invincible_framenum-level.framenum)&4) && ent->client->pers.spectator!=SPECTATING) 
		{
			item = FindItem ("Helmet Armor Heavy");
			ent->client->ps.stats[STAT_ARMOR1] = 2023;
			
			item = FindItem ("Jacket Armor heavy");
			ent->client->ps.stats[STAT_ARMOR2] = 2023;
			
			item = FindItem ("Legs Armor Heavy");
			ent->client->ps.stats[STAT_ARMOR3] = 2023;
		} 
		else 
		  {
			item = FindItem ("Helmet Armor");
			ent->client->ps.stats[STAT_ARMOR1] = ent->client->pers.inventory[ITEM_INDEX(item)];
			
			item = FindItem ("Jacket Armor");
			ent->client->ps.stats[STAT_ARMOR2] = ent->client->pers.inventory[ITEM_INDEX(item)];
			
			item = FindItem ("Legs Armor");
			ent->client->ps.stats[STAT_ARMOR3] = ent->client->pers.inventory[ITEM_INDEX(item)];
			
			item = FindItem ("Helmet Armor Heavy");
			if (ent->client->pers.inventory[ITEM_INDEX(item)])
				ent->client->ps.stats[STAT_ARMOR1] = ent->client->pers.inventory[ITEM_INDEX(item)] + 1024;
			
			item = FindItem ("Jacket Armor heavy");
			if (ent->client->pers.inventory[ITEM_INDEX(item)])
				ent->client->ps.stats[STAT_ARMOR2] = ent->client->pers.inventory[ITEM_INDEX(item)] + 1024;
			
			item = FindItem ("Legs Armor Heavy");
			if (ent->client->pers.inventory[ITEM_INDEX(item)])
				ent->client->ps.stats[STAT_ARMOR3] = ent->client->pers.inventory[ITEM_INDEX(item)] + 1024;
		  }
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	// ammo
	if (!level.bar_lvl)
	{
	if (!ent->client->ammo_index )
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}

	if (ent->client->ammo_index)
	{
		item = &itemlist[ent->client->ammo_index];
	}
	else
	{
		item = NULL;
	}

	if ((item) && (item->pickup_name) && (((!strcmp(item->pickup_name, "Gas")))))
	{
		ent->client->ps.stats[STAT_CLIP] = -1;
	}
	else
	{
		ent->client->ps.stats[STAT_CLIP] = ent->client->pers.weapon_clip[ent->client->clip_index];
	}
	}
	else
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
		ent->client->ps.stats[STAT_CLIP] = -1;
	}
	
	// money
	if (deathmatch->value && teamplay->value!=1)
		ent->client->ps.stats[STAT_CASH] = ent->client->resp.score;
	else	// show cash
		ent->client->ps.stats[STAT_CASH] = ent->client->pers.currentcash;

	if (level.pawn_time)
		ent->client->ps.stats[STAT_FORCE_HUD] = 1;
	else
		ent->client->ps.stats[STAT_FORCE_HUD] = 0;

	if ((level.cut_scene_time) || (level.fadeendtime > level.time))
		ent->client->ps.stats[STAT_HIDE_HUD] = 1;
	else
		ent->client->ps.stats[STAT_HIDE_HUD] = 0;

	if ((level.cut_scene_time || level.cut_scene_end_count) && level.cut_scene_camera_switch)
		ent->client->ps.stats[STAT_SWITCH_CAMERA] = 1;
	else
		ent->client->ps.stats[STAT_SWITCH_CAMERA] = 0;
	
	if (level.time > ent->client->hud_enemy_talk_time)
	{
		ent->client->ps.stats[STAT_HUD_ENEMY_TALK] = 0;
	}
	else if ((ent->client->hud_enemy_talk_time - level.time) > 1.0)
	{
		ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] = 255;		
	}
	else
	{
		ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] = (short)((255.0/1.0)*(ent->client->hud_enemy_talk_time - level.time));
		
		if (ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] < 0)
			ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] = 0;
	}
	
	if (level.time > ent->client->hud_self_talk_time)
	{
		ent->client->ps.stats[STAT_HUD_SELF_TALK] = 0;
	}
	else if ((ent->client->hud_self_talk_time - level.time) > 1.0)
	{
		ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] = 255;		
	}
	else
	{
		ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] = (short)((255.0/1.0)*(ent->client->hud_self_talk_time - level.time));
		
		if (ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] < 0)
			ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] = 0;
	}

	{
		cast_memory_t *mem;
		int firsthire = 0;
		int i;
		edict_t *theent;

		ent->client->ps.stats[STAT_HUD_HIRE1] = 0;
		ent->client->ps.stats[STAT_HUD_HIRE2] = 0;

		theent = &g_edicts[0];
		for (i=0 ; i<globals.num_edicts ; i++, theent++)
		{
			if (!theent->inuse)
				continue;

			if ((theent->leader) && (theent->leader == ent))
			{
				mem = level.global_cast_memory[theent->character_index][ent->character_index];

				if (!mem)
					continue;

				if (mem->flags & MEMORY_HIRED)
				{   
					if (!firsthire)
					{
						if (theent->health > 0)
						{
							ent->client->ps.stats[STAT_HUD_HIRE1] = (int)((100.0/(float)theent->max_health)*((float)theent->health));
							
							if ((theent->enemy) && (theent->enemy->health > 0))
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 3;
							}
							else if (theent->cast_info.aiflags & AI_MOVEOUT || theent->cast_info.aiflags & AI_DOKEY)
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 1;
							}
							else if (theent->cast_info.aiflags & AI_HOLD_POSITION)
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 2;
							}
							else
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 0;
							}
								
							firsthire = 1;
						}
					}
					else
					{
						if (theent->health > 0)
						{
							ent->client->ps.stats[STAT_HUD_HIRE2] = (int)((100.0/(float)theent->max_health)*((float)theent->health));
						
							if ((theent->enemy) && (theent->enemy->health > 0))
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 3;
							}
							else if (theent->cast_info.aiflags & AI_MOVEOUT || theent->cast_info.aiflags & AI_DOKEY)
							{
								ent->client->ps.stats[STAT_HUD_HIRE2_CMD] = 1;
							}
							else if (theent->cast_info.aiflags & AI_HOLD_POSITION)
							{
								ent->client->ps.stats[STAT_HUD_HIRE2_CMD] = 2;
							}
							else
							{
								ent->client->ps.stats[STAT_HUD_HIRE2_CMD] = 0;
							}						
						}
						
						break;
					}
				}
			}
		}
	}

	ent->client->ps.stats[STAT_HUD_INV] = 0;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Battery"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 1;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Coil"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 2;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Watch"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 4;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Safe docs"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 8;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Fuse"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 16;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Valve"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 32;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Lizzy Head"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 64;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Whiskey"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 128;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Oil Can"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 256;	

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Ticket"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 512;		

	// armor
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("Gas"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);

	// pickup message
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
		ent->client->ps.stats[STAT_PICKUP_COUNT] = 0;	
	}
	else if ((ent->client->pickup_msg_time - level.time) > 1.5)
	{
		ent->client->ps.stats[STAT_PICKUP_COUNT] = 255;		
	}
	else
	{
		ent->client->ps.stats[STAT_PICKUP_COUNT] = (short)((255.0/1.5)*(ent->client->pickup_msg_time - level.time));
		
		if (ent->client->ps.stats[STAT_PICKUP_COUNT] < 0)
			ent->client->ps.stats[STAT_PICKUP_COUNT] = 0;
	}

	// Timer for the hud
	if ((int)timelimit->value)
	{
		if (level.modeset == FREEFORALL)
			ent->client->ps.stats[STAT_TIMER] = ((350 -  level.framenum ) / 10);
		else if (level.modeset == FINALCOUNT)
			ent->client->ps.stats[STAT_TIMER] =	((150 - (level.framenum - level.startframe)) / 10);
		else if (level.modeset == ENDMATCHVOTING)
			ent->client->ps.stats[STAT_TIMER] =	((300 - (level.framenum - level.startframe)) / 10);
		else if ((level.modeset == MATCH) || (level.modeset == TEAMPLAY))
			if (level.framenum > (level.startframe + (((int)timelimit->value  * 600) - 605)))  
				ent->client->ps.stats[STAT_TIMER] = ((((int)timelimit->value * 600) + level.startframe - level.framenum ) / 10);
			else
				ent->client->ps.stats[STAT_TIMER] = ((((int)timelimit->value * 600) + level.startframe - level.framenum ) / 600);
		else 
			ent->client->ps.stats[STAT_TIMER] = 0;

		if (ent->client->ps.stats[STAT_TIMER] < 0 )
			ent->client->ps.stats[STAT_TIMER] = 0;
	}
	else
		ent->client->ps.stats[STAT_TIMER] = 0;

	// selected item
	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	// layouts
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	// frags
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
	ent->client->ps.stats[STAT_DEPOSITED] = ent->client->resp.deposited;

	// help icon / current weapon if not shown
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_HELPICON] = level.helpchange;

	// Teamplay
	if (deathmatch->value && teamplay->value)
	{
		int i;

		// show team scores
		for (i=0; i<2; i++)
		{
			ent->client->ps.stats[STAT_TEAM1_SCORE + i] = team_cash[1+i];	// set score

			ent->client->ps.stats[STAT_TEAM1_FLASH + i] = 0;	// set normal

			if (last_safe_withdrawal[i+1] > last_safe_deposit[i+1])
			{
				if (last_safe_withdrawal[i+1] > (level.time - 3.0))
					ent->client->ps.stats[STAT_TEAM1_FLASH + i] = 2;	// flash red
			}
			else
			{
				if (last_safe_deposit[i+1] > (level.time - 3.0))
					ent->client->ps.stats[STAT_TEAM1_FLASH + i] = 1;	// flash green
			}
		}

		if (teamplay->value==1) 
		{
			// show bagged cash
			ent->client->ps.stats[STAT_BAGCASH] = ent->client->pers.bagcash; //STAT_BAG_CASH
			ent->client->ps.stats[23]=0;
		
			if (ent->client->pers.team) 
			{
				vec3_t dir, start, end;
				trace_t tr;
				AngleVectors( ent->client->ps.viewangles, dir, NULL, NULL );
				VectorCopy( ent->s.origin, start );
				start[2] += ent->viewheight;
				VectorMA( start, 4000, dir, end );
				tr = gi.trace( start, NULL, NULL, end, ent, MASK_SHOT );
				if (tr.fraction<1 && tr.ent->client && ent->client->pers.team==tr.ent->client->pers.team)
					ent->client->ps.stats[23]=tr.ent->client->pers.bagcash+tr.ent->client->pers.currentcash;
			}
		}
	}
}

/* FollowEnt */
void FollowEnt (edict_t *ent)
{
	if (ent->target2_ent)
	{
		vec3_t	avect;

		VectorSubtract(ent->target2_ent->s.origin, ent->s.origin, avect);

		VectorNormalize(avect);
		vectoangles(avect, ent->s.angles);	
	}
}

/* BEGIN CUT SCENE - Feb 23rd 1999 */
void BeginCutScene (edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

	level.cut_scene_camera_switch = 1;
	level.cut_scene_time = level.time + 0.1;
	
	VectorCopy(ent->s.angles, ent->save_avel);
	VectorCopy(ent->s.origin, ent->savecameraorigin);
	
	if (ent->cameraangle[0])
	{
		ent->s.angles[0] += ent->cameraangle[0];
	}

	if (ent->cameraangle[1])
	{
		ent->s.angles[1] += ent->cameraangle[1];
	}

	if (ent->cameraangle[2])
	{
		ent->s.angles[2] += ent->cameraangle[2];
	}

	FollowEnt (ent);

	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);
	
	VectorCopy (player->s.origin, level.player_oldpos);
	VectorCopy (player->s.angles, level.player_oldang);
	
	MoveClientToCutSceneCamera (player, ent->deadticks);
}

/* NEW CUT SCENE CAMERA */
void NewCutSceneCamera (edict_t *ent)
{
	edict_t *player;
	player = g_edicts + 1;

	level.cut_scene_camera_switch = 1;

	VectorCopy(ent->s.angles, ent->save_avel);
	VectorCopy(ent->s.origin, ent->savecameraorigin);
	
	level.cut_scene_time = level.time + 0.1;

	ent->alphalevel = level.time + ent->reactdelay;	

	if (ent->cameraangle[0])
	{
		ent->s.angles[0] += ent->cameraangle[0];
	}

	if (ent->cameraangle[1])
	{
		ent->s.angles[1] += ent->cameraangle[1];
	}

	if (ent->cameraangle[2])
	{
		ent->s.angles[2] += ent->cameraangle[2];
	}

	FollowEnt (ent);
	
	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);
	
	MoveClientToCutSceneCamera (player, ent->deadticks);
}

/* ADJUST CUT SCENE CAMERA */
void AdjustCutSceneCamera(edict_t *ent)
{
	edict_t *player;
	player = g_edicts + 1;

	if (ent->accel)
	{
		if (ent->delay < (level.time - ent->timestamp))
		{
			if ((ent->delay + ent->reactdelay) < (level.time - ent->timestamp))
			{
				if (ent->speed > 0)
				{
					ent->speed -= ent->decel * FRAMETIME;

					if (ent->speed < 0)
						ent->speed = 0;
				}
			}
			else if (ent->speed < 1)
			{
				ent->speed += ent->accel * FRAMETIME;
				if (ent->speed > 1)
					ent->speed = 1;
			}
		}
	}

	level.cut_scene_camera_switch = 0;	

	level.cut_scene_time = level.time + 0.1;

	if (ent->rotate[0])
	{
		ent->s.angles[0] += ent->rotate[0]*FRAMETIME*ent->speed;
	}

	if (ent->rotate[1])
	{
		ent->s.angles[1] += ent->rotate[1]*FRAMETIME*ent->speed;
	}

	if (ent->rotate[2])
	{
		ent->s.angles[2] += ent->rotate[2]*FRAMETIME*ent->speed;
	}

	// Ridah, not sure what ->save_avel is, but we need a way of setting an absolute velocity
	VectorMA(ent->s.origin, FRAMETIME*ent->speed, ent->cameravel, ent->s.origin);

	if (ent->cameravelrel[0])
	{
		vec3_t	forward;

		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorMA(ent->s.origin, ent->cameravelrel[0]*FRAMETIME*ent->speed, forward, ent->s.origin);
	}
	
	if (ent->cameravelrel[1])
	{
		vec3_t	right;

		AngleVectors (ent->s.angles, NULL, right, NULL);
		VectorMA(ent->s.origin, ent->cameravelrel[1]*FRAMETIME*ent->speed, right, ent->s.origin);
	}
	
	if (ent->cameravelrel[2])
	{
		vec3_t	up;

		AngleVectors (ent->s.angles, NULL, NULL, up);
		VectorMA(ent->s.origin, ent->cameravelrel[2]*FRAMETIME*ent->speed, up, ent->s.origin);
	}	

	FollowEnt (ent);
	
	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);

	// Fade out
	if (ent->duration)
	{
		if ((level.time + ent->duration) > ent->alphalevel) 	
		{
			level.totalfade = ent->duration;
			level.fadeendtime = level.time + level.totalfade;		
			level.inversefade = 1;
			ent->duration = 0;
		}
	}

	MoveClientToCutSceneCamera (player, ent->deadticks);
}

/* END CUT SCENE */
void EndCutScene (edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

	level.cut_scene_camera_switch = 0;

	player->client->ps.fov = 90;
	
	VectorCopy (level.player_oldpos, player->s.origin);
	VectorCopy (level.player_oldang, player->s.angles);
	
	level.cut_scene_camera_switch = 1;
	level.cut_scene_end_count = 5;

	level.cut_scene_time = 0;
	
	player->client->ps.pmove.origin[0] = player->s.origin[0]*8;
	player->client->ps.pmove.origin[1] = player->s.origin[1]*8;
	player->client->ps.pmove.origin[2] = player->s.origin[2]*8;
	
	VectorCopy (level.player_oldang, player->client->ps.viewangles);

	if (!player->client->pers.weapon)
		return;
	
	{
		int i;
		
		if (!strcmp (player->client->pers.weapon->pickup_name , "SPistol"))
		{
			if ((player->client->pers.pistol_mods) && (player->client->pers.pistol_mods & WEAPON_MOD_DAMAGE))
				player->client->ps.gunindex = gi.modelindex("models/weapons/silencer_mdx/magnum.mdx");
			else
				player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
		}
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Pistol"))
		{
			if ((player->client->pers.pistol_mods) && (player->client->pers.pistol_mods & WEAPON_MOD_DAMAGE))
				player->client->ps.gunindex = gi.modelindex("models/weapons/v_colt/magnum.mdx");
			else
				player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
		}
		else if (!strcmp (player->client->pers.weapon->pickup_name, "Heavy machinegun"))
		{
			if (player->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET)
				player->client->ps.gunindex = gi.modelindex("models/weapons/v_hmgcool/hmgcool.mdx");
			else
				player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
		}
		else
			player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);

		memset(&(player->client->ps.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);

		player->client->ps.num_parts++;
		player->client->ps.model_parts[PART_HEAD].modelindex = player->client->ps.gunindex;
		for (i=0; i<MAX_MODELPART_OBJECTS; i++)
			player->client->ps.model_parts[PART_HEAD].skinnum[i] = 0; // will we have more than one skin???
	
		// set arm model
		if (!strcmp(player->client->pers.weapon->pickup_name, "Shotgun"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/shotgun/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/shotgun/shell.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "Pipe"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/blakjak/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "Crowbar"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/crowbar/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "FlameThrower"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_flamegun/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "Bazooka"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/shell_a.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???		
			
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN2].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN2].skinnum[i] = 0; // will we have more than one skin???		
	
		}
		else if (!strcmp (player->client->pers.weapon->pickup_name , "SPistol"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/silencer_mdx/handl.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/silencer_mdx/handr.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/silencer_mdx/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN].skinnum[i] = 0;

		}
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Pistol"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_colt/handl.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_colt/handr.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/v_colt/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN].skinnum[i] = 0;
		}
		 else if (!strcmp (player->client->pers.weapon->pickup_name , "Heavy machinegun"))
		 {
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_hmg/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_hmg/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
		}
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Grenade Launcher"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_grenade_launcher/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_grenade_launcher/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
	}
}

/* MOVE CLIENT TO CUT SCENE */
void MoveClientToCutScene (edict_t *ent)
{
	VectorCopy (level.cut_scene_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.cut_scene_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.cut_scene_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.cut_scene_origin[2]*8;
	VectorCopy (level.cut_scene_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
		
	// note to self
	// this may cause a problem
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
			
	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;
	ent->client->quadfire_framenum = 0;
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
}

/* MOVE CLIENT TO CUT SCENE CAMERA */
void MoveClientToCutSceneCamera (edict_t *ent, int fov)
{
	ent->client->ps.fov = fov;
	
	VectorCopy (level.cut_scene_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.cut_scene_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.cut_scene_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.cut_scene_origin[2]*8;
	VectorCopy (level.cut_scene_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	
	// note to self
	// this may cause a problem
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;
	ent->client->quadfire_framenum = 0;
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
}

/* MOVE CLIENT TO PAWNOMATIC */
void MoveClientToPawnoMatic (edict_t *ent)
{
	ent->client->ps.pmove.origin[0] = level.cut_scene_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.cut_scene_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.cut_scene_origin[2]*8;
	
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;
	ent->client->quadfire_framenum = 0;
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
}
