#ifndef _GAME_HPP_
#define _GAME_HPP_
#include "SteadyTimer.hpp"
#include <memory>
#include "Tree.hpp"
struct Team
{
    const char *name; // Name of the team
};
class Entity;
typedef std::shared_ptr<Entity> EntityPtr;
class Entity
{
private:
    const Team *m_team; // Pointer to the team this entity belongs to, if any
    const char *m_name; // Name of the entity
    int m_health; // Health of the entity
    int m_healthMax;
    double m_size;
    PointVector m_pos;
    bool m_valid;
public:
    Entity(const Team *team, const char *name, int healthMax, double size, const PointVector &pos)
        : m_team(team), m_name(name), m_health(healthMax), m_healthMax(healthMax), m_size(size), m_pos(pos), m_valid(true){}
    const Team *team(void) const
    {return m_team;}
    const char *name(void) const
    {return m_name;}
    const PointVector &position(void) const
    {return m_pos;}
    int health(void) const
    {return m_health;}
    int healthMax(void) const
    {return m_healthMax;}
    double size(void) const
    {return m_size;}
    bool valid(void) const
    {return m_valid;}
    void setPosition(const PointVector &pos)
    {m_pos = pos;}
    void setHealth(unsigned health)
    {m_health = health ? ((health < m_healthMax) ? health : m_healthMax) : 0;}
protected:
    void destroy(void)
    {m_valid = false;}
public:
    virtual void update(void) = 0; // You update Entity object yourself. (Ex: if hp is 0, set destroy and make the entity invalid)
    virtual void healthEvent(EntityPtr, int deltaHealth)
    {setHealth(health() + deltaHealth);}
    virtual ~Entity(){} // Virtual destructor for proper cleanup
};
#endif // _GAME_HPP_