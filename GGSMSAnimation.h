#pragma once

#include "GGSMSAnimationFrame.h"
#include <string>
#include "AnimationProperties.h"

namespace sms
{

class GGAnimation
{
public:
	GGAnimation(LPVOID galeFileHandle, const Options& options, AnimationProperties& animationProperties);
	void Write(const std::string& outputFolder, const std::string& outputName);

	int GetTileCount() { return m_tileStore.size(); }

	const BITMAP& GetGeneralBitmapInfo() { return  m_generalBitmapInfo; }

private:
	void WriteSprites(const std::string& outputName, std::ofstream& sourceFile);
	void WriteFrames(const std::string& outputName, std::ofstream& sourceFile);
	void WriteFrameArray(const std::string& outputName, std::ofstream& sourceFile);
	void WriteAnimationStruct(const std::string& outputName, std::ofstream& sourceFile);

	void WriteGGAnimationHeaderFile(const std::string& outputFolder, const std::string& outputName);
	void WriteGGAnimationSourceFile(const std::string& outputFolder, const std::string& outputName);

private:
	LPVOID m_galeFileHandle;

	std::vector<GGAnimationFrame>	m_frames;
	std::vector<Tile>				m_tileStore;
	int								m_totalFrameTime = 0;
	const Options&					m_options;
	AnimationProperties&			m_animationProperties;
	BITMAP							m_generalBitmapInfo;
};

}

