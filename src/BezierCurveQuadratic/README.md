# BezierCurveQuadratic

![screenshot](screenshot.png)

# Additional

See the plotted of each term multiplying with each input point of polynomial term, its sum is equal to
1.

From the quadratic equation

```
V(t) = (1-t)^2*P0 + 2*(1-t)*P1 + (t^2)*P2
```

wheres

* `b0 = (1-t)^2`
* `b1 = 2*(1-t)`
* `b2 = (t^2)`
* `P0` and `P2` are the starting and ending point influencing the interior of the curve
* `P1` is the controling point

![summation of b0 b1 b2](summation.png)
