#include <Pataro/Engine.hpp>

#include <Pataro/Map/Room.hpp>
#include <Pataro/Utils.hpp>
#include <Pataro/Constants.hpp>
#include <Pataro/Map/Constants.hpp>
#include <Pataro/Components/AI/Player.hpp>
#include <Pataro/Components/Attacker.hpp>
#include <Pataro/Components/Destructible/Player.hpp>
#include <Pataro/Components/Container.hpp>

#include <libtcod.hpp>

#include <string>
#include <utility>

using namespace pat;

Engine::Engine(unsigned width, unsigned height, const std::string& title, bool show_debug) :
    m_width(width), m_height(height), m_show_debug(show_debug)
{
    TCODConsole::initRoot(width, height, title.c_str(), false);
    TCODSystem::setFps(30);

    // create the player
    m_player = std::make_shared<Entity>(0, 0, '@', "Player", TCODColor::white);
    m_player->set_ai<component::details::PlayerAI>();
    m_player->set_attacker<component::Attacker>(5.f);
    m_player->set_destructible<component::details::PlayerDestructible>(30.f, 2.f, "your cadaver");
    m_player->set_container<component::Container>(26);  ///< One slot per letter in the alphabet

    // instantiate a map with 1 level(s)
    m_map = std::make_unique<Map>(map::details::level_w, map::details::level_h, 1);
    m_map->current_level().enter(m_player);

    // setup gui
    Gui::Proxy_t proxy_player_hp = [this](float* val, float* max_val) -> void {
        *val     = m_player->destructible()->hp();
        *max_val = m_player->destructible()->max_hp();
    };
    m_gui = std::make_unique<Gui>(m_width, 7, proxy_player_hp);

    m_gui->message(TCODColor::red, "Welcome stranger!\nPrepare to perish in the Tombs of the Ancient Kings.");
}

void Engine::update()
{
    if (m_state == GameState::StartUp)
        m_map->compute_fov(m_player->get_x(), m_player->get_y(), details::player_fov);
    m_state = GameState::Idle;

    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &m_lastkey, &m_mouse);
    std::unique_ptr<Action> action = m_player->update(this);
    if (action)
        action->perform(this);

    switch (m_lastkey.vk)
    {
        case TCODK_F3:
            TCODSystem::saveScreenshot(("screenshot_" + details::date_to_string() + ".png").c_str());
            break;

        default:
            break;
    }

    // if it's a new turn, update all the Entities
    if (m_state == GameState::NewTurn)
        m_map->update(this);
}

void Engine::render()
{
    TCODConsole::root->clear();

    m_map->render(this);
    m_gui->render(this, TCODConsole::root, 0, m_height - m_gui->get_height());

    if (m_show_debug)
    {
        TCODConsole::root->setDefaultForeground(TCODColor::white);
        TCODConsole::root->printf(0, 0, "%.2f", 1.f / TCODSystem::getLastFrameLength());
    }
}

bool Engine::is_running() const
{
    return !TCODConsole::isWindowClosed();
}

bool Engine::pick_a_tile(int* x, int* y, float max_range)
{
    int xp = m_player->get_x(),
        yp = m_player->get_y();

    int dx = xp - m_width  / 2,
        dy = yp - m_height / 2;

    while (is_running())
    {
        render();

        TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &m_lastkey, &m_mouse);

        for (int cx = 0, w = m_map->current_level().width(); cx < w; ++cx)
        {
            for (int cy = 0, h = m_map->current_level().height(); cy < h; ++cy)
            {
                float dist = static_cast<float>(details::get_manhattan_distance(cx, cy, xp, yp));

                if (m_map->is_in_fov(cx, cy) && (max_range == 0.f || dist <= max_range))
                {
                    TCODColor col = TCODConsole::root->getCharBackground(cx - dx, cy - dy);
                    col = col * 1.2f;
                    TCODConsole::root->setCharBackground(cx - dx, cy - dy, col);
                }
            }
        }

        float dist = static_cast<float>(details::get_manhattan_distance(m_mouse.cx + dx, m_mouse.cy + dy, xp, yp));
        if (m_map->is_in_fov(m_mouse.cx + dx, m_mouse.cy + dy) && (max_range == 0.f || dist <= max_range))
        {
            TCODConsole::root->setCharBackground(m_mouse.cx, m_mouse.cy, TCODColor::white);

            if (m_mouse.lbutton_pressed)
            {
                *x = m_mouse.cx + dx;
                *y = m_mouse.cy + dy;
                return true;
            }
        }

        if (m_mouse.rbutton_pressed)
            return false;

        TCODConsole::flush();
    }

    return false;
}