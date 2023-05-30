#include "stdafx.h"
#include "GGPlaneAnimation.h"
#include "BitmapUtils.h"
#include "WriteUtils.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>

GGPlaneAnimation::GGPlaneAnimation(LPVOID galeFileHandle, const Options& options)
: m_galeFileHandle(galeFileHandle),
  m_uniqueTileCount(0),
  m_maxUniqueTilesPerFrame(0),
  m_options(options),
  m_numberOfFrames(ggGetFrameCount(m_galeFileHandle))
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

	m_frames.resize(m_numberOfFrames);

    for (DWORD loop = 0; loop < m_numberOfFrames; loop++)
    {
		GGPlaneAnimationFrame& frame = m_frames[loop];

		frame.Init(loop, m_galeFileHandle, m_tiles, m_options, m_uniqueTileCount, m_maxUniqueTilesPerFrame);

		if (findSameFrame(frame) == nullptr)
		{
			m_uniqueFrameData.push_back(&frame);
		}
    }
}

GGPlaneAnimationFrame* GGPlaneAnimation::findSameFrame(const GGPlaneAnimationFrame& frame)
{
	for (auto uniqueFrameData : m_uniqueFrameData)
	{
		if (uniqueFrameData->hasSameFrameData(&frame))
		{
			return uniqueFrameData;
		}
	}

	return nullptr;
}

int GGPlaneAnimation::findSameFrameIndex(const GGPlaneAnimationFrame& frame)
{
	for (size_t loop = 0; loop < m_uniqueFrameData.size(); loop++)
	{
		if (m_uniqueFrameData[loop]->hasSameFrameData(&frame))
		{
			return loop;
		}
	}

	return -1;
}


void GGPlaneAnimation::Write(const std::string& outputFolder, const std::string& outputName)
{
	WriteGGPlaneAnimationHeaderFile(outputFolder, outputName);
	WriteGGPlaneAnimationSourceFile(outputFolder, outputName);
}

void GGPlaneAnimation::WriteGGPlaneAnimationHeaderFile(const std::string& outputFolder, const std::string& outputName)
{
	std::string headerFilename = outputName + ".h";
	std::ofstream headerfile(outputFolder + headerFilename);

    // header guard
    std::string headerGuard = outputName + "_ANIMATION_INCLUDE_H";
    std::transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);
    headerfile << "// This file was generated by gale2c\n";
    headerfile << "#ifndef " << headerGuard << "\n";
    headerfile << "#define " << headerGuard << "\n";
    headerfile << "\n";

	// includes
    headerfile << "#include <genesis.h>\n";
    headerfile << "#include \"engine\\GGAnimationDraw.h\"\n";
    headerfile << "#include \"engine\\vdptypes.h\"\n";
    headerfile << "\n";

	// exported types
    headerfile << "extern const GGPlaneAnimation " << outputName << ";\n"; 

	/*
	if (m_options.mFixedBack)
	{
		headerfile << "extern const VDPTileIndex " << outputName << "VDPTileIndex;\n";  
	}
	else
	{
		headerfile << "extern VDPTileIndex " << outputName << "VDPTileIndex;\n";  
	}

    headerfile << "\n";

	if (m_options.mFixedBack)
	{
		headerfile << "#define " << outputName << "LoadFixed() VDPTileManager_LoadGGPlaneAnimationToVDPFixed(&" << outputName << ", " << outputName << "VDPTileIndex)\n\n";
	}
	else
	{
		headerfile << "#define " << outputName << "Load(region) VDPTileManager_LoadGGPlaneAnimationToVDP(region, &" << outputName << ", &" << outputName << "VDPTileIndex)\n\n";
	}
	*/


    // end header guard
    headerfile << "#endif\n\n";
    headerfile.close();
}

std::string BuildFrameDataName(const std::string outputName, int index)
{
    std::stringstream stringStream;
    stringStream << outputName << "FrameData" << index;

    return stringStream.str();
}

void GGPlaneAnimation::WriteFrameData(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t loop = 0; loop < m_uniqueFrameData.size(); loop++)
	{
		std::string frameName = BuildFrameDataName(outputName, loop);

		const GGPlaneAnimationFrame* frame = m_uniqueFrameData[loop];
		//
        sourceFile << "const u16 const " << frameName << "[" << frame->tileWidth() * frame->tileHeight() << "] = // " << frame->tileWidth() << " x " << frame->tileHeight() << "\n";
        sourceFile << "{\n";

		for (int loopHeight = 0; loopHeight < frame->tileHeight(); loopHeight++)
		{
			for (int loopWidth = 0; loopWidth < frame->tileWidth(); loopWidth++)
			{
				int value = frame->frameData()[loopWidth + (loopHeight * frame->tileWidth())];

				if (m_options.mFixedBack)
				{
					value += (Options::VDP_SIZE - m_uniqueTileCount);
				}

				sourceFile << std::setw(4) << value << ", ";
			}

			sourceFile << "\n";
		}

        sourceFile << "};\n\n";
		
	}
}


