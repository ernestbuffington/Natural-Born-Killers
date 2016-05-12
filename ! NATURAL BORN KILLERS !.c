#include "g_local.h"

//MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
//MSG_WriteString(&cls.netchan.message, va("download5 \"%s\" %i", cls.Download.Name_Original, Core.Console->Variable.Get_String("rate")));
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//  precache
//  Type : Operation
//	Description : Load up all the necessary information before entering a map.
//
//	Note : This is an internal command used to load up all the necessary 
//  models, 
//  sounds, 
//  sprites, 
//  images before starting a level.
//
// This command is send by the server to the client right before the server is ready to accept the client 
// into the game.This command checks what level the server is running, and using that information compiles 
// a list of all necessary games files that should be loaded up for that specific map, and loads them.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// protocol for q2 is 31
// protocol for kp is 32
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// s_show
// Type : Toggle
//	Default : 0
//
//	Description : Toggle the display of all sounds which are currently playing.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// s_testsound
// Type : Toggle
//	Default : 0
//
//	Description : Toggle the generation of a test tone to test the sound hardware.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// scr_conspeed
// Type : Register
//	Default : 3
//
//	Description : The speed at which the console is raised and lowered.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// showpackets
// Type : Toggle
//	Default : 0
//
//	Description : Toggle the display of information about all network packets.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// skin - show your own skins
// skins - show all user skins
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// soundinfo
// Type : Operation
// Description : Display information about the sound system.
// Note : This command will display information such as the directory where sound files are stored, 
// if the sound system is in stereo mode, the sampling rate for the sound system, the speed in hertz 
// that the sound system is initialized in, and the memory location of the DMA buffer.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// stopsound
// Type : Operation
//	Description : Stop any sounds which are currently playing.
//
//	Note : This command is useful if any sounds do not correctly finish up and end up looping themselves for no reason.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// gl_ztrick
// Type : Toggle
//	Default : 0
//
//	Description : Toggle the clearing of the z - buffer between frames.
//
//	Note : If this toggle is enabled the game will not clear the z - buffer between frames.This will result in increased 
// performance but might cause problems for some display hardware.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// imagelist
// Type : Operation
//	Description : Display a list of all loaded images, their types, dimensions, and color pallets.Also display the total texel count.
//
//	Note : There are five columns of information.The first column identifies the image type, consult the list below for all possible 
//  types.The second column shows the x - axis size of the image.The third column shows the y - axis size of the image.The fourth column 
//  shows the image's dependency on the pallet, either the red-green-blue pallet, or the internal game pallet. The fifth column shows 
//  the image's path and filename.
//
//	List :
//	M - Model texture used for models shown in the map.
//	P - Player images, shown on the player's view screen.
//	S - Sprite picture.
//	W - Wall texture used for the map.
//	RGB - The image uses the standard RGB pallet.
//	PAL - The images uses the internal game pallet.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//  link
//  Type : Function
//	Syntax : link(from path) (to path)
//
//	Description : Alter the actual location of a directory inside the game.
//
//	Note : This command will allow you select where the game will look for its files.
//  A set of empty quotes will clear any modifications that you make to a directory location.
//
//	Example :
//	link maps ""
//	link maps "dmmaps"
//	link maps "mission/maps"
//	link sound / player / male "c:/quake2/newsounds/"
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//  log_stats
//  Type : Toggle
//	Default : 0
//
//	Description : Toggle the logging of map statistics.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//	logfile
//	Type : Toggle
//	Default : 0
//
//	Description : Toggle the logging of console messages.
//
//	Values :
//	0 - Disable logging.
//	1 - Enable buffered logging where the log file is only updated when the text buffer fills up. The log file will be deleted and overridden when a new server starts up.
//	2 - Enable continuous logging where the log file is updated with every new line of text. The log file will be deleted and overridden when a new server starts up.
//	3 - Enable continuous logging where the log file is updated with ever new line of text. The log file will be appended to and logging will continue to the same log file 
//      when a new server starts up.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//  netgraph
//  Type : Toggle
//	Default : 0
//
//	Description : Toggle the display of the network performance graph.
//
//	Note : The size of each bar is the latency time.A green bar indicates a successful packet transfer.A red bar indicates a lost packet.
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
