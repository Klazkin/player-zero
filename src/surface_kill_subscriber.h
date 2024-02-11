#ifndef SURFACE_KILL_SUBSCRIBER_H
#define SURFACE_KILL_SUBSCRIBER_H

#include "unit_subscriber.h"
#include "surface.h"
#include "surface_element.h"

class SurfaceKillSubscriber : public UnitSubscriber
{
private:
    Surface *surface;
    SurfaceElement *element;

public:
    SurfaceKillSubscriber(Surface *p_surface, SurfaceElement *p_element);
    void on_death() override;
};

#endif