#ifndef PATARO_ACTOR_HPP
#define PATARO_ACTOR_HPP

#include <Pataro/Actor/AI.hpp>
#include <Pataro/Actor/Attacker.hpp>
#include <Pataro/Actor/Destructible.hpp>
#include <Pataro/Actor/Pickable.hpp>
#include <Pataro/Actor/Container.hpp>

#include <libtcod.hpp>

#include <string>
#include <utility>
#include <memory>

namespace pat
{
    class Engine;

    /**
     * @brief An actor class representing items, traps, doors, monsters, players...
     * @details It uses composition to represent all of those kind of actors.
     * 
     */
    class Actor
    {
    public:
        /**
         * @brief Construct a new Actor object
         * 
         * @param x position on X axis
         * @param y position on Y axis
         * @param ch ascii code representing the character
         * @param name the name of the actor
         * @param color the color for the actor
        */
        Actor(int x, int y, int ch, const std::string& name, const TCODColor& color);

        /**
         * @brief Morph the actor into... something else
         * 
         * @param ch 
         * @param name 
         * @param color 
         */
        void morph_into(int ch, const std::string& name, const TCODColor& color);

        /**
         * @brief display the actor on screen
         * 
         * @param dx x offset on screen
         * @param dy y offset on screen
        */
        void render(int dx=0, int dy=0) const;

        /**
         * @brief Update the actor
         * 
         * @param engine 
         */
        void update(Engine* engine);

        inline int  get_x() const { return m_x; }
        inline int  get_y() const { return m_y; }
        inline void put_at(int x, int y) { m_x = x; m_y = y; }

        inline const std::string& get_name() const { return m_name; }

        inline bool is_blocking() const { return m_blocks; }
        inline void set_blocking(bool value) { m_blocks = value; }

        inline actor::Attacker*     attacker() { return m_attacker.get(); }
        inline actor::Destructible* destructible() { return m_destructible.get(); }
        inline actor::AI*           ai() { return m_ai.get(); }

        template <class A, typename... Args>
        void set_attacker(Args&&... args)
        {
            m_attacker = std::make_unique<A>(std::forward<Args>(args)...);
        }

        template <class D, typename... Args>
        void set_destructible(Args&&... args)
        {
            m_destructible = std::make_unique<D>(std::forward<Args>(args)...);
        }

        template <class AI, typename... Args>
        void set_ai(Args&&... args)
        {
            m_ai = std::make_unique<AI>(std::forward<Args>(args)...);
        }

    private:
        int m_x, m_y;
        int m_ch;
        std::string m_name;
        TCODColor m_color;

        bool m_blocks = true;  ///< Can we walk on this actor?
        std::unique_ptr<actor::Attacker> m_attacker = nullptr;  ///< For actors that deal damages
        std::unique_ptr<actor::Destructible> m_destructible = nullptr;  ///< For destructible actors
        std::unique_ptr<actor::AI> m_ai = nullptr;  ///< For self updating actors
        std::unique_ptr<actor::Pickable> m_pickable = nullptr;  ///< Something that can be picked up and used
        std::unique_ptr<actor::Container> m_container = nullptr;  ///< Something that can contain actors
    };
}

#endif
