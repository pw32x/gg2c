# gg2c
gg2c is a command line tool that exports 16 color (4 bits per pixel) Graphics Gale files (.gal) to C source files. It's been used for Sega Genesis homebrew development.

# Limitations

Works with 16 color images only


# Export Options

There are different options when exporting Graphics Gale file to C source files. Options are specified as part of the source Graphic Gale's filename.

* noloop: Every frame of animation has a pointer to the next frame. This sets the next frame of the last frame to NULL. Meant for animations that are more "fire and forget", like explosions. \
   Example: **explosion.noloop.gal**
  
* sliceongrid<W>x<H> : Specifies to use a set grid to slice the frame into sprites of equal size instead of its default slice algorithm which tries to use fewer, smaller sprites with less empty space. Sliceongrid can also better detect when sliced sprites are mirrored versions of each other and reduces the sprite tiles generated.\
   Example: **explosion.sliceongrid16x16.gal**
* cutsliceasframe : used in coordination with sliceongrid, it tells the exporter to take the sliced sprites and export them as frames. For example, a font texture might be stored on just one frame, but you want each letter to be exported as individual frames.\
   Example: **font.sliceongrid8x8.cutslicesasframe.gal**
 * planeanim: specifies that the animation doesn't use sprites, but is meant to use background tiles and animate background planes.\
    Example: **backgrondanimation.planeanim.gal**
# Examples
   Examples can be found in the **testdata** folder
   
# Per-Frame Options
   
   In Graphics Gale, you can set the name of individual frames. gg2c lets you use a few different names to enable different options. \
   * Triggers. A frame of animation can have one or more triggers. A trigger is an id and some data. The only trigger currently supported is:
       * FRAME_TRIGGER_SPAWN <X> <Y> : will fire a trigger with an XY position. 
   * Anim Properties. Anim Properties are used to inform the exporter about certain settings. The only Anim Property currenly supports is:
       * ANIMPROP_OFFSET <X> <Y> : sets the origin of the entire animation. Be default the origin is at the top left corner. This lets you specify somewhere else.

# Loading Exported Tile Data to VDP
   
   In SGDK, use VDP_loadTileData like so:
   VDP_loadTileData(myGGAnimation->allSpriteData, *vdpTileIndex, myGGAnimation->totalTiles, 1);
   
# Working with GGAnimations
   
   In the **types** folder, you'll find .C and .H files that'll help you work with GG Animations in your project. They've been ripped from my projects and removed of any game specific types so they might need a bit of bootstrapping to be useful.
