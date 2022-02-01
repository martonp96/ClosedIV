#include "main.h"

void(*LoadMpDlc)();
void(*EnableMpDlcMaps)(bool);

bool(*GameStateChangeOrig)(int);
bool GameStateChangeHook(int gameState)
{
	if (gameState == 0) //GAME_STATE_PLAYING
	{
		LoadMpDlc();
		EnableMpDlcMaps(true);
	}
	return GameStateChangeOrig(gameState);
}

static memory::InitFuncs EnableMpDlcMapsHooks([] {
	//get game state to enable mp dlc maps
	if (config::get_config<bool>("dlcmaps"))
	{
		auto mem = memory::scan("E8 ? ? ? ? 84 C0 74 ? E8 ? ? ? ? 0F B6 0D");
		GameStateChangeOrig = mem.add(1).rip().as<decltype(GameStateChangeOrig)>();
		mem.set_call(GameStateChangeHook);

		LoadMpDlc = memory::scan("C6 05 ? ? ? ? 00 E8 ? ? ? ? 48 8B 0D ? ? ? ? BA E2 99 8F 57").add(-0xB).as<decltype(LoadMpDlc)>();
		EnableMpDlcMaps = memory::scan("40 53 48 83 EC 20 8B D9 89 0D").as<decltype(EnableMpDlcMaps)>();
	}
});