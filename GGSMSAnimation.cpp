#include "stdafx.h"
#include "GGSMSAnimation.h"
#include "WriteUtils.h"
#include <fstream>
#include <algorithm>
#include <sstream>

namespace sms
{

GGAnimation::GGAnimation(LPVOID galeFileHandle, const Options& options, AnimationProperties& animationProperties)
: m_galeFileHandle(galeFileHandle),
  m_options(options),
  m_animationProperties(animationProperties)
{
    HBITMAP bitmap = ggGetBitmap(m_galeFileHandle, 0, 0);

    if (bitmap == NULL)
    {
		printf("Error retrieving bitmap data");
		exit(-1);
    }

    if (GetObject(bitmap, sizeof(BITMAP), &m_generalBitmapInfo) == 0)
    {
        printf("BitmapInfo is NULL\n");
		exit(-1);
    }

    unsigned int numberOfFrames = ggGetFrameCount(m_galeFileHandle);
	m_frames.resize(numberOfFrames);

    m_maxTilesInFrame = 0;

    for (DWORD loop = 0; loop < numberOfFrames; loop++)
    {
		GGAnimationFrame& frame = m_frames[loop];

		frame.Init(loop, 
                   m_galeFileHandle, 
                   m_tileStore, 
                   m_options, 
                   m_animationProperties);

        m_maxTilesInFrame = max(m_maxTilesInFrame, frame.getSprites().size() * 2);

		m_totalFrameTime += frame.GetFrameDelayTime();

        if (loop > 0)
        {
            int previousIndex = loop - 1;

            GGAnimationFrame& previousFrame = m_frames[previousIndex];

            if (previousFrame.getNextFrameIndex() == NEXT_FRAME_NOT_SET)
            {
                if (frame.startsAnimation())
                {
                    int animationStartIndex;

                    auto it = m_animationProperties.animationFrameNames.lower_bound(loop);

                    if (it != m_animationProperties.animationFrameNames.begin()) 
                    {
                        --it; // Move the iterator one step back to get the closest key
                        animationStartIndex = it->first;
                    } 
                    else 
                    {
                        animationStartIndex = 0;
                    }

                    previousFrame.setNextFrameIndex(animationStartIndex);
                }
                else
                {
                    previousFrame.setNextFrameIndex(loop);
                }
            }
        }
    }

    int lastFrameIndex = m_frames.size() - 1;

    if (m_frames[lastFrameIndex].getNextFrameIndex() == NEXT_FRAME_NOT_SET)
    {
        if (!m_frames[lastFrameIndex].startsAnimation())
        {
            auto it = m_animationProperties.animationFrameNames.lower_bound(lastFrameIndex);

            if (it != m_animationProperties.animationFrameNames.begin()) 
            {
                --it; // Move the iterator one step back to get the closest key
                m_frames[lastFrameIndex].setNextFrameIndex(it->first);
            } 
            else 
            {
                m_frames[lastFrameIndex].setNextFrameIndex(0);
            }
        }
        else
        {
            m_frames[lastFrameIndex].setNextFrameIndex(lastFrameIndex);
        }
    }
}

void GGAnimation::Write(const std::string& outputFolder, const std::string& outputName)
{
	WriteGGAnimationHeaderFile(outputFolder, outputName);
	WriteGGAnimationSourceFile(outputFolder, outputName);
}

std::string str_toupper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::toupper(c); } // correct
                  );
    return s;
}


void GGAnimation::WriteGGAnimationHeaderFile(const std::string& outputFolder, const std::string& outputName)
{
	std::string headerFilename = outputName + ".h";
	std::ofstream headerfile(outputFolder + headerFilename, std::ios::trunc);

    // header guard
    std::string headerGuard = outputName + "_ANIMATION_INCLUDE_H";
    std::transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);
    headerfile << "// File generated by gg2c. https://github.com/pw32x/gg2c\n";
    headerfile << "#ifndef " << headerGuard << "\n";
    headerfile << "#define " << headerGuard << "\n";
    headerfile << "\n";

	// includes
    headerfile << "#include \"animation_types.h\"\n";
    headerfile << "#include \"resource_types.h\"\n";
    headerfile << "\n";

	// exported types

    std::string streamed = m_options.mStreamed ? "Streamed" : "";

    if (m_options.mSMSBatchedSprites)
        headerfile << "extern const AnimationBatched" << streamed << " " << outputName << ";\n"; 
    else
        headerfile << "extern const Animation" << streamed << " " << outputName << ";\n"; 

    headerfile << "\n";

    headerfile << "// frame numbers for specific animations.\n";
    for (const auto& pair : m_animationProperties.animationFrameNames) 
	{
		std::transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);
		headerfile << "#define " << str_toupper(outputName) << "_" << str_toupper(pair.second) << "_FRAME_INDEX" << " " << pair.first << "\n";
    }
    headerfile << "\n";

    // end header guard
    headerfile << "#endif\n\n";
    headerfile.close();
}


