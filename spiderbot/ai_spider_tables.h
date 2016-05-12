mframe_t	spider_frames_amb[] = 
{
	ai_stand,	  0.000, spider_talk_think,	// frame 0
	ai_stand,	  0.000, spider_talk_think,	// frame 1
	ai_stand,	  0.000, spider_talk_think,	// frame 2
	ai_stand,	  0.000, spider_talk_think,	// frame 3
	ai_stand,	  0.000, spider_talk_think,	// frame 4
	ai_stand,	  0.000, spider_talk_think,	// frame 5
	ai_stand,	  0.000, spider_talk_think,	// frame 6
	ai_stand,	  0.000, spider_talk_think,	// frame 7
	ai_stand,	  0.000, spider_talk_think,	// frame 8
	ai_stand,	  0.000, spider_talk_think,	// frame 9
	ai_stand,	  0.000, spider_talk_think,	// frame 10
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 11

};
mmove_t	spider_move_amb = {FRAME_swait1, FRAME_swait26, spider_frames_amb, spider_end_stand};

mframe_t	spider_frames_pant[] = 
{
	ai_stand,	  0.000, spider_talk_think,	// frame 0
	ai_stand,	  0.000, spider_talk_think,	// frame 1
	ai_stand,	  0.000, spider_talk_think,	// frame 2
	ai_stand,	  0.000, spider_talk_think,	// frame 3
	ai_stand,	  0.000, spider_talk_think,	// frame 4
	ai_stand,	  0.000, spider_talk_think,	// frame 5
	ai_stand,	  0.000, spider_talk_think,	// frame 6
	ai_stand,	  0.000, spider_talk_think,	// frame 7
	ai_stand,	  0.000, spider_talk_think,	// frame 8
	ai_stand,	  0.000, spider_talk_think,	// frame 9
	ai_stand,	  0.000, spider_talk_think,	// frame 10
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 12
	ai_stand,	  0.000, spider_talk_think,	// frame 13
};
mmove_t	spider_move_pant = {FRAME_swait13, FRAME_swait14, spider_frames_pant, spider_end_stand};

mframe_t	spider_frames_sniff[] = 
{
	ai_stand,	  0.000, spider_talk_think,	// frame 0
	ai_stand,	  0.000, spider_talk_think,	// frame 1
	ai_stand,	  0.000, spider_talk_think,	// frame 2
	ai_stand,	  0.000, spider_talk_think,	// frame 3
	ai_stand,	  0.000, spider_talk_think,	// frame 4
	ai_stand,	  0.000, spider_talk_think,	// frame 5
	ai_stand,	  0.000, spider_talk_think,	// frame 6
	ai_stand,	  0.000, spider_talk_think,	// frame 7
	ai_stand,	  0.000, spider_talk_think,	// frame 8
	ai_stand,	  0.000, spider_talk_think,	// frame 9
	ai_stand,	  0.000, spider_talk_think,	// frame 10
	ai_stand,	  0.000, spider_talk_think,	// frame 11
	ai_stand,	  0.000, spider_talk_think,	// frame 12
	ai_stand,	  0.000, spider_talk_think,	// frame 13
	ai_stand,	  0.000, spider_talk_think,	// frame 14
	ai_stand,	  0.000, spider_talk_think,	// frame 15
	ai_stand,	  0.000, spider_talk_think,	// frame 16
	ai_stand,	  0.000, spider_talk_think,	// frame 17
	ai_stand,	  0.000, spider_talk_think,	// frame 18
	ai_stand,	  0.000, spider_talk_think,	// frame 19
};
mmove_t	spider_move_sniff = {FRAME_swait15, FRAME_swait20, spider_frames_sniff, spider_end_stand};

mframe_t	spider_frames_bark[] = 
{
	ai_stand,	  0.000, spider_talk_think,	// frame 0
	ai_stand,	  0.000, spider_bark,	// frame 1
	ai_stand,	  0.000, spider_talk_think,	// frame 2
	ai_stand,	  0.000, spider_talk_think,	// frame 3
	ai_stand,	  0.000, spider_bark,	// frame 4
	ai_stand,	  0.000, spider_talk_think,	// frame 5
	ai_stand,	  0.000, spider_talk_think,	// frame 6
	ai_stand,	  0.000, spider_talk_think,	// frame 7
	ai_stand,	  0.000, spider_talk_think,	// frame 8
};
mmove_t	spider_move_bark = {FRAME_swait21, FRAME_swait26, spider_frames_bark, spider_end_stand};

