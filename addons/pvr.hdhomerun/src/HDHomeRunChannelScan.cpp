/*
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <limits.h>
#include "HDHomeRunChannelScan.h"

#include <sstream>


#define BUTTON_START                    5
#define BUTTON_BACK                     6
#define BUTTON_CANCEL                   7
#define HEADER_LABEL                    8

#define SPIN_CONTROL_SOURCE_TYPE        10
#define CONTROL_RADIO_BUTTON_TV         11
#define CONTROL_RADIO_BUTTON_RADIO      12
#define CONTROL_RADIO_BUTTON_FTA        13
#define CONTROL_RADIO_BUTTON_SCRAMBLED  14
#define CONTROL_RADIO_BUTTON_HD         15
#define CONTROL_SPIN_COUNTRIES          16
#define CONTROL_SPIN_SATELLITES         17
#define CONTROL_SPIN_DVBC_INVERSION     18
#define CONTROL_SPIN_DVBC_SYMBOLRATE    29
#define CONTROL_SPIN_DVBC_QAM           20
#define CONTROL_SPIN_DVBT_INVERSION     21
#define CONTROL_SPIN_ATSC_TYPE          22

#define LABEL_TYPE                      30
#define LABEL_DEVICE                    31
#define PROGRESS_DONE                   32
#define LABEL_TRANSPONDER               33
#define LABEL_SIGNAL                    34
#define PROGRESS_SIGNAL                 35
#define LABEL_STATUS                    36

using namespace ADDON;

HDHomeRunChannelScan::HDHomeRunChannelScan()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);
}

HDHomeRunChannelScan::~HDHomeRunChannelScan()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);
}

bool HDHomeRunChannelScan::Open()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  m_running         = false;
  m_Canceled        = false;
  m_stopped         = true;
  m_progressDone    = NULL;
  m_progressSignal  = NULL;

  /* Load the Window as Dialog */
  m_window = GUI->Window_create("ChannelScan.xml", "Confluence", false, true);
  m_window->m_cbhdl   = this;
  m_window->CBOnInit  = OnInitCB;
  m_window->CBOnFocus = OnFocusCB;
  m_window->CBOnClick = OnClickCB;
  m_window->CBOnAction= OnActionCB;
  m_window->DoModal();

  GUI->Window_destroy(m_window);

  return true;
}

void HDHomeRunChannelScan::StartScan()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  m_header = XBMC->GetLocalizedString(30025);
  m_Signal = XBMC->GetLocalizedString(30029);
  SetProgress(0);
  SetSignal(0, false);

  int source = m_spinSourceType->GetValue();
  switch (source)
  {
    case DVB_TERR:
      m_window->SetControlLabel(LABEL_TYPE, "DVB-T");
      break;
    case DVB_CABLE:
      m_window->SetControlLabel(LABEL_TYPE, "DVB-C");
      break;
    case DVB_SAT:
      m_window->SetControlLabel(LABEL_TYPE, "DVB-S/S2");
      break;
    case PVRINPUT:
      m_window->SetControlLabel(LABEL_TYPE, XBMC->GetLocalizedString(30032));
      break;
    case PVRINPUT_FM:
      m_window->SetControlLabel(LABEL_TYPE, XBMC->GetLocalizedString(30033));
      break;
    case DVB_ATSC:
      m_window->SetControlLabel(LABEL_TYPE, "ATSC");
      break;
  }

  return;
}

void HDHomeRunChannelScan::StopScan()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  return;
}

void HDHomeRunChannelScan::ReturnFromProcessView()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  if (m_running)
  {
    m_running = false;
    m_window->ClearProperties();
    m_window->SetControlLabel(BUTTON_START, XBMC->GetLocalizedString(30010));
    m_window->SetControlLabel(HEADER_LABEL, XBMC->GetLocalizedString(30009));

    if (m_progressDone)
    {
      GUI->Control_releaseProgress(m_progressDone);
      m_progressDone = NULL;
    }
    if (m_progressSignal)
    {
      GUI->Control_releaseProgress(m_progressSignal);
      m_progressSignal = NULL;
    }
  }
}

void HDHomeRunChannelScan::SetProgress(int percent)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  if (!m_progressDone)
    m_progressDone = GUI->Control_getProgress(m_window, PROGRESS_DONE);

  std::stringstream header;
  header << percent;

  m_window->SetControlLabel(HEADER_LABEL, header.str().c_str());
  m_progressDone->SetPercentage((float)percent);
}

void HDHomeRunChannelScan::SetSignal(int percent, bool locked)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  if (!m_progressSignal)
    m_progressSignal = GUI->Control_getProgress(m_window, PROGRESS_SIGNAL);

  std::stringstream signal;
  signal << percent;

  m_window->SetControlLabel(LABEL_SIGNAL, signal.str().c_str());
  m_progressSignal->SetPercentage((float)percent);

  if (locked)
    m_window->SetProperty("Locked", "true");
  else
    m_window->SetProperty("Locked", "");
}

