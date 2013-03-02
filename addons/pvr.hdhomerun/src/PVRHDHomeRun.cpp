#include "PVRHDHomeRun.h"
#include "client.h"

#include <hdhomerun.h>

#include <sstream>

using namespace std;
using namespace ADDON;

PVRHDHomeRun::PVRHDHomeRun() :
	m_bufferOffset(0)
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);

	const int maxDevices = 8;
  struct hdhomerun_discover_device_t devices[maxDevices];

	int numOfDevices = hdhomerun_discover_find_devices_custom(0, HDHOMERUN_DEVICE_TYPE_TUNER, HDHOMERUN_DEVICE_ID_WILDCARD, devices, maxDevices);

	for (int i = 0; i < numOfDevices; ++i)
	{
		XBMC->Log(LOG_DEBUG, "Device %X is type %d and has %d tuners\n", devices[i].device_id, devices[i].device_type, (unsigned int)devices[i].tuner_count);
		
		for (int j = 0; j < devices[i].tuner_count; ++j)
		{
			m_device = hdhomerun_device_create(devices[i].device_id, devices[i].ip_addr, j, NULL);

			string name = hdhomerun_device_get_name(m_device);
			XBMC->Log(LOG_DEBUG, "Name of device: %s\n", name.c_str());
		}
	}

	//this->ChannelScan();
	//m_channels.Save("Bla");
	m_channels.Load("bla");
}

PVRHDHomeRun::~PVRHDHomeRun()
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);
	hdhomerun_device_destroy(m_device);
}

void PVRHDHomeRun::GetSignalStatus(PVR_SIGNAL_STATUS &signalStatus)
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);

	signalStatus.iSNR = 100;
	signalStatus.iSignal = 100;
	signalStatus.iBER = 0;
	signalStatus.iUNC = 0;
}

bool PVRHDHomeRun::OpenLiveStream(const PVR_CHANNEL &_channel)
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);

	HDHomerunChannel channel;
	if(m_channels.GetHDHomeRunChannel(_channel, channel))
	{
		ostringstream chan;
		chan << channel.m_frequency;
		int ret = hdhomerun_device_set_tuner_channel(m_device, chan.str().c_str());
		XBMC->Log(LOG_DEBUG, "hdhomerun_device_set_tuner_channel: %d %d\n", ret, channel.m_frequency);

		ostringstream is;
		is << channel.m_programNum;
		ret = hdhomerun_device_set_tuner_program(m_device, is.str().c_str());
		XBMC->Log(LOG_DEBUG, "hdhomerun_device_set_tuner_program: %d %d\n", ret, channel.m_programNum);

		ret = hdhomerun_device_stream_start(m_device);
		XBMC->Log(LOG_DEBUG, "hdhomerun_device_stream_start: %d\n", ret);
		hdhomerun_device_stream_flush(m_device);

		return true;
	}
	return false;
}

	
void PVRHDHomeRun::CloseLiveStream()
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);

	hdhomerun_device_stream_stop(m_device);
	XBMC->Log(LOG_DEBUG, "hdhomerun_device_stream_stop, stopped\n");
}

bool PVRHDHomeRun::SwitchChannel(const PVR_CHANNEL &channel)
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);

  CloseLiveStream();

  return OpenLiveStream(channel);
}


int PVRHDHomeRun::ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize)
{
	XBMC->Log(LOG_DEBUG, "%s(), wanted buf size: %d\n", __func__, iBufferSize);

	size_t actualRead = 0;
	size_t readTotal = m_bufferOffset;
	uint8_t* data = 0;

	int stop = 0;
	while(readTotal < iBufferSize)
	{
		if(stop > 100) {
			XBMC->Log(LOG_DEBUG, "%s(), not receiving any data, break!\n", __func__);
			m_bufferOffset = 0;
			return 0;
		}
		data = hdhomerun_device_stream_recv(m_device, 20000000/8, &actualRead);

		if((m_bufferOffset + readTotal + actualRead) > sizeof(m_buffer)) {
			XBMC->Log(LOG_ERROR, "Buffer full, clearing/starting over\n");
			readTotal = 0;
		}
		memcpy(m_buffer + readTotal, data, actualRead);
		readTotal = readTotal + actualRead;
		
		usleep(5000);
		stop++;
	}

	XBMC->Log(LOG_DEBUG, "%s(), readTotal: %d\n", __func__, readTotal);

	// Simple fifo buffer.
	memcpy(pBuffer, m_buffer, iBufferSize);
	m_bufferOffset = readTotal - iBufferSize;
	memcpy(m_buffer, m_buffer + iBufferSize, m_bufferOffset);
	
	return iBufferSize;
}

