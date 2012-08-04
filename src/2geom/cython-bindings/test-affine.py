import unittest
from math import pi, sqrt, sin, cos
from random import randint, uniform

import cy2geom

from cy2geom import Point, IntPoint
from cy2geom import Line, Ray, Rect

from cy2geom import Rect

from cy2geom import Affine
from cy2geom import Translate, Scale, Rotate, VShear, HShear, Zoom
from cy2geom import Eigen

class TestPrimitives(unittest.TestCase):
    def affine(self, A, B):
        c0, c1, c2, c3, c4, c5 = A[0], A[1], A[2], A[3], A[4], A[5]
        C = Affine(c0, c1, c2, c3, c4, c5)
        self.assertEqual(C, A)
        E = Affine.identity()
        self.assertEqual(C, C*E)
        self.assertEqual(E*B, B)
        self.assertEqual(E.det(), 1)

        self.assertAlmostEqual(A.det(), c0*c3-c1*c2)
        self.assertAlmostEqual(abs(A.det()), A.descrim2())
        self.assertAlmostEqual(abs(A.det())**0.5, A.descrim())
        #xor
        self.assertFalse( A.flips() ^ (A.det() < 0) )

        if A.isSingular():
            self.assertAlmostEqual(A.det(), 0)
        else:
            self.assertTrue( Affine.are_near (A*A.inverse(), E) )
            self.assertAlmostEqual(A.det(), 1/A.inverse().det())
        self.assertEqual( A.xAxis(), Point(c0, c1) )
        self.assertEqual( A.yAxis(), Point(c2, c3) )
        self.assertEqual( A.translation(), Point(c4, c5) )

        self.assertAlmostEqual(A.expansionX(), A.xAxis().length())
        self.assertAlmostEqual(A.expansionY(), A.yAxis().length())
        
        if abs(A.expansionX()) > 1e-7 and abs(A.expansionY()) > 1e-7:
            A.setExpansionX(2)
            A.setExpansionY(3)
            self.assertAlmostEqual(A.expansionX(), 2)
            self.assertAlmostEqual(A.expansionY(), 3)

        A.setIdentity()

        self.assertTrue(A.isIdentity())
        self.assertTrue(A.isTranslation())
        self.assertFalse(A.isNonzeroTranslation())
        self.assertTrue(A.isScale())
        self.assertTrue(A.isUniformScale())
        self.assertFalse(A.isNonzeroScale())
        self.assertFalse(A.isNonzeroUniformScale())
        self.assertTrue(A.isRotation())
        self.assertFalse(A.isNonzeroRotation())
        self.assertTrue(A.isHShear())
        self.assertTrue(A.isVShear())
        self.assertFalse(A.isNonzeroHShear())
        self.assertFalse(A.isNonzeroVShear())
        self.assertTrue(A.isZoom())

        self.assertTrue(A.preservesArea() and A.preservesAngles() and A.preservesDistances())

        self.assertFalse( A.flips() )
        self.assertFalse( A.isSingular() )

        A.setXAxis(Point(c0, c1))
        A.setYAxis(Point(c2, c3))

        self.assertEqual(A.withoutTranslation(), A)

        A.setTranslation(Point(c4, c5))
        self.assertEqual(C, A)

        self.assertAlmostEqual( (A*B).det(), A.det()*B.det() )

        self.assertEqual( A.translation(), Point()*A )
        self.assertEqual( Point(1, 1)*A, Point( c0+c2+c4, c1+c3+c5 ))

        l = Line(Point(1, 1), 2)
        self.assertEqual( (l.transformed(A)).origin(), l.origin()*A )
        self.assertTrue( Line.are_near( l.pointAt(3)*A, l.transformed(A) ) )

        r = Ray(Point(2, 3), 4)
        self.assertEqual( (r.transformed(A)).origin(), r.origin()*A )
        self.assertTrue( Ray.are_near( r.pointAt(3)*A, r.transformed(A) ) )

        

    def test_affine(self):
        al = []
        for i in range(10):
            al.append(Affine(   uniform(-10, 10),
                                uniform(-10, 10),
                                uniform(-10, 10),
                                uniform(-10, 10),
                                uniform(-10, 10),
                                uniform(-10, 10)))
        for A in al:
            for B in al:
                self.affine(A, B)

        o = Point(2, 4)
        v = Point(-1, 1)/sqrt(2)
        l = Line.from_origin_and_versor(o, v)
        
        R = Affine.reflection(v, o)
        for i in range(100):
            p = Point(randint(0, 100), randint(0, 100))
            self.assertAlmostEqual(Line.distance(p, l), Line.distance(p*R, l))
        self.assertTrue( Affine.are_near( R, R.inverse() ) )
        
        self.affine(R, R.inverse())
        
    def test_translate(self):
        T = Translate()
        U = Translate(Point(2, 4))
        V = Translate(1, -9)

        self.assertTrue(Affine(T).isTranslation())
        self.assertTrue(Affine(U).isNonzeroTranslation())

        self.assertEqual( (U*V).vector(), U.vector()+V.vector() )
        self.assertEqual( U.inverse().vector(), -U.vector() )
        self.assertEqual(T, Translate.identity())
        self.assertEqual( U.vector(), Point(U[0], U[1]) )

        self.affine(Affine(V), Affine(U))
        self.affine(Affine(U), Affine(V))

        r = Rect( Point(0, 2), Point(4, 8) )

        self.assertEqual( ( r*(U*V) ).min(), r.min()+U.vector()+V.vector())

    def test_scale(self):
        S = Scale()
        T = Scale( Point (3, 8) )
        U = Scale( -3, 1)
        V = Scale(sqrt(2))

        self.assertTrue( Affine(T).isScale() )
        self.assertTrue( Affine(T).isNonzeroScale() )
        self.assertTrue( Affine(V).isNonzeroUniformScale())

        self.assertEqual( (T*V).vector(), T.vector()*sqrt(2) )
        self.assertEqual( (T*U)[0], T[0]*U[0] )
        self.assertAlmostEqual( 1/U.inverse()[1], U[1] )

        r = Rect( Point(0, 2), Point(4, 8) )
        self.assertAlmostEqual((r*V).area(), 2*r.area())
        self.assertFalse(Affine(U).preservesArea())
        self.assertTrue(Affine(V).preservesAngles())
        
        self.affine(Affine(T), Affine(U))
        self.affine(Affine(U), Affine(V))
        self.affine(Affine(V), Affine(T))

    def test_rotate(self):
        R = Rotate()
        S = Rotate(pi/3)
        T = Rotate(Point( 1, 1 ))
        U = Rotate( -1, 1 )

        self.assertTrue(S.vector(), Point(cos(pi/3), sin(pi/3)) )
        self.assertEqual( Point(T[0], T[1]), T.vector() )
        self.assertTrue( Affine.are_near( Rotate.from_degrees(60), S ) )
        self.assertEqual(R, Rotate.identity())
        self.assertTrue( Point.are_near(  ( S * T ).vector(),
        Point( cos( pi/3 + pi/4 ), sin( pi/3 + pi/4 ) ) ) )

        self.affine( Affine(R), Affine(S))
        self.affine( Affine(S), Affine(T))
        self.affine( Affine(T), Affine(U))
        self.affine( Affine(U), Affine(R))

    def test_shear(self):
        H = HShear(2.98)
        V = VShear(-sqrt(2))

        self.assertAlmostEqual(H.factor(), 2.98)
        self.assertAlmostEqual(V.inverse().factor(), sqrt(2))

        G = HShear.identity()
        H.setFactor(0)
        self.assertEqual(G, H)
        
        G.setFactor(2)
        H.setFactor(4)
        self.assertAlmostEqual((G*H).factor(), G.factor()+H.factor())

        W = VShear.identity()
        V.setFactor(0)
        self.assertEqual(W, V)
        
        W.setFactor(-2)
        V.setFactor(3)
        self.assertAlmostEqual((W*V).factor(), W.factor()+V.factor())
        
    def test_zoom(self):
        Z = Zoom(3)
        Y = Zoom(Translate(3,2))
        X = Zoom(sqrt(3), Translate(-1, 3))
        
        self.assertEqual( 
            Zoom(Z.scale(), Translate(Y.translation())),
            Y*Z )
            
        Z.setTranslation(Y.translation())
        Y.setScale(Z.scale())
        self.assertEqual(Z, Y)
        
        self.assertEqual(Y.inverse().scale(), 1/Y.scale())
        
        r = Rect.from_xywh( 1, 1, 3, 6) 
        q = Rect.from_xywh( 0, -1, 1, 2)
        W = Zoom.map_rect(r, q)

        self.assertAlmostEqual(W.scale()*r.width(), q.width())
        self.assertTrue(Point.are_near( 
            r.min()+W.translation(), 
            q.min()))
    def test_eigen(self):
        #TODO looks like bug in eigen - (1, 0) should be eigenvector too
        #~ S = Scale(1, 2)
        #~ E_S = Eigen(S)
        #~ print E_S.vectors, E_S.values
        #~ print Affine(S)
        #~ for i in E_S.vectors:
            #~ print i, i*S, Point(1, 0) * S
        
        B = Affine(-2, 2, 2, 1, 0, 0)
        G1 = Eigen(B)
        G2 = Eigen( [[-2, 2], [2, 1]] )
        
        self.assertAlmostEqual(min(G1.values), min(G2.values))
        self.assertAlmostEqual(max(G1.values), max(G2.values))
        
        if Point.are_near( G1.vectors[0]*G1.values[0], G1.vectors[0]*B ):
            self.assertTrue( Point.are_near( G1.vectors[1]*G1.values[1], G1.vectors[1]*B ) )
        else:
            self.assertTrue( Point.are_near( G1.vectors[1]*G1.values[0], G1.vectors[1]*B ) )
            self.assertTrue( Point.are_near( G1.vectors[0]*G1.values[1], G1.vectors[0]*B ) )
            
unittest.main()
