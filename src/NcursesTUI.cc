// -*- C++ -*-

#include "NcursesTUI.hh"

#include <ncurses.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <tuple>
#include <chrono>

#include "CaenControl.hh"
#include "MpodLvStatus.hh"
#include "HypTPCParam.hh"

namespace
{
auto& gCaen = CaenControl::GetInstance();
int NCmd = 0;
int NCh = 0;
}

//_____________________________________________________________________________
void
NcursesTUI::Command::operator ()(bool s)
{
  int slot_cat = gCaen.GetSlotNumberCAT();
  int slot_gem = gCaen.GetSlotNumberGEM();

  switch(index){
  case CMD_CATHODE:
    gCaen.SetChannelParam(slot_cat, 0, "Pw", s);
    break;
  case CMD_GEM:
    gCaen.SetChannelParam(slot_gem, 0, "Pw", s);
    break;
  case CMD_GATE:
    gCaen.SetChannelParam(slot_gem, 3, "Pw", s);
    gCaen.SetChannelParam(slot_gem, 1, "Pw", s);
    gCaen.SetChannelParam(slot_gem, 2, "Pw", s);
    break;
  default:
    break;
  }
}

//_____________________________________________________________________________
NcursesTUI::NcursesTUI()
  : m_lock(),
    m_catlock(),
    m_command_list(),
    m_status_list(),
    m_cursor_x(0),
    m_cursor_y(0),
    m_key(0),
    m_board_status(false),
    m_mpod_status(false),
    m_drift_field(0),
    m_vcat(),
    m_vfield(),
    m_vgem(),
    m_vgate_plus_diff(0),
    m_vgate_minus_diff(0)
{
  m_command_list.push_back(Command(CMD_CATHODE, "Cathode"));
  m_command_list.push_back(Command(CMD_GEM,     "GEM    "));
  m_command_list.push_back(Command(CMD_GATE,    "Gate   "));
  
  NCmd = m_command_list.size();
  int slot_cat = gCaen.GetSlotNumberCAT();
  int slot_gem = gCaen.GetSlotNumberGEM();
  for(int slot : gCaen.GetSlotList()){
    const SlotInfo& info = gCaen.GetSlotInfo(slot);
    int maxch = info.max_channel;
    NCh += maxch;
  }
  
  for(int i=0, n=m_command_list.size(); i<n; ++i){
    m_status_list.push_back(false);
  }
  ::initscr();
  ::timeout(200); // [ms]
  ::cbreak();
  ::noecho();
  //::nodelay(stdscr, FALSE);
  ::curs_set(0);
  ::keypad(stdscr, TRUE);
  //::mousemask(ALL_MOUSE_EVENTS, nullptr);
  if(::has_colors()){
    ::start_color();
    //           Index          Foreground    Background
    ::init_pair(CP_DEFAULT,    COLOR_WHITE,  COLOR_BLACK );
    ::init_pair(CP_RED,        COLOR_RED,    COLOR_BLACK );
    ::init_pair(CP_GREEN,      COLOR_GREEN,  COLOR_BLACK );
    ::init_pair(CP_YELLOW,     COLOR_YELLOW, COLOR_BLACK );
    ::init_pair(CP_BLUE,       COLOR_BLUE,   COLOR_BLACK );
    ::init_pair(CP_WHITE,      COLOR_WHITE,  COLOR_BLACK );
    ::init_pair(CP_CURSOR,     COLOR_BLACK,  COLOR_WHITE );
    ::init_pair(CP_TOPBAR,     COLOR_WHITE,  COLOR_BLUE );
    // ::init_pair(CP_TOPBAR,     COLOR_WHITE,  COLOR_BLACK );
    // ::init_pair(CP_TOPBAR,     COLOR_BLACK,  COLOR_WHITE );
    ::init_pair(CP_RED_REV,    COLOR_WHITE,  COLOR_RED   );
    ::init_pair(CP_GREEN_REV,  COLOR_WHITE,  COLOR_GREEN );
    ::init_pair(CP_YELLOW_REV, COLOR_BLACK,  COLOR_YELLOW);
    ::init_pair(CP_BLUE_REV,   COLOR_WHITE,  COLOR_BLUE  );
    ::init_pair(CP_MAGENTA_REV, COLOR_WHITE,  COLOR_MAGENTA);
    ::init_pair(CP_WHITE_REV,   COLOR_BLACK,  COLOR_WHITE );
    ::bkgd(COLOR_PAIR(CP_DEFAULT) // | A_BOLD
	   );
  }
  gCaen.Update();
  
  m_drift_field = (int)(gCaen.GetChannelParam(slot_cat,0, "V0Set") *
			cm / (Cathode - GemTop));
  m_vgem = gCaen.GetChannelParam(slot_gem, 0, "V0Set");
  m_vgate_plus_diff = gCaen.GetChannelParam(slot_gem,0,"V0Set") - gCaen.GetChannelParam(slot_gem,2,"V0Set");
  m_vgate_minus_diff = gCaen.GetChannelParam(slot_gem,3,"V0Set") - gCaen.GetChannelParam(slot_gem,0,"V0Set");
  
  m_status_list[CMD_CATHODE] = (gCaen.GetChannelParam(slot_cat,0, "Pw") == 1);
  m_status_list[CMD_GEM]     = (gCaen.GetChannelParam(slot_gem,0, "Pw") == 1);
  m_status_list[CMD_GATE]    = (gCaen.GetChannelParam(slot_gem,1, "Pw") +
				gCaen.GetChannelParam(slot_gem,2, "Pw") +
				gCaen.GetChannelParam(slot_gem,3, "Pw") == 3);
}

