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

void SurfaceKillSubscriber::on_death() // TODO need a way better dead unit clean up system, this is awful
{
    // std::cout << "<" << as_unit_ptr(element)->get_faction() << ">";
    if (element != *surface->get_element(element->get_position()))
    {

        if (surface->get_element(element->get_position()) != nullptr)
        {
            std::cout << "deleting a different element\n\tXXX\n";
        }
    }

    if (!element->get_is_on_surface())
    {
        // std::cout << "Called kill subscriber on lifted element.\n";
        return;
    }
    else
    {
        surface->lift_element(element->get_position());
    }

    if ((*surface->turn_get_current_unit()) == element) // Only for units, but the element may be a SE too
    {
        surface->end_current_units_turn();
    }
}