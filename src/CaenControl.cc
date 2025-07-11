// -*- C++ -*-

#include "CaenControl.hh"

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <libgen.h>
#include <sstream>

namespace
{
}

//_____________________________________________________________________________
CaenControl::CaenControl()
  : m_system_handle(-1),
    m_system_type(SY5527),
    m_link_type(LINKTYPE_TCPIP),
    m_host_name("localhost"),
    m_user_name("admin"),
    m_password("admin"),
    m_slot_number(0),
    m_max_channel(6),
    m_slot_list(nullptr),
    m_channel_list(nullptr),
    m_event(nullptr)
{
}

//_____________________________________________________________________________
CaenControl::~CaenControl()
{
  if(m_channel_list)
    delete m_channel_list;
}

//_____________________________________________________________________________
bool
CaenControl::Finalize()
{
  int ret = CAENHV_DeinitSystem(m_system_handle);
  if(ret == CAENHV_OK){
    return true;
  } else {
    std::cerr << "[CaenControl::Finalize()] CAENHV_DeinitSystem: "
	      << CAENHV_GetError(m_system_handle)
  	      << "(" << ret << ")" << std::endl;
    return false;
  }
}

//_____________________________________________________________________________
unsigned int
CaenControl::GetBoardStatus() const
{
  return m_event->board_status;
}

//_____________________________________________________________________________
std::string
CaenControl::GetBoardStatusString() const
{
  switch(m_event->board_status){
  case BDSTATUS_OK: return "OK";
  default: return "UNKNOWN";
  }
}

//_____________________________________________________________________________
float
CaenControl::GetBoardTemp() const
{
  return m_event->board_temp;
}

//_____________________________________________________________________________
StrList
CaenControl::GetChannelName() const
{
  return m_event->channel_name;
}

//_____________________________________________________________________________
float
CaenControl::GetChannelParam(int ch, const std::string& key) const
{
  return m_event->channel_param[ch][key];
}

//_____________________________________________________________________________
std::string
CaenControl::GetLinkTypeString() const
{
  switch(m_link_type){
  case LINKTYPE_TCPIP:   return "TCPIP";
  case LINKTYPE_RS232:   return "RS232";
  case LINKTYPE_CAENET:  return "CAENET";
  case LINKTYPE_USB:     return "USB";
  case LINKTYPE_OPTLINK: return "OPTLINK";
  case LINKTYPE_USB_VCP: return "USB_VCP";
  default:               return "UNKNOWN";
  }
}

//_____________________________________________________________________________
StrList
CaenControl::GetStatusString() const
{
  StrList status_list(m_max_channel);
  for(int i=0; i<m_max_channel; ++i){
    std::bitset<NCHANNELSTATUS> stbit((int)GetChannelParam(i, "Status"));
    if(stbit.count() == 0){
      // status_list[i] = "PwOff";
      continue;
    }
    if(stbit.test(CHSTATUS_POWER_ON) && (int)GetChannelParam(i, "Pw"))
      status_list[i] = "  PwOn  ";
    if(stbit.test(CHSTATUS_RAMP_UP))
      status_list[i] = "   Up   ";
    if(stbit.test(CHSTATUS_RAMP_DOWN))
      status_list[i] = "  Down  ";
    if(stbit.test(CHSTATUS_OVER_CURRENT))
      status_list[i] = " OvCurr ";
    if(stbit.test(CHSTATUS_OVER_VOLTAGE))
      status_list[i] = " OvVolt ";
    if(stbit.test(CHSTATUS_UNDER_VOLTAGE))
      status_list[i] = " UnVolt ";
    if(stbit.test(CHSTATUS_EXTERNAL_TRIP))
      status_list[i] = " ExTrip ";
    if(stbit.test(CHSTATUS_MAX_VOLTAGE))
      status_list[i] = "  MaxV  ";
    if(stbit.test(CHSTATUS_EXTERNAL_DISABLE))
      status_list[i] = " ExtDis ";
    if(stbit.test(CHSTATUS_INTERNAL_TRIP))
      status_list[i] = " InTrip ";
    if(stbit.test(CHSTATUS_UNPLUGGED))
      status_list[i] = " Unplgd ";
    if(stbit.test(CHSTATUS_RESERVED))
      status_list[i] = "  Rsvd  ";
    if(stbit.test(CHSTATUS_OVER_PROTECTION))
      status_list[i] = " OvProt ";
    if(stbit.test(CHSTATUS_POWER_FAIL))
      status_list[i] = " PwFail ";
    if(stbit.test(CHSTATUS_TEMP_ERROR))
      status_list[i] = " TempEr ";
  }
  return status_list;
}

