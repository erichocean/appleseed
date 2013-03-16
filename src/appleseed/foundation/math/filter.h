
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_FOUNDATION_MATH_FILTER_H
#define APPLESEED_FOUNDATION_MATH_FILTER_H

// appleseed.foundation headers.
#include "foundation/math/qmc.h"
#include "foundation/math/scalar.h"
#include "foundation/math/vector.h"
#include "foundation/platform/compiler.h"

// Standard headers.
#include <cmath>
#include <cstddef>

namespace foundation
{

//
// Base class for 2D reconstruction filters.
//
// The filters are not normalized (they don't integrate to 1 over their domain).
// The return value of evaluate() is undefined if (x, y) is outside the filter's domain.
//

template <typename T>
class Filter2
{
  public:
    typedef T ValueType;

    Filter2(const T xradius, const T yradius);

    virtual ~Filter2() {}

    T get_xradius() const;
    T get_yradius() const;

    virtual T evaluate(const T x, const T y) const = 0;

  protected:
    const T m_xradius;
    const T m_yradius;
    const T m_rcp_xradius;
    const T m_rcp_yradius;
};


//
// 2D box filter.
//

template <typename T>
class BoxFilter2
  : public Filter2<T>
{
  public:
    BoxFilter2(const T xradius, const T yradius);

    virtual T evaluate(const T x, const T y) const OVERRIDE;
};


//
// Triangle filter.
//

template <typename T>
class TriangleFilter2
  : public Filter2<T>
{
  public:
    TriangleFilter2(const T xradius, const T yradius);

    virtual T evaluate(const T x, const T y) const OVERRIDE;
};


//
// 2D Gaussian filter.
//

template <typename T>
class GaussianFilter2
  : public Filter2<T>
{
  public:
    GaussianFilter2(
        const T xradius,
        const T yradius,
        const T alpha);

    virtual T evaluate(const T x, const T y) const OVERRIDE;

  private:
    const T m_alpha;
    const T m_shift;

    static T gaussian(const T x, const T alpha);
};


//
// 2D Mitchell-Netravali filter.
//
// Reference:
//
//   http://www.cs.utexas.edu/~fussell/courses/cs384g/lectures/mitchell/Mitchell.pdf
//

template <typename T>
class MitchellFilter2
  : public Filter2<T>
{
  public:
    MitchellFilter2(
        const T xradius,
        const T yradius,
        const T b,
        const T c);

    virtual T evaluate(const T x, const T y) const OVERRIDE;

  private:
    T m_a3, m_a2, m_a0;
    T m_b3, m_b2, m_b1, m_b0;

    static T mitchell(const T x, const T b, const T c);
};


//
// 2D Lanczos filter.
//

template <typename T>
class LanczosFilter2
  : public Filter2<T>
{
  public:
    LanczosFilter2(
        const T xradius,
        const T yradius,
        const T tau);

    virtual T evaluate(const T x, const T y) const OVERRIDE;

  private:
    const T m_rcp_tau;

    static T lanczos(const T x, const T rcp_tau);
    static T sinc(const T x);
};


//
// Utilities.
//

// Compute the normalization factor for a given filter.
template <typename Filter>
typename Filter::ValueType compute_normalization_factor(
    const Filter&   filter,
    const size_t    sample_count = 1024);


//
// Filter2 class implementation.
//

template <typename T>
inline Filter2<T>::Filter2(const T xradius, const T yradius)
  : m_xradius(xradius)
  , m_yradius(yradius)
  , m_rcp_xradius(T(1.0) / xradius)
  , m_rcp_yradius(T(1.0) / yradius)
{
}

template <typename T>
inline T Filter2<T>::get_xradius() const
{
    return m_xradius;
}

template <typename T>
inline T Filter2<T>::get_yradius() const
{
    return m_yradius;
}


//
// BoxFilter2 class implementation.
//

template <typename T>
inline BoxFilter2<T>::BoxFilter2(const T xradius, const T yradius)
  : Filter2<T>(xradius, yradius)
{
}

template <typename T>
inline T BoxFilter2<T>::evaluate(const T x, const T y) const
{
    return T(1.0);
}


//
// TriangleFilter2 class implementation.
//

template <typename T>
inline TriangleFilter2<T>::TriangleFilter2(const T xradius, const T yradius)
  : Filter2<T>(xradius, yradius)
{
}

template <typename T>
inline T TriangleFilter2<T>::evaluate(const T x, const T y) const
{
    const T nx = x * Filter2<T>::m_rcp_xradius;
    const T ny = y * Filter2<T>::m_rcp_yradius;
    return (T(1.0) - std::abs(nx)) * (T(1.0) - std::abs(ny));
}


//
// GaussianFilter2 class implementation.
//

template <typename T>
inline GaussianFilter2<T>::GaussianFilter2(
    const T xradius,
    const T yradius,
    const T alpha)
  : Filter2<T>(xradius, yradius)
  , m_alpha(alpha)
  , m_shift(gaussian(T(1.0), alpha))
{
}

template <typename T>
inline T GaussianFilter2<T>::evaluate(const T x, const T y) const
{
    const T nx = x * Filter2<T>::m_rcp_xradius;
    const T ny = y * Filter2<T>::m_rcp_yradius;
    const T fx = gaussian(nx, m_alpha) - m_shift;
    const T fy = gaussian(ny, m_alpha) - m_shift;
    return fx * fy;
}

template <typename T>
FORCE_INLINE T GaussianFilter2<T>::gaussian(const T x, const T alpha)
{
    return std::exp(-alpha * x * x);
}


//
// MitchellFilter2 class implementation.
//

template <typename T>
inline MitchellFilter2<T>::MitchellFilter2(
    const T xradius,
    const T yradius,
    const T b,
    const T c)
  : Filter2<T>(xradius, yradius)
{
    m_a3 = T(1.0 / 6.0) * (T(12.0) - T(9.0) * b - T(6.0) * c);
    m_a2 = T(1.0 / 6.0) * (T(-18.0) + T(12.0) * b + T(6.0) * c);
    m_a0 = T(1.0 / 6.0) * (T(6.0) - T(2.0) * b);

    m_b3 = T(1.0 / 6.0) * (-b - T(6.0) * c);
    m_b2 = T(1.0 / 6.0) * (T(6.0) * b + T(30.0) * c);
    m_b1 = T(1.0 / 6.0) * (T(-12.0) * b - T(48.0) * c);
    m_b0 = T(1.0 / 6.0) * (T(8.0) * b + T(24.0) * c);
}

template <typename T>
inline T MitchellFilter2<T>::evaluate(const T x, const T y) const
{
    const T nx = x * Filter2<T>::m_rcp_xradius;
    const T x1 = std::abs(nx + nx);
    const T x2 = x1 * x1;
    const T x3 = x2 * x1;

    const T fx =
        x1 < T(1.0)
            ? m_a3 * x3 + m_a2 * x2 + m_a0
            : m_b3 * x3 + m_b2 * x2 + m_b1 * x1 + m_b0;

    const T ny = y * Filter2<T>::m_rcp_yradius;
    const T y1 = std::abs(ny + ny);
    const T y2 = y1 * y1;
    const T y3 = y2 * y1;

    const T fy =
        y1 < T(1.0)
            ? m_a3 * y3 + m_a2 * y2 + m_a0
            : m_b3 * y3 + m_b2 * y2 + m_b1 * y1 + m_b0;

    return fx * fy;
}

template <typename T>
FORCE_INLINE T MitchellFilter2<T>::mitchell(const T x, const T b, const T c)
{
    const T x1 = std::abs(x + x);
    const T x2 = x1 * x1;
    const T x3 = x2 * x1;

    if (x1 < T(1.0))
    {
        return T(1.0 / 6.0) * ((T(12.0) - T(9.0) * b - T(6.0) * c) * x3 +
                               (T(-18.0) + T(12.0) * b + T(6.0) * c) * x2 +
                               (T(6.0) - T(2.0) * b));
    }
    else
    {
        return T(1.0 / 6.0) * ((-b - T(6.0) * c) * x3 +
                               (T(6.0) * b + T(30.0) * c) * x2 +
                               (T(-12.0) * b - T(48.0) * c) * x1 +
                               (T(8.0) * b + T(24.0) * c));
    }
}


//
// LanczosFilter2 class implementation.
//

template <typename T>
inline LanczosFilter2<T>::LanczosFilter2(
    const T xradius,
    const T yradius,
    const T tau)
  : Filter2<T>(xradius, yradius)
  , m_rcp_tau(tau)
{
}

template <typename T>
inline T LanczosFilter2<T>::evaluate(const T x, const T y) const
{
    const T nx = x * Filter2<T>::m_rcp_xradius;
    const T ny = y * Filter2<T>::m_rcp_yradius;
    return lanczos(nx, m_rcp_tau) * lanczos(ny, m_rcp_tau);
}

template <typename T>
FORCE_INLINE T LanczosFilter2<T>::lanczos(const T x, const T rcp_tau)
{
    const T theta = T(Pi) * x;
    return theta == T(0.0) ? T(1.0) : sinc(theta * rcp_tau) * sinc(theta);
}

template <typename T>
FORCE_INLINE T LanczosFilter2<T>::sinc(const T x)
{
    return std::sin(x) / x;
}


//
// Utilities implementation.
//

template <typename Filter>
typename Filter::ValueType compute_normalization_factor(
    const Filter&   filter,
    const size_t    sample_count)
{
    typedef typename Filter::ValueType ValueType;

    const ValueType xradius = filter.get_xradius();
    const ValueType yradius = filter.get_yradius();

    ValueType result(0.0);

    for (size_t i = 0; i < sample_count; ++i)
    {
        static const size_t Bases[1] = { 2 };

        const Vector<ValueType, 2> s =
            hammersley_sequence<ValueType, 2>(Bases, i, sample_count);

        const Vector<ValueType, 2> p(
            xradius * (ValueType(2.0) * s.x - ValueType(1.0)),
            yradius * (ValueType(2.0) * s.y - ValueType(1.0)));

        result += filter.evaluate(p.x, p.y);
    }

    result *= ValueType(4.0) * xradius * yradius;
    result /= static_cast<ValueType>(sample_count);

    return result;
}

}       // namespace foundation

#endif  // !APPLESEED_FOUNDATION_MATH_FILTER_H