mframe_t	spider_frames_growl[] = 
{
	ai_stand,	  0.000, spider_growl,	// frame 0
	ai_stand,	  0.000, NULL,	// frame 1
	ai_stand,	  0.000, NULL,	// frame 2
	ai_stand,	  0.000, NULL,	// frame 3
	ai_stand,	  0.000, NULL,	// frame 4
};
mmove_t spider_move_growl = {FRAME_swait1, FRAME_swait5, spider_frames_growl, spider_end_stand};

mframe_t	enemy_spider_frames_pee[] = 
{
	NULL,	  0.000, NULL,	// frame 0
	NULL,	  0.000, NULL,	// frame 1
	NULL,	  0.000, NULL,	// frame 2
	NULL,	  0.000, NULL,	// frame 3
	NULL,	  0.000, NULL,	// frame 4
	NULL,	  0.000, NULL,	// frame 5
	NULL,	  0.000, NULL,	// frame 6
	NULL,	  0.000, NULL,	// frame 7
	NULL,	  0.000, NULL,	// frame 8
	NULL,	  0.000, NULL,	// frame 9
	NULL,	  0.000, NULL,	// frame 10
	NULL,	  0.000, NULL,	// frame 11
};
mmove_t	enemy_spider_move_pee = {FRAME_swait1, FRAME_swait12, enemy_spider_frames_pee, spider_end_stand};

mframe_t	spider_frames_low_atk[] = 
{
	ai_turn2,	  0.000, NULL,	// frame 0
	ai_turn2,	  8.570, spider_bite,	// frame 1
	ai_turn2,	 11.397, spider_bite,	// frame 2
	ai_turn2,	  5.512, NULL,	// frame 3
	ai_turn2,	  1.776, spider_bite,	// frame 4
	ai_turn2,	 -0.788, spider_bite,	// frame 5
	ai_turn2,	  0.808, NULL,	// frame 6
	ai_turn2,	 -1.987, NULL,	// frame 7
	ai_turn2,	 11.397, spider_bite,	// frame 2
	ai_turn2,	  5.512, NULL,	// frame 3
	ai_turn2,	  1.776, spider_bite,	// frame 4
	ai_turn2,	 -0.788, spider_bite,	// frame 5

};
mmove_t	spider_move_low_atk = {FRAME_sattak1, FRAME_sattak12, spider_frames_low_atk, AI_EndAttack};

mframe_t	spider_frames_med_atk[] = 
{
	ai_turn2,	  0.000, NULL,	// frame 0
	ai_turn2,	 16.169, NULL,	// frame 1
	ai_turn2,	  6.894, spider_bite,	// frame 2
	ai_turn2,	  3.803, spider_bite,	// frame 3
	ai_turn2,	 -4.625, spider_bite,	// frame 4
	ai_turn2,	  1.215, spider_bite,	// frame 5
	ai_turn2,	  0.501, NULL,	// frame 6
	ai_turn2,	 -4.625, spider_bite,	// frame 4
	ai_turn2,	  1.215, spider_bite,	// frame 5
	ai_turn2,	  0.501, NULL,	// frame 6

};
mmove_t	spider_move_med_atk = {FRAME_sbite1, FRAME_sbite10, spider_frames_med_atk, AI_EndAttack};

mframe_t	spider_frames_upr_atk[] = 
{
	NULL,	  0.004, NULL,	// frame 0
	NULL,	  5.304, spider_pounce,	// frame 1
	NULL,	  0.102, spider_pounce,	// frame 2
	NULL,	  8.523, spider_pounce,	// frame 3
	NULL,	 19.589, spider_bite,	// frame 4
	NULL,	  3.179, spider_pounce,	// frame 5
	NULL,	 18.590, spider_bite,	// frame 6
	NULL,	  4.629, spider_pounce,	// frame 7
	NULL,	 -4.434, spider_bite,	// frame 8
	NULL,	  3.514, spider_bite,	// frame 9
	NULL,	 19.589, spider_bite,	// frame 4
	NULL,	  3.179, spider_pounce,	// frame 5
	NULL,	 18.590, spider_bite,	// frame 6
	NULL,	  4.629, spider_pounce,	// frame 7
	NULL,	 -4.434, spider_bite,	// frame 8
	NULL,	  3.514, spider_bite,	// frame 9
	NULL,	 19.589, spider_bite,	// frame 4
	NULL,	  3.179, spider_pounce,	// frame 5
	NULL,	 18.590, spider_bite,	// frame 6
	NULL,	  4.629, spider_pounce,	// frame 7
	NULL,	 -4.434, spider_bite,	// frame 8
	NULL,	  3.514, spider_bite,	// frame 9

};
mmove_t	spider_move_upr_atk = {FRAME_jump1, FRAME_jump16, spider_frames_upr_atk, AI_EndAttack};