//_____________________________________________________________________________
std::string
CaenControl::GetSystemTypeString() const
{
  switch(m_system_type){
  case SY1527:  return "SY1527";
  case SY2527:  return "SY2527";
  case SY4527:  return "SY4527";
  case SY5527:  return "SY5527";
  case N568:    return "N568";
  case V65XX:   return "V65XX";
  case N1470:   return "N1470";
  case V8100:   return "V8100";
  case N568E:   return "N568E";
  case DT55XX:  return "DT55XX";
  case FTK:     return "FTK";
  case DT55XXE: return "DT55XXE";
  case N1068:   return "N1068";
  default:      return "UNKNOWN";
  }
}

//_____________________________________________________________________________
bool
CaenControl::Initialize()
{
  int ret = CAENHV_InitSystem(m_system_type,
			      m_link_type,
			      (void*)m_host_name.c_str(),
			      m_user_name.c_str(),
			      m_password.c_str(),
			      &m_system_handle);
  if(ret != CAENHV_OK){
    std::cerr << "[CaenControl::Initialize()] CAENHV_InitSystem:"
	      << CAENHV_GetError(m_system_handle)
	      << "(" << ret << ")" << std::endl
	      << "   system type = " << GetSystemTypeString()
	      << " (" << m_system_type << ")" << std::endl
	      << "   link type   = " << GetLinkTypeString()
	      << " (" << m_link_type << ")" << std::endl
	      << "   host name   = " << m_host_name << std::endl
	      << "   user name   = " << m_user_name << std::endl
	      << "   password    = " << m_password << std::endl;
    return false;
  } else {
    m_slot_list = new unsigned short[1];
    m_slot_list[0] = m_slot_number;
    m_channel_list = new unsigned short[m_max_channel];
    for(int i=0; i<m_max_channel; ++i)
      m_channel_list[i] = i;
    m_event = new CaenEvent;
    m_event->channel_param.resize(m_max_channel);
    m_event->channel_name.resize(m_max_channel);
    return true;
  }
}

//_____________________________________________________________________________
// void
// CaenControl::Print(unsigned short slot, int max_channel)
// {
//   unsigned short slot_list[] = { slot };
//   const int m = max_channel;
//   unsigned short channel_list[m_max_channel];
//   for(int i=0; i<m; ++i) channel_list[i] = i;
//   char ChName[m][MAX_CH_NAME];
//   float V0Set[m];
//   float I0Set[m];
//   float VMon[m];
//   float IMon[m];
//   float RUp[m];
//   float RDWn[m];
//   unsigned int Pw[m];
//   unsigned int Status[m];
//   unsigned int BdStatus = 0;

//   CAENHV_GetBdParam(m_system_handle, slot, slot_list, "BdStatus", &BdStatus);
//   CAENHV_GetChName(m_system_handle, slot, m, channel_list, ChName);
//   CAENHV_GetChParam(m_system_handle, slot, "V0Set", m, channel_list, V0Set);
//   CAENHV_GetChParam(m_system_handle, slot, "I0Set", m, channel_list, I0Set);
//   CAENHV_GetChParam(m_system_handle, slot, "VMon", m, channel_list, VMon);
//   CAENHV_GetChParam(m_system_handle, slot, "IMon", m, channel_list, IMon);
//   CAENHV_GetChParam(m_system_handle, slot, "RUp", m, channel_list, RUp);
//   CAENHV_GetChParam(m_system_handle, slot, "RDWn", m, channel_list, RDWn);
//   CAENHV_GetChParam(m_system_handle, slot, "Pw", m, channel_list, Pw);
//   CAENHV_GetChParam(m_system_handle, slot, "Status", m, channel_list, Status);

