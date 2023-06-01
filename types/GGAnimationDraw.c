#include "GGAnimationDraw.h"

// could be missing includes here

void GGAnimation_Draw(s16 x, 
                      s16 y, 
                      const GGAnimation* animation, 
                      u16 frameNumber, 
                      u16 tileAttribute)
{
    const GGFrame* frame = *(animation->frames + frameNumber);
	const GGSprite* const* spritePtr = frame->sprites;
    const GGSprite* sprite;

    u16 loop = frame->numSprites;

    if (spriteDrawIndex + frame->numSprites > MAX_VDP_SPRITE)
    {
        loop = MAX_VDP_SPRITE - spriteDrawIndex;
    }

    for (; loop != 0; loop--)
    {
		sprite = *(spritePtr);

        if (GET_DIRECTION(tileAttribute)== DIRECTION_RIGHT)
        {
            currentSpriteDef->x = x + sprite->xoffset + 0x80;
        }
        else
        {
            currentSpriteDef->x = x + sprite->xFlippedOffset + 0x80;
        }

        currentSpriteDef->y = y + sprite->yoffset + 0x80;
        currentSpriteDef->attribut = tileAttribute + sprite->tileAttribute;
        currentSpriteDef->size = sprite->sgdkSpriteSize;
        spriteDrawIndex++;
        currentSpriteDef->link = spriteDrawIndex;

        currentSpriteDef++;
        spritePtr++;
    }
}

void GGAnimation_DrawNoFlip(s16 x, 
                            s16 y, 
                            const GGAnimation* animation, 
                            u16 frameNumber, 
                            u16 tileAttribute)
{
    const GGFrame* frame = *(animation->frames + frameNumber);

	const GGSprite* const* spritePtr = frame->sprites;
    const GGSprite* sprite;

    u16 loop = frame->numSprites;

    if (spriteDrawIndex + frame->numSprites > MAX_VDP_SPRITE)
    {
        loop = MAX_VDP_SPRITE - spriteDrawIndex;
    }

    for (; loop != 0; loop--)
    {
		sprite = *(spritePtr);

        currentSpriteDef->x = x + sprite->xoffset + 0x80;
        currentSpriteDef->y = y + sprite->yoffset + 0x80;
        currentSpriteDef->attribut = tileAttribute + sprite->tileAttribute;
        currentSpriteDef->size = sprite->sgdkSpriteSize;
        spriteDrawIndex++;
        currentSpriteDef->link = spriteDrawIndex;

        currentSpriteDef++;
        spritePtr++;
    }
}
