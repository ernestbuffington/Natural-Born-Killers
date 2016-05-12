#include "g_local.h"
#include "file.h"
#include <stdio.h>

void fgetline (FILE* infile, char* buffer)
{
	int		i=0;
	char	ch;

	ch = fgetc(infile);

	while ((ch != EOF) && (ch != '\n') && (i < (MAX_STRING_LENGTH-1)))
	{ buffer[i++] = ch; ch = fgetc(infile); } 
	
	if ((ch != '\n') && (ch != EOF))	// Keep going to the eof
	ch = fgetc(infile);

	buffer[i] = '\0';
	return;
}

int proccess_line(char*	buffer)
{
	// Make sure the line's not empty
	if (strlen(buffer) == 0 || buffer[0] == '\n') return COMMENT_LINE;
	// Check to see if this is a comment line
	if (buffer[0] == '/' && buffer[1] == '/')	return COMMENT_LINE;
	// Check if this is the custom map keyword
	if (strstr(buffer, ADMIN_CODE_STRING)) return ADMIN_CODE_KEYWORD;
	// Check if this is the custom map keyword
	if (strstr(buffer, CUSTOM_MAP_STRING)) return CUSTOM_MAP_KEYWORD;
	// Check if this is the map rotation keyword
	if (strstr(buffer, MAP_ROTATION_STRING)) return MAP_ROTATION_KEYWORD;
	// Check if this is the default rotation keyword
	if (strstr(buffer, DEFAULT_MAP_STRING)) return DEFAULT_MAP_KEYWORD;

	if (strstr(buffer, DEFAULT_TEAMPLAY_STRING)) return DEFAULT_TEAMPLAY_KEYWORD;

	if (strstr(buffer, DEFAULT_DMFLAGS_STRING)) return DEFAULT_DMFLAGS_KEYWORD;

	if (strstr(buffer, DEFAULT_PASSWORD_STRING)) return DEFAULT_PASSWORD_KEYWORD;

	if (strstr(buffer, DEFAULT_MOTD_STRING)) return DEFAULT_MOTD_KEYWORD;

	if (strstr(buffer, DEFAULT_TIME_STRING)) return DEFAULT_TIME_KEYWORD;

	if (strstr(buffer, DEFAULT_FRAG_STRING)) return DEFAULT_FRAG_KEYWORD;

	if (strstr(buffer, DEFAULT_CASH_STRING)) return DEFAULT_CASH_KEYWORD;

	if (strstr(buffer, ALLOW_MAP_VOTING_STRING)) return MAP_VOTING_KEYWORD;

	if (strstr(buffer, BAN_NAME_STRING)) return BAN_NAME_KEYWORD;

	if (strstr(buffer, BAN_IP_STRING)) return BAN_IP_KEYWORD;

	if (strstr(buffer, SCOREBOARD_FIRST_STRING)) return SCOREBOARD_FIRST_KEYWORD;

	if (strstr(buffer, FPH_SCOREBOARD_STRING)) return FPH_SCOREBOARD_KEYWORD;

	if (strstr(buffer, DISABLE_ADMIN_STRING)) return DISABLE_ADMIN_KEYWORD;

	if (strstr(buffer, DEFAULT_REAL_STRING)) return DEFAULT_REAL_KEYWORD;

	if (strstr(buffer, FIXED_GAMETYPE_STRING)) return FIXED_GAMETYPE_KEYWORD;

	if (strstr(buffer, ENABLE_PASSWORD_STRING)) return ENABLE_PASSWORD_KEYWORD;

	if (strstr(buffer, RCONX_FILE_STRING)) return RCONX_FILE_KEYWORD;

/*    if (strstr(buffer, URL_STRING))
		return URL_KEYWORD;*/

	if (strstr(buffer, KEEP_ADMIN_STRING)) return KEEP_ADMIN_KEYWORD;

	if (strstr(buffer, DEFAULT_RANDOM_MAP_STRING)) return DEFAULT_RANDOM_MAP_KEYWORD;

	if (strstr(buffer, DISABLE_ANON_TEXT_STRING)) return DISABLE_ANON_TEXT_KEYWORD;

	if (strstr(buffer, DISABLE_CURSE_STRING)) return DISABLE_CURSE_KEYWORD;

	if (strstr(buffer, UNLIMITED_CURSE_STRING)) return UNLIMITED_CURSE_KEYWORD;

	if (strstr(buffer, DISABLE_ASC_STRING)) return DISABLE_ASC_KEYWORD;

/*    if (strstr(buffer, ENABLE_NOFLAMEHACK_CHECK_STRING))
		return ENABLE_NOFLAMEHACK_CHECK_KEYWORD;*/

	if (strstr(buffer, ENABLE_SEE_KILLER_HEALTH_STRING)) return ENABLE_SEE_KILLER_HEALTH_KEYWORD;

	return FOUND_STRING;
}

