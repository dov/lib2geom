#include "d2.h"
#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework.h"

using std::vector;
using namespace Geom;

class PathAlongPathToy: public Toy {
    bool should_draw_numbers(){return false;}

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> skeleton = handles_to_sbasis<3>(handles.begin());
        D2<SBasis> pattern  = handles_to_sbasis<3>(handles.begin()+4);

        handles[8][0]=150;
        Geom::Point O = *(handles.begin()+8);
        
        Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(Piecewise<D2<SBasis> >(skeleton));
        //TODO: add linear/cutoff extension...
        Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));

        Piecewise<SBasis> x=Piecewise<SBasis>(pattern[0]-O[0]);
        Piecewise<SBasis> y=Piecewise<SBasis>(pattern[1]-O[1]);

        Interval pattBnds = bounds_exact(x);
        int nbCopies = int(uskeleton.cuts.back()/pattBnds.extent());
        double pattWidth = uskeleton.cuts.back()/nbCopies;
        x-=pattBnds.min();
        x*=pattWidth/pattBnds.extent();
        Piecewise<D2<SBasis> >output;
        for (int i=0; i<nbCopies; i++){
            output.concat(compose(uskeleton,x)+y*compose(n,x));
            x+=pattWidth;
        }
        //Piecewise<D2<SBasis> >output = compose(uskeleton,x)+y*compose(n,x);
        
        cairo_set_line_width(cr,1.);

        cairo_pw_d2(cr, Piecewise<D2<SBasis> >(skeleton));
        cairo_set_source_rgba(cr,0.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        cairo_pw_d2(cr, Piecewise<D2<SBasis> >(pattern));
        cairo_set_source_rgba(cr,1.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        cairo_pw_d2(cr, output);
        cairo_set_source_rgba(cr,1.0,0.0,1.0,1.0);
        cairo_stroke(cr);


        Toy::draw(cr, notify, width, height, save);
    }        

public:
    PathAlongPathToy(){
        if(handles.empty()) {
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(200+50*i,400));
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(100+uniform()*400,
                                              150+uniform()*100));
            handles[4] = Geom::Point(280,150);
            handles[5] = Geom::Point(290,170);
            handles[6] = Geom::Point(300,130);
            handles[7] = Geom::Point(310,150);

            handles.push_back(Geom::Point(150,150));
        }
    }
};


int main(int argc, char **argv) {
    init(argc, argv, "normal-bundle", new PathAlongPathToy);
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

 	  	 
