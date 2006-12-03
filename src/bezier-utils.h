#ifndef __SP_BEZIER_UTILS_H__
#define __SP_BEZIER_UTILS_H__

/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 *
 * Authors:
 *   Philip J. Schneider
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1990 Philip J. Schneider
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */

namespace Geom{

class Point;

/* Bezier approximation utils */
Point bezier_pt(unsigned degree, Point const V[], double t);

int bezier_fit_cubic(Point bezier[], Point const data[], int len, double error);

int bezier_fit_cubic_r(Point bezier[], Point const data[], int len, double error,
                           unsigned max_beziers);

int bezier_fit_cubic_full(Point bezier[], int split_points[], Point const data[], int len,
                              Point const &tHat1, Point const &tHat2,
                              double error, unsigned max_beziers);

Point darray_left_tangent(Point const d[], unsigned const len);
Point darray_left_tangent(Point const d[], unsigned const len, double const tolerance_sq);
Point darray_right_tangent(Point const d[], unsigned const length, double const tolerance_sq);


}
#endif /* __SP_BEZIER_UTILS_H__ */

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
