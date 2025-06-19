#ifndef _GAME_HPP_
#define _GAME_HPP_
//#include "SteadyTimer.hpp"
#include "TreeAlt.hpp"
class Game;
struct Team
{
    const char *name; // Name of the team
};
class Entity;
typedef std::shared_ptr<Entity> EntityPtr;
typedef std::weak_ptr<Entity> EntityWeakPtr;
class Entity : public PositionedObject
{
private:
    Game *m_game;
    const Team *m_team; // Pointer to the team this entity belongs to, if any
    const char *m_name; // Name of the entity
    int m_health; // Health of the entity
    int m_healthMax;
    double m_size;
    bool m_valid;
public:
    Entity(Game *game, const PointVector &pos, const Team *team, const char *name, int healthMax, double size)
        : PositionedObject(pos), m_game(game), m_team(team), m_name(name), m_health(healthMax), m_healthMax(healthMax), m_size(size), m_valid(true){}
    Game *game(void) const
    {return m_game;}
    const Team *team(void) const
    {return m_team;}
    const char *name(void) const
    {return m_name;}
    int health(void) const
    {return m_health;}
    int healthMax(void) const
    {return m_healthMax;}
    double size(void) const
    {return m_size;}
    bool valid(void) const
    {return m_valid;}
    void setHealth(int health)
    {m_health = health ? ((health < m_healthMax) ? health : m_healthMax) : 0;}
protected:
    void destroy(void)
    {m_valid = false;}
public:
    virtual void update(void) = 0; // You update Entity object yourself. (Ex: if hp is 0, set destroy and make the entity invalid)
    virtual void healthEvent(Entity *, int deltaHealth)
    {setHealth(health() + deltaHealth);}
    virtual ~Entity(){} // Virtual destructor for proper cleanup
};

class DroppedItem : public PositionedObject
{
private:
    Game *m_game;
    const char *m_name;
    double m_size;
    int m_cost;
public:
    DroppedItem(Game *game, const PointVector &pos, const char *name, double size)
        : PositionedObject(pos), m_game(game), m_name(name), m_size(size){}
    
    Game *game(void) const
    {return m_game;}
    const char *name(void) const
    {return m_name;}
    double size(void) const
    {return m_size;}
    int cost(void) const
    {return m_cost;}

    virtual void interact(Entity* ent) = 0;
    virtual ~DroppedItem(){}
};

class Bullet : public PositionedObject
{
    Game *m_game;
    Bullet(Game *game, const PointVector &pos)
        : PositionedObject(pos), m_game(game){}
    Game *game(void) const
    {return m_game;}
    virtual void update(void) = 0;
    virtual ~Bullet(){}
};
typedef std::shared_ptr<Bullet> BulletPtr;

class Game
{
    QuadTreeNode m_entityField;
    QuadTreeNode m_itemField;
    std::list<BulletPtr> m_bulletField;
    Game(const Rect &region, size_t playerCapacity = 1, size_t treeCapacity = 8)
        : m_entityField(region, treeCapacity), m_itemField(region, treeCapacity), m_bulletField(){}
    QuadTreeNode &entityField()
    {return m_entityField;}
    QuadTreeNode &itemField()
    {return m_itemField;}
};
typedef std::shared_ptr<DroppedItem> DroppedItemPtr;
//typedef std::unique_ptr<Bullet> BulletPtr;
#endif // _GAME_HPP_