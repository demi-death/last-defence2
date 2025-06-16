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
// 이대로 어떤 설명보다 내 느낌이 정말
// 숨길 수 없이 달아 이끌림 오직 너를 위해 Psychic Lover!
Matrix2x2 rotateMatrix(double angle)
{
    double cosAngle = cos(angle);
    double sinAngle = sin(angle);
    return Matrix2x2({cosAngle, sinAngle}, {-sinAngle, cosAngle});
}

class Rect
{
private:
    PointVector m_min;
    PointVector m_max;

    static constexpr double __max1(double a, double b)
    {return (a < b) ? a : b;}

    static constexpr double __min1(double a, double b)
    {return (a > b) ? a : b;}
private:
public:
    constexpr Rect(PointVector a, PointVector b)
        : m_min(__min1(a[0], b[0]), __min1(a[1], b[1])), m_max(__max1(a[0], b[0]), __max1(a[1], b[1])){}
    
    constexpr Rect(const Rect &other)
        : m_min(other.m_min), m_max(other.m_max){}
    
    constexpr bool contains(const Rect &subRect)
    {
        return 
    }
};

#endif // _GEO_HPP_