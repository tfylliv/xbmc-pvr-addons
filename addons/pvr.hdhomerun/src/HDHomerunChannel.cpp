#include "HDHomerunChannel.h"
#include "client.h"

#include "tinyxml/XMLUtils.h"

#include <hdhomerun.h>
#include <string>

using namespace std;
using namespace ADDON;

HDHomerunChannels::HDHomerunChannels()
{
}

void HDHomerunChannels::Add(const hdhomerun_channelscan_result_t& _result,
														hdhomerun_channelscan_program_t* _program,
														int uniqueId)
{


	HDHomerunChannel channel(uniqueId,
													 _program->program_str,
													 _program->program_number,
													 _result.frequency,
													 _result.channel_str,
													 _program->virtual_major,
													 _program->virtual_minor,
													 _program->type,
													 _program->name
													 );
	m_channels.push_back(channel);
}

vector<PVR_CHANNEL> HDHomerunChannels::GetPVRChannels()
{
	vector<PVR_CHANNEL> channels;

	vector<HDHomerunChannel>::iterator it;
	for(it = m_channels.begin(); it != m_channels.end(); ++it)
	{
		channels.push_back(it->m_xbmcChannel);
	}
	return channels;
}

bool HDHomerunChannels::GetHDHomeRunChannel(const PVR_CHANNEL& _pvrChannel,
																						HDHomerunChannel& _channel)
{
	vector<HDHomerunChannel>::iterator it;
	for(it = m_channels.begin(); it != m_channels.end(); ++it)
	{
		if(it->m_uniqueId == _pvrChannel.iUniqueId)
		{
			_channel = *it;
			return true;
		}
	}
	return false;
}

bool HDHomerunChannels::Load(const std::string& _filename)
{
  string xmlFile = g_strClientPath;
  xmlFile += "/pvrhdhomerun_channels.xml";

  TiXmlDocument xmlDoc;
  if (!xmlDoc.LoadFile(xmlFile, TIXML_ENCODING_LEGACY))
  {
    XBMC->Log(LOG_INFO, "No saved channels for PVR HDHomeRun addon found, error: %s, row %d, col %d, you need to scan for channels. %s\n')", xmlDoc.ErrorDesc(), xmlDoc.ErrorRow(), xmlDoc.ErrorCol(), xmlFile.c_str());
    return false;
  }

	TiXmlElement *pRootElement = xmlDoc.RootElement();
  if (strcmp(pRootElement->Value(), "PVRHDHomeRun") != 0)
  {
    XBMC->Log(LOG_ERROR, "Channel XML file doesn't contain data for PVR HDHomeRun\n");
    return false;
  }

	TiXmlElement *pElement = pRootElement->FirstChildElement("channels");
  if (pElement)
  {
    TiXmlNode *pChannelNode = NULL;
		CStdString strTmp;
		int intTmp;
    while ((pChannelNode = pElement->IterateChildren(pChannelNode)) != NULL)
    {
			HDHomerunChannel channel;

			if (!XMLUtils::GetInt(pChannelNode, "unique-id", intTmp))
				continue;
			channel.m_uniqueId = intTmp;

      if (!XMLUtils::GetString(pChannelNode, "program-string", strTmp))
        continue;
			channel.m_programString = strTmp;

			if (!XMLUtils::GetInt(pChannelNode, "program-number", intTmp))
				continue;
			channel.m_programNum = intTmp;
			
			if (!XMLUtils::GetInt(pChannelNode, "frequency", intTmp))
				continue;
			channel.m_frequency = intTmp;

      if (!XMLUtils::GetString(pChannelNode, "channel-str", strTmp))
        continue;
			channel.m_channelStr = strTmp;

			if (!XMLUtils::GetInt(pChannelNode, "virtual-major", intTmp))
				continue;
			channel.m_virtMajor = intTmp;

			if (!XMLUtils::GetInt(pChannelNode, "virtual-minor", intTmp))
				continue;
			channel.m_virtMinor = intTmp;

			if (!XMLUtils::GetInt(pChannelNode, "type", intTmp))
				continue;
			channel.m_type = intTmp;

			if (!XMLUtils::GetString(pChannelNode, "name", strTmp))
        continue;
			channel.m_name = strTmp;

			XBMC->Log(LOG_INFO, "Adding channel %s\n", channel.m_name.c_str());

			channel.FillPVRCHannel();

			// Ignore ENCRYPTED channels.
			if(channel.m_type == HDHOMERUN_CHANNELSCAN_PROGRAM_NORMAL)
			{
				m_channels.push_back(channel);
			}
		}
	}
	else
	{
		XBMC->Log(LOG_ERROR, "No channels found in XML file\n");
    return false;
	}
	return false;
}

bool HDHomerunChannels::Save(const std::string& _filename)
{
  TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "PVRHDHomeRun" );
	doc.LinkEndChild( root );
	
	TiXmlElement * channels = new TiXmlElement( "channels" );
	root->LinkEndChild( channels );

	vector<HDHomerunChannel>::iterator it;
	for(it = m_channels.begin(); it != m_channels.end(); ++it)
	{
		TiXmlElement * channel = new TiXmlElement( "channel" );
		channels->LinkEndChild( channel );
		
		XMLUtils::SetInt(channel, "unique-id",
												it->m_uniqueId );
		XMLUtils::SetString(channel, "program-string",
												it->m_xbmcChannel.strChannelName );
		XMLUtils::SetInt(channel, "program-number", it->m_xbmcChannel.iChannelNumber);

		XMLUtils::SetInt(channel, "frequency", it->m_frequency);
		XMLUtils::SetString(channel, "channel-str", it->m_channelStr);

		XMLUtils::SetInt(channel, "virtual-major", it->m_virtMajor);
		XMLUtils::SetInt(channel, "virtual-minor", it->m_virtMinor);
		XMLUtils::SetInt(channel, "type", it->m_type);
		XMLUtils::SetString(channel, "name", it->m_name);
	}

  string xmlFile = g_strClientPath;
  xmlFile += "/pvrhdhomerun_channels.xml";
  doc.SaveFile(xmlFile);
	
	return true;
}

HDHomerunChannel::HDHomerunChannel(	int _uniqueId,
																		const string& _programString,
																		int _programNum,
																		int _frequency,
																		const string& _channelStr,
																		int _virtMajor,
																		int _virtMinor,
																		int _type,
																		const string& _name)
	: m_uniqueId(_uniqueId),
		m_programString(_programString),
		m_programNum(_programNum),
		m_frequency(_frequency),
		m_channelStr(_channelStr),
		m_virtMajor(_virtMajor),
		m_virtMinor(_virtMinor),
		m_type(_type),
		m_name(_name)
{
	FillPVRCHannel();
}

HDHomerunChannel::HDHomerunChannel()
{
}

void HDHomerunChannel::FillPVRCHannel()
{
	memset(&m_xbmcChannel, 0, sizeof(PVR_CHANNEL));

	m_xbmcChannel.iUniqueId         = m_uniqueId;
	m_xbmcChannel.bIsRadio          = false;
	m_xbmcChannel.iChannelNumber    = m_programNum;
	strncpy(m_xbmcChannel.strChannelName, m_name.c_str(), m_name.length());
}