bool HDHomeRunChannelScan::OnClick(int controlId)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  if (controlId == SPIN_CONTROL_SOURCE_TYPE)
  {
    int value = m_spinSourceType->GetValue();
    SetControlsVisible((scantype_t)value);
  }
  else if (controlId == BUTTON_BACK)
  {
    m_window->Close();
    GUI->Control_releaseSpin(m_spinSourceType);
    GUI->Control_releaseSpin(m_spinDVBCInversion);
    GUI->Control_releaseSpin(m_spinDVBCSymbolrates);
    GUI->Control_releaseSpin(m_spinDVBCqam);
    GUI->Control_releaseSpin(m_spinDVBTInversion);
    GUI->Control_releaseSpin(m_spinATSCType);
    GUI->Control_releaseRadioButton(m_radioButtonTV);
    GUI->Control_releaseRadioButton(m_radioButtonRadio);
    GUI->Control_releaseRadioButton(m_radioButtonFTA);
    GUI->Control_releaseRadioButton(m_radioButtonScrambled);
    GUI->Control_releaseRadioButton(m_radioButtonHD);
    if (m_progressDone)
    {
      GUI->Control_releaseProgress(m_progressDone);
      m_progressDone = NULL;
    }
    if (m_progressSignal)
    {
      GUI->Control_releaseProgress(m_progressSignal);
      m_progressSignal = NULL;
    }
  }
  else if (controlId == BUTTON_START)
  {
    if (!m_running)
    {
      m_running = true;
      m_stopped = false;
      m_Canceled = false;
      m_window->SetProperty("Scanning", "running");
      m_window->SetControlLabel(BUTTON_START, XBMC->GetLocalizedString(222));
      StartScan();
    }
    else if (!m_stopped)
    {
      m_stopped = true;
      m_Canceled = true;
      StopScan();
    }
    else
      ReturnFromProcessView();
  }
  return true;
}

bool HDHomeRunChannelScan::OnFocus(int controlId)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  return true;
}

bool HDHomeRunChannelScan::OnInit()
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  m_spinSourceType = GUI->Control_getSpin(m_window, SPIN_CONTROL_SOURCE_TYPE);
  m_spinSourceType->Clear();
  m_spinSourceType->AddLabel("DVB-T", DVB_TERR);
  m_spinSourceType->AddLabel("DVB-C", DVB_CABLE);
  m_spinSourceType->AddLabel("DVB-S/S2", DVB_SAT);
  m_spinSourceType->AddLabel("Analog TV", PVRINPUT);
  m_spinSourceType->AddLabel("Analog Radio", PVRINPUT_FM);
  m_spinSourceType->AddLabel("ATSC", DVB_ATSC);

  m_spinDVBCInversion = GUI->Control_getSpin(m_window, CONTROL_SPIN_DVBC_INVERSION);
  m_spinDVBCInversion->Clear();
  m_spinDVBCInversion->AddLabel("Auto", 0);
  m_spinDVBCInversion->AddLabel("On", 1);
  m_spinDVBCInversion->AddLabel("Off", 2);

  m_spinDVBCSymbolrates = GUI->Control_getSpin(m_window, CONTROL_SPIN_DVBC_SYMBOLRATE);
  m_spinDVBCSymbolrates->Clear();
  m_spinDVBCSymbolrates->AddLabel("AUTO", 0);
  m_spinDVBCSymbolrates->AddLabel("6900", 1);
  m_spinDVBCSymbolrates->AddLabel("6875", 2);
  m_spinDVBCSymbolrates->AddLabel("6111", 3);
  m_spinDVBCSymbolrates->AddLabel("6250", 4);
  m_spinDVBCSymbolrates->AddLabel("6790", 5);
  m_spinDVBCSymbolrates->AddLabel("6811", 6);
  m_spinDVBCSymbolrates->AddLabel("5900", 7);
  m_spinDVBCSymbolrates->AddLabel("5000", 8);
  m_spinDVBCSymbolrates->AddLabel("3450", 9);
  m_spinDVBCSymbolrates->AddLabel("4000", 10);
  m_spinDVBCSymbolrates->AddLabel("6950", 11);
  m_spinDVBCSymbolrates->AddLabel("7000", 12);
  m_spinDVBCSymbolrates->AddLabel("6952", 13);
  m_spinDVBCSymbolrates->AddLabel("5156", 14);
  m_spinDVBCSymbolrates->AddLabel("4583", 15);
  m_spinDVBCSymbolrates->AddLabel("ALL (slow)", 16);

  m_spinDVBCqam = GUI->Control_getSpin(m_window, CONTROL_SPIN_DVBC_QAM);
  m_spinDVBCqam->Clear();
  m_spinDVBCqam->AddLabel("AUTO", 0);
  m_spinDVBCqam->AddLabel("64", 1);
  m_spinDVBCqam->AddLabel("128", 2);
  m_spinDVBCqam->AddLabel("256", 3);
  m_spinDVBCqam->AddLabel("ALL (slow)", 4);

  m_spinDVBTInversion = GUI->Control_getSpin(m_window, CONTROL_SPIN_DVBT_INVERSION);
  m_spinDVBTInversion->Clear();
  m_spinDVBTInversion->AddLabel("Auto", 0);
  m_spinDVBTInversion->AddLabel("On", 1);
  m_spinDVBTInversion->AddLabel("Off", 2);

  m_spinATSCType = GUI->Control_getSpin(m_window, CONTROL_SPIN_ATSC_TYPE);
  m_spinATSCType->Clear();
  m_spinATSCType->AddLabel("VSB (aerial)", 0);
  m_spinATSCType->AddLabel("QAM (cable)", 1);
  m_spinATSCType->AddLabel("VSB + QAM (aerial + cable)", 2);

  m_radioButtonTV = GUI->Control_getRadioButton(m_window, CONTROL_RADIO_BUTTON_TV);
  m_radioButtonTV->SetSelected(true);

  m_radioButtonRadio = GUI->Control_getRadioButton(m_window, CONTROL_RADIO_BUTTON_RADIO);
  m_radioButtonRadio->SetSelected(true);

  m_radioButtonFTA = GUI->Control_getRadioButton(m_window, CONTROL_RADIO_BUTTON_FTA);
  m_radioButtonFTA->SetSelected(true);

  m_radioButtonScrambled = GUI->Control_getRadioButton(m_window, CONTROL_RADIO_BUTTON_SCRAMBLED);
  m_radioButtonScrambled->SetSelected(true);

  m_radioButtonHD = GUI->Control_getRadioButton(m_window, CONTROL_RADIO_BUTTON_HD);
  m_radioButtonHD->SetSelected(true);

  SetControlsVisible(DVB_TERR);
  return true;
}