//_____________________________________________________________________________
NcursesTUI::~NcursesTUI()
{
  ::endwin();
  if(m_key < 0)
    std::cerr << "Connection error" << std::endl;
}

//_____________________________________________________________________________
void
NcursesTUI::Clear()
{
  ::clear();
  ::move(0, 0);
  ::refresh();
}

//_____________________________________________________________________________
void
NcursesTUI::DrawChannelList()
{
  int line = 5+NCmd;
  int real_line = NCmd;
  for(int slot : gCaen.GetSlotList()){
    const SlotInfo& info = gCaen.GetSlotInfo(slot);
    int maxch = info.max_channel;
    GotoXY(1,line);
    auto color = COLOR_PAIR(CP_WHITE_REV);
    ::attron(color);
    Printf("Ch Name     V0Set    VMon     I0Set       IMon        RUp      RDWn     Status  ");
    ::attroff(color);
    auto ChannelName = gCaen.GetChannelName(slot);
    auto Status      = gCaen.GetStatusString(slot);
    for(int i=0, n=maxch; i<n; ++i){
      line++;
      GotoXY(1, line);
      Printf("%02d", i);
      GotoXY(4, line);
      Printf("%-7s", ChannelName[i].c_str());
      GotoXY(13, line);
      Printf("%5.0f V", gCaen.GetChannelParam(slot, i, "V0Set"));
      GotoXY(22, line);
      Printf("%5.0f V", gCaen.GetChannelParam(slot, i, "VMon"));
      GotoXY(31, line);
      if(!m_lock && real_line == m_cursor_y)
	color = COLOR_PAIR(CP_CURSOR);
      else
	color = COLOR_PAIR(CP_DEFAULT);
      ::attron(color);
      Printf("%7.2f uA", gCaen.GetChannelParam(slot, i, "I0Set"));
      ::attroff(color);
      GotoXY(43, line);
      Printf("%7.2f uA", gCaen.GetChannelParam(slot, i, "IMon"));
      GotoXY(55, line);
      Printf("%3.0f V/s", gCaen.GetChannelParam(slot, i, "RUp"));
      GotoXY(64, line);
      Printf("%3.0f V/s", gCaen.GetChannelParam(slot, i, "RDWn"));
      GotoXY(73, line);
      
      if (Status.count(slot) == 0 || i >= Status.at(slot).size()) {
	color = COLOR_PAIR(CP_DEFAULT);
      } else {
	const std::string& state = Status.at(slot).at(i);
	if (state.empty()) {
	  color = COLOR_PAIR(CP_DEFAULT);
	} else if (state.find("PwOn") != std::string::npos) {
	  color = COLOR_PAIR(CP_RED_REV);
	} else if (state.find("Up") != std::string::npos || state.find("Down") != std::string::npos) {
	  color = COLOR_PAIR(CP_YELLOW_REV) | A_BLINK;
	} else {
	  color = COLOR_PAIR(CP_BLUE_REV);
	}
      }
      color |= A_BOLD;
      ::attron(color);
      Printf("%-7s", Status[slot][i].c_str());
      ::attroff(color);

      real_line++;

    }
    real_line+=2;
    line+=2;
  }
}

