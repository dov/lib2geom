#include <utest/utest.h>
#include <libnr/matrix.h>
#include <libnr/matrix-fns.h>
#include <libnr/matrix-ops.h>
#include <libnr/matrix-rotate-ops.h>
#include <libnr/matrix-scale-ops.h>
#include <libnr/point-matrix-ops.h>
#include <libnr/rotate.h>
#include <libnr/rotate-ops.h>
#include <libnr/scale-ops.h>
#include <libnr/scale-translate-ops.h>
#include <libnr/translate.h>
#include <libnr/translate-ops.h>
#include <libnr/translate-scale-ops.h>
using Geom::Matrix;
using Geom::X;
using Geom::Y;

inline bool point_equalp(Geom::Point const &a, Geom::Point const &b)
{
    return ( Geom_DF_TEST_CLOSE(a[X], b[X], 1e-5) &&
             Geom_DF_TEST_CLOSE(a[Y], b[Y], 1e-5)   );
}

int main(int argc, char *argv[])
{
    int rc = EXIT_SUCCESS;

    Matrix const m_id(Geom::identity());
    Geom::rotate const r_id(Geom::Point(1, 0));
    Geom::translate const t_id(0, 0);

    utest_start("Matrix");

    Matrix const c16(1.0, 2.0,
                     3.0, 4.0,
                     5.0, 6.0);
    UTEST_TEST("basic constructors, operator=") {
        Matrix const c16_copy(c16);
        Matrix c16_eq(m_id);
        c16_eq = c16;
        for(unsigned i = 0; i < 6; ++i) {
            UTEST_ASSERT( c16[i] == 1.0 + i );
            UTEST_ASSERT( c16[i] == c16_copy[i] );
            UTEST_ASSERT( c16[i] == c16_eq[i] );
            UTEST_ASSERT( m_id[i] == double( i == 0 || i == 3 ) );
        }
    }

    UTEST_TEST("scale constructor") {
        Geom::scale const s(2.0, 3.0);
        Geom::Matrix const ms(s);
        Geom::Point const p(5.0, 7.0);
        UTEST_ASSERT( p * s == Geom::Point(10.0, 21.0) );
        UTEST_ASSERT( p * ms == Geom::Point(10.0, 21.0) );
    }

    Geom::rotate const r86(Geom::Point(.8, .6));
    Geom::Matrix const mr86(r86);
    UTEST_TEST("rotate constructor") {
        Geom::Point const p0(1.0, 0.0);
        Geom::Point const p90(0.0, 1.0);
        UTEST_ASSERT( p0 * r86 == Geom::Point(.8, .6) );
        UTEST_ASSERT( p0 * mr86 == Geom::Point(.8, .6) );
        UTEST_ASSERT( p90 * r86 == Geom::Point(-.6, .8) );
        UTEST_ASSERT( p90 * mr86 == Geom::Point(-.6, .8) );
        UTEST_ASSERT(matrix_equalp(Matrix( r86 * r86 ),
                                   mr86 * mr86,
                                   1e-14));
    }

    Geom::translate const t23(2.0, 3.0);
    UTEST_TEST("translate constructor") {
        Geom::Matrix const mt23(t23);
        Geom::Point const b(-2.0, 3.0);
        UTEST_ASSERT( b * t23 == b * mt23 );
    }

    Geom::scale const s_id(1.0, 1.0);
    UTEST_TEST("test_identity") {
        UTEST_ASSERT(m_id.test_identity());
        UTEST_ASSERT(Matrix(t_id).test_identity());
        UTEST_ASSERT(!(Matrix(Geom::translate(-2, 3)).test_identity()));
        UTEST_ASSERT(Matrix(r_id).test_identity());
        Geom::rotate const rot180(Geom::Point(-1, 0));
        UTEST_ASSERT(!(Matrix(rot180).test_identity()));
        UTEST_ASSERT(Matrix(s_id).test_identity());
        UTEST_ASSERT(!(Matrix(Geom::scale(1.0, 0.0)).test_identity()));
        UTEST_ASSERT(!(Matrix(Geom::scale(0.0, 1.0)).test_identity()));
        UTEST_ASSERT(!(Matrix(Geom::scale(1.0, -1.0)).test_identity()));
        UTEST_ASSERT(!(Matrix(Geom::scale(-1.0, -1.0)).test_identity()));
    }

    UTEST_TEST("inverse") {
        UTEST_ASSERT( m_id.inverse() == m_id );
        UTEST_ASSERT( Matrix(t23).inverse() == Matrix(Geom::translate(-2.0, -3.0)) );
        Geom::scale const s2(-4.0, 2.0);
        Geom::scale const sp5(-.25, .5);
        UTEST_ASSERT( Matrix(s2).inverse() == Matrix(sp5) );
    }

    UTEST_TEST("nr_matrix_invert") {
        NRMatrix const nr_m_id(m_id);
        Matrix const m_s2(Geom::scale(-4.0, 2.0));
        NRMatrix const nr_s2(m_s2);
        Matrix const m_sp5(Geom::scale(-.25, .5));
        NRMatrix const nr_sp5(m_sp5);
        Matrix const m_t23(t23);
        NRMatrix const nr_t23(m_t23);
        NRMatrix inv;
        nr_matrix_invert(&inv, &nr_m_id);
        UTEST_ASSERT( Matrix(inv) == m_id );
        nr_matrix_invert(&inv, &nr_t23);
        UTEST_ASSERT( Matrix(inv) == Matrix(Geom::translate(-2.0, -3.0)) );
        nr_matrix_invert(&inv, &nr_s2);
        UTEST_ASSERT( Matrix(inv) == Matrix(nr_sp5) );
        nr_matrix_invert(&inv, &nr_sp5);
        UTEST_ASSERT( Matrix(inv) == Matrix(nr_s2) );

        /* Test that nr_matrix_invert handles src == dest. */
        inv = nr_s2;
        nr_matrix_invert(&inv, &inv);
        UTEST_ASSERT( Matrix(inv) == Matrix(nr_sp5) );
        inv = nr_t23;
        nr_matrix_invert(&inv, &inv);
        UTEST_ASSERT( Matrix(inv) == Matrix(Geom::translate(-2.0, -3.0)) );
    }

    UTEST_TEST("elliptic quadratic form") {
        Geom::Matrix const aff(1.0, 1.0,
                             0.0, 1.0,
                             5.0, 6.0);
        Geom::Matrix const invaff = aff.inverse();
        UTEST_ASSERT( invaff[1] == -1.0 );
		
        Geom::Matrix const ef(elliptic_quadratic_form(invaff));
        Geom::Matrix const exp_ef(2, -1,
                                -1, 1,
                                0, 0);
        UTEST_ASSERT( ef == exp_ef );
    }

    UTEST_TEST("Matrix * rotate") {
        Geom::Matrix const ma(2.0, -1.0,
                            4.0, 4.0,
                            -0.5, 2.0);
        Geom::Matrix const a_r86( ma * r86 );
        Geom::Matrix const ma1( a_r86 * r86.inverse() );
        UTEST_ASSERT(matrix_equalp(ma1, ma, 1e-12));
        Geom::Matrix const exp_a_r86( 2*.8 + -1*-.6,  2*.6 + -1*.8,
                                    4*.8 + 4*-.6,   4*.6 + 4*.8,
                                    -.5*.8 + 2*-.6, -.5*.6 + 2*.8 );
        UTEST_ASSERT(matrix_equalp(a_r86, exp_a_r86, 1e-12));
    }

    UTEST_TEST("translate*scale, scale*translate") {
        Geom::translate const t2n4(2, -4);
        Geom::scale const sn2_8(-2, 8);
        Geom::Matrix const exp_ts(-2, 0,
                                0,  8,
                                -4, -32);
        Geom::Matrix const exp_st(-2, 0,
                                0,  8,
                                2, -4);
        UTEST_ASSERT( exp_ts == t2n4 * sn2_8 );
        UTEST_ASSERT( exp_st == sn2_8 * t2n4 );
    }

    UTEST_TEST("Matrix * scale") {
        Geom::Matrix const ma(2.0, -1.0,
                            4.0, 4.0,
                            -0.5, 2.0);
        Geom::scale const sn2_8(-2, 8);
        Geom::Matrix const exp_as(-4, -8,
                                -8, 32,
                                1,  16);
        UTEST_ASSERT( ma * sn2_8 == exp_as );
    }

    if (!utest_end()) {
        rc = EXIT_FAILURE;
    }

    return rc;
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
