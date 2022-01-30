#include "main.h"

void logger::info(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf("\n");
	va_end(args);
}