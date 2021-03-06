/**
 * \file
 * \brief parse SVG path specifications
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
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
 *
 */


#include <cmath>
#include <vector>
#include <glib.h>

#include <2geom/point.h>
#include <2geom/svg-path-parser.h>
#include <2geom/angle.h>

namespace Geom {

namespace {

class Parser {
public:
    Parser(SVGPathSink &sink) : _absolute(false), _sink(sink) {}

    void parse(char const *str) throw(SVGPathParseError);

private:
    bool _absolute;
    Point _current;
    Point _initial;
    Point _cubic_tangent;
    Point _quad_tangent;
    std::vector<double> _params;
    SVGPathSink &_sink;

    void _reset() {
        _absolute = false;
        _current = _initial = Point(0, 0);
        _quad_tangent = _cubic_tangent = Point(0, 0);
        _params.clear();
    }

    void _push(double value) {
        _params.push_back(value);
    }

    double _pop() {
        double value = _params.back();
        _params.pop_back();
        return value;
    }

    bool _pop_flag() {
        return _pop() != 0.0;
    }

    double _pop_coord(Geom::Dim2 axis) {
        if (_absolute) {
            return _pop();
        } else {
            return _pop() + _current[axis];
        }
    }

    Point _pop_point() {
        double y = _pop_coord(Geom::Y);
        double x = _pop_coord(Geom::X);
        return Point(x, y);
    }

    void _moveTo(Point p) {
        _quad_tangent = _cubic_tangent = _current = _initial = p;
        _sink.moveTo(p);
    }
    
    void _hlineTo(Point p) {
        _quad_tangent = _cubic_tangent = _current = p;
        _sink.hlineTo(p[Geom::X]);
    }
    
    void _vlineTo(Point p) {
        _quad_tangent = _cubic_tangent = _current = p;
        _sink.vlineTo(p[Geom::Y]);
    }

    void _lineTo(Point p) {
        _quad_tangent = _cubic_tangent = _current = p;
        _sink.lineTo(p);
    }

    void _curveTo(Point c0, Point c1, Point p) {
        _quad_tangent = _current = p;
        _cubic_tangent = p + ( p - c1 );
        _sink.curveTo(c0, c1, p);
    }

    void _quadTo(Point c, Point p) {
        _cubic_tangent = _current = p;
        _quad_tangent = p + ( p - c );
        _sink.quadTo(c, p);
    }

    void _arcTo(double rx, double ry, double angle,
                bool large_arc, bool sweep, Point p)
    {
        _quad_tangent = _cubic_tangent = _current = p;
        _sink.arcTo(rx, ry, angle, large_arc, sweep, p);
    }

