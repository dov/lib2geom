#include "d2.h"
#include "sbasis.h"
#include "sbasis-geometric.h"
#include "svg-path-parser.h"
#include "sbasis-math.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

//Random walkers toy, written by mgsloan for a school video proj.

using namespace Geom;

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double min, double max, double space=10){
    for( double t = min; t < max; t += space) {
        Point pos = M(t), perp = M.valueAndDerivatives(t, 2)[1].cw() * 3;
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_stroke(cr);
}

D2<SBasis> random_d2() {
    D2<SBasis> ret;
    ret[0].push_back(Linear(uniform()*720, uniform()*720));
    ret[1].push_back(Linear(uniform()*480, uniform()*480));
    
    for(int i = 0; i < 5; i++) {
        ret[0].push_back(Linear(uniform()*2000 - 1000, uniform()*2000 - 1000));
        ret[1].push_back(Linear(uniform()*2000 - 1000, uniform()*2000 - 1000));
    }
    return ret;
}

class Walker {
    Piecewise<D2<SBasis> > path;
    int spawn_time, last_time;
    double red, green, blue, length;
  public:
    void add_section(const D2<SBasis> x) {
        Piecewise<D2<SBasis> > new_path(path);
        D2<SBasis> seg(x);
        seg[0][0][0] = path.segs.back()[0][0][1];
        seg[1][0][0] = path.segs.back()[1][0][1];
        new_path.push(seg, path.domain().max()+1);
        path = arc_length_parametrization(new_path);
    }
    Walker (int t, double r, double g, double b, double l) : spawn_time(t), last_time(t), red(r), green(g), blue(b), length(l) {
        path = Piecewise<D2<SBasis> >(random_d2());
        add_section(random_d2());
    }
    void draw(cairo_t *cr, int t) {
        if(t - last_time > path.domain().max()) add_section(random_d2());
        if(t - last_time - length > path.cuts[1]) {
            Piecewise<D2<SBasis> > new_path;
            new_path.push_cut(0);
            for(unsigned i = 1; i < path.size(); i++) {
                new_path.push(path[i], path.cuts[i+1] - path.cuts[1]);
            }
            last_time = t - length;
            path = new_path;
        }
        cairo_set_source_rgb(cr, red, green, blue);
        Piecewise<D2<SBasis> > port = portion(path, std::max(t - last_time - length, 0.), t - last_time);
        cairo_pw_d2(cr, port);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0., 0., 1.);
        dot_plot(cr, path, std::max(t - last_time - length, 0.), t - last_time);
    }
};

class Intro: public Toy {
    int t;
    vector<Walker> walkers;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        t++;
        if(t < 40 && t % 2 == 0) {
            walkers.push_back(Walker(t, uniform(), uniform(), uniform(), uniform() * 100));
        }
        for(unsigned i = 0; i < walkers.size(); i++) {
            walkers[i].draw(cr, t);
        }

        Toy::draw(cr, notify, width, height, save);
        redraw();
    }

    virtual int should_draw_bounds() { return 0; }
    
    public:
    Intro () {
        t = 0;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Intro(), 720, 480);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
