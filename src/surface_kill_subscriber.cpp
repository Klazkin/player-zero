#include "surface_kill_subscriber.h"

SurfaceKillSubscriber::SurfaceKillSubscriber(Surface *p_surface, SurfaceElement *p_element)
{
    element = p_element;
    surface = p_surface;
    sub_id = SURFACE_KILL;
}

void SurfaceKillSubscriber::clone_to(CloneContext &clone_context) const
{
    std::cerr << "Called *SurfaceKillSubscriber::clone() THIS MUST NEVER HAPPEN!!\n";
}

void SurfaceKillSubscriber::on_death()
{
    if (element->get_is_on_surface())
    {
        surface->lift_element(element->get_position());
    }

    if ((*surface->turn_get_current_unit()) == element)
    {
        surface->end_current_units_turn();
        std::cout;
    }
}