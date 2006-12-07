#!/usr/bin/python

import lib2geom_py as g

a = g.Point(1,2)
b = g.Point(31,2)
print a, b

point_fns_1 = ["L1", "L2", "L2sq", "LInfty", "is_zero", "is_unit_vector",
             "atan2", "rot90", 
             "unit_vector", "abs"]
point_fns_2 = ["dot", "angle_between", "distance", "dist_sq", "cross"]

for i in point_fns_1:
    print "%s:" % i, g.__dict__[i](a)
for i in point_fns_2:
    print "%s:" % i, g.__dict__[i](a,b)
print "point_equalp:", g.point_equalp(a,b, 100)
print "Lerp:", g.Lerp(0.3, a,b)

bo = g.BezOrd(2,3)
print bo
print bo.point_at(0.3)

print bo.reverse()
