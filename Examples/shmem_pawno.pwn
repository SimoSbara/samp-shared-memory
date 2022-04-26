//----------------------------------------------------------
//
//  GRAND LARCENY  1.0
//  A freeroam gamemode for SA-MP 0.3
//
//----------------------------------------------------------

#include <a_samp>
#include <core>
#include <float>
#include <shmem>

#pragma tabsize 0

new var[5] = {70000, 60000, 30000, 30000, 50000};

//----------------------------------------------------------

main()
{
	new res = OpenShMemory("memory_name", sizeof var);

	printf("result of the creation = %d", res);
	
	res = SetShMemoryData("memory_name", var, sizeof var);
	
	printf("set in memory values of the array \"var\", result = %d", res);
	
	new nbyte = GetShMemorySize("memory_name");
	printf("byte allocated from shared memory = %d", nbyte);
	
	printf("how many bytes from pawno? %d bytes", sizeof var * 4); //sizeof returns numbers of elements, multiplication by 4 bytes for each element (32 bit) works also for other tags (ex: Float:)
	
	while(true)
	{
		new res = GetShMemoryData("memory_name", var);
		//var is overwritten in sharedmemory.dll to get data from the shared memory created
		
		if(res == 2)
		    printf("collision with another process in read/write, very rare");
		    
		printf("value in index 0 of array var = %d", var[0]);
		
		sleep(1000);
	}
}

//----------------------------------------------------------

public OnPlayerConnect(playerid)
{
 	return 1;
}

//----------------------------------------------------------

public OnPlayerSpawn(playerid)
{
	return 1;
}

//----------------------------------------------------------

public OnPlayerDeath(playerid, killerid, reason)
{
   	return 1;
}

//----------------------------------------------------------

ClassSel_SetupCharSelection(playerid)
{
	
}

//----------------------------------------------------------
// Used to init textdraws of city names

ClassSel_InitCityNameText(Text:txtInit)
{

}

//----------------------------------------------------------

ClassSel_InitTextDraws()
{

}

//----------------------------------------------------------

ClassSel_SetupSelectedCity(playerid)
{

}

//----------------------------------------------------------

ClassSel_SwitchToNextCity(playerid)
{

}

//----------------------------------------------------------

ClassSel_SwitchToPreviousCity(playerid)
{

}

//----------------------------------------------------------

ClassSel_HandleCitySelection(playerid)
{

}

//----------------------------------------------------------

public OnPlayerRequestClass(playerid, classid)
{
	return 0;
}

//----------------------------------------------------------

public OnGameModeInit()
{
	return 1;
}

//----------------------------------------------------------

public OnPlayerUpdate(playerid)
{
	return 1;
}

//----------------------------------------------------------
