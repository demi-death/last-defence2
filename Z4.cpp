#include <stdio.h>
#include <stdarg.h>
#include "Geo.hpp"
#include "SteadyTimer.hpp"
#include <list>
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
struct WeaponPreset;
struct BulletPreset;
struct Bullet;
struct Team;
struct Entity;
struct Zombie;
struct Player;

struct BulletPreset
{
    int damage;
    int penetration; // How many objects the bullet can penetrate
    double speed; // Speed of the bullet in units per second
    double range; // Maximum range of the bullet
};
typedef std::shared_ptr<Entity> EntityPtr;
typedef std::list<EntityPtr> EntityList;
typedef typename EntityList::iterator EntityIterator;
struct Bullet
{
    EntityPtr owner; // Owner of the bullet, can be a player or an NPC
    const PointVector initialPosition;
    PointVector position;
    PointVector velocity; // Velocity of the bullet in x and y directions
    double radius;
    double lifeTime;
public:
    struct HitInfo
    {
        EntityIterator target;
        double closest // [0.0, 1.0]
    }
    Bullet(EntityPtr _owner, const BulletPreset &_bulletPreset, const PointVector &_initialPosition, double _angle)
        : owner(_owner), initialPosition{_initialPosition}, position(_initialPosition), velocity(), lifeTime(lifeTime) {}
    virtual void onHit(EntityList &entityList, const std::list<typename EntityList::iterator> &hitList) = 0;
    static double distancePointToSegment(const PointVector& p, const PointVector& a, const PointVector& b, double& tOut)
    {
        PointVector ab = b - a;
        PointVector ap = p - a;
        double abLenSq = ab * ab;
        if (abLenSq == 0.0) {
            tOut = 0.0;
            return (p - a).length();
        }
        double t = (ap * ab) / abLenSq;
        if(t < 0.0)
            t = 0.0;
        else if(t > 1.0)
            t = 1.0;
        tOut = t;
        PointVector closest = a + ab * t;
        return (p - closest).length();
    }
    std::list<HitInfo> getHitEntities(const EntityList &entityList, double duration)
    {
        PointVector start = position;
        PointVector end = start + velocity * duration;

        Rect boundingBox = computeBoundingRectWithRadius(start, end, radius);

        std::list<EntityIterator> candidates;
        quadTree.query(boundingBox, candidates); // 후보만 추출 (빠름)

        std::list<HitInfo> hits;
        for (EntityIterator entity : candidates) {
            double tClosest;
            double dist = distancePointToSegment((**entity).position, start, end, tClosest);

            if (dist <= radius + (**entity).radius) {
                hits.push_back({ entity, tClosest });
            }
        }
        hits.sort([](auto& a, auto& b) {return a.tClosest < b.tClosest;});
        return std::move(hits);
    }
public:
    virtual ~Bullet()
    {}
};

struct WeaponPreset
{
    const char *name;
    int damage;
    double weight;
    double recoil;
    int burstCount = 1; // Default burst count for single-shot weapons (if 0, it is automatic)
    int burstDelayms = 0; // Default burst delay for single-shot weapons (if burstCount is 0, this is ignored)
    int delayms = 10;
    int ammoCapacity = 30; // Default ammo capacity for weapons
    int reloadTimems = 2000; // Default reload time in milliseconds
};

struct Team
{
    const char *name; // Name of the team
};
struct Entity
{
    Team *team; // Pointer to the team this entity belongs to, if any
    const char *name; // Name of the entity
    double radius;
    PointVector position; // Position of the entity in 2D space
    double angle;
    int health; // Health of the entity

    virtual void onHit(Entity &entity)
    {}

    virtual ~Entity() = default; // Virtual destructor for proper cleanup
};
struct Zombie : public Entity
{
    int damage; // Damage dealt by the zombie
    double speed; // Speed of the zombie in units per second
    double attackRange; // Range within which the zombie can attack
};
struct Player : public Entity
{
    int score;
    virtual void onHit(){}
};
std::list<Bullet> bullets;
std::list<Entity> entities;
void updateBullets(double deltaTime)
{
    for(auto it = bullets.begin(); it != bullets.end();)
    {
        Bullet &bullet = *it;
        bullet.lifeTime -= deltaTime;
        if(bullet.lifeTime <= 0)
        {
            it = bullets.erase(it); // Remove bullet if its lifetime has expired
            continue;
        }
        // Update bullet position based on its velocity
        bullet.position += bullet.velocity * deltaTime;

        // Check for collisions with entities or the world here (not implemented)
        
        ++it; // Move to the next bullet
    }
}
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