//_____________________________________________________________________________
void
NcursesTUI::DrawCommandList()
{
  int color;
  for(int i=0, n=m_command_list.size(); i<n; ++i){
    GotoXY(1, i+4);
    color = COLOR_PAIR(CP_DEFAULT);
    if(!m_lock && 0 == m_cursor_x && i == m_cursor_y)
      color = COLOR_PAIR(CP_CURSOR);
    else
      color = COLOR_PAIR(CP_DEFAULT);
    ::attron(color);
    auto bgcolor = color;
    Printf("[");
    if(!m_lock && 0 == m_cursor_x && i == m_cursor_y)
      color = m_status_list[i] ? COLOR_PAIR(CP_RED_REV) : COLOR_PAIR(CP_BLUE_REV);
    else
      color = m_status_list[i] ? COLOR_PAIR(CP_RED) : COLOR_PAIR(CP_BLUE);
    color |= A_BOLD;
    ::attron(color);
    Printf("%5s", m_status_list[i] ? " ON  " : " OFF ");
    ::attroff(color);
    ::attron(bgcolor);
    Printf("] %s", m_command_list[i].label.c_str());
    ::attroff(bgcolor);
    m_command_list[i](m_status_list[i]);
  }

  
  GotoXY(20, 4);
  Printf("VFLD=");
  color = (!m_lock && 1 == m_cursor_x && 0 == m_cursor_y ?
	   COLOR_PAIR(CP_WHITE_REV) : COLOR_PAIR(CP_DEFAULT));
  ::attron(color);
  Printf("%5.1f V/cm", m_drift_field);
  ::attroff(color);
  Printf("-> VCAT-VGEM=");
  Printf("%5.0f V",m_vfield);

  GotoXY(60, 4); 
  Printf("VCAT=");
  color = (!m_lock && 2 == m_cursor_x && 0 == m_cursor_y ?
	   COLOR_PAIR(CP_WHITE_REV) : COLOR_PAIR(CP_DEFAULT)); 
  ::attron(color);
  Printf("%5.0f V", m_vcat);
  ::attroff(color); 
  
  GotoXY(20, 5);
  Printf("VGEM=");
  color = (!m_lock && 1 == m_cursor_x && 1 == m_cursor_y ?
	   COLOR_PAIR(CP_WHITE_REV) : COLOR_PAIR(CP_DEFAULT));
  ::attron(color);
  Printf("%5.0f V   ", m_vgem);
  ::attroff(color);
  GotoXY(20, 6);
  Printf("VGATE+ =");
  color = (!m_lock && 1 == m_cursor_x && 2 == m_cursor_y ?
	   COLOR_PAIR(CP_WHITE_REV) : COLOR_PAIR(CP_DEFAULT));
  ::attron(color);
  Printf("%5.0f V   ", m_vgate_plus_diff);
  ::attroff(color);

  GotoXY(40, 6);
  Printf("VGATE- =");
  color = (!m_lock && 2 == m_cursor_x && 2 == m_cursor_y ?
	   COLOR_PAIR(CP_WHITE_REV) : COLOR_PAIR(CP_DEFAULT));
  ::attron(color);
  Printf("%5.0f V   ", m_vgate_minus_diff);
  ::attroff(color);

}

//_____________________________________________________________________________
void
NcursesTUI::DrawDebug()
{
  static int refresh = 0;
  GotoXY(1, NCmd + NCh + 10);
  ::attron(COLOR_PAIR(CP_MAGENTA_REV));
  Puts(std::string("DEBUG") + std::string(75, ' '));
  ::attroff(COLOR_PAIR(CP_MAGENTA_REV));
  Printf("Key=%3d %c", m_key, (m_key < 0) ? ' ' : m_key);
  GotoXY(13, NCmd + NCh + 11);
  Printf("Cursor=(%d, %d)   ", m_cursor_x, m_cursor_y);
  Printf("BdStatus=%d   ", m_board_status);
  Printf("MpodStatus=%d   ", m_mpod_status);
  Printf("Refresh=%8d\n", ++refresh);
  Printf("CmdFlag=(,  ,");
  for(int i=0; i<NCmd; ++i){
    GotoXY(10 + i*3, NCmd + NCh + 12);
    Printf("%d", m_status_list[i]);
  }
  Printf(")  DriftField=%.3f\n", m_drift_field);
  Printf("PadPlane=%.1f  GemTop=%.1f  GatringGrid=%.1f  Cathode=%.1f",
	 PadPlane, GemTop, GatingGrid, Cathode);
}

