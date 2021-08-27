/*

	About: Vehicle health changed callback v2.1
	Author: Noname

*/

#if !defined _samp_included
	#error "Please include a_samp or a_npc before m_vehhealth"
#endif

#if !defined _FOREACH_INC_TEST
	#error "You must have foreach include in order to use this one."
#endif

#if defined _m_vehhealth_included_
	#endinput
#endif
#define _m_vehhealth_included_

/*
	Define const
*/

#if !defined VHEALTH_TICKRATE
	#define VHEALTH_TICKRATE 300
#endif

#if !defined FIX_const
	#define FIX_const 0
	#define M_VEHHEALTH_FIX_CLEAR
#endif

#if !FIX_const
	native MVH_SetTimer(const funcname[], interval, repeating) = SetTimer;

	#if defined _ALS_SetTimer
		#undef SetTimer
	#else
		#define _ALS_SetTimer
	#endif

	#define SetTimer( MVH_SetTimer(

	#if defined M_VEHHEALTH_FIX_CLEAR
		#undef M_VEHHEALTH_FIX_CLEAR
		#undef FIX_const
	#endif
#endif

/*
	Forwards
*/

forward OnVehicleHealthChanged(vehicleid, playerid, Float:newhealth, Float:oldhealth, bool:scriptcall);

/*
	Vars
*/

static
	vh_server_timer_id = -1,
	vh_player_vehicle_id[MAX_PLAYERS] = {INVALID_VEHICLE_ID, ...},
	vh_vehicle_player_id[MAX_VEHICLES] = {INVALID_PLAYER_ID, ...},
	Float:vh_vehicle_old_health[MAX_VEHICLES] = {1000.0, ...},
	Iterator:vh_OccupiedVehicle<MAX_VEHICLES>;

/*
	Private functions
*/

static stock CLF_Callback(vehicleid, playerid, &Float:oldhealth, Float:newhealth, bool:scriptcall)
{
	if (newhealth != oldhealth) {
		CallLocalFunction(!"OnVehicleHealthChanged", !"iiffi", vehicleid, playerid, newhealth, oldhealth, _:scriptcall);
		oldhealth = newhealth;
	}
}

/*
	SetVehicleHealth
*/

stock VHealth_SetVehicleHealth(vehicleid, Float:health)
{
	new
		ret = SetVehicleHealth(vehicleid, health);

	if (ret == 1 && vh_vehicle_player_id[vehicleid] != INVALID_PLAYER_ID)
		CLF_Callback(vehicleid, vh_vehicle_player_id[vehicleid], vh_vehicle_old_health[vehicleid], health, true);
	return ret;
}
#if defined _ALS_SetVehicleHealth
	#undef SetVehicleHealth
#else
	#define _ALS_SetVehicleHealth
#endif

#define SetVehicleHealth( VHealth_SetVehicleHealth(

/*
	RepairVehicle
*/

stock VHealth_RepairVehicle(vehicleid)
{
	new
		ret = RepairVehicle(vehicleid);

	if (ret == 1 && vh_vehicle_player_id[vehicleid] != INVALID_PLAYER_ID)
		CLF_Callback(vehicleid, vh_vehicle_player_id[vehicleid], vh_vehicle_old_health[vehicleid], 1000.0, true);
	return ret;
}
#if defined _ALS_RepairVehicle
	#undef RepairVehicle
#else
	#define _ALS_RepairVehicle
#endif

#define RepairVehicle( VHealth_RepairVehicle(

/*
	VHealth_OccupiedVehicleTimer
*/

forward VHealth_OccupiedVehicleTimer();
public VHealth_OccupiedVehicleTimer()
{
	new
		Float:new_health;

	foreach (new vehicleid : vh_OccupiedVehicle) {
		GetVehicleHealth(vehicleid, new_health);
		CLF_Callback(vehicleid, vh_vehicle_player_id[vehicleid], vh_vehicle_old_health[vehicleid], new_health, false);
	}
	vh_server_timer_id = SetTimer(!"VHealth_OccupiedVehicleTimer", VHEALTH_TICKRATE, false);
	return 1;
}

