# Lab2 Answers

## Assignment1: 

+ Saving space / compression / reuse
+ Easier to specify data.
    * Easier to use other geometric shapes.

## Assignment2:

+ field of view, fov = angle of the camera / viewing field.
+ aspect ratio = width / height.
+ near plane = nothing visible before.
    
You don't need a far clipping plane but always a near clipping plane. It's a perspective projection with the vanishing point is your camera position. When the near-clipping plane distance would be set to 0 your whole scene would be projected onto a single point.

The mathematical background is that when setting up the projection matrix you have to calculate this:

    -(2 * zFar * zNear) / (zFar - zNear)

When zNear is 0 all z values would map to 0.

+ far plane = nothing visible after.
    * performance benefits: don't render until infinity in Z-axis.
    * |far - near| -> 0    ==>  the higher the precision of Z-axis coordinates.

## Anisotropy:

Far away repeats of the texture are rendered with
a minified version of the texture, using the average.

Result: far away pixels are less blurry, more sharp.

In layman's terms, anisotropic filtering retains the "sharpness" of a texture normally lost by MIP map texture's attempts to avoid aliasing. Anisotropic filtering can therefore be said to maintain crisp texture detail at all viewing orientations while providing fast anti-aliased texture filtering.