#ifndef GRAPHICSGALE_ANIMATION_DRAW_HEADER_H
#define GRAPHICSGALE_ANIMATION_DRAW_HEADER_H

#include <genesis.h>
#include "GGAnimationTypes.h"

void GGAnimation_Draw(s16 x, s16 y, const GGAnimation* animation, u16 frameNumber, u16 tileAttribute);
void GGAnimation_DrawNoFlip(s16 x, s16 y, const GGAnimation* animation, u16 frameNumber, u16 tileAttribute);


#endif
