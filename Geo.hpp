#ifndef _GEO_HPP_
#define _GEO_HPP_

#include <math.h>

struct PointVector
{
    double coord[2];

    constexpr const double &operator[](ptrdiff_t idx) const
    {return coord[idx];}

    constexpr double &operator[](ptrdiff_t idx)
    {return coord[idx];}

    constexpr PointVector(double _x = 0.0, double _y = 0.0) : coord{_x, _y} {}

    constexpr PointVector(const PointVector &other) : coord{other[0], other[1]} {}

    constexpr friend PointVector operator+(const PointVector &_this, const PointVector &other)
    {return PointVector(_this[0] + other[0], _this[1] + other[1]);}

    constexpr friend PointVector operator-(const PointVector &_this, const PointVector &other)
    {return PointVector(_this[0] - other[0], _this[1] - other[1]);}

    constexpr friend PointVector operator*(const PointVector &_this, double scalar)
    {return PointVector(_this[0] * scalar, _this[1] * scalar);}

    constexpr friend double operator*(const PointVector &_this, const PointVector &other)
    {return _this[0] * other[0] + _this[1] * other[1];}

    constexpr friend PointVector operator/(const PointVector &_this, double scalar)
    {return (scalar == 0.0) ? PointVector(NAN, NAN) : PointVector(_this[0] / scalar, _this[1] / scalar);}

    constexpr friend PointVector operator-(const PointVector &_this)
    {return PointVector(-(_this[0]), -(_this[1]));}

    inline PointVector &operator+=(const PointVector &other)
    {
        (*this)[0] += other[0];
        (*this)[1] += other[1];
        return *this;
    }

    inline PointVector &operator-=(const PointVector &other)
    {
        (*this)[0] -= other[0];
        (*this)[1] -= other[1];
        return *this;
    }

    inline PointVector &operator*=(double scalar)
    {
        (*this)[0] *= scalar;
        (*this)[1] *= scalar;
        return *this;
    }

    inline PointVector &operator/=(double scalar)
    {
        if(scalar == 0.0)
        {
            (*this)[0] = NAN;
            (*this)[1] = NAN; // Set to zero vector to avoid undefined behavior
        }
        else
        {
            (*this)[0] /= scalar;
            (*this)[1] /= scalar;
        }
        return *this;
    }

    double length() const
    {return sqrt((*this) * (*this));}
};

struct Matrix2x2
{
    PointVector basis[2];

    constexpr const PointVector &operator[](ptrdiff_t idx) const
    {return basis[idx];}

    constexpr PointVector &operator[](ptrdiff_t idx)
    {return basis[idx];}

    constexpr Matrix2x2(const PointVector &bx = {1.0, 0.0}, const PointVector &by = {0.0, 1.0}) : basis{bx, by} {}
    constexpr Matrix2x2(const Matrix2x2 &other) : basis{other[0], other[1]} {}

    constexpr friend Matrix2x2 operator*(const Matrix2x2 &_this, const Matrix2x2 &other)
    {
        return Matrix2x2(
            PointVector(
                _this[0][0] * other[0][0] + _this[1][0] * other[0][1],
                _this[0][1] * other[0][0] + _this[1][1] * other[0][1]
            ),
            PointVector(
                _this[0][0] * other[1][0] + _this[1][0] * other[1][1],
                _this[0][1] * other[1][0] + _this[1][1] * other[1][1]
            )
        );
    }

    constexpr friend PointVector operator*(const Matrix2x2 &_this, const PointVector &vec)
    {
        return PointVector(
            _this[0][0] * vec[0] + _this[1][0] * vec[1],
            _this[0][1] * vec[0] + _this[1][1] * vec[1]
        );
    }
};

Matrix2x2 rotateMatrix(double angle)
{
    double cosAngle = cos(angle);
    double sinAngle = sin(angle);
    return Matrix2x2({cosAngle, sinAngle}, {-sinAngle, cosAngle});
}

class RectLooseness
{
private:
    bool m_boolAry[2][2] = {{false, false}, {false, false}};
public:
    constexpr RectLooseness(){}
    constexpr RectLooseness(const bool (&m_boolAry)[2][2])
        : m_boolAry{{m_boolAry[0][0], m_boolAry[0][1]}, {m_boolAry[1][0], m_boolAry[1][1]}}{}
    constexpr RectLooseness(const RectLooseness &other)
        : RectLooseness(other.m_boolAry){}
    
