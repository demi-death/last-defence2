#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#if defined(__unix__) || defined(__unix)
#include <sys/socket.h>
typedef int SocketT;
#elif defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
typedef SOCKET SocketT;
#endif
#include "SteadyTimer.hpp"
#include "Game.hpp"
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
#define BUFSIZE1 512
class Player : public Entity 
{
private:
    SocketT m_sockFd;
    std::string m_requestBuf;
    char m_received[512];

    // 이동 상태 플래그 (m_ 접두사 사용)
    typedef int _CastType;

    enum class Flags : _CastType
    {
        none           = 0,
        moveForward  = 1 << 0,
        moveBackward = 1 << 1,
        moveLeft     = 1 << 2,
        moveRight    = 1 << 3,
        shoot       = 1 << 4
    };
    Flags m_flags = Flags::none;

    static constexpr double m_speed = 1.0;

    PointVector m_screenSize = {0, 0};
    PointVector m_direction = {0, 1}; // 기본 방향: 위쪽
public:
    friend constexpr Flags operator~(Flags lhs)
    {return static_cast<Flags>(~static_cast<_CastType>(lhs));}
    friend constexpr Flags operator&(Flags lhs, Flags rhs)
    {return static_cast<Flags>(static_cast<_CastType>(lhs) & static_cast<_CastType>(rhs));}
    friend constexpr Flags operator^(Flags lhs, Flags rhs)
    {return static_cast<Flags>(static_cast<_CastType>(lhs) ^ static_cast<_CastType>(rhs));}
    friend constexpr Flags operator|(Flags lhs, Flags rhs)
    {return static_cast<Flags>(static_cast<_CastType>(lhs) | static_cast<_CastType>(rhs));}

    friend inline Flags& operator&=(Flags& lhs, Flags rhs)
    {return lhs = lhs & rhs;}
    friend inline Flags& operator^=(Flags& lhs, Flags rhs)
    {return lhs = lhs ^ rhs;}
    friend inline Flags& operator|=(Flags& lhs, Flags rhs)
    {return lhs = lhs | rhs;}
    Player(const char* name, SocketT sockFd, const PointVector &pos, const Team *team)
        : Entity(pos, team, name, 120, 8), m_sockFd(sockFd) {}
    void enableFlag(Flags f) {
        m_flags |= f;
    }
    void disableFlag(Flags f) {
        m_flags &= ~f;
    }
    void toggleFlag(Flags f) {
        m_flags ^= f;
    }
    bool hasAllFlags(Flags f) const {
        return (m_flags & f) == f;
    }
    bool hasAnyFlag(Flags f) const {
        return (m_flags & f) != Flags::none;
    }
    void setDirection(double rad){
        m_direction = PointVector(cos(rad), sin(rad));
    }
    void applyMove(double moveDist){
        PointVector delta{0, 0};
        bool moveX = false;
        if (hasAllFlags(Flags::moveForward) != hasAllFlags(Flags::moveBackward))
        {
            if(hasAllFlags(Flags::moveForward))
                delta += m_direction * m_speed;
            else
                delta -= m_direction * m_speed;
            moveX = true;
        }
        if (hasAllFlags(Flags::moveLeft) != hasAllFlags(Flags::moveRight))
        {
            if(hasAllFlags(Flags::moveLeft))
                delta += PointVector(-m_direction[1], m_direction[0]) * m_speed;
            else
                delta -= PointVector(-m_direction[1], m_direction[0]) * m_speed;
            if(moveX)
                delta /= sqrt(2);
        }
        setPosition(position() + delta);
    }
    virtual void update(void) override
    {
        int len = recv(m_sockFd, m_received, sizeof(m_received) - 1, 0);
        if (len <= 0) {
            disconnect();
            return;
        }
        m_received[len] = '\0';
        for (int i = 0; i < len; ++i) {
            if (m_received[i] == '\n') {
                _processCommand(m_requestBuf.c_str());
                m_requestBuf.clear();
            } else {
                m_requestBuf += m_received[i];
            }
        }
        applyMove(m_speed);
        if (hasAllFlags(Flags::shoot)) _doShoot();
        const char* response = "ACK\n";
        send(m_sockFd, response, strlen(response), 0);
    }

private:
    void _doShoot(void)
    {

    }
    void _doReload(void)
    {
        disableFlag(Flags::shoot);
    }
    void _doInteract(void)
    {

    }
    // 명령 처리 함수
    void _processCommand(const char *cmd)
    {
        if (!cmd) return;
        char op = cmd[0];
        switch (op)
        {
        case '\0': break;
        case '0': disconnect(); break;
        case '1': enableFlag(Flags::moveForward); break;
        case '2': disableFlag(Flags::moveForward); break;
        case '3': enableFlag(Flags::moveLeft); break;
        case '4': disableFlag(Flags::moveLeft); break;
        case '5': enableFlag(Flags::moveBackward); break;
        case '6': disableFlag(Flags::moveBackward); break;
        case '7': enableFlag(Flags::moveRight); break;
        case '8': disableFlag(Flags::moveRight); break;
        case '9': enableFlag(Flags::shoot); break;
        case 'A': disableFlag(Flags::shoot); break;
        case 'B': _doReload(); break;
        case 'C': _doInteract(); break;
        case 'D': {
            double rad = 0.0;
            if (sscanf(cmd + 1, "%lf", &rad) == 1)
                setDirection(rad);
            break;
        }
        case 'E': {
            double x = 0.0, y = 0.0;
            if (sscanf(cmd + 1, "%lf %lf", &x, &y) == 2)
                m_screenSize = {x, y};
            break;
        }
        default: break;
        }

    }
    // 이동 벡터 계산
    void _processMove()
    {
        PointVector delta{0, 0};
        if (hasAllFlags(Flags::moveForward) != hasAllFlags(Flags::moveBackward))
        {
            if(hasAllFlags(Flags::moveForward))
                delta += m_direction * m_speed;
            else
                delta -= m_direction * m_speed;
        }
        if (hasAllFlags(Flags::moveLeft) != hasAllFlags(Flags::moveRight))
        {
            if(hasAllFlags(Flags::moveLeft))
                delta += PointVector(-m_direction[1], m_direction[0]) * m_speed;
            else
                delta -= PointVector(-m_direction[1], m_direction[0]) * m_speed;
        }
        setPosition(position() + delta);
    }

