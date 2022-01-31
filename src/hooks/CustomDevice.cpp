#include "main.h"

/*
	List of stuff you can replace with this: https://gist.github.com/martonp96/59f731446c7f17db3f400c2be458c4a4

	How to do it:
	Find something from the list, eg. common:/data/dlclist.xml

	Copy the original dlclist.xml from the files and place it in [your GTA5 folder]/newmods/common/data/dlclist.xml
	Edit the file, add a new entry like <Item>dlcpacks:/mrpd/</Item>
	Place the single player dlc.rpf like [your GTA5 folder]/newmods/dlcpacks/mrpd/dlc.rpf

	Start the game and the modded stuff should be loaded automatically.

	I used this for the example: https://hu.gta5-mods.com/maps/community-mission-row-pd
*/

void(*InitialMountOrig)();
void InitialMountHook()
{
	InitialMountOrig();

	std::filesystem::path cwd = std::filesystem::current_path() / "newmods/";
	cwd.make_preferred();

	//logger::info("path: %s", cwd.string().c_str());

	rage::fiDeviceRelative* rootDevice = new rage::fiDeviceRelative();
	rootDevice->SetPath(cwd.string().c_str(), true, nullptr);

	if (rootDevice->Mount("mods:/"))
		logger::info("Root device mounted!");

	rage::fiDeviceRelative* platformDevice = new rage::fiDeviceRelative();
	rage::fiDeviceRelative* platformDeviceCRC = new rage::fiDeviceRelative();
	rage::fiDeviceRelative* commonDevice = new rage::fiDeviceRelative();
	rage::fiDeviceRelative* commonDeviceCRC = new rage::fiDeviceRelative();
	rage::fiDeviceRelative* dlcDevice = new rage::fiDeviceRelative();

	platformDevice->SetPath("mods:/platform/", true, nullptr);
	platformDevice->Mount("platform:/");

	platformDeviceCRC->SetPath("mods:/platform/", true, nullptr);
	platformDeviceCRC->Mount("platformcrc:/");

	commonDevice->SetPath("mods:/common/", true, nullptr);
	commonDevice->Mount("common:/");

	commonDeviceCRC->SetPath("mods:/common/", true, nullptr);
	commonDeviceCRC->Mount("commoncrc:/");

	dlcDevice->SetPath("mods:/dlcpacks/", true, nullptr);
	dlcDevice->Mount("dlcpacks:/");
}

static memory::InitFuncs CustomDevice([] {
	auto mem = memory::scan("E8 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 48 8B D8 48 85 C0 74 14 48 8B C8 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 03 EB 02 33 DB 48 8D 15");
	InitialMountOrig = mem.add(1).rip().as<decltype(InitialMountOrig)>();
	mem.set_call(InitialMountHook);
});