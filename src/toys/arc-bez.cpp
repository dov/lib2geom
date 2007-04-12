#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "sb-geometric.h"

#include "path-cairo.h"

#include <iterator>
#include "translate.h"

#include "toy-framework.h"

#include "d2.h"

using std::vector;
using namespace Geom;

class ArcBez: public Toy {
public:
    ArcBez() {
        for(int i = 0; i < 4; i++)
            handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> B = bezier_to_sbasis<3>(handles.begin());
        cairo_md_sb(cr, B);
        cairo_stroke(cr);
        
        cairo_set_source_rgba (cr, 0.5, 0.5, 0, 0.8);
        D2<SBasis> dB = derivative(B);
        cairo_set_source_rgba (cr, 0.25, 0.5, 0, 0.8);
        {
            D2<SBasis> plot;
            plot[0] = SBasis(width*Linear(0.25,0.75));

            D2<SBasis> dB = derivative(B);
            D2<SBasis> ddB = derivative(dB);
            SBasis n = dB[0]*ddB[1] -dB[1]*ddB[0];
            SBasis den = dot(dB, dB);

            plot[1] = derivative(divide(n, sqrt(den*den*den,5), 10));
            std::vector<double> r = roots(plot[1]);
            plot[1] = Linear(width*3/4) - (1./1000)*plot[1];
            cairo_md_sb(cr, plot);
            cairo_stroke(cr);
            for(int i = 0; i < r.size(); i++) {
                draw_cross(cr, B(r[i]));
                draw_cross(cr, plot(r[i]));
            }
            *notify << plot[1].tailError(0) << std::endl;
        }

        {
            D2<SBasis> plot;

            Piecewise<SBasis> als = arc_length_sb(B);
            double t0 = 0, t1;
            for(int i = 0; i < als.segs.size();i++){
                t1 = als.cuts[i+1];
                plot[0] = SBasis(width*Linear(t0,t1));
                plot[1] = Linear(height-5) - als.segs[i];
                cairo_md_sb(cr,plot);
                cairo_set_source_rgba (cr, 1, 0, 0.6, 0.5);
                cairo_stroke(cr);

                t0 = t1;
            }
            
            D2<Piecewise<SBasis> > grf;
            grf.f[1] = als; // Piecewise<SBasis>(SBasis(Linear(height-5))) - 
            grf.f[0] = Piecewise<SBasis>(SBasis(Linear(0, width)));
            
            cairo_md_pw(cr, grf);
        }

        cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
        double prev_seg = 0;
        int N = 10;
        for(int subdivi = 0; subdivi < N; subdivi++) {
            double dsubu = 1./N;
            double subu = dsubu*subdivi;
            Linear dt(subu, dsubu + subu);
            D2<SBasis> dBp = compose(dB, dt);
            SBasis arc = L2(dBp, 2);
            arc = (1./N)*integral(arc);
            arc = arc - Linear(Hat(arc(0) - prev_seg));
            prev_seg = arc(1);
        
            D2<SBasis> plot;
            plot[0] = SBasis(width*dt);
            plot[1] = Linear(height) - arc;
        
            cairo_md_sb(cr, plot);
            cairo_stroke(cr);
        }
        *notify << "arc length = " << prev_seg << std::endl;
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "arc-bez", new ArcBez());

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