//   printf("BdStatus %u\n", BdStatus);
//   printf("CH Name       V0Set  I0Set   VMon   IMon    RUp   RDWn Pw  Status\n");
//   for(int i=0;i<m;i++){
//     printf("%2d %9s %6.0f %6.2f %6.0f %6.2f %6.0f %6.0f %3s %6u\n",
// 	    i, ChName[i], V0Set[i], I0Set[i], VMon[i], IMon[i],
// 	    RUp[i], RDWn[i], Pw[i] ? "ON":"OFF", Status[i]);
//   }
// }

//_____________________________________________________________________________
bool
CaenControl::SetChannelParam(int ch, const std::string& key, float val)
{
  if(val == m_event->channel_param[ch][key])
    return true;
  unsigned short channel_list[] = { (unsigned short)ch };
  unsigned int uint_list[] = { (unsigned int)val };
  float float_list[] = { val };
  int ret = 0;
  if(key == "Pw" || key == "Status"){
    ret = CAENHV_SetChParam(m_system_handle, m_slot_number, key.c_str(),
			    1, channel_list, uint_list);
  } else {
    ret = CAENHV_SetChParam(m_system_handle, m_slot_number, key.c_str(),
			    1, channel_list, float_list);
  }
  if(ret == CAENHV_OK)
    return true;
  else
    return false;
}

//_____________________________________________________________________________
bool
CaenControl::Update()
{
  // Board
  CAENHV_GetBdParam(m_system_handle, m_slot_number, m_slot_list,
		    "BdStatus", &m_event->board_status);
  CAENHV_GetBdParam(m_system_handle, m_slot_number, m_slot_list,
		    "Temp", &m_event->board_temp);
  // Channel Name
  char ChName[m_max_channel][MAX_CH_NAME];
  CAENHV_GetChName(m_system_handle, m_slot_number, m_max_channel,
		   m_channel_list, ChName);
  for(int i=0; i<m_max_channel; ++i){
    m_event->channel_name[i] = ChName[i];
  }
  // Channel Param
  StrList float_chname = { "V0Set", "VMon", "I0Set", "IMon", "RUp", "RDWn" };
  StrList uint_chname = { "Pw", "Status" };
  for(const auto& n : float_chname){
    float ChParam[m_max_channel];
    CAENHV_GetChParam(m_system_handle, m_slot_number, n.c_str(),
		      m_max_channel, m_channel_list, ChParam);
    for(int i=0; i<m_max_channel; ++i){
      m_event->channel_param[i][n] = ChParam[i];
    }
  }
  for(const auto& n : uint_chname){
    unsigned int ChParam[m_max_channel];
    CAENHV_GetChParam(m_system_handle, m_slot_number, n.c_str(),
		      m_max_channel, m_channel_list, ChParam);
    for(int i=0; i<m_max_channel; ++i){
      m_event->channel_param[i][n] = (float)ChParam[i];
    }
  }
  return true;

#if 0 // OLD
  static int sock = (int)(*m_host_name.c_str());
  unsigned int ItemCount;
  CAENHV_SYSTEMSTATUS_t stat;
  int ret = CAENHV_GetEventData(sock, &stat, &m_event, &ItemCount);
  if(ret != CAENHV_OK){
    /* we assume we lost connection
    ** with the power supply,
    ** so we can exit thread */
    return false;
  }
  // printf("%d", stat.System);
  for(unsigned int k=0; k<ItemCount; ++k){
    switch (m_event[k].Type){
    case EVENTTYPE_PARAMETER:
      // std::cout << &m_event[k] << std::endl;
      /* handle parameter update */
      break;
    case EVENTTYPE_ALARM:
      /* handle alert */
      break;
    case EVENTTYPE_KEEPALIVE:
      /* handle keepalive */
      break;
    }
  }
  if(m_event)
    CAENHV_FreeEventData(&m_event);
  return true;
#endif
}
