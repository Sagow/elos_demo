#include "../common/includes.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

void print_oil(Display *display, Window win, GC gc, XColor color, XColor fontColor)
{
    XSetForeground(display, gc, color.pixel);
    XFillArc(display,win, gc, 25, 1, 50,50,0,23040);
    XSetForeground(display, gc, fontColor.pixel);
    XDrawString (display, win, gc, 1, 70, "Oil temperature", 15);
}

void print_speed(Display *display, Window win, GC gc, XColor fontColor, char *speed)
{
    XSetForeground(display, gc, fontColor.pixel);
    if (speed)
        XDrawString (display, win, gc, 145, 35, speed, strlen(speed));
    XDrawString (display, win, gc, 120, 70, "dist/time", 9);
}

void print_brakes(Display *display, Window win, GC gc, XColor color, XColor fontColor)
{
    XSetForeground(display, gc, color.pixel);
    XFillArc(display,win, gc, 225, 1, 50,50,0,23040);
    XSetForeground(display, gc, fontColor.pixel);
    XDrawString (display, win, gc, 230, 70, "Brakes'wear", 6);
}

int main() {

    json_object *interface = NULL;
    elosEventSource_t source;
    elosEventVector_t *events;
    json_object *eventsJson;

    json_object_get_object(interface);
    interface = json_object_from_file("config/source.json");
    elosEventSourceFromJsonObject(&source, interface);
    eventsJson = json_object_new_array();
    eventsJson = json_object_from_file("config/events.json");
    elosEventVectorFromJsonArray(eventsJson, &events);

    elosSession_t *session = NULL;
    if (elosConnectTcpip("127.0.0.1", 54321, &session) == SAFU_RESULT_FAILED)
    {
        std::cout << "connection failed" << std::endl;
        return (1);
    }
    GC      gc; 
    Display *display; 
    int     screen; 
    Window  win, root; 
    unsigned long white_pixel, black_pixel;

    if ((display = XOpenDisplay ("")) == NULL) { 
        fprintf (stderr, "Can't open Display\n"); 
        exit (1); 
    } 
    gc = DefaultGC (display, screen); 
    screen = DefaultScreen (display); 
    root = RootWindow (display, screen); 
    white_pixel = WhitePixel (display, screen); 
    black_pixel = BlackPixel (display, screen); 
    win = XCreateSimpleWindow (display, root,  
                    0, 0, 500, 200, 2, black_pixel, white_pixel);
    XSelectInput (display, win, ExposureMask | KeyPressMask); 
    XStoreName (display, win, "elos demo"); 
    XMapWindow (display, win);
    Colormap map = XDefaultColormap(display, screen);
    XColor red;
    XAllocNamedColor(display, map, "red", &red, &red);
    XColor black;
    XAllocNamedColor(display, map, "black", &black, &black);
    XColor darkorange;
    XAllocNamedColor(display, map, "darkorange", &darkorange, &darkorange);
    XColor green1;
    XAllocNamedColor(display, map, "green1", &green1, &green1);

    const char *filterOil = ".event.source.appName 'oil thermometer' STRCMP";
    const char *filterBrakes = ".event.source.appName 'brakes' STRCMP";
    const char *filterSpeed = ".event.source.appName 'speed sensor' STRCMP";
    const char *filterShutdown = ".event.source.appName 'computer' STRCMP .event.messageCode 7002 EQ AND";
    elosEventQueueId_t queueOil;
    elosEventQueueId_t queueBrakes;
    elosEventQueueId_t queueSpeed;
    elosEventQueueId_t queueShutdown;
    if (elosEventSubscribe(session, &filterOil, 1, &queueOil) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterOil << " from " << source.appName << std::endl;
        return (1);
    }
    if (elosEventSubscribe(session, &filterBrakes, 1, &queueBrakes) == SAFU_RESULT_FAILED)
    {
        //creer un event subscription failure et l'envoyer
        std::cout << "Failed subscription to " << filterBrakes << " from " << source.appName << std::endl;
        return (1);
    }
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
    int temp_oil = 0;
    int brakes_alert = 0;
    char *speed = NULL;
    elosEventVector_t *oilVec;
    elosEventVector_t *brakesVec;
    elosEventVector_t *speedVec;
    elosEventVector_t *shutdownVec;

    while (power)
    {
        if (!elosSessionValid(session))
        {
            power = false;
            std::cout << "invalid session from " << source.appName << std::endl;

        }
        XEvent *event = NULL;
        event = (XEvent *)calloc(1, sizeof(XEvent));
        XCheckWindowEvent(display, win, KeyPressMask | ExposureMask, event);

        if (event && event->xkey.keycode == 111) //upper arrow
        {
            std::cout << "speed++"<< std::endl;
            elosEvent_t *speedUpPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 1));
            speedUpPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, speedUpPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing of speed++ from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            elosEventDelete(speedUpPublication);
        }
        if (event && event->xkey.keycode == 116) //lower arrow
        {
            std::cout << "speed--"<< std::endl;
            elosEvent_t *slowDownPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 2));
            slowDownPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, slowDownPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing of speed-- from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            elosEventDelete(slowDownPublication);
        }
        if (event && event->xkey.keycode == 33) //P-key
        {
            std::cout << "power off "<< std::endl;
            elosEvent_t *powerOffPublication;
            elosEvent_t *basisEvent;
            elosEventNew(&basisEvent);
            elosEventClone(&basisEvent, (elosEvent_t *)safuVecGet(events, 0));
            powerOffPublication = update_event(*basisEvent, &source);
            if (elosEventPublish(session, powerOffPublication) == SAFU_RESULT_FAILED)
            {
                //publish fail
                std::cout << "Failed publishing of power off from " << source.appName << std::endl;
            }
            elosEventDelete(basisEvent);
            elosEventDelete(powerOffPublication);
        }
        
        // //read les infos cockpit et les print

        //oil_temp
        if (elosEventQueueRead(session, queueOil, &oilVec) == SAFU_RESULT_FAILED)
        {
            std::cout << "Failed read of " << filterOil << " from " << source.appName << std::endl;
            
            //creer un event de fail de lecture
        }
        else if (safuVecElements(oilVec) > 0)
        {
            elosEvent_t *currentEvent = (elosEvent_t *)safuVecGetLast(oilVec);
            
            switch ((int)currentEvent->messageCode)
            {
            case 7126: //oil too hot
                temp_oil = 1;
                break;
            case 6003: //oil critically too hot
                temp_oil = 2;
                break;
            case 7127: //oil normal temp
                temp_oil = 0;
                break;
            default:
                break;
            }
            elosEventVectorDeleteMembers(oilVec);
        }
        //brakes
        if (elosEventQueueRead(session, queueBrakes, &brakesVec) == SAFU_RESULT_FAILED)
        {
            std::cout << "Failed read of " << filterBrakes << " from " << source.appName << std::endl;
            //creer un event de fail de lecture
        }
        else if (safuVecElements(brakesVec) > 0)
        {
            elosEvent_t *currentEvent = (elosEvent_t *)safuVecGetLast(brakesVec);
            switch (currentEvent->messageCode)
            {
            case 7126: //brakes too worn
                brakes_alert = 1;
                break;
            case 6003: //brakes critically too worn
                brakes_alert = 2;
                break;
            default: //brakes normal wear
                brakes_alert = 0;
                break;
            }
            elosEventVectorDeleteMembers(brakesVec);

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
            if (speed)
                free(speed);
            speed = (char *)calloc(strlen(currentEvent->payload)+1, sizeof(char));
            strcpy(speed, currentEvent->payload);
            elosEventVectorDeleteMembers(speedVec);
        }
        //shutdown
        if (elosEventQueueRead(session, queueShutdown, &shutdownVec) == SAFU_RESULT_FAILED)
        {
            std::cout << "Failed read of " << filterShutdown << " from " << source.appName << std::endl;
            //creer un event de fail de lecture
        }
        else if (safuVecElements(shutdownVec) > 0)
        {
            power = false;
            elosEventVectorDeleteMembers(shutdownVec);

        }
        XClearWindow(display, win);
        //XFlushGC(display, gc);
        switch (temp_oil)
        {
        case 0:
            print_oil(display, win, gc, green1, black);
            break;
        case 1:
            print_oil(display, win, gc, darkorange, black);
            break;
        default:
            print_oil(display, win, gc, red, black);
            break;        
        }
        print_speed(display, win, gc, black, speed);
        switch (brakes_alert)
        {
        case 0:
            print_brakes(display, win, gc, green1, black);
            break;
        case 1:
            print_brakes(display, win, gc, darkorange, black);
            break;
        case 2:
            print_brakes(display, win, gc, red, black);
            break;
        default:
            break;
        }
        free(event);
        usleep(5000);
    }
    //print power off
    elosEventVectorDelete(oilVec);
    elosEventVectorDelete(brakesVec);
    elosEventVectorDelete(speedVec);
    elosEventVectorDelete(shutdownVec);
    elosDisconnect(session);
    if (speed)
        free(speed);
    // XCloseDisplay(dpy);
    return (0);
}