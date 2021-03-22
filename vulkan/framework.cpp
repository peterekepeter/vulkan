#include "stdafx.h"
#include "framework.h"

void __throw_formatted_message(const char* message)
{
	throw new std::runtime_error(message);
}