mframe_t	spider_frames_pain1[] = 
{
	ai_move,	  0.712, NULL,	// frame 0
	ai_move,	 -0.637, NULL,	// frame 1
	ai_move,	-35.731, NULL,	// frame 2
	ai_move,	-17.782, NULL,	// frame 3
	ai_move,	 -1.044, NULL,	// frame 4
};
mmove_t	spider_move_pain1 = {FRAME_sdpain1, FRAME_sdpain5, spider_frames_pain1, AI_EndAttack};

mframe_t	spider_frames_pain2[] = 
{
	ai_move,	 -0.405, NULL,	// frame 0
	ai_move,	 -2.011, NULL,	// frame 1
	ai_move,	-25.452, NULL,	// frame 2
	ai_move,	-24.575, NULL,	// frame 3
	ai_move,	 -3.124, NULL,	// frame 4
	ai_move,	 -2.106, NULL,	// frame 5
	ai_move,	  2.975, NULL,	// frame 6
};
mmove_t	spider_move_pain2 = {FRAME_supain1, FRAME_supain7, spider_frames_pain2, AI_EndAttack};

mframe_t	spider_frames_pain3[] = 
{
	ai_move,	  0.000, NULL,	// frame 0
	ai_move,	  0.000, NULL,	// frame 1
	ai_move,	  0.000, NULL,	// frame 2
	ai_move,	  0.000, NULL,	// frame 3
	ai_move,	  0.000, NULL,	// frame 4
};
mmove_t	spider_move_pain3 = {FRAME_sdpain6, FRAME_sdpain4, spider_frames_pain3, AI_EndAttack};

mframe_t	spider_frames_death1[] = 
{
	ai_move,	  0.000, NULL,	// frame 0
	ai_move,	 -9.109, NULL,	// frame 1
	ai_move,	 -8.657, NULL,	// frame 2
	ai_move,	-10.863, NULL,	// frame 3
	ai_move,	 -2.848, NULL,	// frame 4
	ai_move,	 -0.382, NULL,	// frame 5
	ai_move,	  0.707, NULL,	// frame 6
	ai_move,	 -0.066, NULL,	// frame 7
	ai_move,	  0.067, NULL,	// frame 8
	ai_move,	 -9.109, NULL,	// frame 1
	ai_move,	 -8.657, NULL,	// frame 2
	ai_move,	-10.863, NULL,	// frame 3
	ai_move,	 -2.848, NULL,	// frame 4
	ai_move,	 -0.382, NULL,	// frame 5
	ai_move,	  0.707, NULL,	// frame 6
	ai_move,	 -0.066, NULL,	// frame 7
	ai_move,	  0.067, NULL,	// frame 8
	ai_move,	 -0.382, NULL,	// frame 5
	ai_move,	  0.707, NULL,	// frame 6
	ai_move,	 -0.066, NULL,	// frame 7
	ai_move,	  0.067, NULL,	// frame 8
};
mmove_t	spider_move_death1 = {FRAME_sdeath1, FRAME_sdeath20, spider_frames_death1, AI_EndDeath};

mframe_t	spider_frames_death2[] = 
{
	ai_move,	  0.000, NULL,	// frame 0
	ai_move,	 -9.109, NULL,	// frame 1
	ai_move,	 -8.657, NULL,	// frame 2
	ai_move,	-10.863, NULL,	// frame 3
	ai_move,	 -2.848, NULL,	// frame 4
	ai_move,	 -0.382, NULL,	// frame 5
	ai_move,	  0.707, NULL,	// frame 6
	ai_move,	 -0.066, NULL,	// frame 7
	ai_move,	  0.067, NULL,	// frame 8
	ai_move,	 -9.109, NULL,	// frame 1
	ai_move,	 -8.657, NULL,	// frame 2
	ai_move,	-10.863, NULL,	// frame 3
	ai_move,	 -2.848, NULL,	// frame 4
	ai_move,	 -0.382, NULL,	// frame 5
	ai_move,	  0.707, NULL,	// frame 6
	ai_move,	 -0.066, NULL,	// frame 7
	ai_move,	  0.067, NULL,	// frame 8
	ai_move,	 -0.382, NULL,	// frame 5
	ai_move,	  0.707, NULL,	// frame 6
	ai_move,	 -0.066, NULL,	// frame 7
	ai_move,	  0.067, NULL,	// frame 8


};
mmove_t	spider_move_death2 = {FRAME_sdeath1, FRAME_sdeath20, spider_frames_death2, AI_EndDeath};