//_____________________________________________________________________________
void
NcursesTUI::DrawKeyList()
{
  auto color = COLOR_PAIR(CP_DEFAULT);
  GotoXY(1, NCmd + NCh + 8);
  Printf("[q] Quit");

  Printf(" [l] Lock=");
  if(!m_lock)
    color = COLOR_PAIR(CP_YELLOW) | A_BOLD;
  else
    color = COLOR_PAIR(CP_DEFAULT);
  ::attron(color);
  Printf("%s", (m_lock ? "ON" : "OFF"));
  ::attroff(color);
  
  Printf(" [c] Cathode Lock=");
  if(!m_catlock)
    color = COLOR_PAIR(CP_YELLOW) | A_BOLD;
  else
    color = COLOR_PAIR(CP_DEFAULT);
  ::attron(color);
  Printf("%s", (m_catlock ? "ON" : "OFF"));
  ::attroff(color);
  
}

//_____________________________________________________________________________
void
NcursesTUI::DrawTopBar()
{
  std::time_t t = std::time(nullptr);
  const tm* lt = std::localtime(&t);
  std::stringstream s;
  s << lt->tm_year + 1900 << "/"
    << std::setw(2) << std::setfill('0') << lt->tm_mon + 1 << "/"
    << std::setw(2) << std::setfill('0') << lt->tm_mday << " "
    << std::setw(2) << std::setfill('0') << lt->tm_hour << ":"
    << std::setw(2) << std::setfill('0') << lt->tm_min << ":"
    << std::setw(2) << std::setfill('0') << lt->tm_sec;
  auto color = COLOR_PAIR(CP_TOPBAR);
  ::attron(color);
  Printf("%s\n", std::string(80, ' ').c_str());
  GotoXY(1, 1);
  Printf("CAENHV Controller for HypTPC %s (%s@%s)",
	 gCaen.GetSystemTypeString().c_str(),
	 gCaen.GetUserName().c_str(), gCaen.GetHostName().c_str());
  GotoXY(80 - s.str().size() + 1, 1);
  Puts(s.str());
  ::attroff(color);
  
  auto bdstatus = gCaen.GetBoardStatusString();
  GotoXY(61 - bdstatus.size(), 2);
  Printf("BdStatus=", bdstatus.c_str());
  color = (m_board_status ? COLOR_PAIR(CP_DEFAULT) :
	   COLOR_PAIR(CP_YELLOW) | A_BOLD | A_BLINK);
  ::attron(color);
  Printf("%s", bdstatus.c_str());
  ::attroff(color);
  GotoXY(72, 2);
  Printf("MPOD=");
  color = (m_mpod_status ? COLOR_PAIR(CP_DEFAULT) :
	   COLOR_PAIR(CP_YELLOW) | A_BOLD | A_BLINK);
  ::attron(color);
  Printf("%s", m_mpod_status ? "OK" : "OFF");
  ::attroff(color);

  /*
  int line = 2;
  for(int slot : gCaen.GetSlotList()){
    auto info = gCaen.GetSlotInfo(slot);
    GotoXY(1,line);

    std::cout << "SLOT#" << std::setw(2) << slot << " "
	      << std::left << std::setw(10) << info.modelName
	      << std::left << std::setw(24) << info.description
	      << "Temp=" << gCaen.GetBoardTemp() << "°C"<<std::endl;
    

    line ++;
  }
  */


}

//_____________________________________________________________________________
std::string
NcursesTUI::GetChar()
{
  int x = -1, y = -1;
  // VFLD
  if(m_cursor_x == 1 && m_cursor_y == 0){
    x = 25;
    y =  4;
  }
  // VCathode
  if(m_cursor_x == 2 && m_cursor_y == 0){
    x = 70;
    y =  4;
  }
  // VGEM
  if(m_cursor_x == 1 && m_cursor_y == 1){
    x = 25;
    y =  5;
  }
  // VGATE+
  if(m_cursor_x == 1 && m_cursor_y == 2){
    x = 25;
    y =  6;
  }
  // VGATE-
  if(m_cursor_x == 2 && m_cursor_y == 2){
    x = 50;
    y =  6;
  }
  // I0Set
  if(m_cursor_y >= NCmd){
    x = 31;
    y = m_cursor_y + NCmd + 3;
  }
  
  auto color = COLOR_PAIR(CP_WHITE_REV);
  ::attron(color);
  std::string input;
  GotoXY(x, y);
  Printf("%10s", input.c_str());
  while((m_key = ::getch()) != '\n'){
    if(m_key < 0)
      continue;
    if(m_key == KEY_BACKSPACE){
      if(input.size() > 0) input.pop_back();
    } else {
      if(input.size() >= 10) input.erase(input.begin());
      input.push_back((char)m_key);
    }
    GotoXY(x, y);
    Printf("%10s", input.c_str());
  }
  ::attroff(color);
  return input;
}