// TODO
// Write block of Y first, then block of XN
// write sprites as one big block. frames have index into it.
void GGAnimation::WriteSprites(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t frameLoop = 0; frameLoop < m_frames.size(); frameLoop++)
	{
		const GGAnimationFrame& frame = m_frames[frameLoop];

        std::string spriteName = BuildFrameName(outputName, frameLoop) + "Sprites";

        sourceFile << "const AnimationSprite " << spriteName << "[] = \n";
        sourceFile << "{\n";

        int tileStoreModifier = 0;

        if (m_options.mStreamed)
        {
            tileStoreModifier = frame.getSprites().begin()->tileStoreIndex;
        }

        for (const auto& sprite : frame.getSprites())
        {
            sourceFile << "    { ";
            sourceFile << sprite.xPositionOffset - m_animationProperties.mOffsetX << ", ";
            sourceFile << sprite.yPositionOffset - m_animationProperties.mOffsetY << ", ";
            sourceFile << sprite.tileStoreIndex - tileStoreModifier;
            sourceFile << " },\n";
        }

        sourceFile << "};\n\n";
    }
}

void GGAnimation::WriteSpritesBatched(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t frameLoop = 0; frameLoop < m_frames.size(); frameLoop++)
	{
		const GGAnimationFrame& frame = m_frames[frameLoop];

        std::string spriteName = BuildFrameName(outputName, frameLoop) + "SpriteBatched";

        sourceFile << "const AnimationSpriteBatched " << spriteName << "[] = \n";
        sourceFile << "{\n";

        int tileStoreModifier = 0;

        if (m_options.mStreamed)
        {
            tileStoreModifier = frame.getAdjoiningSprites().begin()->sprite->tileStoreIndex;
        }

        for (const auto& adjoiningSprite : frame.getAdjoiningSprites())
        {
            sourceFile << "    { ";
            sourceFile << adjoiningSprite.adjoiningCount << ", ";
            sourceFile << "{ ";
            sourceFile << adjoiningSprite.sprite->xPositionOffset - m_animationProperties.mOffsetX << ", ";
            sourceFile << adjoiningSprite.sprite->yPositionOffset - m_animationProperties.mOffsetY << ", ";
            sourceFile << adjoiningSprite.sprite->tileStoreIndex - tileStoreModifier;
            sourceFile << " }";
            sourceFile << " },\n";
        }
        sourceFile << "    {0},\n";
        sourceFile << "};\n\n";
    }
}

void GGAnimation::WriteFramesBatched(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t frameLoop = 0; frameLoop < m_frames.size(); frameLoop++)
	{
		const GGAnimationFrame& frame = m_frames[frameLoop];
        std::string frameName = BuildFrameName(outputName, frameLoop);

        if (m_options.mStreamed)
            sourceFile << "extern const AnimationFrameBatchedStreamed " << frameName << ";\n";
        else
            sourceFile << "extern const AnimationFrameBatched " << frameName << ";\n";
	}

    sourceFile << "\n";

	for (size_t frameLoop = 0; frameLoop < m_frames.size(); frameLoop++)
	{
		const GGAnimationFrame& frame = m_frames[frameLoop];

        std::string frameName = BuildFrameName(outputName, frameLoop);
        std::string nextFrameName;
        

        if (frame.getNextFrameIndex() == NO_LOOP)
            nextFrameName = "NULL";
        else
            nextFrameName = "&" + BuildFrameName(outputName, frame.getNextFrameIndex());

		sourceFile << "\n";

        if (m_options.mStreamed)
            sourceFile << "const AnimationFrameBatchedStreamed " << frameName << " = \n";
        else
		    sourceFile << "const AnimationFrameBatched " << frameName << " = \n";

		sourceFile << "{\n";
        sourceFile << "    " << frameName << "SpriteBatched,\n";

        if (m_options.mStreamed)
        {
            int tileIndex = frame.getSprites().begin()->tileStoreIndex;
            sourceFile << "    " << tileIndex << ", // tile index\n"; 
        }

		sourceFile << "    " << frame.GetFrameDelayTime() << ", // frame time\n"; 
        sourceFile << "    " << nextFrameName << ", // next frame\n";
		sourceFile << "};\n";
	}
}