    constexpr bool looseMin(size_t axis) const
    {return m_boolAry[axis][0];}
    constexpr bool looseMax(size_t axis) const
    {return m_boolAry[axis][1];}

    friend constexpr RectLooseness operator|(const RectLooseness &_this, const RectLooseness &other)
    {
        return RectLooseness({
            {_this.looseMin(0) || other.looseMin(0), _this.looseMax(0) || other.looseMax(0)},
            {_this.looseMin(1) || other.looseMin(1), _this.looseMax(1) || other.looseMax(1)}
        });
    }

    friend constexpr RectLooseness operator&(const RectLooseness &_this, const RectLooseness &other)
    {
        return RectLooseness({
            {_this.looseMin(0) && other.looseMin(0), _this.looseMax(0) && other.looseMax(0)},
            {_this.looseMin(1) && other.looseMin(1), _this.looseMax(1) && other.looseMax(1)}
        });
    }

    friend constexpr bool operator==(const RectLooseness &_this, const RectLooseness &other)
    {
        return (_this.looseMin(0) == other.looseMin(0)) && (_this.looseMax(0) == other.looseMax(0)) &&
            (_this.looseMin(1) == other.looseMin(1)) && (_this.looseMax(1) == other.looseMax(1));
    }
    
    friend constexpr bool operator!=(const RectLooseness &_this, const RectLooseness &other)
    {
        return !(_this == other);
    }
};
static constexpr RectLooseness completelyLoose({{true, true}, {true, true}});
static constexpr RectLooseness completelyStrict;

class Rect
{
public:
    typedef RectLooseness Looseness;
private:
    PointVector m_min;
    PointVector m_max;
    Looseness m_looseness;

    static constexpr double __max1(double a, double b)
    { return (a < b) ? b : a; }

    static constexpr double __min1(double a, double b)
    { return (a > b) ? b : a; }

public:
    constexpr Rect(const PointVector &a, const PointVector &b, const Looseness &looseness = {})
        : m_min(__min1(a[0], b[0]), __min1(a[1], b[1])), m_max(__max1(a[0], b[0]), __max1(a[1], b[1])), m_looseness(looseness) {}

    constexpr Rect(const Rect &other)
        : m_min(other.m_min), m_max(other.m_max), m_looseness(other.m_looseness) {}

    constexpr bool contains(const PointVector &pos) const
    {
        return 
            (m_looseness.looseMin(0) || m_min[0] <= pos[0]) &&
            (m_looseness.looseMax(0) || pos[0] <= m_max[0]) &&
            (m_looseness.looseMin(1) || m_min[1] <= pos[1]) &&
            (m_looseness.looseMax(1) || pos[1] <= m_max[1]);
    }

    constexpr bool intersects(const Rect &other) const
    {
        return
            (other.m_looseness.looseMin(0) || m_looseness.looseMax(0) || other.m_min[0] <= m_max[0]) &&
            (m_looseness.looseMin(0) || other.m_looseness.looseMax(0) || m_min[0] <= other.m_max[0])
            &&
            (other.m_looseness.looseMin(1) || m_looseness.looseMax(1) || other.m_min[1] <= m_max[1]) &&
            (m_looseness.looseMin(1) || other.m_looseness.looseMax(1) || m_min[1] <= other.m_max[1]);
    }

    constexpr Looseness looseness(void) const
    {return m_looseness;}

    constexpr bool discrete(const Rect &other) const
    {return !intersects(other);}

    constexpr PointVector center() const { return (m_min + m_max) * 0.5; }
    constexpr PointVector size() const { return m_max - m_min; }
    constexpr PointVector minPoint() const { return m_min; }
    constexpr PointVector maxPoint() const { return m_max; }
    constexpr PointVector vertex(const bool(&isMax)[2]) const 
    {
        return PointVector{
            isMax[0] ? m_max[0] : m_min[0],
            isMax[1] ? m_max[1] : m_min[1]
        };
    }
};

#endif // _GEO_HPP_