//_____________________________________________________________________________
void
NcursesTUI::GotoXY(int x, int y)
{
  ::move(y-1, x-1);
  ::refresh();
}

//_____________________________________________________________________________
bool
NcursesTUI::Next()
{
  m_key = ::getch();
  if(m_key == 'q')
    return FALSE;
  switch(m_key){
  case 'l':
    m_lock = !m_lock;
    break;
  case 'c':
    m_catlock = !m_catlock;
  case KEY_UP:
    if(!m_lock && m_cursor_y > 0)
      m_cursor_y--;
    break;
  case KEY_DOWN:
    if(!m_lock &&
       m_cursor_y < NCmd + NCh + 1)
      m_cursor_y++;
    break;
  case KEY_LEFT:
    if(!m_lock && m_cursor_x > 0)
      m_cursor_x--;
    break;
  case KEY_RIGHT:
    if(!m_lock && m_cursor_x < NCmd - 1)
      m_cursor_x++;
    break;
  case ' ':
    if(m_lock) break;
    if(m_cursor_x == 0 && m_cursor_y < NCmd)
      m_status_list[m_cursor_y] = !m_status_list[m_cursor_y];
    
    // VFLD
    if(m_cursor_x == 1 && m_cursor_y == 0){
      auto input = GetChar();
      if(!input.empty()){
	long val = std::strtol(input.c_str(), nullptr, 10);
	m_drift_field = (float)val;
	m_vfield = m_drift_field*(Cathode - GatingGrid)/cm * (R_Cat_FCB + R_Cathode) / R_Cat_FCB;
	m_vcat = m_vfield + m_vgem;
	gCaen.SetChannelParam(gCaen.GetSlotNumberCAT(), 0, "V0Set",m_vcat);
      }
    }
    if(m_cursor_x == 2 && m_cursor_y == 0){
      auto input = GetChar();
      if(!input.empty()){
	long val = std::strtol(input.c_str(), nullptr, 10);
	m_vcat = (float)val;
	gCaen.SetChannelParam(gCaen.GetSlotNumberCAT(), 0, "V0Set", m_vcat);
	m_vfield = m_vcat - m_vgem;
	m_drift_field = (float)m_vfield / ((float)(Cathode - GatingGrid)/cm) * (R_Cat_FCB) / (R_Cat_FCB + R_Cathode);
      }
    }
    // VGEM
    if(m_cursor_x == 1 && m_cursor_y == 1){
      auto input = GetChar();
      if(!input.empty()){
	long val = std::strtol(input.c_str(), nullptr, 10);
	m_vgem = (float)val;

	gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(), 0, "V0Set", m_vgem);
	gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(), 1, "V0Set", m_vgem);
	gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(), 2, "V0Set", m_vgem - m_vgate_plus_diff);
	gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(), 3, "V0Set", m_vgem + m_vgate_minus_diff);
	
	if(!m_catlock){
	  m_vcat = m_vfield + m_vgem;
	  //m_vcat = m_vfield;
	  gCaen.SetChannelParam(gCaen.GetSlotNumberCAT(), 0, "V0Set",m_vcat);
	}
      }
    }

    // VGATE+
    if(m_cursor_x == 1 && m_cursor_y == 2){
      auto input = GetChar();
      if(!input.empty()){
	long val = std::strtol(input.c_str(), nullptr, 10);
	m_vgate_plus_diff = (float)val;

	if(m_vgem < m_vgate_plus_diff)m_vgate_plus_diff = m_vgem;
	gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(), 2, "V0Set", m_vgem - m_vgate_plus_diff);
      }
    }

    // VGATE-
    if(m_cursor_x == 2 && m_cursor_y == 2){
      auto input = GetChar();
      if(!input.empty()){
	long val = std::strtol(input.c_str(), nullptr, 10);
	m_vgate_minus_diff = (float)val;
   
	gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(), 3, "V0Set", m_vgem + m_vgate_minus_diff);
      }
    }

    
    
    
    // I0Set
    if(NCmd <= m_cursor_y){
      auto input = GetChar();
      if(!input.empty()){
	const SlotInfo& info = gCaen.GetSlotInfo(gCaen.GetSlotNumberGEM());
	int slot_ch_gem = info.max_channel;
	double val = std::strtod(input.c_str(), nullptr);
	if(m_cursor_y < slot_ch_gem + NCmd + 2)
	  gCaen.SetChannelParam(gCaen.GetSlotNumberGEM(),m_cursor_y - NCmd, "I0Set", val); 
	
	else if(m_cursor_y >= slot_ch_gem + NCmd + 2)
	  gCaen.SetChannelParam(gCaen.GetSlotNumberCAT(),m_cursor_y - NCmd - slot_ch_gem - 2, "I0Set", val);
	
      }
    }
    break;
  case KEY_MOUSE:
    break;
  default:
    break;
  }
  m_lock |= !m_board_status | !m_mpod_status;
  m_catlock |= !m_board_status | !m_mpod_status;
  
  return TRUE;
}

