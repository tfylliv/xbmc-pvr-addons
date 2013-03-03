#pragma once

#include "client.h"

#include "HDHomerunChannel.h"

#include <vector>

struct hdhomerun_device_t;

class PVRHDHomeRun
{
public:
  PVRHDHomeRun();
  virtual ~PVRHDHomeRun();

  PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio);
  int GetCurrentChannel();
  int GetChannelsAmount();

  void GetSignalStatus(PVR_SIGNAL_STATUS &signalStatus);

  bool OpenLiveStream(const PVR_CHANNEL &channel);
  int ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize);
  bool SwitchChannel(const PVR_CHANNEL &channel);
  void CloseLiveStream();

private:
  int ChannelScan();
  
private:
  struct hdhomerun_device_t* m_device;

  uint8_t m_buffer[2000000];
  int m_bufferOffset;
  
  HDHomerunChannels m_channels;
};