int PVRHDHomeRun::GetCurrentChannel()
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);
	return 0;
}

PVR_ERROR PVRHDHomeRun::GetChannels(ADDON_HANDLE handle, bool bRadio)
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);

	vector<PVR_CHANNEL> channels = m_channels.GetPVRChannels();
	
	vector<PVR_CHANNEL>::iterator it;
	for(it = channels.begin(); it != channels.end(); ++it)
	{
	 	PVR->TransferChannelEntry(handle, &(*it));
	}
	
	return PVR_ERROR_NO_ERROR;
}

int PVRHDHomeRun::GetChannelsAmount()
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);
	return 1;
}


int PVRHDHomeRun::ChannelScan()
{
	XBMC->Log(LOG_DEBUG, "%s()", __func__);
	// char *ret_error;
	// if (hdhomerun_device_tuner_lockkey_request(hd, &ret_error) <= 0) {
	// 	fprintf(stderr, "failed to lock tuner\n");
	// 	if (ret_error) {
	// 		fprintf(stderr, "%s\n", ret_error);
	// 	}
	// 	return -1;
	// }

	int unique_id = 1;
	
	hdhomerun_device_set_tuner_target(m_device, "none");

	char *channelmap;
	if (hdhomerun_device_get_tuner_channelmap(m_device, &channelmap) <= 0) {
		XBMC->Log(LOG_ERROR, "failed to query channelmap from device\n");
		return -1;
	}

	const char *channelmap_scan_group = hdhomerun_channelmap_get_channelmap_scan_group(channelmap);
	if (!channelmap_scan_group) {
		XBMC->Log(LOG_ERROR, "unknown channelmap '%s'\n", channelmap);
		return -1;
	}

	if (hdhomerun_device_channelscan_init(m_device, channelmap_scan_group) <= 0) {
		XBMC->Log(LOG_ERROR, "failed to initialize channel scan\n");
		return -1;
	}

	// FILE *fp = NULL;
	// if (filename) {
	// 	fp = fopen(filename, "w");
	// 	if (!fp) {
	// 		fprintf(stderr, "unable to create file: %s\n", filename);
	// 		return -1;
	// 	}
	// }

	//register_signal_handlers(sigabort_handler, sigabort_handler, siginfo_handler);

	int ret = 0;
	int stop=0;
	while (stop < 1001) {
		struct hdhomerun_channelscan_result_t result;
		ret = hdhomerun_device_channelscan_advance(m_device, &result);
		if (ret <= 0) {
			break;
		}

		XBMC->Log(LOG_DEBUG, "Progress %d\n", hdhomerun_device_channelscan_get_progress(m_device));
		
		XBMC->Log(LOG_DEBUG, "SCANNING: %lu (%s)\n",
			(unsigned long)result.frequency, result.channel_str
		);

		ret = hdhomerun_device_channelscan_detect(m_device, &result);
		if (ret < 0) {
			break;
		}
		if (ret == 0) {
			continue;
		}

		XBMC->Log(LOG_DEBUG, "LOCK: %s (ss=%u snq=%u seq=%u)\n",
			result.status.lock_str, result.status.signal_strength,
			result.status.signal_to_noise_quality, result.status.symbol_error_quality
		);

		if (result.transport_stream_id_detected) {
			XBMC->Log(LOG_DEBUG, "TSID: 0x%04X\n", result.transport_stream_id);
		}

		int i;
		for (i = 0; i < result.program_count; i++) {
			struct hdhomerun_channelscan_program_t *program = &result.programs[i];
			XBMC->Log(LOG_DEBUG, "PROGRAM %s\n", program->program_str);

			m_channels.Add(result, program, unique_id);
			unique_id++;
		}
		stop++;
	}

	hdhomerun_device_tuner_lockkey_release(m_device);

	return 0;
}