void NcursesTUI::SaveFile()
{
  std::ofstream ofs("/home/oper/share/monitor-tmp/caenhv-hyptpc.txt", std::ios::app); 
  if (!ofs.is_open()) {
    std::cerr<<"Cannot Open the File"<<std::endl;
    return;
  }

  std::time_t unix_time = std::time(nullptr);
 
  double VCatSet  = gCaen.GetChannelParam(gCaen.GetSlotNumberCAT(), 0, "V0Set");
  double VCATMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberCAT(), 0, "VMon");
  double ICatMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberCAT(), 0, "IMon");
  
  double VGEMSet  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 0, "V0Set");
  double VGEMMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 0, "VMon");
  double IGEMMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 0, "IMon");

  double VG0Set  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 1, "V0Set");
  double VG0Mon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 1, "VMon");
  double IG0Mon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 1, "IMon");

  double VGPSet  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 2, "V0Set");
  double VGPMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 2, "VMon");
  double IGPMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 2, "IMon");

  double VGMSet  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 3, "V0Set");
  double VGMMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 3, "VMon");
  double IGMMon  = gCaen.GetChannelParam(gCaen.GetSlotNumberGEM(), 3, "IMon");


  

  ofs << std::fixed << std::setprecision(2)
      << unix_time << " "
      << VCatSet << " "
      << VCATMon << " "
      << ICatMon << " "
      << VGEMSet << " "
      << VGEMMon << " "
      << IGEMMon << " "
      << VG0Set << " "
      << VG0Mon << " "
      << IG0Mon << " "
      << VGPSet << " "
      << VGPMon << " "
      << IGPMon << " "
      << VGMSet << " "
      << VGMMon << " "
      << IGMMon << std::endl;
    
  ofs.close();
}


//_____________________________________________________________________________
int
NcursesTUI::Printf(const char* fmt, ...)
{
  int i;
  va_list marker;
#if 0
  char buf[256];
  ::va_start(marker, fmt);
  i = ::vsprintf(buf, fmt, marker);
  va_end(marker);
  if(buf[i-1] == '\n'){
    buf[i-1] = '\r';
    buf[i] = '\n';
    buf[i+1] = '\0';
    i++;
  }
  ::printf(buf);
#else
  {
    ::va_start(marker, fmt);
    i = ::vw_printw(stdscr, fmt, marker);
    ::va_end(marker);
  }
#endif
  ::refresh();
  return i;
}

//_____________________________________________________________________________
int
NcursesTUI::Puts(const std::string& str)
{
  return Printf("%s\n", str.c_str());
}

//_____________________________________________________________________________
void
NcursesTUI::Run()
{
  using clock = std::chrono::steady_clock;
  auto last_save = clock::now();
  
  while(Next() && gCaen.Update()){
    auto now = clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_save).count() >= 1) {
      SaveFile();
      last_save = now;
    }
    m_board_status = gCaen.GetBoardStatusString() == "OK";
    m_mpod_status = MpodLvStatus::IsOk();
    Clear();
    DrawTopBar();
    DrawCommandList();
    DrawChannelList();
    DrawKeyList();
#ifdef DEBUG
    DrawDebug();
#endif
    ::refresh();
  }
}