    // 방향 설정

    // 이동 상태 설정 함수들

    void disconnect()
    {
        if (m_sockFd != INVALID_SOCKET)
            closesocket(m_sockFd);
        m_sockFd = INVALID_SOCKET;
    }

    ~Player()
    {
        disconnect();
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
    Entity* m_defaultTrack;
    Entity* m_attackedBy;
    int m_damage;
    double m_speed;
    double m_attackCooldown;
    double m_lastTick;
    double m_lastAttackTime;

public:
    Zombie(Game *game, const char *name, const ZombiePreset &zp, const PointVector &pos, const Team *team, Entity* defaultTarget)
        : Entity(game, pos, team, zp.name, zp.healthMax, zp.size),
          m_defaultTrack(defaultTarget), m_attackedBy(), m_damage(zp.damage),
          m_speed(zp.speed), m_attackCooldown(zp.attackCooldown),
          m_lastTick(0.0), m_lastAttackTime(0.0) {}

    virtual void update(void) override
    {
        Entity* target = nullptr;
        if (m_attackedBy)
        {
            if(!m_attackedBy->valid())
                m_attackedBy = nullptr;
            else
                target = m_attackedBy;
        }
        else if (m_defaultTrack)
        {
            if(!m_defaultTrack->valid())
                m_defaultTrack = nullptr;
            else
                target = m_defaultTrack;
        }
        double now = timer();
        if(target)
        {
            PointVector toTarget = target->position() - position();
            double dist = toTarget.length();
            if (dist <= size() + target->size()) // Within attack range
            {
                if (now - m_lastAttackTime >= m_attackCooldown)
                {
                    target->healthEvent(this, -m_damage);
                    m_lastAttackTime = now;
                }
            }
            else
            {
                if (now != m_lastTick)
                {
                    double dt = now - m_lastTick;
                    PointVector moveVec = toTarget.normalized() * m_speed * dt;
                    setPosition(position() + moveVec);
                }
            }
        }
        m_lastTick = now;
    }

    virtual void healthEvent(Entity* entityPtr, int deltaHealth) override
    {
        setHealth(health() + deltaHealth);
        m_attackedBy = entityPtr;
    }
};

class Gun
{
public:
    // 비트 플래그 정의
    typedef int _CastType;
    enum class Flags : _CastType
    {
        none         = 0,
        isAuto       = 1 << 0,
        isReloading  = 1 << 1,
        isFiring     = 1 << 2
    };

protected:
    Game *m_game;
    const char *m_name        = "";
    double      m_size        = 1.0;
    int         m_ammoCount   = 0;
    int         m_ammoMax     = 0;
    double      m_lastTick    = 0.0;
    double      m_rate        = 0.1;
    double      m_fireTime    = 0;
    double      m_reloadTime  = 0.0;
    double      m_reloadStart = 0.0;
    Flags       m_flags       = Flags::none;

public:
    friend constexpr Flags operator~(Flags lhs)
    {return static_cast<Flags>(~static_cast<_CastType>(lhs));}
    friend constexpr Flags operator&(Flags lhs, Flags rhs)
    {return static_cast<Flags>(static_cast<_CastType>(lhs) & static_cast<_CastType>(rhs));}
    friend constexpr Flags operator^(Flags lhs, Flags rhs)
    {return static_cast<Flags>(static_cast<_CastType>(lhs) ^ static_cast<_CastType>(rhs));}
    friend constexpr Flags operator|(Flags lhs, Flags rhs)
    {return static_cast<Flags>(static_cast<_CastType>(lhs) | static_cast<_CastType>(rhs));}

