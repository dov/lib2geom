#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "path-cairo.h"

#include "toy-framework.h"

using std::vector;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

class PreciseFlat: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
    multidim_sbasis<2> dB = derivative(B);
    multidim_sbasis<2> ddB = derivative(dB);
    cairo_md_sb(cr, B);
    
    // draw the longest chord that is no worse than tol from the curve.
    
    Geom::Point st = unit_vector(point_at(dB, 0));
    double s3 = fabs(dot(handles[2] - handles[0], rot90(st)));
    
    SBasis inflect = dot(dB, rot90(ddB));
    std::vector<double> rts = roots(inflect);
    double f = 3;
    for(int i = 0; i < rts.size(); i++) {
        draw_handle(cr, point_at(B, rts[i]));
        
        double tp = rts[i];
        Geom::Point st = unit_vector(point_at(dB, tp));
        Geom::Point O = point_at(B, tp);
        double s4 = fabs(dot(handles[3] - O, rot90(st)));
        double tf = pow(f/s4, 1./3);
        Geom::Point t1p = point_at(B, tp + tf*(1-tp));
        Geom::Point t1m = point_at(B, tp - tf*(1-tp));
        cairo_move_to(cr, t1m);
        cairo_line_to(cr, t1p);
        cairo_stroke(cr);
        //std::cout << tp << ", " << t1m << ", " << t1p << std::endl;
    }
    
    cairo_move_to(cr, point_at(B, 0));
    double t0 = 2*sqrt(f/(3*s3));
    //std::cout << t0 << std::endl;
    cairo_line_to(cr, point_at(B, t0));
    cairo_stroke(cr);
    Toy::draw(cr, notify, width, height, save);
}

public:
PreciseFlat () {
    for(int i = 0; i < 4; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}

};

int main(int argc, char **argv) {
    init(argc, argv, "precise-flat", new PreciseFlat());

    return 0;
}

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