std::string BuildFrameDrawFunctionName(const std::string outputName, int index)
{
    std::stringstream stringStream;
    stringStream << outputName << "DrawFunction" << index;

    return stringStream.str();
}

void GGPlaneAnimation::WriteFunctions(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t frameLoop = 0; frameLoop < m_uniqueFrameData.size(); frameLoop++)
	{
		std::string frameName = BuildFrameDataName(outputName, frameLoop);

		const GGPlaneAnimationFrame* frame = m_uniqueFrameData[frameLoop];
		auto width = frame->tileWidth();
		auto height = frame->tileHeight();
		auto frameData = frame->frameData();

		sourceFile << "void " << BuildFrameDrawFunctionName(outputName, frameLoop) << "(u16 plan, u16 x, u16 y, u16 tileIndex)\n";
        sourceFile << "{\n";

        sourceFile << "    vu32 *plctrl;\n";
        sourceFile << "    vu16 *pwdata;\n";
		sourceFile << "    u32 planwidth;\n";
		sourceFile << "    u32 addr;\n";
        sourceFile << "\n";
        sourceFile << "    VDP_setAutoInc(2);\n";
        sourceFile << "\n";
        sourceFile << "    /* point to vdp port */\n";
        sourceFile << "    plctrl = (u32 *) GFX_CTRL_PORT;\n";
        sourceFile << "    pwdata = (u16 *) GFX_DATA_PORT;\n";

		sourceFile << "    planwidth = VDP_getPlanWidth();\n";

		sourceFile << "    addr = plan + ((x + (planwidth * y)) << 1);\n";
		sourceFile << "    *plctrl = GFX_WRITE_VRAM_ADDR(addr);\n";

		for (size_t loop = 0; loop < frameData.size(); loop++)
		{
			auto tileValue = frameData[loop];

			if (tileValue > 0)
			{
				sourceFile << "    *pwdata = " << tileValue << " + tileIndex;\n";
			}
			else
				sourceFile << "    *pwdata = 0;\n";

			if ((loop + 1) % width == 0 && loop < frameData.size() - 1)
			{
				sourceFile << "    addr += PLANE_WIDTH << 1;\n";
				sourceFile << "    *plctrl = GFX_WRITE_VRAM_ADDR(addr);\n";
			}
		}

		/*
		// exports the animation, without empty tiles and without 
		// erasing the previous frame.
		int addrOffset = 0;
		int lastValue = -1;

		for (size_t loop = 0; loop < frameData.size(); loop++)
		{
			auto tileValue = frameData[loop];

			if (lastValue != tileValue && lastValue == 0)
			{
				sourceFile << "    addr += " << addrOffset << " << 1;\n";
				sourceFile << "    *plctrl = GFX_WRITE_VRAM_ADDR(addr);\n";
				addrOffset = 0;
			}

			if (tileValue != 0)
			{
				sourceFile << "    *pwdata = " << tileValue << " + tileIndex;\n";
			}				

			lastValue = tileValue;
			addrOffset++;

			if ((loop + 1) % width == 0)
			{
				addrOffset += 64 - width;

				if (lastValue > 0)
				{
					sourceFile << "    addr += " << addrOffset << " << 1;\n";
					sourceFile << "    *plctrl = GFX_WRITE_VRAM_ADDR(addr);\n";
					addrOffset = 0;
				}
			}
		}
		*/

		/*
		for (int yloop = 0; yloop < height; yloop++)
		{
			sourceFile << "\n// line " << yloop << "\n";

			sourceFile << "    *plctrl = GFX_WRITE_VRAM_ADDR(addr);\n";

			int storedZeros = 0;

			for (int xloop = 0; xloop < width; xloop++)			
			{
				auto tileValue = frameData[xloop + (width * yloop)];

				if (tileValue > 0)
				{
					if (storedZeros)
					{ 
						//sourceFile << "    *pwdata = 0;\n";
						sourceFile << "    addr += " << storedZeros << " << 1;\n";
						sourceFile << "    *plctrl = GFX_WRITE_VRAM_ADDR(addr);\n";
						storedZeros = 0;
					}
					sourceFile << "    *pwdata = " << tileValue << " + tileIndex;\n";
				}
				else
				{
					storedZeros++;
				}
			}

			if (storedZeros > 0 && storedZeros < width)
				sourceFile << "    addr += (planwidth - " << storedZeros << ") << 1;\n";
			else
				sourceFile << "    addr += planwidth << 1;\n";
		}
		*/

        sourceFile << "}\n\n";
	}
}