    friend inline Flags& operator&=(Flags& lhs, Flags rhs)
    {return lhs = lhs & rhs;}
    friend inline Flags& operator^=(Flags& lhs, Flags rhs)
    {return lhs = lhs ^ rhs;}
    friend inline Flags& operator|=(Flags& lhs, Flags rhs)
    {return lhs = lhs | rhs;}
protected:
    void enableFlag(Flags f) {
        m_flags |= f;
    }
    void disableFlag(Flags f) {
        m_flags &= ~f;
    }
    void toggleFlag(Flags f) {
        m_flags ^= f;
    }
public:
    bool hasAllFlags(Flags f) const {
        return (m_flags & f) == f;
    }
    bool hasAnyFlag(Flags f) const {
        return (m_flags & f) != Flags::none;
    }
    double reloadStart(void) const{
        return m_reloadStart;
    }
    double reloadTime(void) const{
        return m_reloadTime;
    }
    Gun(Game *game, const char *name, double size, int ammoCount, double rate, double reloadTime = -1)
        : m_game(game), m_name(name), m_size(size), m_ammoCount(ammoCount), m_ammoMax(ammoCount), m_rate(rate), m_reloadTime(reloadTime), m_reloadStart(0){}
    virtual BulletPtr shoot(void) = 0;
    BulletPtr updateGun(void)
    {
        double now = timer();
        if(hasAllFlags(Flags::isReloading))
        {
            if(now - m_reloadStart >= m_reloadTime)
                m_ammoCount = m_ammoMax;
            disableFlag(Flags::isReloading);
        }
        else if(hasAllFlags(Flags::isFiring))
        {
            if(now - m_fireTime >= m_rate)
                return shoot();
            m_fireTime = now;
            if(!hasAnyFlag(Flags::isAuto))
                disableFlag(Flags::isFiring);
        }
        return nullptr;
    }
    void reload(void)
    {
        m_reloadStart = timer();
        m_lastTick = 
    }
    virtual ~Gun() = default;
};

Team human = {"human"};
Team zombie = {"zombie"};
class Crystal : public Entity
{
public:
    Crystal(Game *game,const Team *team)
        : Entity(game, {0,0}, team, "Crystal", 10000, 30){}
    virtual void update(void) override{}
};
EntityPtr crystal = std::make_shared<Crystal>(&human);

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