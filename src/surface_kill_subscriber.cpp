#include "surface_kill_subscriber.h"

SurfaceKillSubscriber::SurfaceKillSubscriber(Surface *p_surface, SurfaceElement *p_element)
{
    element = p_element;
    surface = p_surface;
    sub_id = SURFACE_KILL;
}

void SurfaceKillSubscriber::on_death()
{
    if (element->get_is_on_surface())
    {
        surface->lift_element(element->get_position());
    }
}