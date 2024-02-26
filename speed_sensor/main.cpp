#include "../common/includes.hpp"
#include <cstdlib>

int main() {

    json_object *sensor = NULL;
    elosEventSource_t source;
    elosEventVector_t *events;
    json_object *eventsJson;

    json_object_get_object(sensor);
    sensor = json_object_from_file("config/source.json");
    elosEventSourceFromJsonObject(&source, sensor);
    eventsJson = json_object_new_array();
    eventsJson = json_object_from_file("config/events.json");
    elosEventVectorFromJsonArray(eventsJson, &events);
    elosSession_t *session = NULL;
    if (elosConnectTcpip("127.0.0.1", 54321, &session) == SAFU_RESULT_FAILED)
    {
        std::cout << "connection failed" << std::endl;
        return (1);
    }
    //subscribe à {freiner, accélérer} par computer, shutdown par computer
    const char *filterSlow = ".event.source.appName 'computer' STRCMP .event.payload 'speed down' STRCMP AND";
    const char *filterSpeed = ".event.source.appName 'computer' STRCMP .event.payload 'speed up' STRCMP AND";
    const char *filterShutdown = ".event.source.appName 'computer' STRCMP .event.messageCode 7002 EQ AND";
    elosEventQueueId_t queueSlow;
    elosEventQueueId_t queueSpeed;
    elosEventQueueId_t queueShutdown;
    if (elosEventSubscribe(session, &filterSlow, 1, &queueSlow) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSlow << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, (const char**)&filterSpeed, 1, &queueSpeed) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSpeed << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, (const char**)&filterShutdown, 1, &queueShutdown) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterShutdown << " from " << source.appName << std::endl;
        return (1);
    }

    bool power = true;
    int speed = 0;
    while (power)
    {
        if (!elosSessionValid(session))
        {
            power = false;
            std::cout << "invalid session from " << source.appName << std::endl;

        }
        //si freiner et speed > 0, speed --
        elosEventVector_t *slowVec;
        elosEventVectorNew(&slowVec, 1);
        if (elosEventQueueRead(session, queueSlow, &slowVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        if (safuVecElements(slowVec) > 0)
        {
            if (speed > 0)
            {
                speed--;
            }
        }
        //si accélérer, speed++
        elosEventVector_t *speedVec;
        elosEventVectorNew(&speedVec, 1);
        if (elosEventQueueRead(session, queueSpeed, &speedVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        if (safuVecElements(speedVec) > 0)
        {
            speed++;
        }

        //publish speed
        elosEvent_t *speedPublication;
        elosEvent_t *basisEvent;
        elosEventNew(&basisEvent);
        elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 0));
        speedPublication = update_event(*basisEvent, &source);
        std::string str = std::to_string(speed);
        if (str.length() < 4)
            speedPublication->payload = (char *)str.c_str();
        if (elosEventPublish(session, speedPublication) == SAFU_RESULT_FAILED)
        {
            //publish fail
            std::cout << "Failed publishing from " << source.appName << std::endl;
        }
        elosEventDelete(basisEvent);
        speedPublication->payload = NULL;
        elosEventDelete(speedPublication);

        //si shutdown, stop while
        elosEventVector_t *shutdownVec;
        elosEventVectorNew(&shutdownVec, 1);
        if (elosEventQueueRead(session, queueShutdown, &shutdownVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        if (safuVecElements(shutdownVec) > 0)
        {
            power = false;
            //publish event shutdown
        }
        usleep(50000);
    }
    elosDisconnect(session);
    return (0);

}