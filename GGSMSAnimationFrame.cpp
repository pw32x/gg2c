#include "stdafx.h"
#include "GGSMSAnimationFrame.h"
#include "BitmapUtils.h"
#include "SpriteUtils.h"

namespace sms
{

GGAnimationFrame::GGAnimationFrame() 
: mFrameDelayTime(-1)
, mFrameNumber(-1)
{
}

void GGAnimationFrame::Init(int frameNumber, 
							LPVOID galeFile, 
							std::vector<Tile>& tileStore, 
							const Options& options, 
							AnimationProperties& animationProperties)
{
	mFrameNumber = frameNumber;
	GetGGInfo(galeFile, animationProperties);
	BuildFrame(galeFile, tileStore, m_sprites, options);
}

void GGAnimationFrame::GetGGInfo(LPVOID galeFile, AnimationProperties& animationProperties)
{
	LONG ggFrameDelayTime = ggGetFrameInfo(galeFile, mFrameNumber, 2); // the 2 means frame time?
	mFrameDelayTime = (LONG)(myround((float)ggFrameDelayTime / 17.0f)); // 17 ms per frame

	const int length =  256;
	char frameName[length];
	ggGetFrameName(galeFile, mFrameNumber, frameName, length);

	if (strstr(frameName, "ANIMPROP_"))
	{
		char* token = strtok(frameName, " "); 

		if (strcmp(frameName, "ANIMPROP_OFFSET") == 0)
		{
			token = strtok(NULL, " "); 
			animationProperties.mOffsetX = atoi(token);

			token = strtok(NULL, " "); 
			animationProperties.mOffsetY = atoi(token);
		}
	}
}


int FileTileInStore(std::vector<Tile>& tileStore, 
					const Tile& tileToFind)
{
	for (size_t loop = 0; loop < tileStore.size(); loop++)
	{
		const Tile& storedTile = tileStore[loop];

		if (std::equal(storedTile.begin(), storedTile.end(), tileToFind.begin()))
		{
			return loop;
		}
	}

	// Tile didn't exist already so create one.
	tileStore.push_back(tileToFind);

	return tileStore.size() - 1;
}

void SliceImageIntoTiles(BYTE* byteData, 
						 int width, 
						 int height, 
						 int topMost, 
						 int bottomMost, 
						 std::vector<Tile>& tileStore, 
						 std::vector<Sprite>& sprites,

						 bool sliceSpritesOnGrid)
{

	// SMS tiles are always 8x8
	int sliceWidth = 8;
	int sliceHeight = 8;

    int rectHeight = (bottomMost - topMost);	

    int numberOfSlices = 0;
    if (rectHeight % sliceHeight != 0)
    {
        numberOfSlices = (rectHeight / sliceHeight) + 1;
    }
    else
    {
        numberOfSlices = (rectHeight / sliceHeight);
    }


    // cut the image into slices, then sprites.
    for (int sliceLoop = 0; sliceLoop < numberOfSlices; sliceLoop++)
    {
        // find left and right extents for slice.
        int sliceTop = (sliceLoop * sliceHeight) + topMost;
        int sliceBottom = sliceTop + sliceHeight;
        if (sliceBottom > bottomMost)
        {
            sliceBottom = bottomMost;
        }

        int leftMost;
        int rightMost;

		FindLeftRightExtentsForSlice(byteData, width, sliceTop, sliceBottom, leftMost, rightMost, sliceSpritesOnGrid);
		
        //printf("\nLeft/Right Extents - Left: %d, Right: %d\n", leftMost, rightMost);

        int rectWidth = (rightMost - leftMost);

        // no pixels detected, just skip.
        if (rectWidth < 0)
        {
            continue;
        }

        int maxNumberOfTilesInSlice = 0;
        
        if (rectWidth % sliceWidth != 0)
        {
            maxNumberOfTilesInSlice = (rectWidth / sliceWidth) + 1;
        }
        else
        {
            maxNumberOfTilesInSlice = (rectWidth / sliceWidth);
        }

        for (int tileLoop = 0; tileLoop < maxNumberOfTilesInSlice; tileLoop++)
        {
			// Start with a full sliceWidth x sliceHeight area.
            int startPositionX = leftMost + (tileLoop * sliceWidth);
            int endPositionX = startPositionX + sliceWidth;

            if (endPositionX > rightMost)
            {
                endPositionX = rightMost;
            }

            int startPositionY = sliceTop;
            int endPositionY = sliceBottom;

            if (endPositionY > bottomMost)
            {
                endPositionY = bottomMost;
            }

			// Get the sprite. This also modifies the start and end positions to the area actually copied.
			std::vector<BYTE> tileData;
			bool atLeastOnePixel = CopySpriteFromByteData(byteData, width, tileData, startPositionX, startPositionY, endPositionX, endPositionY, sliceSpritesOnGrid);

            if (!atLeastOnePixel)
            {
				continue;
			}

			// See if the sprite already exists.
			int tileStoreIndex = FileTileInStore(tileStore, tileData);

			// Create tile properties
			Sprite sprite;
			sprite.tileStoreIndex = tileStoreIndex;
			sprite.xPositionOffset = startPositionX;
			sprite.yPositionOffset = startPositionY;

			sprites.push_back(sprite);
        }
    }
}


void GGAnimationFrame::BuildFrame(LPVOID galeFile, 
									 std::vector<Tile>& tileStore, 
									 std::vector<Sprite>& sprites, 
									 const Options& options)
{
	HBITMAP				hBitmap;
	BITMAP				bitmapInfo;

	hBitmap = ggGetBitmap(galeFile, mFrameNumber, 0);
	GetObject(hBitmap, sizeof(BITMAP), &bitmapInfo);
	BYTE* byteData = CreateByteDataFromBitmap(bitmapInfo);

	int topMost;
	int bottomMost;

	FindTopAndBottomExtents(byteData, 
							bitmapInfo.bmWidth, 
							bitmapInfo.bmHeight, 
							&topMost, 
							&bottomMost, 
							options.mSliceSpritesOnGrid);

	SliceImageIntoTiles(byteData, 
						bitmapInfo.bmWidth, 
						bitmapInfo.bmHeight, 
						topMost, 
						bottomMost, 
						tileStore, 
						sprites, 
						options.mSliceSpritesOnGrid);

	delete [] byteData;
}
}