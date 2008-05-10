/*
 * point-curve nearest point routines testing 
 *
 * Authors:
 * 		Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2008  authors
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

#include "d2.h"
#include "sbasis.h"
#include "path.h"
#include "angle.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"
#include "piecewise.h"


#include "path-cairo.h"
#include "toy-framework.h"


#include <algorithm>

using namespace Geom;



class NearestPoints : public Toy
{
	enum menu_item_t
	{
		FIRST_ITEM = 1,
		LINE_SEGMENT = FIRST_ITEM,
		ELLIPTICAL_ARC,
		SBASIS_CURVE,
		PIECEWISE,
		PATH,
		TOTAL_ITEMS
	};
	
	static const char* menu_items[TOTAL_ITEMS];
	
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
    	
    	Point p = handles.back();
    	Point np = p;
    	std::vector<Point> nps;
    	
    	cairo_set_line_width (cr, 0.3);
    	switch ( choice )
    	{
    		case '1': 
    		{
    			LineSegment seg(handles[0], handles[1]);
    			cairo_move_to(cr, handles[0]);
    			cairo_curve(cr, seg);
    			double t = seg.nearestPoint(p);
    			np = seg.pointAt(t);
    			if ( toggles[0].on )
    			{
    				nps.push_back(np);
    			}
    			break;
    		}
    		case '2':
    		{
    	        SVGEllipticalArc earc;
    	        bool earc_constraints_satisfied = true;
    	        try
    	        {
    	        	earc.set(handles[0], 200, 150, 0, true, true, handles[1]);
    	        }
    	        catch( RangeError e )
    	        {
    	        	earc_constraints_satisfied = false;
    	        }
    	        if ( earc_constraints_satisfied )
    	        {
    	        	cairo_md_sb(cr, earc.toSBasis());
    	        	if ( toggles[0].on )
    	        	{
    	        		std::vector<double> t = earc.allNearestPoints(p);
    	        		for ( unsigned int i = 0; i < t.size(); ++i )
    	        			nps.push_back(earc.pointAt(t[i])); 
    	        	}
    	        	else
    	        	{
    	        		double t = earc.nearestPoint(p);
    	        		np = earc.pointAt(t);
    	        	}
    	        } 
    	        break;
    		}
    		case '3':
    		{
    			D2<SBasis> A = handles_to_sbasis(handles.begin(), total_handles-1);
    			cairo_md_sb(cr, A);
    	        if ( toggles[0].on )
    	        {
    	        	std::vector<double> t = Geom::all_nearest_points(p, A);
    	        	for ( unsigned int i = 0; i < t.size(); ++i )
    	        		nps.push_back(A(t[i])); 
    	        }
    	        else
    	        {
    	        	double t = nearest_point(p, A);
    	        	np = A(t);
    	        }
    			break;
    		}
    		case '4':
    		{
    			D2<SBasis> A = handles_to_sbasis(handles.begin(), 3);
    			D2<SBasis> B = handles_to_sbasis(handles.begin() + 3, 3);
    			D2<SBasis> C = handles_to_sbasis(handles.begin() + 6, 3);
    			D2<SBasis> D = handles_to_sbasis(handles.begin() + 9, 3);
    			cairo_md_sb(cr, A);
    			cairo_md_sb(cr, B);
    			cairo_md_sb(cr, C);
    			cairo_md_sb(cr, D);
    	        Piecewise< D2<SBasis> > pwc;
    	        pwc.push_cut(0);
    	        pwc.push_seg(A);
    	        pwc.push_cut(0.25);
    	        pwc.push_seg(B);
    	        pwc.push_cut(0.50);
    	        pwc.push_seg(C);
    	        pwc.push_cut(0.75);
    	        pwc.push_seg(D);
    	        pwc.push_cut(1);
    	        if ( toggles[0].on )
    	        {
    	        	std::vector<double> t = Geom::all_nearest_points(p, pwc);
    	        	for ( unsigned int i = 0; i < t.size(); ++i )
    	        		nps.push_back(pwc(t[i])); 
    	        }
    	        else
    	        {
    	        	double t = Geom::nearest_point(p, pwc);
    	        	np = pwc(t);
    	        }
    			break;
    		}
    		case '5':
    		{
    			closed_toggle = true;
    			BezierCurve<2> A(handles[0], handles[1], handles[2]);
    			BezierCurve<3> B(handles[2], handles[3], handles[4], handles[5]);
    			BezierCurve<3> C(handles[5], handles[6], handles[7], handles[8]);
    			Path path;
    	        path.append(A);
    	        path.append(B);
    	        path.append(C);
    	        SVGEllipticalArc D;    	        
    	        bool earc_constraints_satisfied = true;
    	        try
    	        {
    	        	D.set(handles[8], 160, 80, 0, true, true, handles[9]);
    	        }
    	        catch( RangeError e )
    	        {
    	        	earc_constraints_satisfied = false;
    	        }
    	        if ( earc_constraints_satisfied ) path.append(D);
    	        if ( toggles[1].on ) path.close(true);
    	        cairo_path(cr, path);
    	        if ( path.closed() )
    	        {
    	        	cairo_line_to( cr, path[0].initialPoint() );
    	        }
    	        if ( toggles[0].on )
    	        {
    	        	std::vector<double> t = path.allNearestPoints(p);
    	        	for ( unsigned int i = 0; i < t.size(); ++i )
    	        		nps.push_back(path.pointAt(t[i])); 
    	        }
    	        else
    	        {
    	        	double t = path.nearestPoint(p);
    	        	np = path.pointAt(t);
    	        }
    			break;
    		}
    		default:
    		{
    			*notify << std::endl;
				for (int i = FIRST_ITEM; i < TOTAL_ITEMS; ++i)
				{
					*notify << "   " << i << " -  " <<  menu_items[i] << std::endl;
				}
				Toy::draw(cr, notify, width, height, save);
				return;
    		}
    	}
   	
    	if ( toggles[0].on )
    	{
    		for ( unsigned int i = 0; i < nps.size(); ++i )
    		{
    			cairo_move_to(cr, p);
    			cairo_line_to(cr, nps[i]);
    		}
    	}
    	else
    	{
    		cairo_move_to(cr, p);
    		cairo_line_to(cr, np);
    	}
        cairo_stroke(cr);
        
    	toggles[0].bounds = Rect( Point(10, height - 50), Point(10, height - 50) + Point(80,25) );
    	toggles[0].draw(cr);
    	if ( closed_toggle )
    	{
    		toggles[1].bounds = Rect( Point(100, height - 50), Point(100, height - 50) + Point(80,25) );
    		toggles[1].draw(cr);
    		closed_toggle = false;
    	}
    	
    	Toy::draw(cr, notify, width, height, save);
    }
    
    void key_hit(GdkEventKey *e)
    {
    	choice = e->keyval;
    	switch ( choice )
    	{
    		case '1': 
    			total_handles = 2;
    			break;
    		case '2':
    			total_handles = 2;
    			break;
    		case '3':
    			total_handles = 6;
    			break;
    		case '4':
    			total_handles = 13;
    			break;    
    		case '5':
    			total_handles = 10;
    			break;    
    		default:
    			total_handles = 0;
    	}
    	handles.clear();
    	for ( unsigned int i = 0; i <= total_handles; ++i )
    	{
    		handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    	}
    	redraw();
    }
    
    void mouse_pressed(GdkEventButton* e) 
    {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }

  public:
	NearestPoints()
		: total_handles(0), choice('0'), closed_toggle(false)
	{
		handles.push_back(Geom::Point(uniform()*400, uniform()*400));
		toggles.push_back( Toggle("ALL NP", false) );
		toggles.push_back( Toggle("CLOSED", false) );
	}
	
  private:
	std::vector<Toggle> toggles;
	unsigned int total_handles;
	char choice;
	bool closed_toggle;
};

const char* NearestPoints::menu_items[] = 
{
	"",
	"LineSegment",
	"EllipticalArc",
	"SBasisCurve",
	"Piecewise",
	"Path"
};



int main(int argc, char **argv) 
{	
    init( argc, argv, new NearestPoints() );
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
