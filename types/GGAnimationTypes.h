#ifndef GRAPHICSGALE_ANIMATION_TYPES_HEADER_H
#define GRAPHICSGALE_ANIMATION_TYPES_HEADER_H

#include <stdint.h>

typedef struct
{
    int16_t xoffset;
    int16_t yoffset;
	int16_t xFlippedOffset;
    uint16_t tileAttribute;
    uint16_t sgdkSpriteSize;
} GGSprite;

typedef struct GGFrame
{
    const GGSprite* const* sprites;
    uint8_t numSprites; // no more than 255 sprites
	uint8_t frameTriggerDataOffset; // only 255 entries for trigger data
    uint16_t frameDelayTime;
    const struct GGFrame* nextFrame;
} GGFrame;

typedef struct
{
    const GGFrame* const* frames;	    // pointer to animation frames
    uint16_t numberOfFrames;			// number of frames
    uint16_t width;						// width of the total area of the sprite in pixels
    uint16_t height;					// height of the total area of the sprite in pixels
    uint16_t maxNumberOfTilesPerFrame;	// the number of tiles required to store one frame
    uint16_t totalTiles;				// the total number of tiles in the animation
	uint16_t totalAnimationTime;		// the total time duration of the animation
    uint32_t const* allSpriteData;		// start of the sprite data
	uint16_t const* frameTriggerData;	// data that might be attached to a frame
} GGAnimation;

typedef void (*drawPlaneAnimationFrameFunctionType)(uint16_t plan, uint16_t x, uint16_t y, uint16_t tileIndex);

typedef struct GGPlaneAnimationFrame
{
	drawPlaneAnimationFrameFunctionType drawFrame;	
	uint16_t frameDelayTime;
	const struct GGPlaneAnimationFrame* nextFrame;
} GGPlaneAnimationFrame;

typedef struct
{
    const GGPlaneAnimationFrame* const* frames;	// pointer to animation frames
    uint16_t numberOfFrames;					// number of frames
    uint16_t tileWidth;							// width of the total area of the frame in tiles
    uint16_t tileHeight;						// height of the total area of the frame in tiles
    uint16_t totalTiles;						// the total number of tiles in the animation
    uint32_t const* tileData;					// start of the tile data
} GGPlaneAnimation;

#define DIRECTION_LEFT  1
#define DIRECTION_RIGHT 0

#endif