mframe_t	spider_frames_walk[] = 
{
	ai_run,	  0.022, NULL,	// frame 0
	ai_run,	  0.120, NULL,	// frame 1
	ai_run,	  0.127, NULL,	// frame 2
	ai_run,	  0.026, NULL,	// frame 3
	ai_run,	 -0.324, NULL,	// frame 4
	ai_run,	 -0.720, NULL,	// frame 5
	ai_run,	 -0.664, NULL,	// frame 6
	ai_run,	 -0.627, NULL,	// frame 7
	ai_run,	 -0.346, NULL,	// frame 8
	ai_run,	  0.031, NULL,	// frame 9
	ai_run,	  0.408, NULL,	// frame 10
	ai_run,	  0.707, NULL,	// frame 11
	ai_run,	  0.761, NULL,	// frame 12
};
mmove_t	spider_move_walk = {FRAME_swalk1, FRAME_swalk13, spider_frames_walk, NULL};

mframe_t	spider_frames_trot[] = 
{
	ai_run,	  3.512, NULL,	// frame 0
	ai_run,	  6.911, NULL,	// frame 1
	ai_run,	  2.456, NULL,	// frame 2
	ai_run,	  4.272, NULL,	// frame 3
	ai_run,	  5.160, NULL,	// frame 4
	ai_run,	  4.942, NULL,	// frame 5
	ai_run,	  4.272, NULL,	// frame 6
	ai_run,	  6.615, NULL,	// frame 7
};
mmove_t	spider_move_trot = {FRAME_swalk1, FRAME_swalk8, spider_frames_trot, AI_EndRun};

mframe_t	spider_frames_run[] = 
{
	ai_run,	 11.155, NULL,	// frame 0
	ai_run,	 25.162, NULL,	// frame 1
	ai_run,	 27.589, NULL,	// frame 2
	ai_run,	 21.692, NULL,	// frame 3
};
mmove_t	spider_move_run = {FRAME_swalk1, FRAME_swalk4, spider_frames_run, AI_EndRun};

mframe_t	spider_frames_run_on_fire[] = 
{
	ai_onfire_run,	 11.155, NULL,	// frame 0
	ai_onfire_run,	 25.162, NULL,	// frame 1
	ai_onfire_run,	 27.589, NULL,	// frame 2
	ai_onfire_run,	 21.692, NULL,	// frame 3
};
mmove_t	spider_move_run_on_fire = {FRAME_swalk1, FRAME_swalk4, spider_frames_run_on_fire, NULL};


mframe_t	spider_frames_avoid_walk[] = 
{
	ai_turn,	  3.512, NULL,	// frame 0
	ai_turn,	  6.911, NULL,	// frame 1
	ai_turn,	  2.456, NULL,	// frame 2
	ai_turn,	  4.272, NULL,	// frame 3
	ai_turn,	  5.160, NULL,	// frame 4
	ai_turn,	  4.942, NULL,	// frame 5
	ai_turn,	  4.272, NULL,	// frame 6
	ai_turn,	  6.615, NULL,	// frame 7
};
mmove_t	spider_move_avoid_walk = {FRAME_swalk1, FRAME_swalk8, spider_frames_avoid_walk, AI_EndAttack};

mframe_t	spider_frames_avoid_reverse_walk[] = 
{
	ai_turn,	  6.615, NULL,	// frame 0
	ai_turn,	  4.272, NULL,	// frame 1
	ai_turn,	  4.942, NULL,	// frame 2
	ai_turn,	  5.160, NULL,	// frame 3
	ai_turn,	  4.272, NULL,	// frame 4
	ai_turn,	  2.456, NULL,	// frame 5
	ai_turn,	  6.911, NULL,	// frame 6
	ai_turn,	  3.512, NULL,	// frame 7
};
mmove_t	spider_move_avoid_reverse_walk = {FRAME_swalk8, FRAME_swalk1, spider_frames_avoid_reverse_walk, AI_EndAttack};

mframe_t	spider_frames_avoid_run[] = 
{
	ai_turn,	 11.155, NULL,	// frame 0
	ai_turn,	 25.162, NULL,	// frame 1
	ai_turn,	 27.589, NULL,	// frame 2
	ai_turn,	 21.692, NULL,	// frame 3
};
mmove_t	spider_move_avoid_run = {FRAME_swalk1, FRAME_swalk4, spider_frames_avoid_run, AI_EndAttack};

mframe_t	spider_frames_avoid_reverse_run[] = 
{
	ai_turn,	 21.692, NULL,	// frame 0
	ai_turn,	 27.589, NULL,	// frame 1
	ai_turn,	 25.162, NULL,	// frame 2
	ai_turn,	 11.155, NULL,	// frame 3
};
mmove_t	spider_move_avoid_reverse_run = {FRAME_swalk4, FRAME_swalk1, spider_frames_avoid_reverse_run, AI_EndAttack};