    void _closePath() {
        _quad_tangent = _cubic_tangent = _current = _initial;
        _sink.closePath();
    }
};

%%{
    machine svg_path;
    write data noerror;
}%%

void Parser::parse(char const *str)
throw(SVGPathParseError)
{
    char const *p = str;
    char const *start = NULL;
    int cs;

    _reset();

    %%{
        action start_number {
            start = p;
        }

        action push_number {
            char const *end=p;
            std::string buf(start, end);
            _push(g_ascii_strtod(buf.c_str(), NULL));
            start = NULL;
        }

        action push_true {
            _push(1.0);
        }

        action push_false {
            _push(0.0);
        }

        action mode_abs {
            _absolute = true;
        }
    
        action mode_rel {
            _absolute = false;
        }
    
        action moveto {
            _moveTo(_pop_point());
        }    

        action lineto {
            _lineTo(_pop_point());
        }

        action horizontal_lineto {
            _hlineTo(Point(_pop_coord(X), _current[Y]));
        }

        action vertical_lineto {
            _vlineTo(Point(_current[X], _pop_coord(Y)));
        }

        action curveto {
            Point p = _pop_point();
            Point c1 = _pop_point();
            Point c0 = _pop_point();
            _curveTo(c0, c1, p);
        }

        action smooth_curveto {
            Point p = _pop_point();
            Point c1 = _pop_point();
            _curveTo(_cubic_tangent, c1, p);
        }

        action quadratic_bezier_curveto {
            Point p = _pop_point();
            Point c = _pop_point();
            _quadTo(c, p);
        }

        action smooth_quadratic_bezier_curveto {
            Point p = _pop_point();
            _quadTo(_quad_tangent, p);
        }

        action elliptical_arc {
            Point point = _pop_point();
            bool sweep = _pop_flag();
            bool large_arc = _pop_flag();
            double angle = deg_to_rad(_pop());
            double ry = _pop();
            double rx = _pop();

            _arcTo(rx, ry, angle, large_arc, sweep, point);
        }
        
        action closepath {
            _closePath();
        }

        wsp = (' ' | 9 | 10 | 13);
        sign = ('+' | '-');
        digit_sequence = digit+;
        exponent = ('e' | 'E') sign? digit_sequence;
        fractional_constant =
            digit_sequence? '.' digit_sequence
            | digit_sequence '.';
        floating_point_constant =
            fractional_constant exponent?
            | digit_sequence exponent;
        integer_constant = digit_sequence;
        comma = ',';
        comma_wsp = (wsp+ comma? wsp*) | (comma wsp*);

        flag = ('0' %push_false | '1' %push_true);
        
        number =
            ( sign? integer_constant
            | sign? floating_point_constant )
            >start_number %push_number;

        nonnegative_number =
            ( integer_constant
            | floating_point_constant)
            >start_number %push_number;

        coordinate = number $(number,1) %(number,0);
        coordinate_pair = (coordinate $(coordinate_pair_a,1) %(coordinate_pair_a,0) comma_wsp? coordinate $(coordinate_pair_b,1) %(coordinate_pair_b,0)) $(coordinate_pair,1) %(coordinate_pair,0);
        elliptical_arc_argument =
            (nonnegative_number $(elliptical_arg_a,1) %(elliptical_arg_a,0) comma_wsp?
             nonnegative_number $(elliptical_arg_b,1) %(elliptical_arg_b,0) comma_wsp?
             number comma_wsp
             flag comma_wsp flag comma_wsp
             coordinate_pair)
            %elliptical_arc;
        elliptical_arc_argument_sequence =
            elliptical_arc_argument $1 %0
            (comma_wsp? elliptical_arc_argument $1 %0)*;
        elliptical_arc =
            ('A' %mode_abs| 'a' %mode_rel) wsp*
            elliptical_arc_argument_sequence;
        
        smooth_quadratic_bezier_curveto_argument =
            coordinate_pair %smooth_quadratic_bezier_curveto;
        smooth_quadratic_bezier_curveto_argument_sequence =
            smooth_quadratic_bezier_curveto_argument $1 %0
            (comma_wsp?
             smooth_quadratic_bezier_curveto_argument $1 %0)*;
        smooth_quadratic_bezier_curveto =
            ('T' %mode_abs| 't' %mode_rel) wsp*
             smooth_quadratic_bezier_curveto_argument_sequence;

        quadratic_bezier_curveto_argument =
            (coordinate_pair $1 %0 comma_wsp? coordinate_pair)
            %quadratic_bezier_curveto;
        quadratic_bezier_curveto_argument_sequence =
            quadratic_bezier_curveto_argument $1 %0
            (comma_wsp? quadratic_bezier_curveto_argument $1 %0)*;
        quadratic_bezier_curveto =
            ('Q' %mode_abs| 'q' %mode_rel) wsp* 
            quadratic_bezier_curveto_argument_sequence;

        smooth_curveto_argument =
            (coordinate_pair $1 %0 comma_wsp? coordinate_pair)
            %smooth_curveto;
        smooth_curveto_argument_sequence =
            smooth_curveto_argument $1 %0
            (comma_wsp? smooth_curveto_argument $1 %0)*;
        smooth_curveto =
            ('S' %mode_abs| 's' %mode_rel)
            wsp* smooth_curveto_argument_sequence;

        curveto_argument =
            (coordinate_pair $1 %0 comma_wsp?
             coordinate_pair $1 %0 comma_wsp?
             coordinate_pair) 
            %curveto;
        curveto_argument_sequence =
            curveto_argument $1 %0
            (comma_wsp? curveto_argument $1 %0)*;
        curveto =
            ('C' %mode_abs| 'c' %mode_rel)
            wsp* curveto_argument_sequence;

        vertical_lineto_argument = coordinate %vertical_lineto;
        vertical_lineto_argument_sequence =
            vertical_lineto_argument $(vertical_lineto_argument_a,1) %(vertical_lineto_argument_a,0)
            (comma_wsp? vertical_lineto_argument $(vertical_lineto_argument_b,1) %(vertical_lineto_argument_b,0))*;
        vertical_lineto =
            ('V' %mode_abs| 'v' %mode_rel)
            wsp* vertical_lineto_argument_sequence;

        horizontal_lineto_argument = coordinate %horizontal_lineto;
        horizontal_lineto_argument_sequence =
            horizontal_lineto_argument $(horizontal_lineto_argument_a,1) %(horizontal_lineto_argument_a,0)
            (comma_wsp? horizontal_lineto_argument $(horizontal_lineto_argument_b,1) %(horizontal_lineto_argument_b,0))*;
        horizontal_lineto =
            ('H' %mode_abs| 'h' %mode_rel)
            wsp* horizontal_lineto_argument_sequence;

        lineto_argument = coordinate_pair %lineto;
        lineto_argument_sequence =
            lineto_argument $1 %0
            (comma_wsp? lineto_argument $1 %0)*;
        lineto =
            ('L' %mode_abs| 'l' %mode_rel) wsp*
            lineto_argument_sequence;

        closepath = ('Z' | 'z') %closepath;

        moveto_argument = coordinate_pair %moveto;
        moveto_argument_sequence =
            moveto_argument $1 %0
            (comma_wsp? lineto_argument $1 %0)*;
        moveto =
            ('M' %mode_abs | 'm' %mode_rel)
            wsp* moveto_argument_sequence;

        drawto_command =
            closepath | lineto |
            horizontal_lineto | vertical_lineto |
            curveto | smooth_curveto |
            quadratic_bezier_curveto |
            smooth_quadratic_bezier_curveto |
            elliptical_arc;

        drawto_commands = drawto_command (wsp* drawto_command)*;
        moveto_drawto_command_group = moveto wsp* drawto_commands?;
        moveto_drawto_command_groups =
            moveto_drawto_command_group
            (wsp* moveto_drawto_command_group)*;

        end_of_string = 0 @{fbreak;};

        svg_path = wsp* moveto_drawto_command_groups? wsp* end_of_string;
                

        main := svg_path;

        # Inintialize and execute.
        write init;
        write exec noend;
    }%%

    if ( cs < svg_path_first_final ) {
        throw SVGPathParseError();
    }
}

}

void parse_svg_path(char const *str, SVGPathSink &sink)
throw(SVGPathParseError)
{
    Parser parser(sink);
    parser.parse(str);
    sink.finish();
}

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
