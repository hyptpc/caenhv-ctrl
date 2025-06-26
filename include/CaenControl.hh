// -*- C++ -*-

#ifndef CAEN_CONTROL_HH
#define CAEN_CONTROL_HH

#include <map>
#include <string>
#include <vector>

#include <CAENHVWrapper.h>

typedef std::vector< std::map<std::string, float> > ChParam;
typedef std::vector<std::string> StrList;

class CaenControl
{
public:
  static std::string& ClassName();
  static CaenControl& GetInstance();
  ~CaenControl();

private:
  CaenControl();
  CaenControl(const CaenControl&);
  CaenControl& operator=(const CaenControl&);

private:
  enum EBoardStatus {
    BDSTATUS_OK,
    BDSTATUS_POWERFAIL,
    BDSTATUS_FIRMWARE_ERROR,
    BDSTATUS_CALIB_ERROR_HV,
    BDSTATUS_CALIB_ERROR_TEMP,
    BDSTATUS_UNDER_TEMP,
    BDSTATUS_OVER_TEMP,
    NBOARDSTATUS
  };
  enum EChannelStatus {
    CHSTATUS_POWER_ON,
    CHSTATUS_RAMP_UP,
    CHSTATUS_RAMP_DOWN,
    CHSTATUS_OVER_CURRENT,
    CHSTATUS_OVER_VOLTAGE,
    CHSTATUS_UNDER_VOLTAGE,
    CHSTATUS_EXTERNAL_TRIP,
    CHSTATUS_MAX_VOLTAGE,
    CHSTATUS_EXTERNAL_DISABLE,
    CHSTATUS_INTERNAL_TRIP,
    CHSTATUS_UNPLUGGED,
    CHSTATUS_RESERVED,
    CHSTATUS_OVER_PROTECTION,
    CHSTATUS_POWER_FAIL,
    CHSTATUS_TEMP_ERROR,
    NCHANNELSTATUS
  };
  int                  m_system_handle;
  CAENHV_SYSTEM_TYPE_t m_system_type;
  int                  m_link_type;
  std::string          m_host_name;
  std::string          m_user_name;
  std::string          m_password;
  int                  m_slot_number;
  int                  m_max_channel;
  unsigned short*      m_slot_list;
  unsigned short*      m_channel_list;
  // CAENHVEVENT_TYPE_t*  m_event;
  struct CaenEvent {
    unsigned int board_status;
    float        board_temp;
    ChParam      channel_param;
    StrList      channel_name;
  };
  CaenEvent*           m_event;

public:
  bool                 Finalize();
  unsigned int         GetBoardStatus() const;
  std::string          GetBoardStatusString() const;
  float                GetBoardTemp() const;
  StrList              GetChannelName() const;
  float                GetChannelParam(int ch, const std::string& key) const;
  const std::string&   GetHostName() const { return m_host_name; }
  int                  GetLinkType() const { return m_link_type; }
  std::string          GetLinkTypeString() const;
  int                  GetMaxChannel() const { return m_max_channel; }
  const std::string&   GetPassword() const { return m_password; }
  int                  GetSlotNumber() const { return m_slot_number; }
  StrList              GetStatusString() const;
  CAENHV_SYSTEM_TYPE_t GetSystemType() const { return m_system_type; }
  std::string          GetSystemTypeString() const;
  const std::string&   GetUserName() const { return m_user_name; }
  bool                 Initialize();
  // void                 Print(unsigned short slot, int max_channel);
  bool                 SetChannelParam(int ch, const std::string& key,
				       float val);
  void                 SetHostName(const std::string& h){ m_host_name = h; }
  void                 SetLinkType(int l){ m_link_type = l; }
  void                 SetMaxChannel(int m){ m_max_channel = m; }
  void                 SetPassword(const std::string& p){ m_password = p; }
  void                 SetSlotNumber(int s){ m_slot_number = s; }
  void                 SetSystemType(CAENHV_SYSTEM_TYPE_t s){ m_system_type = s; }
  void                 SetUserName(const std::string& u){ m_user_name = u; }
  bool                 Update();
};

//_____________________________________________________________________________
inline std::string&
CaenControl::ClassName()
{
  static std::string s_name("CaenControl");
  return s_name;
}

//_____________________________________________________________________________
inline CaenControl&
CaenControl::GetInstance()
{
  static CaenControl s_instance;
  return s_instance;
}

#endif