void GGAnimation::WriteFrames(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t frameLoop = 0; frameLoop < m_frames.size(); frameLoop++)
	{
		const GGAnimationFrame& frame = m_frames[frameLoop];
        std::string frameName = BuildFrameName(outputName, frameLoop);

        if (m_options.mStreamed)
            sourceFile << "extern const AnimationFrameStreamed " << frameName << ";\n";
        else
            sourceFile << "extern const AnimationFrame " << frameName << ";\n";
	}

	for (size_t frameLoop = 0; frameLoop < m_frames.size(); frameLoop++)
	{
		const GGAnimationFrame& frame = m_frames[frameLoop];

        std::string frameName = BuildFrameName(outputName, frameLoop);
        std::string nextFrameName;
        

        if (frame.getNextFrameIndex() == NO_LOOP)
            nextFrameName = "NULL";
        else
            nextFrameName = "&" + BuildFrameName(outputName, frame.getNextFrameIndex());

		sourceFile << "\n";


        if (m_options.mStreamed)
            sourceFile << "const AnimationFrameStreamed " << frameName << " = \n";
        else
		    sourceFile << "const AnimationFrame " << frameName << " = \n";


		sourceFile << "{\n";
        sourceFile << "    " << frameName << "Sprites,\n";
		sourceFile << "    " << frame.getSprites().size() << ", // number of sprites\n";

        if (m_options.mStreamed)
        {
            int tileIndex = frame.getSprites().begin()->tileStoreIndex;
            sourceFile << "    " << tileIndex << ", // tile index\n"; 
        }

		sourceFile << "    " << frame.GetFrameDelayTime() << ", // frame time\n"; 
        sourceFile << "    " << nextFrameName << ", // next frame\n";
		sourceFile << "};\n";
	}
}

void GGAnimation::WriteFrameArrayBatched(const std::string& outputName, std::ofstream& sourceFile)
{
    if (m_options.mStreamed)
        sourceFile << "const AnimationFrameBatchedStreamed* const " << outputName << "Frames[" << m_frames.size() << "] = \n";
    else
        sourceFile << "const AnimationFrameBatched* const " << outputName << "Frames[" << m_frames.size() << "] = \n";
    sourceFile << "{\n";

    for (size_t loop = 0; loop < m_frames.size(); loop++)
    {
        sourceFile << "    &" << BuildFrameName(outputName, loop) << ",\n";
    }

    sourceFile << "};\n\n";
}


void GGAnimation::WriteFrameArray(const std::string& outputName, std::ofstream& sourceFile)
{
    if (m_options.mStreamed)
        sourceFile << "const AnimationFrameStreamed* const " << outputName << "Frames[" << m_frames.size() << "] = \n";
    else
        sourceFile << "const AnimationFrame* const " << outputName << "Frames[" << m_frames.size() << "] = \n";
    sourceFile << "{\n";

    for (size_t loop = 0; loop < m_frames.size(); loop++)
    {
        sourceFile << "    &" << BuildFrameName(outputName, loop) << ",\n";
    }

    sourceFile << "};\n\n";
}


void GGAnimation:: WriteAnimationStructBatched(const std::string& outputName, std::ofstream& sourceFile)
{
    sourceFile << "u8 " << outputName << "VdpLocation;\n\n";

    if (m_options.mStreamed)
    {
        // final struct
        sourceFile << "const AnimationBatchedStreamed " << outputName << " = \n";
        sourceFile << "{\n";
        sourceFile << "    BATCHED_STREAMED_ANIMATION_RESOURCE_TYPE, \n";
        sourceFile << "    (const AnimationFrameBatchedStreamed** const)" << outputName << "Frames,\n";
    }
    else
    {
        // final struct
        sourceFile << "const AnimationBatched " << outputName << " = \n";
        sourceFile << "{\n";
        sourceFile << "    BATCHED_ANIMATION_RESOURCE_TYPE, \n";
        sourceFile << "    (const AnimationFrameBatched** const)" << outputName << "Frames,\n";
    }


    sourceFile << "    (unsigned char* const)" << outputName << "TileData, // start of the sprite data\n";
    sourceFile << "    " << m_frames.size() << ", // number of frames\n";
    sourceFile << "    " << m_generalBitmapInfo.bmWidth << ", // width in pixels\n";
    sourceFile << "    " << m_generalBitmapInfo.bmHeight << ", // height in pixels\n";
    sourceFile << "    " << m_tileStore.size() << ", // the total amount of tiles in animation\n";

    if (m_options.mStreamed)
        sourceFile << "    " << m_maxTilesInFrame << ", // the max amount of sprite tiles in a frame\n";    

    sourceFile << "    &" << outputName << "VdpLocation,\n";
    sourceFile << "};\n";
}