/*
	OnGameModeInit
*/

public OnGameModeInit()
{
	vh_server_timer_id = SetTimer(!"VHealth_OccupiedVehicleTimer", VHEALTH_TICKRATE, false);

	#if defined m_vhealth_OnGameModeInit
		return m_vhealth_OnGameModeInit();
	#else
		return 1;
	#endif
}

#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif
#define OnGameModeInit m_vhealth_OnGameModeInit
#if defined m_vhealth_OnGameModeInit
	forward m_vhealth_OnGameModeInit();
#endif

/*
	OnGameModeExit
*/

public OnGameModeExit()
{
	if (vh_server_timer_id != -1) {
		KillTimer(vh_server_timer_id);
		vh_server_timer_id = -1;
	}
	Iter_Clear(vh_OccupiedVehicle);

	#if defined m_vhealth_OnGameModeExit
		return m_vhealth_OnGameModeExit();
	#else
		return 1;
	#endif
}

#if defined _ALS_OnGameModeExit
	#undef OnGameModeExit
#else
	#define _ALS_OnGameModeExit
#endif
#define OnGameModeExit m_vhealth_OnGameModeExit
#if defined m_vhealth_OnGameModeExit
	forward m_vhealth_OnGameModeExit();
#endif

/*
	OnPlayerDisconnect
*/

public OnPlayerDisconnect(playerid, reason)
{
	if (vh_player_vehicle_id[playerid] != INVALID_VEHICLE_ID) {
		Iter_Remove(vh_OccupiedVehicle, vh_player_vehicle_id[playerid]);
		vh_vehicle_player_id[vh_player_vehicle_id[playerid]] = INVALID_PLAYER_ID;
		vh_player_vehicle_id[playerid] = INVALID_VEHICLE_ID;
	}

	#if defined m_vhealth_OnPlayerDisconnect
		return m_vhealth_OnPlayerDisconnect(playerid, reason);
	#else
		return 1;
	#endif
}

#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect m_vhealth_OnPlayerDisconnect
#if defined m_vhealth_OnPlayerDisconnect
	forward m_vhealth_OnPlayerDisconnect(playerid, reason);
#endif

/*
	OnPlayerStateChange
*/

public OnPlayerStateChange(playerid, newstate, oldstate)
{
	if (newstate == PLAYER_STATE_DRIVER) {
		vh_player_vehicle_id[playerid] = GetPlayerVehicleID(playerid);
		vh_vehicle_player_id[vh_player_vehicle_id[playerid]] = playerid;
		Iter_Add(vh_OccupiedVehicle, vh_player_vehicle_id[playerid]);
	}
	else if (oldstate == PLAYER_STATE_DRIVER) {
		if (vh_player_vehicle_id[playerid] != INVALID_VEHICLE_ID) {
			Iter_Remove(vh_OccupiedVehicle, vh_player_vehicle_id[playerid]);
			vh_vehicle_player_id[vh_player_vehicle_id[playerid]] = INVALID_PLAYER_ID;
			vh_player_vehicle_id[playerid] = INVALID_VEHICLE_ID;
		}
	}

	#if defined m_vhealth_OnPlayerStateChange
		return m_vhealth_OnPlayerStateChange(playerid, newstate, oldstate);
	#else
		return 1;
	#endif
}

#if defined _ALS_OnPlayerStateChange
	#undef OnPlayerStateChange
#else
	#define _ALS_OnPlayerStateChange
#endif
#define OnPlayerStateChange m_vhealth_OnPlayerStateChange
#if defined m_vhealth_OnPlayerStateChange
	forward m_vhealth_OnPlayerStateChange(playerid, newstate, oldstate);
#endif