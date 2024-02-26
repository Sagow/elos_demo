#include "../common/includes.hpp"

int main() {

    json_object *thermometer = NULL;
    elosEventSource_t source;
    elosEventVector_t *events;
    json_object *eventsJson;

    json_object_get_object(thermometer);
    thermometer = json_object_from_file("config/source.json");
    elosEventSourceFromJsonObject(&source, thermometer);
    eventsJson = json_object_new_array();
    eventsJson = json_object_from_file("config/events.json");
    elosEventVectorFromJsonArray(eventsJson, &events);

    elosSession_t *session = NULL;
    if (elosConnectTcpip("127.0.0.1", 54321, &session) == SAFU_RESULT_FAILED)
    {
        std::cout << "connection failed" << std::endl;
        return (1);
    }
    //subscribe à {speed} par speed_sensor, shutdown par computer
    const char *filterSpeed = ".event.source.appName 'speed sensor' STRCMP";
    const char *filterShutdown = ".event.source.appName 'computer' STRCMP .event.messageCode 7002 EQ AND";
    elosEventQueueId_t queueSpeed;
    elosEventQueueId_t queueShutdown;
    if (elosEventSubscribe(session, &filterSpeed, 1, &queueSpeed) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSpeed << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterShutdown, 1, &queueShutdown) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterShutdown << " from " << source.appName << std::endl;
        return (1);
    }
    bool power = true;
    int speed = 0;
    elosEventVector_t *speedVec;
    while (power)
    {
        if (!elosSessionValid(session))
        {
            power = false;
            std::cout << "invalid session from " << source.appName << std::endl;

        }
        //si speed > seuil, temp trop chaude, par paliers
        if (elosEventQueueRead(session, queueSpeed, &speedVec) == SAFU_RESULT_FAILED)
        {
            std::cout << "Failed read of " << filterSpeed << " from " << source.appName << std::endl;
            //creer un event de fail de lecture
        }
        else if (safuVecElements(speedVec) > 0)
        {
            elosEvent_t *currentEvent = (elosEvent_t *)safuVecGetLast(speedVec);
            speed = atoi(currentEvent->payload);
            elosEventVectorDeleteMembers(speedVec);
        }
        //si temp trop chaude, publish event
        //si shutdown, stop while
        if (speed > 16) //critical
        {
            //publish  "oil critically too hot"
            elosEvent_t *criticalPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 2));
            criticalPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, criticalPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            elosEventDelete(criticalPublication);
        }
        else if (speed > 13) //hot
        {
            //publish "oil too hot"
            elosEvent_t *hotPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 1));
            hotPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, hotPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            elosEventDelete(hotPublication);
        }
        else //normal
        {
            //publish "oil normal temp"
            elosEvent_t *normalPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 0));
            normalPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, normalPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            elosEventDelete(normalPublication);
            elosEventVector_t *shutdownVec;
            elosEventVectorNew(&shutdownVec, 1);
            if (elosEventQueueRead(session, queueShutdown, &shutdownVec) == SAFU_RESULT_FAILED)
            {
                //creer un event de fail de lecture
            }
            else if (safuVecElements(shutdownVec) > 0)
            {
                power = false;
                //publish event shutdown
            }
            elosEventVectorDelete(shutdownVec);
        }
        usleep(50000);
    }
    elosDisconnect(session);
    return (0);
}