void GGAnimation:: WriteAnimationStruct(const std::string& outputName, std::ofstream& sourceFile)
{
    sourceFile << "u8 " << outputName << "VdpLocation;\n\n";

    if (m_options.mStreamed)
    {
        // final struct
        sourceFile << "const AnimationStreamed " << outputName << " = \n";
        sourceFile << "{\n";
        sourceFile << "    REGULAR_STREAMED_ANIMATION_RESOURCE_TYPE, \n";
        sourceFile << "    (const AnimationFrameStreamed** const)" << outputName << "Frames,\n";
    }
    else
    {
        // final struct
        sourceFile << "const Animation " << outputName << " = \n";
        sourceFile << "{\n";
        sourceFile << "    REGULAR_ANIMATION_RESOURCE_TYPE, \n";
        sourceFile << "    (const AnimationFrame** const)" << outputName << "Frames,\n";
    }
    sourceFile << "    (unsigned char* const)" << outputName << "TileData, // start of the sprite data\n";
    sourceFile << "    " << m_frames.size() << ", // number of frames\n";
    sourceFile << "    " << m_generalBitmapInfo.bmWidth << ", // width in pixels\n";
    sourceFile << "    " << m_generalBitmapInfo.bmHeight << ", // height in pixels\n";
    sourceFile << "    " << m_tileStore.size() << ", // the total amount of tiles in animation\n";

    if (m_options.mStreamed)
        sourceFile << "    " << m_maxTilesInFrame << ", // the max amount of sprite tiles in a frame\n";    

    sourceFile << "    &" << outputName << "VdpLocation,\n";
    sourceFile << "};\n";
}


void GGAnimation::WriteGGAnimationSourceFile(const std::string& outputFolder, const std::string& outputName)
{
	std::ofstream sourceFile(outputFolder + outputName + ".c");

    // includes
    sourceFile << "#include \"" << outputName << ".h\"\n";
	
    sourceFile << "\n";
    sourceFile << "\n";

	// tile data
	WriteTileStore(outputName, sourceFile, m_tileStore);

    if (m_options.mSMSBatchedSprites)
    {
        WriteSpritesBatched(outputName, sourceFile);
        WriteFramesBatched(outputName, sourceFile);
	    WriteFrameArrayBatched(outputName, sourceFile);
        WriteAnimationStructBatched(outputName, sourceFile);
    }
    else
    {
        WriteSprites(outputName, sourceFile);
	    WriteFrames(outputName, sourceFile);
        WriteFrameArray(outputName, sourceFile);
        WriteAnimationStruct(outputName, sourceFile);
    }

    sourceFile.close();
}

/*
#include "player.h"


unsigned short const playerTileData[32] = 
{
// tile: 0
    0x0000, 0x22,
    0x0000, 0x2ee,
    0x0000, 0x2eee,
    0x0000, 0x29e9,
    0x0002, 0x9ee6,
    0x0002, 0xee66,
    0x002e, 0xfe6e,
    0x002e, 0xfe9c,

	[...]

};

const Sprite playerFrame0Sprites[] =
{
	x, y, n,
	x, y, n,
	[...]
};

const Sprite playerFrame0XFlippedSprites[] =
{
	x, y, n,
	x, y, n,
	[...]
};

(align to 4)
const AnimationFrame playerFrame0 =
{
	playerFrame0Sprites
	0, // frame time
	X, // num sprites
}

(align to 4)
const AnimationFrame playerFrame0XFlipped =
{
	playerFrame0XFlippedSprites
	0, // frame time
	X, // num sprites

}

const AnimationFrames* playerFrames[] = // need pointers???
{
	playerFrame0,
	playerFrame0XFlipped
};

(align to 4)
const Animation player_animation =
{
    playerFrames,										(unsigned int)
    (u32*)playerTileData, // start of the sprite data	(unsigned int)
    2, // number of frames								(unsigned char)
    24, // width in pixels								(unsigned char)
    32, // height in pixels								(unsigned char)
    12, // max tiles per frame							(unsigned char)
    20, // the total number of tiles in the animation	(unsigned char)
    118, // the total time of the animation				(unsigned short)
};



*/

}