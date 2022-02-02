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

rage::fiDeviceLocal* platformDevice;
rage::fiDeviceLocal* platformDeviceCRC;
rage::fiDeviceLocal* commonDevice;
rage::fiDeviceLocal* commonDeviceCRC;
rage::fiDeviceLocal* dlcDevice;

bool IsCustomDevice(rage::fiDeviceLocal* dev)
{
	if (dev == platformDevice || dev == platformDeviceCRC || dev == commonDevice || dev == commonDeviceCRC || dev == dlcDevice)
		return true;
	return false;
}

void(*InitialMountOrig)();
void InitialMountHook()
{
	InitialMountOrig();

	std::filesystem::path cwd = std::filesystem::current_path() / "newmods/";
	cwd.make_preferred();

	logger::write("device", "Mods path: %s", cwd.string().c_str());

	rage::fiDeviceLocal* rootDevice = new rage::fiDeviceLocal();
	rootDevice->SetPath(cwd.string().c_str(), true, nullptr);

	if (rootDevice->Mount("mods:/"))
		logger::write("device", "Root device mounted!");

	platformDevice = new rage::fiDeviceLocal();
	platformDeviceCRC = new rage::fiDeviceLocal();
	commonDevice = new rage::fiDeviceLocal();
	commonDeviceCRC = new rage::fiDeviceLocal();
	dlcDevice = new rage::fiDeviceLocal();

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

static memory::func<rage::fiDevice*, const char*, bool> GetDeviceHook("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 20 48 8D 15 ? ? ? ? 41 B8");

char (*OpenArchiveOrig)(rage::fiPackfile* a1, const char* path, char smth, int32_t type);
char OpenArchiveHook(rage::fiPackfile* a1, const char* path, char smth, int32_t type)
{
	auto device = GetDeviceHook(path, true);
	if (device && IsCustomDevice((rage::fiDeviceLocal*)device))
	{
		if (device->GetAttributes(path) & FILE_ATTRIBUTE_DIRECTORY)
		{
			logger::write("device", "Archive %s is a directory!", path);
			type = 2;
		}
		logger::write("device", "Opening archive %s %d %d", path, smth, type);
	}
	return OpenArchiveOrig(a1, path, smth, type);
}

static memory::InitFuncs CustomDevice([] {
	auto mem = memory::scan("0F B7 05 ? ? ? ? 48 03 C3 44 88 34 38").add(21);
	InitialMountOrig = mem.add(1).rip().as<decltype(InitialMountOrig)>();
	mem.set_call(InitialMountHook);

	memory::scan("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D 68 98 48 81 EC ? ? ? ? 41 8B F9 4C 8B E2 48 8B D9 4C 8B CA 48 8D 05")
		.hook(OpenArchiveHook, &OpenArchiveOrig);
});