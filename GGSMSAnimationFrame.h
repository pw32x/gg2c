#pragma once

#include "galefile151119\galefile.h"
#include <vector>
#include "Options.h"
#include "Sprite.h"
#include "AnimationProperties.h"

namespace sms
{

typedef std::vector<BYTE> Tile;

#define SMS_TILE_WIDTH 8
#define SMS_TILE_HEIGHT 8

struct Sprite
{
	Sprite() :
		tileStoreIndex(-1),
		xPositionOffset(0),
		yPositionOffset(0)
	{

	}

	int tileStoreIndex;
    int xPositionOffset;
    int yPositionOffset;
};

struct AdjoiningSprite
{
	AdjoiningSprite() : adjoiningCount(0) {}

	int adjoiningCount;
	const Sprite* sprite;
};

class GGAnimationFrame
{
public:
	GGAnimationFrame();

	void Init(int frameNumber, 
			  LPVOID galeFile, 
	          std::vector<Tile>& tileStore, 
			  const Options& options, 
			  AnimationProperties& animationProperties);

	LONG GetFrameDelayTime() const { return mFrameDelayTime; }

	const std::vector<Sprite>& getSprites() const { return m_sprites; }
	const std::vector<AdjoiningSprite>& getAdjoiningSprites() const { return m_adjoiningSprites; }

private:

	void GetGGInfo(LPVOID galeFile, AnimationProperties& animationProperties);
	void BuildFrame(LPVOID galeFile, 
					std::vector<Tile>& tileStore, 
					std::vector<Sprite>& sprites, 
					const Options& options);

	void BuildAdjoiningSprites();

private:
	LONG			mFrameDelayTime;
	int				mFrameNumber;

	std::vector<Sprite> m_sprites;
	std::vector<AdjoiningSprite> m_adjoiningSprites;
};
}