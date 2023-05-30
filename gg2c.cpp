// gale2c.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "galefile151119\galefile.h"
#include <string>
#include "GGAnimation.h"
#include "GGPlaneAnimation.h"
#include "Options.h"
#include "AnimationProperties.h"
#include "Shlwapi.h"

std::string						gGaleFilename;
std::string						gOutputName;
std::string						gOutputFolder;
LPVOID							gGaleFileHandle = NULL;
DWORD							gNumberOfFrames = 0;
BITMAP							gGeneralBitmapInfo;

DWORD							gTotalNumberOfTiles = 0;

Options							gOptions;
AnimationProperties				gAnimationProperties;


void ValidateArguments(char* argv[])
{
    if (argv[1] == NULL)
    {
        printf("No Graphics Gale file specified\n");
        printf("\ngale2c.exe [input .gal file] [optional_destination_folder]\n");
		exit(-1);
    }

    gGaleFilename = argv[1];

	// figure out the output name
	std::size_t index;
    index = gGaleFilename.rfind("\\"); // remove path
    if (index != std::string::npos)
	{
        gOutputName = gGaleFilename.substr(index + 1);
	}
	else
	{
		gOutputName = gGaleFilename;
	}

    index = gOutputName.find(".");
    gOutputName = gOutputName.substr(0, index);


    if (argv[2] != NULL)
    {
        gOutputFolder = argv[2];
        gOutputFolder += "\\";
    }
}

void OpenGaleFile()
{
    gGaleFileHandle = ggOpen(gGaleFilename.c_str());

    if (gGaleFileHandle == NULL)
    {
		printf("Graphics Gale file not found.");
        exit(-1);
    }

    gNumberOfFrames = ggGetFrameCount(gGaleFileHandle);

    if (gNumberOfFrames == 0)
    {
		printf("No frames found in file\n");
		exit(-1);
    }

    HBITMAP bitmap = ggGetBitmap(gGaleFileHandle, 0, 0);

    if (bitmap == NULL)
    {
		printf("Error retrieving bitmap data");
		exit(-1);
    }

    if (GetObject(bitmap, sizeof(BITMAP), &gGeneralBitmapInfo) == 0)
    {
        printf("BitmapInfo is NULL\n");
		exit(-1);
    }

    if (gGeneralBitmapInfo.bmBitsPixel != 4)
    {
        printf("Bitmap data is not 4 bits per pixel\n");
		exit(-1);
    }
}

void CloseGaleFile()
{
    if (gGaleFileHandle != NULL)
    {
        ggClose(gGaleFileHandle);
    }
}

int main(int argc, char* argv[])
{
    printf("gg2c.exe Graphics Gale to C exporter by pw_32x. https://github.com/pw32x/gg2c\n");

	ValidateArguments(argv);
	OpenGaleFile();

	gOptions.ProcessOptions(gGaleFilename);

	if (gOptions.mBackgroundPlaneAnimation)
	{
		GGPlaneAnimation animation(gGaleFileHandle, gOptions);
		animation.Write(gOutputFolder, gOutputName);
	}
	else
	{
		GGAnimation animation(gGaleFileHandle, gOptions, gAnimationProperties);
		animation.Write(gOutputFolder, gOutputName);
	}

	CloseGaleFile();

    return 0;
}

