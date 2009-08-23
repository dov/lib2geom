/*
 * SVG Elliptical Arc Class
 *
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2008-2009 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <cfloat>
#include <limits>
#include <memory>

#include <2geom/elliptical-arc.h>
#include <2geom/ellipse.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-curve.h>
#include <2geom/poly.h>
#include <2geom/transforms.h>
#include <2geom/utils.h>

#include <2geom/numeric/vector.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>


namespace Geom
{

Rect EllipticalArc::boundsExact() const
{
    double extremes[4];
    double sinrot, cosrot;
    sincos(m_rot_angle, sinrot, cosrot);

    extremes[0] = std::atan2( -ray(Y) * sinrot, ray(X) * cosrot );
    extremes[1] = extremes[0] + M_PI;
    if ( extremes[0] < 0 ) extremes[0] += 2*M_PI;
    extremes[2] = std::atan2( ray(Y) * cosrot, ray(X) * sinrot );
    extremes[3] = extremes[2] + M_PI;
    if ( extremes[2] < 0 ) extremes[2] += 2*M_PI;


    double arc_extremes[4];
    arc_extremes[0] = initialPoint()[X];
    arc_extremes[1] = finalPoint()[X];
    if ( arc_extremes[0] < arc_extremes[1] )
        std::swap(arc_extremes[0], arc_extremes[1]);
    arc_extremes[2] = initialPoint()[Y];
    arc_extremes[3] = finalPoint()[Y];
    if ( arc_extremes[2] < arc_extremes[3] )
        std::swap(arc_extremes[2], arc_extremes[3]);


    if ( m_start_angle < m_end_angle )
    {
        if ( m_sweep )
        {
            for ( unsigned int i = 0; i < 4; ++i )
            {
                if ( m_start_angle < extremes[i] && extremes[i] < m_end_angle )
                {
                    arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
                }
            }
        }
        else
        {
            for ( unsigned int i = 0; i < 4; ++i )
            {
                if ( initialAngle() > extremes[i] || extremes[i] > finalAngle() )
                {
                    arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
                }
            }
        }
    }
    else
    {
        if ( m_sweep )
        {
            for ( unsigned int i = 0; i < 4; ++i )
            {
                if ( m_start_angle < extremes[i] || extremes[i] < m_end_angle )
                {
                    arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
                }
            }
        }
        else
        {
            for ( unsigned int i = 0; i < 4; ++i )
            {
                if ( m_start_angle > extremes[i] && extremes[i] > m_end_angle )
                {
                    arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
                }
            }
        }
    }

    return Rect( Point(arc_extremes[1], arc_extremes[3]) ,
                 Point(arc_extremes[0], arc_extremes[2]) );
}


Point EllipticalArc::pointAtAngle(Coord t) const
{
    Point ret = Point::polar(t) * unitCircleTransform();
    return ret;
}

Coord EllipticalArc::valueAtAngle(Coord t, Dim2 d) const
{
    Coord sinrot, cosrot, cost, sint;
    sincos(m_rot_angle, sinrot, cosrot);
    sincos(t, sint, cost);

    if ( d == X ) {
        return    ray(X) * cosrot * cost
                - ray(Y) * sinrot * sint
                + center(X);
    } else {
        return    ray(X) * sinrot * cost
                + ray(Y) * cosrot * sint
                + center(Y);
    }
}

Matrix EllipticalArc::unitCircleTransform() const
{
    Matrix ret = Rotate(m_rot_angle) * Scale(ray(X), ray(Y));
    ret.setTranslation(center());
    return ret;
}

std::vector<Coord> EllipticalArc::roots(Coord v, Dim2 d) const
{
    std::vector<Coord> sol;

    if ( are_near(ray(X), 0) && are_near(ray(Y), 0) ) {
        if ( center(d) == v )
            sol.push_back(0);
        return sol;
    }

    static const char* msg[2][2] =
    {
        { "d == X; ray(X) == 0; "
          "s = (v - center(X)) / ( -ray(Y) * std::sin(m_rot_angle) ); "
          "s should be contained in [-1,1]",
          "d == X; ray(Y) == 0; "
          "s = (v - center(X)) / ( ray(X) * std::cos(m_rot_angle) ); "
          "s should be contained in [-1,1]"
        },
        { "d == Y; ray(X) == 0; "
          "s = (v - center(X)) / ( ray(Y) * std::cos(m_rot_angle) ); "
          "s should be contained in [-1,1]",
          "d == Y; ray(Y) == 0; "
          "s = (v - center(X)) / ( ray(X) * std::sin(m_rot_angle) ); "
          "s should be contained in [-1,1]"
        },
    };

    for ( unsigned int dim = 0; dim < 2; ++dim )
    {
        if ( are_near(ray((Dim2) dim), 0) )
        {
            if ( initialPoint()[d] == v && finalPoint()[d] == v )
            {
                THROW_INFINITESOLUTIONS(0);
            }
            if ( (initialPoint()[d] < finalPoint()[d])
                 && (initialPoint()[d] > v || finalPoint()[d] < v) )
            {
                return sol;
            }
            if ( (initialPoint()[d] > finalPoint()[d])
                 && (finalPoint()[d] > v || initialPoint()[d] < v) )
            {
                return sol;
            }
            double ray_prj;
            switch(d)
            {
                case X:
                    switch(dim)
                    {
                        case X: ray_prj = -ray(Y) * std::sin(m_rot_angle);
                                break;
                        case Y: ray_prj = ray(X) * std::cos(m_rot_angle);
                                break;
                    }
                    break;
                case Y:
                    switch(dim)
                    {
                        case X: ray_prj = ray(Y) * std::cos(m_rot_angle);
                                break;
                        case Y: ray_prj = ray(X) * std::sin(m_rot_angle);
                                break;
                    }
                    break;
            }

            double s = (v - center(d)) / ray_prj;
            if ( s < -1 || s > 1 )
            {
                THROW_LOGICALERROR(msg[d][dim]);
            }
            switch(dim)
            {
                case X:
                    s = std::asin(s); // return a value in [-PI/2,PI/2]
                    if ( logical_xor( m_sweep, are_near(initialAngle(), M_PI/2) )  )
                    {
                        if ( s < 0 ) s += 2*M_PI;
                    }
                    else
                    {
                        s = M_PI - s;
                        if (!(s < 2*M_PI) ) s -= 2*M_PI;
                    }
                    break;
                case Y:
                    s = std::acos(s); // return a value in [0,PI]
                    if ( logical_xor( m_sweep, are_near(initialAngle(), 0) ) )
                    {
                        s = 2*M_PI - s;
                        if ( !(s < 2*M_PI) ) s -= 2*M_PI;
                    }
                    break;
            }

            //std::cerr << "s = " << rad_to_deg(s);
            s = map_to_01(s);
            //std::cerr << " -> t: " << s << std::endl;
            if ( !(s < 0 || s > 1) )
                sol.push_back(s);
            return sol;
        }
    }

    double rotx, roty;
    sincos(m_rot_angle, roty, rotx);
    if (d == X) roty = -roty;

    double rxrotx = ray(X) * rotx;
    double c_v = center(d) - v;

    double a = -rxrotx + c_v;
    double b = ray(Y) * roty;
    double c = rxrotx + c_v;
    //std::cerr << "a = " << a << std::endl;
    //std::cerr << "b = " << b << std::endl;
    //std::cerr << "c = " << c << std::endl;

    if ( are_near(a,0) )
    {
        sol.push_back(M_PI);
        if ( !are_near(b,0) )
        {
            double s = 2 * std::atan(-c/(2*b));
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
        }
    }
    else
    {
        double delta = b * b - a * c;
        //std::cerr << "delta = " << delta << std::endl;
        if ( are_near(delta, 0) )
        {
            double s = 2 * std::atan(-b/a);
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
        }
        else if ( delta > 0 )
        {
            double sq = std::sqrt(delta);
            double s = 2 * std::atan( (-b - sq) / a );
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
            s = 2 * std::atan( (-b + sq) / a );
            if ( s < 0 ) s += 2*M_PI;
            sol.push_back(s);
        }
    }

    std::vector<double> arc_sol;
    for (unsigned int i = 0; i < sol.size(); ++i )
    {
        //std::cerr << "s = " << rad_to_deg(sol[i]);
        sol[i] = map_to_01(sol[i]);
        //std::cerr << " -> t: " << sol[i] << std::endl;
        if ( !(sol[i] < 0 || sol[i] > 1) )
            arc_sol.push_back(sol[i]);
    }
    return arc_sol;
}


// D(E(t,C),t) = E(t+PI/2,O), where C is the ellipse center
// the derivative doesn't rotate the ellipse but there is a translation
// of the parameter t by an angle of PI/2 so the ellipse points are shifted
// of such an angle in the cw direction
Curve *EllipticalArc::derivative() const
{
    EllipticalArc *result = static_cast<EllipticalArc*>(duplicate());
    result->m_center[X] = result->m_center[Y] = 0;
    result->m_start_angle += M_PI/2;
    if( !( result->m_start_angle < 2*M_PI ) )
    {
        result->m_start_angle -= 2*M_PI;
    }
    result->m_end_angle += M_PI/2;
    if( !( result->m_end_angle < 2*M_PI ) )
    {
        result->m_end_angle -= 2*M_PI;
    }
    result->m_initial_point = result->pointAtAngle( result->initialAngle() );
    result->m_final_point = result->pointAtAngle( result->finalAngle() );
    return result;
}


std::vector<Point>
EllipticalArc::pointAndDerivatives(Coord t, unsigned int n) const
{
    unsigned int nn = n+1; // nn represents the size of the result vector.
    std::vector<Point> result;
    result.reserve(nn);
    double angle = map_unit_interval_on_circular_arc(t, initialAngle(),
                                                     finalAngle(), m_sweep);
    std::auto_ptr<EllipticalArc> ea( static_cast<EllipticalArc*>(duplicate()) );
    ea->m_center = Point(0,0);
    unsigned int m = std::min(nn, 4u);
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( ea->pointAtAngle(angle) );
        angle += M_PI/2;
        if ( !(angle < 2*M_PI) ) angle -= 2*M_PI;
    }
    m = nn / 4;
    for ( unsigned int i = 1; i < m; ++i )
    {
        for ( unsigned int j = 0; j < 4; ++j )
            result.push_back( result[j] );
    }
    m = nn - 4 * m;
    for ( unsigned int i = 0; i < m; ++i )
    {
        result.push_back( result[i] );
    }
    if ( !result.empty() ) // nn != 0
        result[0] = pointAtAngle(angle);
    return result;
}

bool EllipticalArc::containsAngle(Coord angle) const
{
    if ( m_sweep )
        if ( m_start_angle < m_end_angle )
            return ( !( angle < m_start_angle || angle > m_end_angle ) );
        else
            return ( !( angle < m_start_angle && angle > m_end_angle ) );
    else
        if ( m_start_angle > m_end_angle )
            return ( !( angle > m_start_angle || angle < m_end_angle ) );
        else
            return ( !( angle > m_start_angle && angle < m_end_angle ) );
}

Curve* EllipticalArc::portion(double f, double t) const
{
    // fix input arguments
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    if ( are_near(f, t) )
    {
        EllipticalArc *arc = static_cast<EllipticalArc*>(duplicate());
        arc->m_center = arc->m_initial_point = arc->m_final_point = pointAt(f);
        arc->m_start_angle = arc->m_end_angle = m_start_angle;
        arc->m_rot_angle = m_rot_angle;
        arc->m_sweep = m_sweep;
        arc->m_large_arc = m_large_arc;
        return arc;
    }

    EllipticalArc *arc = static_cast<EllipticalArc*>(duplicate());
    arc->m_initial_point = pointAt(f);
    arc->m_final_point = pointAt(t);
    double sa = m_sweep ? sweepAngle() : -sweepAngle();
    arc->m_start_angle = m_start_angle + sa * f;
    if ( !(arc->m_start_angle < 2*M_PI) )
        arc->m_start_angle -= 2*M_PI;
    if ( arc->m_start_angle < 0 )
        arc->m_start_angle += 2*M_PI;
    arc->m_end_angle = m_start_angle + sa * t;
    if ( !(arc->m_end_angle < 2*M_PI) )
        arc->m_end_angle -= 2*M_PI;
    if ( arc->m_end_angle < 0 )
        arc->m_end_angle += 2*M_PI;
    if ( f > t ) arc->m_sweep = !m_sweep;
    if ( m_large_arc && (arc->sweepAngle() < M_PI) )
        arc->m_large_arc = false;
    return arc;
}

// the arc is the same but traversed in the opposite direction
Curve *EllipticalArc::reverse() const {
    EllipticalArc *rarc = static_cast<EllipticalArc*>(duplicate());
    rarc->m_sweep = !m_sweep;
    rarc->m_initial_point = m_final_point;
    rarc->m_final_point = m_initial_point;
    rarc->m_start_angle = m_end_angle;
    rarc->m_end_angle = m_start_angle;
    return rarc;
}

std::vector<double> EllipticalArc::allNearestPoints( Point const& p, double from, double to ) const
{
    std::vector<double> result;

    if ( from > to ) std::swap(from, to);
    if ( from < 0 || to > 1 )
    {
        THROW_RANGEERROR("[from,to] interval out of range");
    }

    if ( ( are_near(ray(X), 0) && are_near(ray(Y), 0) )  || are_near(from, to) )
    {
        result.push_back(from);
        return result;
    }
    else if ( are_near(ray(X), 0) || are_near(ray(Y), 0) )
    {
        LineSegment seg(pointAt(from), pointAt(to));
        Point np = seg.pointAt( seg.nearestPoint(p) );
        if ( are_near(ray(Y), 0) )
        {
            if ( are_near(m_rot_angle, M_PI/2)
                 || are_near(m_rot_angle, 3*M_PI/2) )
            {
                result = roots(np[Y], Y);
            }
            else
            {
                result = roots(np[X], X);
            }
        }
        else
        {
            if ( are_near(m_rot_angle, M_PI/2)
                 || are_near(m_rot_angle, 3*M_PI/2) )
            {
                result = roots(np[X], X);
            }
            else
            {
                result = roots(np[Y], Y);
            }
        }
        return result;
    }
    else if ( are_near(ray(X), ray(Y)) )
    {
        Point r = p - center();
        if ( are_near(r, Point(0,0)) )
        {
            THROW_INFINITESOLUTIONS(0);
        }
        // TODO: implement case r != 0
//      Point np = ray(X) * unit_vector(r);
//      std::vector<double> solX = roots(np[X],X);
//      std::vector<double> solY = roots(np[Y],Y);
//      double t;
//      if ( are_near(solX[0], solY[0]) || are_near(solX[0], solY[1]))
//      {
//          t = solX[0];
//      }
//      else
//      {
//          t = solX[1];
//      }
//      if ( !(t < from || t > to) )
//      {
//          result.push_back(t);
//      }
//      else
//      {
//
//      }
    }

    // solve the equation <D(E(t),t)|E(t)-p> == 0
    // that provides min and max distance points
    // on the ellipse E wrt the point p
    // after the substitutions:
    // cos(t) = (1 - s^2) / (1 + s^2)
    // sin(t) = 2t / (1 + s^2)
    // where s = tan(t/2)
    // we get a 4th degree equation in s
    /*
     *  ry s^4 ((-cy + py) Cos[Phi] + (cx - px) Sin[Phi]) +
     *  ry ((cy - py) Cos[Phi] + (-cx + px) Sin[Phi]) +
     *  2 s^3 (rx^2 - ry^2 + (-cx + px) rx Cos[Phi] + (-cy + py) rx Sin[Phi]) +
     *  2 s (-rx^2 + ry^2 + (-cx + px) rx Cos[Phi] + (-cy + py) rx Sin[Phi])
     */

    Point p_c = p - center();
    double rx2_ry2 = (ray(X) - ray(Y)) * (ray(X) + ray(Y));
    double sinrot, cosrot;
    sincos(m_rot_angle, sinrot, cosrot);
    double expr1 = ray(X) * (p_c[X] * cosrot + p_c[Y] * sinrot);
    Poly coeff;
    coeff.resize(5);
    coeff[4] = ray(Y) * ( p_c[Y] * cosrot - p_c[X] * sinrot );
    coeff[3] = 2 * ( rx2_ry2 + expr1 );
    coeff[2] = 0;
    coeff[1] = 2 * ( -rx2_ry2 + expr1 );
    coeff[0] = -coeff[4];

