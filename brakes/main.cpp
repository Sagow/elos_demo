#include "../common/includes.hpp"

int main() {

    json_object *brakes = NULL;
    elosEventSource_t source;
    elosEventVector_t *events;
    json_object *eventsJson;

    json_object_get_object(brakes);
    brakes = json_object_from_file("config/source.json");
    elosEventSourceFromJsonObject(&source, brakes);
    eventsJson = json_object_new_array();
    eventsJson = json_object_from_file("config/events.json");
    elosEventVectorFromJsonArray(eventsJson, &events);

    elosSession_t *session = NULL;
    if (elosConnectTcpip("127.0.0.1", 54321, &session) == SAFU_RESULT_FAILED)
    {
        std::cout << "connection failed" << std::endl;
        return (1);
    }
    //subscribe à freiner par computer, shutdown par computer
    const char *filterSlow = ".event.source.appName 'computer' STRCMP .event.payload 'speed down' STRCMP AND";
    const char *filterShutdown = ".event.source.appName 'computer' STRCMP .event.messageCode 7002 EQ AND";
    elosEventQueueId_t queueSlow;
    elosEventQueueId_t queueShutdown;
    if (elosEventSubscribe(session, &filterSlow, 1, &queueSlow) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSlow << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterShutdown, 1, &queueShutdown) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterShutdown << " from " << source.appName << std::endl;
        return (1);
    }

    bool power = true;
    int wear = 0;
    while (power)
    {
        if (!elosSessionValid(session))
        {
            power = false;
            std::cout << "invalid session from " << source.appName << std::endl;
        }
        elosEventVector_t *slowVec;
        elosEventVectorNew(&slowVec, 1);
        if (elosEventQueueRead(session, queueSlow, &slowVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        //read, si freiner, monter la température, sinon l'approcher de 0
        if (safuVecElements(slowVec) > 0)
        {
            wear++;
            elosEventVectorDeleteMembers(slowVec);
        }
        //si wear > seuils, event
        if (wear > 15) //critical
        {
            //publish  "brakes critically too hot"
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
        else if (wear > 10) //hot
        {
            //publish "brakes too hot"
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
            //publish "brakes normal wear"
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
        }
        //si shutdown, stop while
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
        elosEventVectorDelete(slowVec);
        elosEventVectorDelete(shutdownVec);
        usleep(50000);
    }
    elosDisconnect(session);
    return (0);
}