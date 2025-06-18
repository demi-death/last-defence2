#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "Geo.hpp"
#include "SteadyTimer.hpp"
#include "Tree.hpp"
#include <memory>
SteadyTimer timer;
double startTime;
double pastTime;
void debugPrintln(const char *format, ...)
{
    fprintf(stdout, "[%12.6lf]: ", timer() - startTime);
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    fputc('\n', stdout);
    va_end(args);
}
void debugErrPrintln(const char *format, ...)
{
    fprintf(stderr, "[%12.6lf]: ", timer() - startTime);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
}
struct Team
{
    const char *name; // Name of the team
};
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
typedef std::shared_ptr<Entity> EntityPtr;
typedef QuadTree<EntityPtr> EntityField;
class Bullet
{
private:
    EntityField *m_field;
    EntityPtr m_owner; // Owner of the bullet, can be a player or an NPC
    double m_size;
    PointVector m_pos;
    bool m_valid;
public:
    Bullet(EntityField *field, EntityPtr owner, double size, const PointVector &pos)
        : m_field(field), m_owner(std::move(owner)), m_size(size), m_pos(pos){}
    EntityField *field() const
    {return m_field;}
    const EntityPtr &owner(void) const
    {return m_owner;}
    double size(void) const
    {return m_size;}
    const PointVector &pos(void) const
    {return m_pos;}
protected:
    void destroy(void)
    {m_valid = false;}
public:
    virtual void update(void) = 0;
    virtual ~Bullet(){}
};
typedef std::unique_ptr<Bullet> BulletPtr;
Team human = {"human"};
Team zombie = {"zombie"};
std::list<BulletPtr> bullets;
class Crystal : public Entity
{
public:
    Crystal()
        : Entity(&human, "Crystal", 10000, 30, {0,0}){}
    virtual void update(void) override{}
};
EntityPtr crystal = std::make_shared<Crystal>();
class Player : public Entity
{
private:
    int m_sockFd;
public:
    Player(const char *name)
        : Entity(&human, name, 120, 8, {0,0}){}
    virtual void update(void) override
    {

    }
};
struct ZombiePreset
{
    const char *name;
    int healthMax;
    double size;
    int damage; // Damage dealt by the zombie
    double attackCooldown;
    double speed; // Speed of the zombie in units per second
};
class Zombie : public Entity
{
private:
    int m_damage; // Damage dealt by the zombie
    double m_attackCooldown;
    double m_speed; // Speed of the zombie in units per second
    double m_tick;
    double m_lastAttackTime;
    EntityPtr m_attackedBy;

    void _moveTo(const PointVector &p)
    {
        double tick = timer();
        double diff = tick - m_tick;



        m_tick = tick;
    }
public:
    Zombie(const Team *team, const ZombiePreset &zp, const PointVector &pos)
        : Entity(team, zp.name,  zp.healthMax, zp.size, pos), m_damage(zp.damage), m_attackCooldown(zp.attackCooldown), m_speed(zp.speed), m_lastAttackTime(-1){}
    virtual void update(void) override
    {
        if(m_attackedBy != nullptr)
        {
            if(!m_attackedBy->valid())
                m_attackedBy = nullptr;
            else
            {
                _moveTo(m_attackedBy->position());
                return;
            }
        }
        _moveTo(crystal->position());
    }
    virtual void healthEvent(EntityPtr entityPtr, int deltaHealth)
    {
        setHealth(health() + deltaHealth);
        m_attackedBy = entityPtr;
    }
};
class Item
{
    double size;
    void interact(void)
    {

    }
};
void atexit1(void)
{
    debugPrintln("Server ended");
}
int main(void)
{
    startTime = timer();
    atexit(atexit1);
    double pastTime = startTime;
    double currentTime = startTime;
    bool gameRunning = true;
    debugPrintln("Server started");
    Matrix2x2 rmatrix = rotateMatrix(3.1415926535897932 / 4.0); // Example rotation matrix for 45 degrees
    PointVector vec(2.0, 3.0);
    PointVector vec1 = rmatrix * vec; // Rotate the vector using the rotation matrix
    
    debugPrintln("Rotated vector: (%lf, %lf)", vec1[0], vec1[1]);
    do
    {
        double currentTime = timer();
        if(currentTime - pastTime < 0)
        {
            debugErrPrintln("Error: Current time is less than past time. This should not happen.");
            break;
        }
        pastTime = currentTime;


    }while(gameRunning);
    
}