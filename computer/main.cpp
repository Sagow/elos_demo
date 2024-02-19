#include "../common/includes.hpp"
#include <cstdlib>

int main() {

    json_object *sensor = NULL;
    elosEventSource_t source;
    elosEventVector_t *events;
    json_object *eventsJson;

    json_object_get_object(sensor);
    sensor = json_object_from_file("source.json");
    elosEventSourceFromJsonObject(&source, sensor);
    eventsJson = json_object_new_array();
    eventsJson = json_object_from_file("events.json");
    elosEventVectorFromJsonArray(eventsJson, &events);
    elosSession_t *session = NULL;
    if (elosConnectTcpip("127.0.0.1", 54321, &session) == SAFU_RESULT_FAILED)
    {
        std::cout << "connection failed" << std::endl;
        return (1);
    }
    //subscribe à {freiner, accélérer} par interface, shutdown par interface, speed par speed_sensor, oil_temp par oil_thermometer
    const char *filterSlowDown = ".event.source.appName 'interface' STRCMP .event.payload 'speed down' STRCMP AND";
    const char *filterSpeedUp = ".event.source.appName 'interface' STRCMP .event.payload 'speed up' STRCMP AND";
    const char *filterShutdown = ".event.source.appName 'interface' STRCMP .event.messageCode 7002 EQ AND";
    const char *filterOil = ".event.source.appName 'oil thermometer' STRCMP";
    const char *filterSpeed = ".event.source.appName 'speed sensor' STRCMP";

    elosEventQueueId_t queueSlowDown;
    elosEventQueueId_t queueSpeedUp;
    elosEventQueueId_t queueShutdown;
    elosEventQueueId_t queueOil;
    elosEventQueueId_t queueSpeed;

    if (elosEventSubscribe(session, &filterSlowDown, 1, &queueSlowDown) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSlowDown << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterSpeedUp, 1, &queueSpeedUp) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSpeedUp << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterShutdown, 1, &queueShutdown) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterShutdown << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterOil, 1, &queueOil) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterOil << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterSpeed, 1, &queueSpeed) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterSpeed << " from " << source.appName << std::endl;
        return (1);
    }

    bool power = true;
    int temp_oil = 0;
    int speed = 0;
    elosEventVector_t *oilVec;
    elosEventVector_t *speedVec;

    while (power)
    {
        if (!elosSessionValid(session))
        {
            power = false;
            std::cout << "invalid session from " << source.appName << std::endl;

        }
        //si freiner et speed > 0, speed --
        elosEventVector_t *slowDownVec;
        elosEventVectorNew(&slowDownVec, 1);
        if (elosEventQueueRead(session, queueSlowDown, &slowDownVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        else if (safuVecElements(slowDownVec) > 0)
        {
            elosEvent_t *slowDownPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 2));
            slowDownPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, slowDownPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            slowDownPublication->payload = NULL;
            elosEventDelete(slowDownPublication);
        }
        if (temp_oil > 1)
        {
            elosEvent_t *slowDownPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 2));
            slowDownPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, slowDownPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            slowDownPublication->payload = NULL;
            elosEventDelete(slowDownPublication);
        }
        //si accélérer, speed++
        elosEventVector_t *speedUpVec;
        elosEventVectorNew(&speedUpVec, 1);
        if (elosEventQueueRead(session, queueSpeedUp, &speedUpVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        else if (safuVecElements(speedUpVec) > 0)
        {
            elosEvent_t *speedUpPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 1));
            speedUpPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, speedUpPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            speedUpPublication->payload = NULL;
            elosEventDelete(speedUpPublication);
        }
        if (elosEventQueueRead(session, queueOil, &oilVec) == SAFU_RESULT_FAILED)
        {
            std::cout << "Failed read of " << filterOil << " from " << source.appName << std::endl;
            
            //creer un event de fail de lecture
        }
        else if (safuVecElements(oilVec) > 0)
        {
            elosEvent_t *currentEvent = (elosEvent_t *)safuVecGetLast(oilVec);
            
            switch (currentEvent->messageCode)
            {
            case 7126: //oil too hot
                temp_oil = 1;
                break;
            case 6003: //oil critically too hot
                temp_oil = 2;
                break;
            default: //oil normal temp
                temp_oil = 0;
                break;
            }
            elosEventVectorDeleteMembers(oilVec);
        }
        //speed
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
        //si shutdown, stop while
        elosEventVector_t *shutdownVec;
        elosEventVectorNew(&shutdownVec, 1);
        if (elosEventQueueRead(session, queueShutdown, &shutdownVec) == SAFU_RESULT_FAILED)
        {
            //creer un event de fail de lecture
        }
        else if (safuVecElements(shutdownVec) > 0 && speed == 0)
        {
            power = false;
            //publish event shutdown
            elosEvent_t *powerOffPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 0));
            powerOffPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, powerOffPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            powerOffPublication->payload = NULL;
            elosEventDelete(powerOffPublication);
        }
        usleep(50000);
    }
    elosDisconnect(session);
    return (0);

}