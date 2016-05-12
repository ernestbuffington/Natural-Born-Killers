///////////////////////////////////////////////////////////////////////
//
//  kooglebot_cmds.c - Main internal command processor
//
///////////////////////////////////////////////////////////////////////
#include "..\g_local.h"
#include "kooglebot.h"

qboolean debug_mode = false;
///////////////////////////////////////////////////////////////////////
// Special command processor
///////////////////////////////////////////////////////////////////////
qboolean KOOGLECM_Commands(edict_t *ent)
{
	char	*cmd;
	int node;

	cmd = gi.argv(0);

	if (Q_stricmp(cmd, "addnode") == 0 && debug_mode)
		ent->last_node = KOOGLEND_AddNode(ent, atoi(gi.argv(1)));

	else if (Q_stricmp(cmd, "removelink") == 0 && debug_mode)
		KOOGLEND_RemoveNodeEdge(ent, atoi(gi.argv(1)), atoi(gi.argv(2)));

	else if (Q_stricmp(cmd, "addlink") == 0 && debug_mode)
		KOOGLEND_UpdateNodeEdge(atoi(gi.argv(1)), atoi(gi.argv(2)));

	else if (Q_stricmp(cmd, "showpath") == 0 && debug_mode)
		KOOGLEND_ShowPath(ent, atoi(gi.argv(1)));

	else if (Q_stricmp(cmd, "findnode") == 0 && debug_mode)
	{
		node = KOOGLEND_FindClosestReachableNode(ent, BOTNODE_DENSITY, BOTNODE_ALL);
		safe_bprintf(PRINT_MEDIUM, "node: %d type: %d x: %f y: %f z %f\n", node, nodes[node].type, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]);
	}

	else if (Q_stricmp(cmd, "movenode") == 0 && debug_mode)
	{
		node = atoi(gi.argv(1));
		nodes[node].origin[0] = atof(gi.argv(2));
		nodes[node].origin[1] = atof(gi.argv(3));
		nodes[node].origin[2] = atof(gi.argv(4));
		safe_bprintf(PRINT_MEDIUM, "node: %d moved to x: %f y: %f z %f\n", node, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]);
	}

	else
		return false;

	return true;
}


///////////////////////////////////////////////////////////////////////
// Called when the level changes, store maps and bots (disconnected)
///////////////////////////////////////////////////////////////////////
void KOOGLECM_Store()
{
	KOOGLEND_SaveNodes();
}

///////////////////////////////////////////////////////////////////////
// These routines are bot safe print routines, all id code needs to be 
// changed to these so the bots do not blow up on messages sent to them. 
// Do a find and replace on all code that matches the below criteria. 
//
// (Got the basic idea from Ridah)
//	
//  change: gi.cprintf to safe_cprintf
//  change: gi.bprintf to safe_bprintf
//  change: gi.centerprintf to safe_centerprintf
// 
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Debug print, could add a "logging" feature to print to a file
///////////////////////////////////////////////////////////////////////
void debug_printf(char *fmt, ...)
{
	int     i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
	edict_t	*cl_ent;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_MEDIUM, bigbuffer);

	for (i = 0; i<maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->is_bot)
			continue;

		gi.cprintf(cl_ent, PRINT_MEDIUM, bigbuffer);
	}

}

///////////////////////////////////////////////////////////////////////
// botsafe cprintf
///////////////////////////////////////////////////////////////////////
void safe_cprintf(edict_t *ent, int printlevel, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (ent && (!ent->inuse || ent->is_bot))
		return;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	gi.cprintf(ent, printlevel, bigbuffer);

}

///////////////////////////////////////////////////////////////////////
// botsafe centerprintf
///////////////////////////////////////////////////////////////////////
void safe_centerprintf(edict_t *ent, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (!ent->inuse || ent->is_bot)
		return;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	gi.centerprintf(ent, bigbuffer);

}

///////////////////////////////////////////////////////////////////////
// botsafe bprintf
///////////////////////////////////////////////////////////////////////
void safe_bprintf(int printlevel, char *fmt, ...)
{
	int i;
	char	bigbuffer[0x10000];
	int		len;
	va_list		argptr;
	edict_t	*cl_ent;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	for (i = 0; i<maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		
		if (!cl_ent->inuse || cl_ent->is_bot)
			continue;

		gi.cprintf(cl_ent, printlevel, bigbuffer);
	}
}

