// -*- C++ -*-

#ifndef CAEN_CONTROL_HH
#define CAEN_CONTROL_HH

#include <map>
#include <string>
#include <vector>

#include <CAENHVWrapper.h>

typedef std::map<int, std::vector<std::map<std::string, float>>> ChParam;
typedef std::map<int, std::vector<std::string>> StrList;

struct SlotInfo {
  int max_channel;
  std::vector<unsigned short> channel_list;
  std::string modelName;
  std::string description;
};


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
  int                  m_max_slot;
  std::map<int, SlotInfo> m_slot_info;
  int                  m_slot_number_cat;
  int                  m_slot_number_gem;
  std::vector<ushort> m_slot_list;
  
  
  
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
  //StrList              GetChannelName(int slot) const;
  std::vector<std::string> GetChannelName(int slot) const;
  float                GetChannelParam(int slot, int ch, const std::string& key) const;
  const std::string&   GetHostName() const { return m_host_name; }
  int                  GetLinkType() const { return m_link_type; }
  std::string          GetLinkTypeString() const;
  int                  GetMaxSlot() const {return m_max_slot; }
  int                  GetMaxChannel(int slot) const {
    auto it = m_slot_info.find(slot);
    if(it != m_slot_info.end())return it->second.max_channel;
    return 0;
  }
  const std::string&   GetPassword() const { return m_password; }
  int                  GetSlotNumberCAT() const { return m_slot_number_cat; }
  int                  GetSlotNumberGEM() const { return m_slot_number_gem; }
  const std::vector<ushort>& GetSlotList() const {return m_slot_list;}
  const std::map<int, SlotInfo>& GetSlotInfo() const {return m_slot_info; }
  const SlotInfo&      GetSlotInfo(int slot) const {
    return m_slot_info.at(slot);
  }
  StrList              GetStatusString(int slot) const;
  CAENHV_SYSTEM_TYPE_t GetSystemType() const { return m_system_type; }
  std::string          GetSystemTypeString() const;
  const std::string&   GetUserName() const { return m_user_name; }
  bool                 Initialize();
  // void                 Print(unsigned short slot, int max_channel);
  bool                 SetChannelParam(int slot, int ch, const std::string& key,
				       float val);
  void                 SetHostName(const std::string& h){ m_host_name = h; }
  void                 SetLinkType(int l){ m_link_type = l; }
  void                 SetMaxSlot(int slot){ m_max_slot = slot; }
  void                 SetMaxChannel(int slot, int m){
    m_slot_info[slot].max_channel = m;
  }
  void                 SetPassword(const std::string& p){ m_password = p; }
  void                 SetSlotNumberCAT(int s){ m_slot_number_cat = s; }
  void                 SetSlotNumberGEM(int s){ m_slot_number_gem = s; }
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
