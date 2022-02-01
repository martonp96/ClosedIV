#pragma once
#include "inicpp.h"

class config
{
public:
	static ini::IniFile& get_ini()
	{
		static ini::IniFile iniFile;
		return iniFile;
	}

	static void load()
	{
		if (std::filesystem::exists("ClosedIV.cfg"))
		{
			get_ini().load("ClosedIV.cfg");
		}
		else
		{	//defaults
			get_ini()["config"]["console"] = false;
			get_ini()["config"]["dlcmaps"] = false;
			get_ini()["log"]["info"] = true;
			get_ini()["log"]["mods"] = false;
			get_ini()["log"]["rpf"] = false;
			save();
		}
	}

	static void save()
	{
		get_ini().save("ClosedIV.cfg");
	}

	template<typename T>
	static T get_config(const char* key)
	{
		return get_ini()["config"][key].as<T>();
	}

	static bool get_log(const char* key)
	{
		return get_ini()["log"][key].as<bool>();
	}
};