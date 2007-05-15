/*
 * d2.cpp - Lifts one dimensional objects into 2d 
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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

#include "d2.h"

namespace Geom {

D2<SBasis> derivative(D2<SBasis> const & a) {
    return D2<SBasis>(derivative(a[X]), derivative(a[Y]));
}

D2<SBasis> integral(D2<SBasis> const & a) {
    return D2<SBasis>(integral(a[X]), integral(a[Y]));
}

SBasis L2(D2<SBasis> const & a, unsigned k) { return sqrt(dot(a, a), k); }
double L2(D2<double> const & a) { return hypot(a[0], a[1]); }

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b) {
    return D2<SBasis>(multiply(a, b[X]), multiply(a, b[Y]));
}

D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b) {
    return D2<SBasis>(multiply(a, b[X]), multiply(a, b[Y]));
}

D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms) {
    return D2<SBasis>(truncate(a[X], terms), truncate(a[Y], terms));
}

unsigned sbasisSize(D2<SBasis> const & a) {
    return std::max((unsigned) a[0].size(), (unsigned) a[1].size());
}

//TODO: Is this sensical? shouldn't it be like pythagorean or something?
double tailError(D2<SBasis> const & a, unsigned tail) {
    return std::max(a[0].tailError(tail), a[1].tailError(tail));
}

bool isFinite(D2<SBasis> const & a) {
    for(unsigned i = 0; i < 2; i++)
        if(!a[i].isFinite())
            return false;
    return true;
}

Piecewise<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a) {
    Piecewise<SBasis> x = partition(a[0], a[1].cuts), y = partition(a[1], a[0].cuts);
    assert(x.size() == y.size());
    Piecewise<D2<SBasis> > ret;
    for(unsigned i = 0; i < x.size(); i++)
        ret.push_seg(D2<SBasis>(x[i], y[i]));
    ret.cuts.insert(ret.cuts.end(), x.cuts.begin(), x.cuts.end());
    return ret;
}

D2<Piecewise<SBasis> > makeCutsIndependant(Piecewise<D2<SBasis> > const &a) {
    D2<Piecewise<SBasis> > ret;
    for(unsigned d = 0; d < 2; d++) {
        for(unsigned i = 0; i < a.size(); i++)
            ret[d].push_seg(a[i][d]);
        ret[d].cuts.insert(ret[d].cuts.end(), a.cuts.begin(), a.cuts.end());
    }
    return ret;
}

Piecewise<D2<SBasis> > rot90(Piecewise<D2<SBasis> > const &M){
  Piecewise<D2<SBasis> > result;
  if (M.empty()) return M;
  result.push_cut(M.cuts[0]);
  for (unsigned i=0; i<M.size(); i++){
    result.push(rot90(M[i]),M.cuts[i+1]);
  }
  return result;
}

Piecewise<SBasis> dot(Piecewise<D2<SBasis> > const &a, 
		      Piecewise<D2<SBasis> > const &b){
  Piecewise<SBasis > result;
  if (a.empty() || b.empty()) return result;
  Piecewise<D2<SBasis> > aa = partition(a,b.cuts);
  Piecewise<D2<SBasis> > bb = partition(b,a.cuts);

  result.push_cut(aa.cuts.front());
  for (unsigned i=0; i<aa.size(); i++){
    result.push(dot(aa.segs[i],bb.segs[i]),aa.cuts[i+1]);
  }
  return result;
}

Piecewise<SBasis> cross(Piecewise<D2<SBasis> > const &a, 
			Piecewise<D2<SBasis> > const &b){
  Piecewise<SBasis > result;
  if (a.empty() || b.empty()) return result;
  Piecewise<D2<SBasis> > aa = partition(a,b.cuts);
  Piecewise<D2<SBasis> > bb = partition(b,a.cuts);

  result.push_cut(aa.cuts.front());
  for (unsigned i=0; i<a.size(); i++){
    result.push(cross(aa.segs[i],bb.segs[i]),aa.cuts[i+1]);
  }
  return result;
}

  
};