//  for ( unsigned int i = 0; i < 5; ++i )
//      std::cerr << "c[" << i << "] = " << coeff[i] << std::endl;

    std::vector<double> real_sol;
    // gsl_poly_complex_solve raises an error
    // if the leading coefficient is zero
    if ( are_near(coeff[4], 0) )
    {
        real_sol.push_back(0);
        if ( !are_near(coeff[3], 0) )
        {
            double sq = -coeff[1] / coeff[3];
            if ( sq > 0 )
            {
                double s = std::sqrt(sq);
                real_sol.push_back(s);
                real_sol.push_back(-s);
            }
        }
    }
    else
    {
        real_sol = solve_reals(coeff);
    }

    for ( unsigned int i = 0; i < real_sol.size(); ++i )
    {
        real_sol[i] = 2 * std::atan(real_sol[i]);
        if ( real_sol[i] < 0 ) real_sol[i] += 2*M_PI;
    }
    // when s -> Infinity then <D(E)|E-p> -> 0 iff coeff[4] == 0
    // so we add M_PI to the solutions being lim arctan(s) = PI when s->Infinity
    if ( (real_sol.size() % 2) != 0 )
    {
        real_sol.push_back(M_PI);
    }

    double mindistsq1 = std::numeric_limits<double>::max();
    double mindistsq2 = std::numeric_limits<double>::max();
    double dsq;
    unsigned int mi1, mi2;
    for ( unsigned int i = 0; i < real_sol.size(); ++i )
    {
        dsq = distanceSq(p, pointAtAngle(real_sol[i]));
        if ( mindistsq1 > dsq )
        {
            mindistsq2 = mindistsq1;
            mi2 = mi1;
            mindistsq1 = dsq;
            mi1 = i;
        }
        else if ( mindistsq2 > dsq )
        {
            mindistsq2 = dsq;
            mi2 = i;
        }
    }

    double t = map_to_01( real_sol[mi1] );
    if ( !(t < from || t > to) )
    {
        result.push_back(t);
    }

    bool second_sol = false;
    t = map_to_01( real_sol[mi2] );
    if ( real_sol.size() == 4 && !(t < from || t > to) )
    {
        if ( result.empty() || are_near(mindistsq1, mindistsq2) )
        {
            result.push_back(t);
            second_sol = true;
        }
    }

    // we need to test extreme points too
    double dsq1 = distanceSq(p, pointAt(from));
    double dsq2 = distanceSq(p, pointAt(to));
    if ( second_sol )
    {
        if ( mindistsq2 > dsq1 )
        {
            result.clear();
            result.push_back(from);
            mindistsq2 = dsq1;
        }
        else if ( are_near(mindistsq2, dsq) )
        {
            result.push_back(from);
        }
        if ( mindistsq2 > dsq2 )
        {
            result.clear();
            result.push_back(to);
        }
        else if ( are_near(mindistsq2, dsq2) )
        {
            result.push_back(to);
        }

    }
    else
    {
        if ( result.empty() )
        {
            if ( are_near(dsq1, dsq2) )
            {
                result.push_back(from);
                result.push_back(to);
            }
            else if ( dsq2 > dsq1 )
            {
                result.push_back(from);
            }
            else
            {
                result.push_back(to);
            }
        }
    }

    return result;
}