int proccess_ini_file()
{
	FILE*	infile;		// Config file to be opened
	int	status = OK;
	int	mode = -1;
	char	buffer[MAX_STRING_LENGTH];
	char	map[32], dummy[32];
	char	filename[32],dir[32];
	cvar_t	*game_dir;

	default_map[0]=0;
	default_teamplay[0]=0;
	default_dmflags[0]=0;
	default_password[0]=0;
	default_dm_realmode[0]=0;

	admincode[0]=0;

	custom_map_filename[0]=0;
	ban_name_filename[0]=0;
	ban_ip_filename[0]=0;
	rconx_file[0]=0;
//    server_url[0]=0;

	// Set number of custom/rotation maps to 0
//	game.num_rmaps = 0;
	game.num_cmaps = 0;
	allow_map_voting = false;
	disable_admin_voting = false;
	scoreboard_first = false;
	num_custom_maps = 0;
	num_netnames = 0;
	num_ips = 0;
	fixed_gametype = false;
	enable_password = false;
	keep_admin_status = false;
	default_random_map = false;
	disable_anon_text = false;
	disable_curse = false;
	unlimited_curse = false;
//    enable_asc = false;
	enable_killerhealth = false;

	// Open config file
	game_dir = gi.cvar("game", "", 0);
	
	if (game_dir->string[0] == 0)
	{
		strcpy(dir, "main");
	}
	else
	{
		strcpy(dir, game_dir->string);
	}

	Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"config-files"DIR_SLASH"comp.ini",dir);
	
	infile = fopen(filename, "r");
	
	if (infile == NULL)	// if the fucking file is not there trow me a fucking error!
	return FILE_OPEN_ERROR;
	
	// Read first line of the file
	fgetline(infile, buffer);

	while (!feof(infile))	// while there's still shit to parse!
	{
		switch (proccess_line(buffer))	// Determine what to do with the line, based off of what's in the fucking return!
		{
		 // Admin Password
		 case ADMIN_CODE_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	   
			strncpy(admincode, map, 16);
			break;
		 // Return Comment Line
		 case COMMENT_LINE:	                           
			break;
		 // Add Maps to Map Rotation
		 case MAP_ROTATION_KEYWORD:	
			mode = ADD_ROTATION;
			break;
		 // Add MOTD Line
		 case DEFAULT_MOTD_KEYWORD:	
			mode = ADD_MOTD_LINE;
			break;
		 // Return Found String
		 case FOUND_STRING:	
			if (mode == ADD_CUSTOM)
				add_to_custom_maps(buffer);
			if (mode == ADD_MOTD_LINE)
				add_to_MOTD(buffer);
			break;
		 // Return Default Map
		 case DEFAULT_MAP_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_map, map, 32);
			break;
		 // Return Default Teamplay
		 case DEFAULT_TEAMPLAY_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_teamplay, map, 16);
			break;
		 // Return Default DMFLAGS
		 case DEFAULT_DMFLAGS_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_dmflags, map, 16);
			break;
		 // Return Server Password If Any
		 case DEFAULT_PASSWORD_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_password, map, 16);
			break;
		 // Return Deafult Timelimit
		 case DEFAULT_TIME_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_timelimit, map, 16);
			break;
		 // Return Default Cashlimit
		 case DEFAULT_CASH_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_cashlimit, map, 16);
			break;
		 // Return Default Fraglimit
		 case DEFAULT_FRAG_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_fraglimit, map, 16);
			break;
		 // Add Maps To Custom Map List
		 case CUSTOM_MAP_KEYWORD:                   
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(custom_map_filename, map, 32);
			break;
		 // Allow Map Voting
		 case MAP_VOTING_KEYWORD: 
			allow_map_voting = true;
			break;
		 // Return Ban Filename
		 case BAN_NAME_KEYWORD: 
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(ban_name_filename, map, 32);
			break;
		 // Return Ban IP File Name
		 case BAN_IP_KEYWORD: 
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(ban_ip_filename, map, 32);
			break;
		 // Show The Scoreboard 1st If Map Vote Enabled
		 case SCOREBOARD_FIRST_KEYWORD:             
			scoreboard_first = true;
			break;
		 // Show The Frags Per Hour Scoreboard
		 case FPH_SCOREBOARD_KEYWORD:              
			fph_scoreboard = true;
			break;
		 // Disable Admin Voting
		 case DISABLE_ADMIN_KEYWORD:              
			disable_admin_voting = true;
			break;
		 // Return The Default Real Mode
		 case DEFAULT_REAL_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(default_dm_realmode, map, 16);
			break;
		 // Make The Server Stay In One Game Mode
		 case FIXED_GAMETYPE_KEYWORD:              
			fixed_gametype = true;
			break;
		 // Enable The Server Password
		 case ENABLE_PASSWORD_KEYWORD:            
			enable_password = true;
			break;
		 // Return The Rcon File Name
		 case RCONX_FILE_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	
			strncpy(rconx_file, map, 32);
			break;
 
		 //////////////////////////////////////////////////////////////////////////
         //case URL_KEYWORD:
		 //	sscanf(buffer, "%s %s", dummy, map);	// not sure  wtf this is
		 //	strncpy(server_url, map, 64);
		 //	break;
		 //////////////////////////////////////////////////////////////////////////

		 // Make Sure The Admin Stays An Admin After Map Change
		 case KEEP_ADMIN_KEYWORD:                     
			keep_admin_status = true;
			break;
		 // Let The Server Pick A Random Map To Change Too If Map Vote Is Enabled And Nobody Votes
		 case DEFAULT_RANDOM_MAP_KEYWORD:           
			default_random_map = true;
			break;
		 // Ban Anon Text
		 case DISABLE_ANON_TEXT_KEYWORD:            
			disable_anon_text = true;
			break;
		 // Disable Cursing
		 case DISABLE_CURSE_KEYWORD:            
			disable_curse = true;
			break;
		 // Allow As Much Cursing As Players Like
		 case UNLIMITED_CURSE_KEYWORD:
			unlimited_curse = true;
			break;
		 // Anti Spawn Camp Addon
		 case DISABLE_ASC_KEYWORD:                 
		 //	enable_asc = true;
			gi.cvar_set("anti_spawncamp", "1");
			break;

         //////////////////////////////////////////////////////////////////////////
	     //case ENABLE_NOFLAMEHACK_CHECK_KEYWORD:    // not sure what happened here??
		 //	//noflamehackcheck = true;
		 //	gi.cvar_set("kick_flamehack", "1");
		 //	break;
		 //////////////////////////////////////////////////////////////////////////
		
         // This Appears To Be Disabled? SLOPPY SHIT <- Alfred?
		 case ENABLE_SEE_KILLER_HEALTH_KEYWORD:      
			enable_killerhealth = true;
			break;
		
		 default:
			// KOOGLEBOT_ADD added by Ghost 5/4/2016
			// This has been modified to let you know just what the un-handled line is
			// when it finds one inside the ini file. 
			// This is like it should be and should have been in the first place!
			gi.dprintf("Unhandled line %s in nbk/config-files/comp.ini\n",buffer); 
			// KOOGLEBOT_END added by Ghost 5/4/2016
		}
		
		fgetline(infile, buffer);		// Retrieve next line from the input file
	}

	// Close which ever ini file happens to be open at the moment!
	fclose(infile);

	if (ban_name_filename[0]) 
	{
	  Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, ban_name_filename);
	
	  infile = fopen(filename, "r");
	
	  if (infile != NULL)	
	  {
	      // Read first line of the file
		  fgetline(infile, buffer);
		  
		  num_netnames = 0; // Set net names to zero
		  
		  while (!feof(infile))	// While there's still stuff
		  {
			  int i;
			  if (strlen(buffer) == 0 || buffer[0] == '\n')
			  {
				fgetline(infile, buffer);
				continue;
			  }
		      // Check to see if this is a comment line
			  if (buffer[0] == '/' && buffer[1] == '/')
			  {
				fgetline(infile, buffer);
				continue;
			  }
			
			  for (i=0;i<strlen(buffer);i++) buffer[i]=tolower(buffer[i]);
			  strncpy(netname[num_netnames].value,buffer,16);
			  fgetline(infile, buffer);
			  num_netnames++;
			  if (num_netnames==100) 
			  break;
		  }
		
		  fclose(infile);
	  }
	}

	if (ban_ip_filename[0]) 
	{
	  Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, ban_ip_filename);
	
	  infile = fopen(filename, "r");
	
	  if (infile != NULL)	
	  {
	     // Read first line of the file
		 fgetline(infile, buffer);
		
		 num_ips = 0; // set num ip's to zero
		
		 while (!feof(infile))	// while there's still shit left to parse!
		 {
			if (strlen(buffer) == 0 || buffer[0] == '\n')
			{
				fgetline(infile, buffer);
				continue;
			}
		    
			// Check to see if this is a comment line
			if (buffer[0] == '/' && buffer[1] == '/')
			{
				fgetline(infile, buffer);
				continue;
			}
			
			strncpy(ip[num_ips].value,buffer,16);
			
			fgetline(infile, buffer);
			
			num_ips++;
			
			if (num_ips==100) 
			break;
		 }
		
		 fclose(infile);
	  }
	}

	if (rconx_file[0]) 
	{
	   Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, rconx_file);
	
	   infile = fopen(filename, "r");
	
	   if (infile != NULL)	
	   {
	     // Read first line of the file
		 fgetline(infile, buffer);
		
		 num_rconx_pass = 0; // set rcon passwords to zero
		
		 while (!feof(infile))	// while there's still shit to parse!
		 {
			if (strlen(buffer) == 0 || buffer[0] == '\n')
			{
				fgetline(infile, buffer);
				continue;
			}
		    
			// Check to see if this is a comment line
			if (buffer[0] == '/' && buffer[1] == '/')
			{
				fgetline(infile, buffer);
				continue;
			}
			
			strncpy(rconx_pass[num_rconx_pass].value,buffer,32);
			rconx_pass[num_rconx_pass].value[31]=0;
			
			fgetline(infile, buffer);
			
			num_rconx_pass++;
			
			if (num_rconx_pass==100) 
			break;
		 }
		
		 fclose(infile);
	   
	   }
	}

	return OK;
}

