# Lab4 - Cubemapping

## Assignment1:

The quad is rendered as a strip of triangles.
So if we have the vertrices: v = [A, B, C, D, E, F],
it will render the triangles with the algorithm:
 for i in 2.. |v|
    draw_triangle(v[i-2], v[i-1], [i])

Why?
+ space efficiency! no indices need to be specified.

However: not faster than when you specify indices manually.

## Assignment2:

Code...