/*
 * NOTE: this implementation follows Standard SVG 1.1 implementation guidelines
 * for elliptical arc curves. See Appendix F.6.
 */
void EllipticalArc::_updateCenterAndAngles()
{
    Point d = initialPoint() - finalPoint();

    if (isSVGCompliant())
    {
        if ( initialPoint() == finalPoint() )
        {
            m_rx = m_ry = m_rot_angle = m_start_angle = m_end_angle = 0;
            m_center = initialPoint();
            m_large_arc = m_sweep = false;
            return;
        }

        m_rx = std::fabs(m_rx);
        m_ry = std::fabs(m_ry);

        if ( are_near(ray(X), 0) || are_near(ray(Y), 0) )
        {
            m_rx = L2(d) / 2;
            m_ry = 0;
            m_rot_angle = std::atan2(d[Y], d[X]);
            if (m_rot_angle < 0) m_rot_angle += 2*M_PI;
            m_start_angle = 0;
            m_end_angle = M_PI;
            m_center = middle_point(initialPoint(), finalPoint());
            m_large_arc = false;
            m_sweep = false;
            return;
        }
    }
    else
    {
        if ( are_near(initialPoint(), finalPoint()) )
        {
            if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
            {
                m_start_angle = m_end_angle = 0;
                m_center = initialPoint();
                return;
            }
            else
            {
                THROW_RANGEERROR("initial and final point are the same");
            }
        }
        if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
        { // but initialPoint != finalPoint
            THROW_RANGEERROR(
                "there is no ellipse that satisfies the given constraints: "
                "ray(X) == 0 && ray(Y) == 0 but initialPoint != finalPoint"
            );
        }
        if ( are_near(ray(Y), 0) )
        {
            Point v = initialPoint() - finalPoint();
            if ( are_near(L2sq(v), 4*ray(X)*ray(X)) )
            {
                double angle = std::atan2(v[Y], v[X]);
                if (angle < 0) angle += 2*M_PI;
                if ( are_near( angle, m_rot_angle ) )
                {
                    m_start_angle = 0;
                    m_end_angle = M_PI;
                    m_center = v/2 + finalPoint();
                    return;
                }
                angle -= M_PI;
                if ( angle < 0 ) angle += 2*M_PI;
                if ( are_near( angle, m_rot_angle ) )
                {
                    m_start_angle = M_PI;
                    m_end_angle = 0;
                    m_center = v/2 + finalPoint();
                    return;
                }
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(Y) == 0 "
                    "and slope(initialPoint - finalPoint) != rotation_angle "
                    "and != rotation_angle + PI"
                );
            }
            if ( L2sq(v) > 4*ray(X)*ray(X) )
            {
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(Y) == 0 and distance(initialPoint, finalPoint) > 2*ray(X)"
                );
            }
            else
            {
                THROW_RANGEERROR(
                    "there is infinite ellipses that satisfy the given constraints: "
                    "ray(Y) == 0  and distance(initialPoint, finalPoint) < 2*ray(X)"
                );
            }

        }

        if ( are_near(ray(X), 0) )
        {
            Point v = initialPoint() - finalPoint();
            if ( are_near(L2sq(v), 4*ray(Y)*ray(Y)) )
            {
                double angle = std::atan2(v[Y], v[X]);
                if (angle < 0) angle += 2*M_PI;
                double rot_angle = m_rot_angle + M_PI/2;
                if ( !(rot_angle < 2*M_PI) ) rot_angle -= 2*M_PI;
                if ( are_near( angle, rot_angle ) )
                {
                    m_start_angle = M_PI/2;
                    m_end_angle = 3*M_PI/2;
                    m_center = v/2 + finalPoint();
                    return;
                }
                angle -= M_PI;
                if ( angle < 0 ) angle += 2*M_PI;
                if ( are_near( angle, rot_angle ) )
                {
                    m_start_angle = 3*M_PI/2;
                    m_end_angle = M_PI/2;
                    m_center = v/2 + finalPoint();
                    return;
                }
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(X) == 0 "
                    "and slope(initialPoint - finalPoint) != rotation_angle + PI/2 "
                    "and != rotation_angle + (3/2)*PI"
                );
            }
            if ( L2sq(v) > 4*ray(Y)*ray(Y) )
            {
                THROW_RANGEERROR(
                    "there is no ellipse that satisfies the given constraints: "
                    "ray(X) == 0 and distance(initialPoint, finalPoint) > 2*ray(Y)"
                );
            }
            else
            {
                THROW_RANGEERROR(
                    "there is infinite ellipses that satisfy the given constraints: "
                    "ray(X) == 0  and distance(initialPoint, finalPoint) < 2*ray(Y)"
                );
            }

        }

    }

    Rotate m(m_rot_angle);

    Point p = (d / 2) * m;
    double rx2 = m_rx * m_rx;
    double ry2 = m_ry * m_ry;
    double rxpy = m_rx * p[Y];
    double rypx = m_ry * p[X];
    double rx2py2 = rxpy * rxpy;
    double ry2px2 = rypx * rypx;
    double num = rx2 * ry2;
    double den = rx2py2 + ry2px2;
    assert(den != 0);
    double rad = num / den;
    Point c(0,0);
    if (rad > 1)
    {
        rad -= 1;
        rad = std::sqrt(rad);

        if (m_large_arc == m_sweep) rad = -rad;
        c = rad * Point(rxpy / m_ry, -rypx / m_rx);

        Matrix mr(m);
        mr[1] = -mr[1];
        mr[2] = -mr[2];

        m_center = c * mr + middle_point(initialPoint(), finalPoint());
    }
    else if (rad == 1 || isSVGCompliant())
    {
        double lamda = std::sqrt(1 / rad);
        m_rx *= lamda;
        m_ry *= lamda;
        m_center = middle_point(initialPoint(), finalPoint());
    }
    else
    {
        THROW_RANGEERROR(
            "there is no ellipse that satisfies the given constraints"
        );
    }

    Point sp((p[X] - c[X]) / m_rx, (p[Y] - c[Y]) / m_ry);
    Point ep((-p[X] - c[X]) / m_rx, (-p[Y] - c[Y]) / m_ry);
    Point v(1, 0);
    m_start_angle = angle_between(v, sp);
    double sweep_angle = angle_between(sp, ep);
    if (!m_sweep && sweep_angle > 0) sweep_angle -= 2*M_PI;
    if (m_sweep && sweep_angle < 0) sweep_angle += 2*M_PI;

    if (m_start_angle < 0) m_start_angle += 2*M_PI;
    m_end_angle = m_start_angle + sweep_angle;
    if (m_end_angle < 0) m_end_angle += 2*M_PI;
    if (m_end_angle >= 2*M_PI) m_end_angle -= 2*M_PI;
}

