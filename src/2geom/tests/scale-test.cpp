#include <utest/utest.h>
#include <libnr/scale.h>
#include <libnr/scale-ops.h>
using Geom::X;
using Geom::Y;

int main(int argc, char *argv[])
{
    utest_start("Geom::scale");

    Geom::scale const sa(1.5, 2.0);
    UTEST_TEST("x,y constructor and operator[] const") {
        UTEST_ASSERT(sa[X] == 1.5);
        UTEST_ASSERT(sa[Y] == 2.0);
        UTEST_ASSERT(sa[0u] == 1.5);
        UTEST_ASSERT(sa[1u] == 2.0);
    }

    Geom::Point const b(-2.0, 3.0);
    Geom::scale const sb(b);

    UTEST_TEST("copy constructor, operator==, operator!=") {
        Geom::scale const sa_copy(sa);
        UTEST_ASSERT( sa == sa_copy );
        UTEST_ASSERT(!( sa != sa_copy ));
        UTEST_ASSERT( sa != sb );
    }

    UTEST_TEST("operator=") {
        Geom::scale sa_eq(sb);
        sa_eq = sa;
        UTEST_ASSERT( sa == sa_eq );
    }

    UTEST_TEST("point constructor") {
        UTEST_ASSERT(sb[X] == b[X]);
        UTEST_ASSERT(sb[Y] == b[Y]);
    }

    UTEST_TEST("operator*(Point, scale)") {
        Geom::Point const ab( b * sa );
        UTEST_ASSERT( ab == Geom::Point(-3.0, 6.0) );
    }

    UTEST_TEST("operator*(scale, scale)") {
        Geom::scale const sab( sa * sb );
        UTEST_ASSERT( sab == Geom::scale(-3.0, 6.0) );
    }

    UTEST_TEST("operator/(scale, scale)") {
        Geom::scale const sa_b( sa / sb );
        Geom::scale const exp_sa_b(-0.75, 2./3.);
        UTEST_ASSERT( sa_b[0] == exp_sa_b[0] );
        UTEST_ASSERT( fabs( sa_b[1] - exp_sa_b[1] ) < 1e-10 );
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
