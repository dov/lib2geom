#include "solver.h"
#include "point-fns.h"
#include <algorithm>

/*** Find the zeros of the parametric function in 2d defined by two beziers X(t), Y(t).  The
 * original code simply kept subdividing until it was happy with the linearity of the bezier.  This requires an n^2 subdivision for each step, even when there is only one solution.
 * 
 * Perhaps it would be better to subdivide particularly around nodes with changing sign, rather than simply cutting in half.
 */

#define SGN(a)      (((a)<0) ? -1 : 0)

/*
 *  Forward declarations
 */
static Geom::Point 
Bezier(Geom::Point *V,
       unsigned degree,
       double t,
       Geom::Point *Left,
       Geom::Point *Right);

unsigned
crossing_count(Geom::Point *V, unsigned degree);
static unsigned 
control_poly_flat_enough(Geom::Point *V, unsigned degree);
static double
compute_x_intercept(Geom::Point *V, unsigned degree);

int	MAXDEPTH = 64;	/*  Maximum depth for recursion */

#define	EPSILON	(ldexp(1.0,-MAXDEPTH-1)) /*Flatness control value */

bool use_secant = true;

/*
 *  find_bezier_roots : Given an equation in Bernstein-Bezier form, find all 
 *    of the roots in the interval [0, 1].  Return the number of roots found.
 */
void find_bezier_roots(Geom::Point *w, /* The control points  */
                       unsigned degree,	/* The degree of the polynomial */
                       std::vector<double> &solutions, /* RETURN candidate t-values */
                       unsigned depth)	/* The depth of the recursion */
{  
    const unsigned max_crossings = crossing_count(w, degree);
    switch (max_crossings) {
    case 0: 	/* No solutions here	*/
        return;
	
    case 1:
 	/* Unique solution	*/
        /* Stop recursion when the tree is deep enough	*/
        /* if deep enough, return 1 solution at midpoint  */
        if (depth >= MAXDEPTH) {
            solutions.push_back((w[0][Geom::X] + w[degree][Geom::X]) / 2.0);
            return;
        }
        
        // I would rather use something like secant method here, rather than needlessly
        // subdividing. -- njh

        // secant search - this should be faster than the subdivision search
        if(use_secant) {
            double ys[degree+1];
            double right_t = 1, left_t = 0;
            double right_y = w[degree][Geom::Y];
            double left_y = w[degree][Geom::Y];
            while(right_t - left_t > EPSILON) {
                double dt = (left_y / (right_y - left_y));
                // one danger is slow convergence due to salami tactics
                const double linear_convergence = 1./8;
                if(dt < linear_convergence)
                    dt = linear_convergence;
                if(dt > 1- linear_convergence)
                    dt = 1- linear_convergence;
                
                const double t = (1-dt)*left_t + dt*right_t;
                for(unsigned i = 0; i <= degree; i++) {
                    ys[i] = w[i][Geom::Y];
                }
                // triangle - note the order of updates
                for(unsigned d = 0; d < degree; d++)
                    for(unsigned i = d; i <= degree; i++)
                        ys[i] = (1-t)*ys[i] + t*ys[i+1];
                const double y = ys[degree]; // value at t
                if(SGN(y) == left_y) {
                    left_y = y;
                    left_t = t;
                }
                if(SGN(y) == right_y) {
                    right_y = y;
                    right_t = t;
                }
                assert(SGN(left_y) != SGN(right_y));
            }
            double xs[degree+1];
            double t = (left_t + right_t)/2; // perhaps the mid_point is better?
            for(unsigned i = 0; i <= degree; i++) {
                xs[i] = w[i][Geom::X];
            }
            // triangle - note the order of updates
            for(unsigned d = 0; d < degree; d++)
                for(unsigned i = d; i <= degree; i++)
                    xs[i] = (1-t)*xs[i] + t*xs[i+1];
            solutions.push_back(xs[degree]);
            return;
            // is not secant, use subdivision
        } else if (control_poly_flat_enough(w, degree)) {
            solutions.push_back(compute_x_intercept(w, degree));
            return;
        }
        break;
    }

    /* Otherwise, solve recursively after subdividing control polygon  */
    Geom::Point Left[degree+1],	/* New left and right  */
        Right[degree+1];	/* control polygons  */
    Bezier(w, degree, 0.5, Left, Right);
    
    find_bezier_roots(Left,  degree, solutions, depth+1);
    find_bezier_roots(Right, degree, solutions, depth+1);
}