std::string BuildFrameName(const std::string outputName, int frameNumber)
{
    std::stringstream stringStream;
    stringStream << outputName << "Frame" << frameNumber;

    return stringStream.str();
}

void GGPlaneAnimation::WriteFrameNames(const std::string& outputName, std::ofstream& sourceFile)
{
    // forward declare frame names.
	for (size_t loop = 0; loop < m_frames.size(); loop++)
	{
		sourceFile << "extern const GGPlaneAnimationFrame " << BuildFrameName(outputName, loop) << ";\n";
	}

	sourceFile << "\n\n";
}


void GGPlaneAnimation::WriteFrames(const std::string& outputName, std::ofstream& sourceFile)
{
	for (size_t loop = 0; loop < m_frames.size(); loop++)
	{
		auto& frame = m_frames[loop];

		sourceFile << "\n";
		sourceFile << "const GGPlaneAnimationFrame " << BuildFrameName(outputName, frame.frameNumber()) << " = \n";
		sourceFile << "{\n";

		int uniqueFrameIndex = findSameFrameIndex(frame);
		sourceFile << "    " << BuildFrameDrawFunctionName(outputName, uniqueFrameIndex) << ", // frame drawing function\n";
		sourceFile << "    " << frame.getFrameDelayTime() << ", // frame time\n"; 

		if (loop + 1 > m_frames.size() - 1)
		{
			if (m_options.mNoLoop)
			{
				sourceFile << "    NULL, // stop animation. no looping\n";
			}
			else
			{
				sourceFile << "    &" << BuildFrameName(outputName, 0) << ", // loop to next frame. \n";
			}
		}
		else
		{
			sourceFile << "    &" << BuildFrameName(outputName, loop + 1) << ", // next frame\n";
		}

		sourceFile << "};\n";
		sourceFile << "\n";
	}	
}

std::string BuildFrameArrayName(const std::string& outputName)
{
    std::stringstream stringStream;
    stringStream << outputName << "Frames";

    return stringStream.str();
}


void GGPlaneAnimation::WriteFrameArray(const std::string& outputName, std::ofstream& sourceFile)
{
	sourceFile << "const GGPlaneAnimationFrame* const " << BuildFrameArrayName(outputName) << "[" << m_frames.size() << "] =\n";
	sourceFile << "{\n";		

	for (size_t loop = 0; loop < m_frames.size(); loop++)
	{
		auto& frame = m_frames[loop];

		sourceFile << "    &" << BuildFrameName(outputName, frame.frameNumber()) << ",\n";
	}

	sourceFile << "};\n\n";		
}

void GGPlaneAnimation::WriteAnimationStruct(const std::string& outputName, std::ofstream& sourceFile)
{
    // final struct
    sourceFile << "\n";
    sourceFile << "\n";

	int vdpPosition = Options::VDP_SIZE - m_uniqueTileCount;

	/*
	if (m_options.mFixedBack)
	{
		sourceFile << "const VDPTileIndex " << outputName << "VDPTileIndex = " << vdpPosition << ";\n";
	}
	else
	{
		sourceFile << "VDPTileIndex " << outputName << "VDPTileIndex;\n";   
	}
	*/
    sourceFile << "\n";
	

    sourceFile << "const GGPlaneAnimation " << outputName << " = \n";
    sourceFile << "{\n";
    sourceFile << "    " << outputName << "Frames,\n";
    sourceFile << "    " << m_frames.size() << ", // number of frames\n";
	sourceFile << "    " << m_frames[0].tileWidth() << ", // width of the total area of the frame in tiles\n";
	sourceFile << "    " << m_frames[0].tileHeight() << ", // height of the total area of the frame in tiles\n";
    sourceFile << "    " << m_uniqueTileCount << ", // the total number of tiles in the animation\n";
    sourceFile << "    " << outputName << "TileData, // start of the tile data\n";
    //sourceFile << "    &" << outputName << "VDPTileIndex, // vdp tile index\n";

    sourceFile << "};\n";
}

void GGPlaneAnimation::WriteGGPlaneAnimationSourceFile(const std::string& outputFolder, const std::string& outputName)
{
	std::ofstream sourceFile(outputFolder + outputName + ".c");

    // includes
    sourceFile << "#include <genesis.h>\n";
    sourceFile << "#include \"" << outputName << ".h\"\n";
	sourceFile << "#include \"engine\\Config.h\"";
    sourceFile << "\n";
    sourceFile << "\n";

	WriteTileData(outputName, sourceFile, m_tiles);

	//WriteFrameData(outputName, sourceFile);
	WriteFunctions(outputName, sourceFile);
	WriteFrameNames(outputName, sourceFile);
	WriteFrames(outputName, sourceFile);

	WriteFrameArray(outputName, sourceFile);

	WriteAnimationStruct(outputName, sourceFile);

    sourceFile.close();
}