int read_map_file()
{

	FILE*	infile;		// Config file to be opened
	char	buffer[MAX_STRING_LENGTH];
	char	map[32], rank[16];
	char	filename[32], dir[32];
	cvar_t	*game_dir;

	game_dir = gi.cvar("game", "", 0);
	
	if (game_dir->string[0] == 0)
	{
		strcpy(dir, "main");
	}
	else
	{
		strcpy(dir, game_dir->string);
	}

	if (custom_map_filename[0]) 
	{
	   Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, custom_map_filename);
	   
	   infile = fopen(filename, "r");
	
	   if (infile == NULL)	return FILE_OPEN_ERROR;

	   num_custom_maps = 0; // set custom maps to zero

	  // Read first line of the file
	  fgetline(infile, buffer);
	
	  while (!feof(infile))	// while there's still stuff
	  {
		 if (strlen(buffer) == 0 || buffer[0] == '\n')
		 {
			fgetline(infile, buffer);
			continue;
		 }
	     
		 // Check to see if this is a comment line
		 if (buffer[0] == '/' && buffer[1] == '/')
		 {
			fgetline(infile, buffer);
			continue;
		 }

		 sscanf(buffer, "%s %s", rank, map);	
		 strncpy(custom_list[num_custom_maps].custom_map, map, 32);

		 custom_list[num_custom_maps].rank = atoi(rank);

		 total_rank += custom_list[num_custom_maps].rank;
		
		 num_custom_maps++;
		
		 fgetline(infile, buffer);
	  }
	}

	return OK;
}

void add_to_custom_maps(char* buffer)
{
	strncpy(custom_list[game.num_cmaps].custom_map, buffer, 16);
	game.num_cmaps++;
}

void add_to_MOTD(char* buffer)
{
	strncpy(MOTD[game.num_MOTD_lines].textline, buffer, 99);
	game.num_MOTD_lines++;
}


// write to maps file
int write_map_file()
{
	FILE*	outfile;
	char	temp[32],dir[32];
	int		i;
	cvar_t	*game_dir;

	game_dir = gi.cvar("game", "", 0);

	if (game_dir->string[0] == 0)
	{
		strcpy(dir, "main");
	}
	else
	{
		strcpy(dir, game_dir->string);
	}

	if (custom_map_filename[0]) 
	{
	   Com_sprintf (temp, sizeof(temp), "%s"DIR_SLASH"%s",dir,custom_map_filename);
	
	   outfile = fopen(temp, "w+");
	
	   if (outfile == NULL)	
	   return FILE_OPEN_ERROR;

	   for (i=0; i< num_custom_maps; i++)	
	   fprintf(outfile, "%d %s\n",custom_list[i].rank, custom_list[i].custom_map);

	   fprintf(outfile, "\n");

	   fclose(outfile);
	}

	return OK;
}