/*
 * crossing_count:
 *  Count the number of times a Bezier control polygon 
 *  crosses the 0-axis. This number is >= the number of roots.
 *
 */
unsigned
crossing_count(Geom::Point *V,	/*  Control pts of Bezier curve	*/
	       unsigned degree)	/*  Degree of Bezier curve 	*/
{
    unsigned 	n_crossings = 0;	/*  Number of zero-crossings */
    
    int old_sign = SGN(V[0][Geom::Y]);
    for (int i = 1; i <= degree; i++) {
        int sign = SGN(V[i][Geom::Y]);
        if (sign != old_sign)
            n_crossings++;
        old_sign = sign;
    }
    return n_crossings;
}



/*
 *  control_poly_flat_enough :
 *	Check if the control polygon of a Bezier curve is flat enough
 *	for recursive subdivision to bottom out.
 *
 */
static unsigned 
control_poly_flat_enough(Geom::Point *V, /* Control points	*/
			 unsigned degree)	/* Degree of polynomial	*/
{
    /* Find the perpendicular distance from each interior control point to line connecting V[0] and
     * V[degree] */
    double distance[degree]; /* Distances from pts to line */

    /* Derive the implicit equation for line connecting first */
    /*  and last control points */
    const double a = V[0][Geom::Y] - V[degree][Geom::Y];
    const double b = V[degree][Geom::X] - V[0][Geom::X];
    const double c = V[0][Geom::X] * V[degree][Geom::Y] - V[degree][Geom::X] * V[0][Geom::Y];

    const double abSquared = (a * a) + (b * b);

    for (unsigned i = 1; i < degree; i++) {
        /* Compute distance from each of the points to that line */
        double & dist(distance[i-1]);
        const double d = a * V[i][Geom::X] + b * V[i][Geom::Y] + c;
        dist = d*d / abSquared;
        if (d < 0.0)
            dist = -dist;
    }


    // Find the largest distance
    double max_distance_above = 0.0;
    double max_distance_below = 0.0;
    for (unsigned i = 0; i < degree-1; i++) {
        const double d = distance[i];
        if (d < 0.0)
            max_distance_below = MIN(max_distance_below, d);
        if (d > 0.0)
            max_distance_above = MAX(max_distance_above, d);
    }

    const double intercept_1 = (c + max_distance_above) / -a;
    const double intercept_2 = (c + max_distance_below) / -a;

    /* Compute bounding interval*/
    const double left_intercept = std::min(intercept_1, intercept_2);
    const double right_intercept = std::max(intercept_1, intercept_2);

    const double error = 0.5 * (right_intercept - left_intercept);
    
    if (error < EPSILON)
        return 1;
    
    return 0;
}



/*
 *  compute_x_intercept :
 *	Compute intersection of chord from first control point to last
 *  	with 0-axis.
 * 
 */
static double
compute_x_intercept(Geom::Point *V, /*  Control points	*/
		    unsigned degree) /*  Degree of curve	*/
{
    const Geom::Point A = V[degree] - V[0];

    return (A[Geom::X]*V[0][Geom::Y] - A[Geom::Y]*V[0][Geom::X]) / -A[Geom::Y];
}


/*
 *  Bezier : 
 *	Evaluate a Bezier curve at a particular parameter value
 *      Fill in control points for resulting sub-curves.
 * 
 */
static Geom::Point 
Bezier(Geom::Point *V, /* Control pts	*/
       unsigned degree,	/* Degree of bezier curve */
       double t,	/* Parameter value */
       Geom::Point *Left,	/* RETURN left half ctl pts */
       Geom::Point *Right)	/* RETURN right half ctl pts */
{
    Geom::Point Vtemp[degree+1][degree+1];

    /* Copy control points	*/
    std::copy(V, V+degree+1, Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i <= degree; i++) {	
        for (unsigned j = 0; j <= degree - i; j++) {
            Vtemp[i][j] = Lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }
    
    for (unsigned j = 0; j <= degree; j++)
        Left[j]  = Vtemp[j][0];
    for (unsigned j = 0; j <= degree; j++)
        Right[j] = Vtemp[degree-j][j];

    return (Vtemp[degree][0]);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

