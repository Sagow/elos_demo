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
#include <sstream>

std::string slurp(std::ifstream &file)
{
    std::ostringstream string;
    string << file.rdbuf();
    return (string.str());
}

elosEvent_t *update_event(elosEvent_t &basis, elosEventSource_t *source)
{
    elosEvent_t *updated;

    elosEventNew(&updated);

    elosEventDeepCopy(updated, (const elosEvent_t *)&basis);
    elosEventSourceDeepCopy(&(updated->source), source);
    clock_gettime(CLOCK_REALTIME, &(updated->date));
    updated->source.pid = getpid();
    return (updated);
}