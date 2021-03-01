#include <Pataro/Actions/Move.hpp>

#include <Pataro/Constants.hpp>
#include <Pataro/Engine.hpp>
#include <Pataro/Entity.hpp>
#include <Pataro/Components/Destructible.hpp>
#include <Pataro/Actions/Attack.hpp>

#include <libtcod.hpp>

using namespace pat::action;

MoveAction::MoveAction(pat::Entity* source, int dx, int dy) :
    m_source(source), m_dx(dx), m_dy(dy)
{}

pat::ActionResult MoveAction::perform([[maybe_unused]] pat::Engine* engine)
{
    int x = m_source->get_x(),
        y = m_source->get_y();

    // if we've found a possible ennemy, attack it
    if (Entity* e = engine->get_map()->get_entity(x + m_dx, y + m_dy); e != nullptr &&
        // so that our player can attack, and monsters will only attack them
        (m_source == engine->get_player() || engine->get_player() == e))
        return alternate<AttackAction>(m_source, e);

    // physics
    if (engine->get_map()->is_wall(x + m_dx, y + m_dy))
        return pat::ActionResult::Fail;
    m_source->move(m_dx, m_dy);

    // update player field of view
    // list everything under the player
    if (m_source == engine->get_player())
    {
        engine->get_map()->compute_fov(x, y, pat::details::player_fov);

        if (pat::Entity* e = engine->get_map()->get_entity(x, y); e != nullptr)
        {
            pat::component::Destructible* d = e->destructible();
            if (d != nullptr && d->is_dead())
                engine->get_gui()->message(TCODColor::lightGrey, "There is a ", e->get_name(), " here");
        }
    }

    return pat::ActionResult::Success;
}