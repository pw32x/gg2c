// gale2c.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "galefile151119\galefile.h"
#include <string>
#include "GGAnimation.h"
#include "GGPlaneAnimation.h"
#include "GGSMSAnimation.h"
#include "GGSMSPlaneAnimation.h"
#include "GGSMSTileAnimation.h"
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
    else if (parameter == "-updateonly")
    {
        gOptions.mUpdateOnly = true;
    }
    else
    {
        gOutputFolder = parameter;
        gOutputFolder += "\\";
    }
}

void ValidateArguments(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("No Graphics Gale file or folder specified\n");
        printf("\ngale2c.exe [.gal file or folder] [optional_destination_folder] [-sms] [-updateonly]\n");
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
            printf("No animations found in %s.\n", fileOrPath.c_str());
		    exit(-1);
        }
    }

    for (int loop = 2; loop < argc; loop++)
    {
        parseOptionalParameter(argv[loop]);
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

FILETIME getLastWriteTime(const std::string& path)
{
    FILETIME lastWriteTime = {0};

    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) 
    {
        if (GetFileTime(hFile, NULL, NULL, &lastWriteTime)) 
        {
            SYSTEMTIME st;
            FileTimeToSystemTime(&lastWriteTime, &st);
        }
    }

    CloseHandle(hFile);

    return lastWriteTime;
}

bool isNewer(FILETIME a, FILETIME b)
{
    if (CompareFileTime(&a, &b) > 0)
        return true;

    return false;
}

bool needsUpdate(const std::string& filename, const std::string& outputFolder, const std::string& outputFilename)
{
    // we need an update if the app or the filename is newer than the exported files.

    char filePath[MAX_PATH];
    DWORD size = GetModuleFileNameA(nullptr, filePath, MAX_PATH);

    FILETIME appTime = getLastWriteTime(filePath);
    FILETIME fileTime = getLastWriteTime(filename.c_str());

    std::string sourceFilename = outputFolder + outputFilename + ".c";
    FILETIME sourceTime = getLastWriteTime(sourceFilename.c_str());

    std::string headerFilename = outputFolder + outputFilename + ".h";
    FILETIME headerTime = getLastWriteTime(headerFilename.c_str());
     
    // if app or filename is newer than source and header, then update.    

    if (isNewer(appTime, sourceTime))
        return true;

    if (isNewer(appTime, headerTime))
        return true;

    if (isNewer(fileTime, sourceTime))
        return true;

    if (isNewer(fileTime, headerTime))
        return true;

    return false;
}

int main(int argc, char* argv[])
{
    printf("gg2c.exe Graphics Gale to C exporter by pw_32x. https://github.com/pw32x/gg2c\n");

	ValidateArguments(argc, argv);

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

        if (gOptions.mUpdateOnly && !needsUpdate(filename, gOutputFolder, outputFilename))
        {
            printf("%s is already up to date.\n", filename.c_str());
            continue;
        }
        else
        {
            printf("Exporting %s \n", filename.c_str());
        }

	    gOptions.ProcessOptions(filename);

        if (gOptions.mExportToSMSFormat)
        {
	        if (gOptions.mBackgroundPlaneAnimation)
	        {
                sms::GGPlaneAnimation animation(gGaleFileHandle, gOptions);
		        animation.Write(gOutputFolder, outputFilename);
            }
            else if (gOptions.mTileAnimation)
            {
                sms::GGTileAnimation animation(gGaleFileHandle, gOptions, gAnimationProperties);
		        animation.Write(gOutputFolder, outputFilename);
            }
            else
            {
                sms::GGAnimation animation(gGaleFileHandle, gOptions, gAnimationProperties);
                animation.Write(gOutputFolder, outputFilename);
            }
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

    printf("done\n");

    return 0;
}

