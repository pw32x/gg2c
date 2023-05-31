# gg2c
gg2c is a command line tool that exports 16 color (4 bits per pixel) Graphics Gale files (.gal) to C source files. Used for Sega Genesis homebrew development.

# Limitations

Works with 16 color images only


# Export options

There are different options when exporting Graphics Gale file to C source files. Options are specified as part of the source Graphic Gale's filename.

* noloop: Every frame of animation has a pointer to the next frame. This sets the next frame of the last frame to NULL. Meant for animations that are more "fire and forget", like explosions. \
   Example: **explosion.noloop.gal**
  
* sliceongrid<W>x<H> : Specifies to use a set grid to slice the frame into sprites of equal size instead of its default slice algorithm which tries to use fewer, smaller sprites with less empty space. Sliceongrid can also better detect when sliced sprites are mirrored versions of each other and reduces the sprite tiles generated.\
   Example: **explosion.sliceongrid16x16.gal**
* cutsliceasframe : used in coordination with sliceongrid, it tells the exporter to take the sliced sprites and export them as frames. For example, a font texture might be stored on just one frame, but you want each letter to be exported as individual frames.\
   Example: **font.sliceongrid8x8.cutslicesasframe.gal**
 * planeanim: specifies that the animation doesn't use sprites, but is meant to use background tiles and animate background planes.\
    Example: **backgrondanimation.planeanim.gal**

