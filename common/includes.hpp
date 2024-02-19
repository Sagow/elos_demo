#pragma once
extern "C" {
#include "elos/libelos/libelos.h"
}
#include <chrono>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <json-c/json.h>
#include <stdlib.h>
#include <sstream>

std::string slurp(std::ifstream &file);
elosEvent_t *update_event(elosEvent_t &basis, elosEventSource_t *source);

