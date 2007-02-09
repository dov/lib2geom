#include "sb-pw.h"

namespace Geom {
SBasis elem_portion(const pw_sb &a, int i, double from, double to) {
    double rwidth = 1 / (a.cuts[i+1] - a.cuts[i]);
    return portion( a.segs[i], (from - a.cuts[i]) * rwidth, (to - a.cuts[i]) * rwidth );
}

pw_sb partition(const pw_sb &t, vector<double> const &c) {
    pw_sb ret = pw_sb();
    //just a bit of optimizing reservation
    ret.cuts.reserve(c.size() + t.cuts.size());
    ret.segs.reserve(c.size() + t.cuts.size() - 1);

    int si = 0, ci = 0;     //Segment index, Cut index

    //if the input cuts have something earlier than this pw_sb, add sections of zero
    while(ci < c.size() && c[ci] < t.cuts[0]) {
        ret.cuts.push_back(c[ci]);
        ret.segs.push_back(SBasis());
        ci++;
    }
    ret.cuts.push_back(t.cuts[0]);
    double prev = t.cuts[0];      //Previous cut made
    while(si < t.segs.size() && ci < c.size()) {
        if(c[ci] >= t.cuts[si + 1]) {  //no more cuts within this segment
            if(prev > t.cuts[si]) {
                ret.segs.push_back(portion(t.segs[si], (prev - t.cuts[si]) / (t.cuts[si+1] - t.cuts[si]), 1.0));
            } else {
                ret.segs.push_back(t.segs[si]);
            }
            si++;
            ret.cuts.push_back(t.cuts[si]);
            prev = t.cuts[si];
        } else if(c[ci] == t.cuts[si]) { //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {
	    ret.segs.push_back(elem_portion(t, si, prev, c[ci]));
            ret.cuts.push_back(c[ci]);
            prev = c[ci];
            ci++;
        }
    }
    //if the input cuts extend further than this pw_sb, add sections of zero
    while(ci < c.size()) {
        ret.cuts.push_back(c[ci]);
        ret.segs.push_back(SBasis());
        ci++;
    }
    return ret;
}

pw_sb portion(const pw_sb &a, double from, double to) {
    int fi = 0, ti = 0; //from/to indexes
    for(int i = 0; i < a.segs.size(); i++) {
	if(a.cuts[i] < from && from < a.cuts[i+1]) fi = i; 
	if(a.cuts[i] < to && to < a.cuts[i+1]) ti = i;
    }
    if (fi > a.cuts[a.segs.size()]) fi = a.segs.size() - 1;
    if (ti > a.cuts[a.segs.size()]) ti = a.segs.size() - 1;

    pw_sb ret = pw_sb();
    
    if (fi < ti) {
	ret.segs.reserve(ti - fi + 1);
	ret.cuts.reserve(ti - fi + 2);
	//TODO: consider coincidince
	ret.cuts.push_back(from);
	ret.segs.push_back( portion(a.segs[fi], (from - a.cuts[fi]) / (a.cuts[fi+1] - a.cuts[fi]), 1.0) );
        for(int i = fi + 1; i < ti; i++) {
	    ret.cuts.push_back(a.cuts[i]);
	    ret.segs.push_back(a.segs[i]);
        }
	ret.cuts.push_back(a.cuts[ti]);
	ret.segs.push_back( portion(a.segs[ti], 0.0, (to - a.cuts[ti]) / (a.cuts[ti+1] - a.cuts[ti])) );
	ret.cuts.push_back(a.cuts[ti + 1]);
    } else if (fi > ti) {
	ret.segs.reserve(fi - ti + 1);
	ret.cuts.reserve(fi - ti + 2);
	//TODO: use above as model.  Use SBasis reverse(SBasis) function
        for(int i = fi - 1; i < ti; i--) {
	    
        }
    } else {
	ret.segs.reserve(1);
	ret.cuts.reserve(2);
        ret.cuts.push_back(from);
	ret.segs.push_back(elem_portion(a, fi, from, to));
	ret.cuts.push_back(to);
    }
}


/* This macro provides a mapping of operations on pw_sb pieces
 * It assumes that it is the contents of a function, and there is a pw_sb parameter, a.
 */
#define MapSB(op)                               \
    pw_sb ret = pw_sb();                        \
    for(int i = 0; i < ret.segs.size();i++) {   \
        ret.segs.push_back( op );               \
        ret.cuts.push_back( a.cuts[i] );        \
    }                                           \
    return ret;

//pw_sb operator+(BezOrd b, SBasis a)

pw_sb operator-(pw_sb const &a) { MapSB(- a.segs[i]) }
pw_sb operator-(BezOrd const &b, const pw_sb&a) { MapSB(b- a.segs[i]) }

pw_sb operator+=(pw_sb& a, const BezOrd& b) {
    for(int i = 0; i < a.segs.size();i++) {
        a.segs[i] += b;
    }
    return a;
}
pw_sb operator+=(pw_sb& a, double b) {
    for(int i = 0; i < a.segs.size();i++) {
        a.segs[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, const BezOrd& b) {
    for(int i = 0; i < a.segs.size();i++) {
        a.segs[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, double b) {
    for(int i = 0;i < a.segs.size();i++) {
        a.segs[i] -= b;
    }
    return a;
}

// Semantically-correct zipping of pw_sbs, with an arbitrary operation
template <typename F>
inline pw_sb ZipSBWith(pw_sb const &a, pw_sb const &b) {
  pw_sb pa = partition(a, b.cuts), pb = partition(b, a.cuts);
  pw_sb ret = pw_sb();
  for ( int i = 0 ; i < pa.segs.size() && i < pb.segs.size() ; i++ ) {
    ret.segs.push_back(F::op(pa.segs[i], pb.segs[i]));
    ret.cuts.push_back(pa.cuts[i]);
  }
  return ret;
}

//Dummy structs
struct sbasis_add{static SBasis op(SBasis const &a, SBasis const &b) {return a + b;} };
struct sbasis_sub{static SBasis op(SBasis const &a, SBasis const &b) {return a - b;} };
struct sbasis_mul{static SBasis op(SBasis const &a, SBasis const &b) {return a * b;} };

pw_sb operator+(pw_sb const &a, pw_sb const &b) { ZipSBWith<sbasis_add>(a, b); }
pw_sb operator-(pw_sb const &a, pw_sb const &b) { ZipSBWith<sbasis_sub>(a, b); }

pw_sb multiply(pw_sb const &a, pw_sb const &b) { ZipSBWith<sbasis_mul> (a, b); }

/*TODO: should k really be necessary?
template <int K>
struct sbasis_div{static SBasis op(SBasis const &a, SBasis const &b) {return divide(a, b, K);} };
pw_sb divide(pw_sb const &a, pw_sb const &b, int k) { ZipSBWith<sbasis_div<k> >(a, b);}
*/

/* General Plan:
 * First, the range of b must be found.  One method for this might be roots of
 * the derivative in order to find extrema.  There's no need to worry about
 * the second derivative, as even in the cases of f''=0, it will just be an
 * intermediate value on the way to an actual extrema.  In the case of
 * consistant runs of f'=0, one representative point should be selected.
 * 
 * At this point, the domain of a which is outside the range of b may be ignored.
 * 
 * The roots of the derivative are once again used to find points of 'loopback'
 * where a is retraversed, in the opposite direction.
 *
 * Hmm, at this point its a bit fuzzy - compositions and manipulations on
 * individual sbasis.
 */
pw_sb compose(pw_sb const &a, pw_sb const &b) {
    
}

/*
class pw_sb {
    vector<pw_piece> pieces;

    const_iterator first() {pieces.begin();}
    const_iterator last()  {pieces.end();}

    pw_sb() : pieces() {}
};

class pw_piece {
    double start, end;
    SBasis seg;

    pw_piece(SBasis s, double st)            : seg(s), start(st) { end = t + 1; }
    pw_piece(SBasis s, double st, double en) : seg(s), start(st), end(en) {}
}
*/

    /*splitSeg(int i, double t) {
        SBasis a, b;
        segs[i].split((t-cuts[i]) / (cuts[i+1]-cuts[i]), a, b);
        vector<SBasis>::iterator it = segs.begin() + i
        segs.remove(it);
        segs.insert(it, a);
        segs.insert(it+1, b);
        cuts.insert(cuts.begin() + i + 1, c[ci]);
    }*/

}
