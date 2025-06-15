#ifndef _GEO_HPP_
#define _GEO_HPP_

#include <math.h>

struct PointVector
{
    double x;
    double y;

    constexpr PointVector(double _x = 0.0, double _y = 0.0) : x(_x), y(_y) {}

    constexpr PointVector(const PointVector &other) : x(other.x), y(other.y) {}

    constexpr PointVector operator+(const PointVector &other) const
    {return PointVector(x + other.x, y + other.y);}

    constexpr PointVector operator-(const PointVector &other) const
    {return PointVector(x - other.x, y - other.y);}

    constexpr PointVector operator*(double scalar) const
    {return PointVector(x * scalar, y * scalar);}

    constexpr PointVector operator/(double scalar) const
    {
        if(scalar == 0.0)
            return PointVector(NAN, NAN); // Return a zero vector to avoid undefined behavior
        return PointVector(x / scalar, y / scalar);
    }

    constexpr PointVector operator-() const
    {return PointVector(-x, -y);}

    inline PointVector &operator+=(const PointVector &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline PointVector &operator-=(const PointVector &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline PointVector &operator*=(double scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    inline PointVector &operator/=(double scalar)
    {
        if(scalar == 0.0)
        {
            x = NAN;
            y = NAN; // Set to zero vector to avoid undefined behavior
        }
        else
        {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }

    double length() const
    {return sqrt(x * x + y * y);}
};

struct Matrix2x2
{
    PointVector basisX;
    PointVector basisY;

    constexpr Matrix2x2(const PointVector &bx = {1.0, 0.0}, const PointVector &by = {0.0, 1.0}) : basisX(bx), basisY(by) {}
    constexpr Matrix2x2(const Matrix2x2 &other) : basisX(other.basisX), basisY(other.basisY) {}

    constexpr friend Matrix2x2 operator*(const Matrix2x2 &_this, const Matrix2x2 &other)
    {
        return Matrix2x2(
            PointVector(
                _this.basisX.x * other.basisX.x + _this.basisY.x * other.basisX.y,
                _this.basisX.y * other.basisX.x + _this.basisY.y * other.basisX.y
            ),
            PointVector(
                _this.basisX.x * other.basisY.x + _this.basisY.x * other.basisY.y,
                _this.basisX.y * other.basisY.x + _this.basisY.y * other.basisY.y
            )
        );
    }

    constexpr friend PointVector operator*(const Matrix2x2 &_this, const PointVector &vec)
    {
        return PointVector(
            _this.basisX.x * vec.x + _this.basisY.x * vec.y,
            _this.basisX.y * vec.x + _this.basisY.y * vec.y
        );
    }
};

Matrix2x2 rotateMatrix(double angle)
{
    double cosAngle = cos(angle);
    double sinAngle = sin(angle);
    return Matrix2x2({cosAngle, sinAngle}, {-sinAngle, cosAngle});
}

#endif // _GEO_HPP_