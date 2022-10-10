#include "log.h"

#include <iostream>

void Log::log(std::string_view msg)
{
    std::clog << "=> " << msg << std::endl;
}
