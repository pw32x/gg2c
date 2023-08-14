// gale2c.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "galefile151119\galefile.h"
#include <string>
#include "GGAnimation.h"
#include "GGPlaneAnimation.h"
#include "GGSMSAnimation.h"
#include "Options.h"
#include "AnimationProperties.h"
#include "Shlwapi.h"
#include "windows.h"

std::string						gOutputFolder;
LPVOID							gGaleFileHandle = NULL;
DWORD							gNumberOfFrames = 0;
BITMAP							gGeneralBitmapInfo;

DWORD							gTotalNumberOfTiles = 0;

Options							gOptions;
AnimationProperties				gAnimationProperties;

std::vector<std::string>        gFilenames;

void parseOptionalParameter(const std::string& parameter)
{
    if (parameter == "-sms")
    {
        gOptions.mExportToSMSFormat = true;
    }
    else
    {
        gOutputFolder = parameter;
        gOutputFolder += "\\";
    }
}

void ValidateArguments(char* argv[])
{
    if (argv[1] == NULL)
    {
        printf("No Graphics Gale file or folder specified\n");
        printf("\ngale2c.exe [.gal file or folder] [optional_destination_folder] [-sms]\n");
		exit(-1);
    }

    std::string fileOrPath = argv[1];

    // if ends with gal, then it's indivdual file.
    int findIndex = fileOrPath.rfind(".gal");
    if (findIndex != -1)
    {
        gFilenames.push_back(fileOrPath);
    }
    else // else assume it's a folder
    {
        std::string extension = ".gal";

        WIN32_FIND_DATAA findFileData;

        HANDLE hFind;

        std::string searchPattern = fileOrPath + "\\*.gal";
        hFind = FindFirstFileA(searchPattern.c_str(), &findFileData);

        if (hFind != INVALID_HANDLE_VALUE) 
        {
            do 
            {
                std::string fileName = findFileData.cFileName;
                gFilenames.push_back(fileOrPath + "\\" + fileName);

            } while (FindNextFileA(hFind, &findFileData) != 0);

            FindClose(hFind);
        } 
        else 
        {
            printf("Folder %s not found.\n", fileOrPath.c_str());
		    exit(-1);            
        }
    }

    if (argv[2] != NULL)
    {
        parseOptionalParameter(argv[2]);
    }

    if (argv[3] != NULL)
    {
        parseOptionalParameter(argv[3]);
    }
}

void OpenGaleFile(const std::string& filename)
{
    gGaleFileHandle = ggOpen(filename.c_str());

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

    for (auto& filename : gFilenames)
    {
	    OpenGaleFile(filename);

        std::string outputFilename;

	    // figure out the output name
	    std::size_t index;
        index = filename.rfind("\\"); // remove path
        if (index != std::string::npos)
	    {
            outputFilename = filename.substr(index + 1);
	    }
	    else
	    {
		    outputFilename = filename;
	    }

        index = outputFilename.find(".");
        outputFilename = outputFilename.substr(0, index);

	    gOptions.ProcessOptions(filename);

        if (gOptions.mExportToSMSFormat)
        {
            sms::GGAnimation animation(gGaleFileHandle, gOptions, gAnimationProperties);
		    animation.Write(gOutputFolder, outputFilename);
        }
        else
        {
	        if (gOptions.mBackgroundPlaneAnimation)
	        {
		        GGPlaneAnimation animation(gGaleFileHandle, gOptions);
		        animation.Write(gOutputFolder, outputFilename);
	        }
	        else
	        {
		        GGAnimation animation(gGaleFileHandle, gOptions, gAnimationProperties);
		        animation.Write(gOutputFolder, outputFilename);
	        }
        }

	    CloseGaleFile();
    }

    return 0;
}