D2<SBasis> EllipticalArc::toSBasis() const
{
    D2<SBasis> arc;
    // the interval of parametrization has to be [0,1]
    Coord et = initialAngle() + ( m_sweep ? sweepAngle() : -sweepAngle() );
    Linear param(initialAngle(), et);
    Coord cos_rot_angle, sin_rot_angle;
    sincos(m_rot_angle, sin_rot_angle, cos_rot_angle);

    // order = 4 seems to be enough to get a perfect looking elliptical arc
    SBasis arc_x = ray(X) * cos(param,4);
    SBasis arc_y = ray(Y) * sin(param,4);
    arc[0] = arc_x * cos_rot_angle - arc_y * sin_rot_angle + Linear(center(X),center(X));
    arc[1] = arc_x * sin_rot_angle + arc_y * cos_rot_angle + Linear(center(Y),center(Y));

    // ensure that endpoints remain exact
    for ( int d = 0 ; d < 2 ; d++ ) {
        arc[d][0][0] = initialPoint()[d];
        arc[d][0][1] = finalPoint()[d];
    }

    return arc;
}


Curve *EllipticalArc::transformed(Matrix const& m) const
{
    Ellipse e(center(X), center(Y), ray(X), ray(Y), m_rot_angle);
    Ellipse et = e.transformed(m);
    Point inner_point = pointAt(0.5);
    return et.arc( initialPoint() * m,
                                  inner_point * m,
                                  finalPoint() * m,
                                  isSVGCompliant() );
}

/*
 * helper routine to convert the parameter t value
 * btw [0,1] and [0,2PI] domain and back
 *
 */
Coord EllipticalArc::map_to_02PI(Coord t) const
{
    Coord angle = initialAngle();
    if ( m_sweep )
    {
        angle += sweepAngle() * t;
    }
    else
    {
        angle -= sweepAngle() * t;
    }
    angle = std::fmod(angle, 2*M_PI);
    if ( angle < 0 ) angle += 2*M_PI;
    return angle;
}

Coord EllipticalArc::map_to_01(Coord angle) const
{
    return map_circular_arc_on_unit_interval(angle, initialAngle(),
                                             finalAngle(), m_sweep);
}

} // end namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

