#pragma once

namespace logger
{
	void init();
	void write(const char* type, const char* msg, ...);
}