bool HDHomeRunChannelScan::OnAction(int actionId)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  if (actionId == ADDON_ACTION_CLOSE_DIALOG || actionId == ADDON_ACTION_PREVIOUS_MENU)
    OnClick(BUTTON_BACK);

  return true;
}

bool HDHomeRunChannelScan::OnInitCB(GUIHANDLE cbhdl)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  HDHomeRunChannelScan* scanner = static_cast<HDHomeRunChannelScan*>(cbhdl);
  return scanner->OnInit();
}

bool HDHomeRunChannelScan::OnClickCB(GUIHANDLE cbhdl, int controlId)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  HDHomeRunChannelScan* scanner = static_cast<HDHomeRunChannelScan*>(cbhdl);
  return scanner->OnClick(controlId);
}

bool HDHomeRunChannelScan::OnFocusCB(GUIHANDLE cbhdl, int controlId)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  HDHomeRunChannelScan* scanner = static_cast<HDHomeRunChannelScan*>(cbhdl);
  return scanner->OnFocus(controlId);
}

bool HDHomeRunChannelScan::OnActionCB(GUIHANDLE cbhdl, int actionId)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  HDHomeRunChannelScan* scanner = static_cast<HDHomeRunChannelScan*>(cbhdl);
  return scanner->OnAction(actionId);
}

void HDHomeRunChannelScan::SetControlsVisible(scantype_t type)
{
  XBMC->Log(LOG_DEBUG, "%s()", __func__);

  m_spinDVBCInversion->SetVisible(type == DVB_CABLE);
  m_spinDVBCSymbolrates->SetVisible(type == DVB_CABLE);
  m_spinDVBCqam->SetVisible(type == DVB_CABLE);
  m_spinDVBTInversion->SetVisible(type == DVB_TERR);
  m_spinATSCType->SetVisible(type == DVB_ATSC);
  m_radioButtonTV->SetVisible(type == DVB_TERR || type == DVB_CABLE || type == DVB_SAT || type == DVB_ATSC);
  m_radioButtonRadio->SetVisible(type == DVB_TERR || type == DVB_CABLE || type == DVB_SAT || type == DVB_ATSC);
  m_radioButtonFTA->SetVisible(type == DVB_TERR || type == DVB_CABLE || type == DVB_SAT || type == DVB_ATSC);
  m_radioButtonScrambled->SetVisible(type == DVB_TERR || type == DVB_CABLE || type == DVB_SAT || type == DVB_ATSC);
  m_radioButtonHD->SetVisible(type == DVB_TERR || type == DVB_CABLE || type == DVB_SAT || type == DVB_ATSC);
}

