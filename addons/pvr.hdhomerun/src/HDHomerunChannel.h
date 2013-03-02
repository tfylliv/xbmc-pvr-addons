#pragma once

#include "client.h"

#include <hdhomerun.h>

#include <string>
#include <vector>

class HDHomerunChannel
{
public:
	HDHomerunChannel(	int uniqueId,
										const std::string& _programString,
										int _programNum,
										int _frequency,
										const std::string& _channelStr,
										int _virtMajor,
										int _virtMinor,
										int _type,
										const std::string& _name);

	HDHomerunChannel();

	void FillPVRCHannel();

public:
	unsigned int m_uniqueId;
	std::string m_programString;
	int m_programNum;
	int m_frequency;
	std::string m_channelStr;
	int m_virtMajor;
	int m_virtMinor;
	int m_type;
	std::string m_name;
	PVR_CHANNEL m_xbmcChannel;
};

class HDHomerunChannels
{
public:
	HDHomerunChannels();
	bool Load(const std::string& _filename);
	bool Save(const std::string& _filename);

	void Add(const hdhomerun_channelscan_result_t& _result,
					 hdhomerun_channelscan_program_t* _program,
					 int uniqueId);

	std::vector<PVR_CHANNEL> GetPVRChannels();
	bool GetHDHomeRunChannel(const PVR_CHANNEL& _pvrChannel,
																				HDHomerunChannel& _channel);
	
private:
	std::vector<HDHomerunChannel> m_channels;
};

