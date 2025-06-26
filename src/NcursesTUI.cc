// -*- C++ -*-

#include "NcursesTUI.hh"

#include <ncurses.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <tuple>

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
  switch(index){
  case CMD_CATHODE:
    break;
  case CMD_GEM:
    break;
  case CMD_GATE:
    gCaen.SetChannelParam(2, "Pw", s);
    gCaen.SetChannelParam(3, "Pw", s);
    gCaen.SetChannelParam(4, "Pw", s);
    break;
  default:
    break;
  }
}

//_____________________________________________________________________________
NcursesTUI::NcursesTUI()
  : m_lock(true),
    m_command_list(),
    m_status_list(),
    m_cursor_x(0),
    m_cursor_y(0),
    m_key(0),
    m_board_status(false),
    m_mpod_status(false),
    m_drift_field(),
    m_vgem()
{
  m_command_list.push_back(Command(CMD_CATHODE, "Cathode"));
  m_command_list.push_back(Command(CMD_GEM,     "GEM    "));
  m_command_list.push_back(Command(CMD_GATE,    "Gate   "));
  NCmd = m_command_list.size();
  NCh = gCaen.GetMaxChannel();
  for(int i=0, n=m_command_list.size(); i<n; ++i){
    m_status_list.push_back(false);
  }
  ::initscr();
  ::timeout(200); // [ms]
  ::cbreak();
  ::noecho();
  // ::nodelay(stdscr, FALSE);
  ::curs_set(FALSE);
  ::keypad(stdscr, TRUE);
  // ::mousemask(ALL_MOUSE_EVENTS, nullptr);
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
  m_drift_field = (int)(gCaen.GetChannelParam(0, "V0Set") *
			cm / (Cathode - GemTop));
  m_vgem = gCaen.GetChannelParam(1, "V0Set");
  m_status_list[CMD_CATHODE] = (gCaen.GetChannelParam(0, "Pw") == 1);
  m_status_list[CMD_GEM]     = (gCaen.GetChannelParam(1, "Pw") == 1);
  m_status_list[CMD_GATE]    = (gCaen.GetChannelParam(2, "Pw") +
				gCaen.GetChannelParam(3, "Pw") +
				gCaen.GetChannelParam(4, "Pw") == 3);
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
  GotoXY(1, 5+NCmd);
  auto color = COLOR_PAIR(CP_WHITE_REV);
  ::attron(color);
  // Printf("%s\n", std::string(80, '=').c_str());
  // Puts(std::string(80, ' '));
  Printf("Ch Name     V0Set    VMon     I0Set       IMon        RUp      RDWn     Status  ");
  ::attroff(color);
  auto ChannelName = gCaen.GetChannelName();
  auto Status      = gCaen.GetStatusString();
  for(int i=0, n=NCh; i<n; ++i){
    int l = i+6+NCmd;
    GotoXY(1, l);
    Printf("%02d", i);
    GotoXY(4, l);
    Printf("%-7s", ChannelName[i].c_str());
    GotoXY(13, l);
    Printf("%5.0f V", gCaen.GetChannelParam(i, "V0Set"));
    GotoXY(22, l);
    Printf("%5.0f V", gCaen.GetChannelParam(i, "VMon"));
    GotoXY(31, l);
    if(!m_lock && i + NCmd == m_cursor_y)
      color = COLOR_PAIR(CP_CURSOR);
    else
      color = COLOR_PAIR(CP_DEFAULT);
    ::attron(color);
    Printf("%7.2f uA", gCaen.GetChannelParam(i, "I0Set"));
    ::attroff(color);
    GotoXY(43, l);
    Printf("%7.2f uA", gCaen.GetChannelParam(i, "IMon"));
    GotoXY(55, l);
    Printf("%3.0f V/s", gCaen.GetChannelParam(i, "RUp"));
    GotoXY(64, l);
    Printf("%3.0f V/s", gCaen.GetChannelParam(i, "RDWn"));
    GotoXY(73, l);
    if(Status[i].size() == 0)
      color = COLOR_PAIR(CP_DEFAULT);
    else if(Status[i].find("PwOn") != std::string::npos)
      color = COLOR_PAIR(CP_RED_REV);
    // else if(Status[i] == "PwOff")
    // 	color = COLOR_PAIR(CP_BLUE_REV);
    else if(Status[i].find("Up") != std::string::npos ||
	    Status[i].find("Down") != std::string::npos)
      color = COLOR_PAIR(CP_YELLOW_REV) | A_BLINK;
    else
      color = COLOR_PAIR(CP_YELLOW_REV) | A_BLINK;
    color |= A_BOLD;
    ::attron(color);
    Printf("%-7s", Status[i].c_str());
    ::attroff(color);
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
  Printf("%5.0f V/cm", m_drift_field);
  ::attroff(color);
  float vfield = m_drift_field*(Cathode - GemTop)/cm;
  gCaen.SetChannelParam(0, "V0Set", vfield);
  Printf(" -> %5.0f V", vfield);
  GotoXY(20, 5);
  Printf("VGEM=");
  color = (!m_lock && 1 == m_cursor_x && 1 == m_cursor_y ?
	   COLOR_PAIR(CP_WHITE_REV) : COLOR_PAIR(CP_DEFAULT));
  ::attron(color);
  Printf("%5.0f V   ", m_vgem);
  ::attroff(color);
}

//_____________________________________________________________________________
void
NcursesTUI::DrawDebug()
{
  static int refresh = 0;
  GotoXY(1, NCmd + NCh + 9);
  ::attron(COLOR_PAIR(CP_MAGENTA_REV));
  Puts(std::string("DEBUG") + std::string(75, ' '));
  ::attroff(COLOR_PAIR(CP_MAGENTA_REV));
  Printf("Key=%3d %c", m_key, (m_key < 0) ? ' ' : m_key);
  GotoXY(13, NCmd + NCh + 10);
  Printf("Cursor=(%d, %d)   ", m_cursor_x, m_cursor_y);
  Printf("BdStatus=%d   ", m_board_status);
  Printf("MpodStatus=%d   ", m_mpod_status);
  Printf("Refresh=%8d\n", ++refresh);
  Printf("CmdFlag=(,  ,");
  for(int i=0; i<NCmd; ++i){
    GotoXY(10 + i*3, NCmd + NCh + 11);
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
  GotoXY(1, NCmd + NCh + 7);
  Printf("[q] Quit");
  if(!m_board_status || !m_mpod_status)
    return;
  Printf(" [l] Lock=");
  if(!m_lock)
    color = COLOR_PAIR(CP_YELLOW) | A_BOLD;
  else
    color = COLOR_PAIR(CP_DEFAULT);
  ::attron(color);
  Printf("%s", (m_lock ? "ON" : "OFF"));
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
  GotoXY(1, 2);
  Printf("SLOT#%d A1526 6Ch Neg. 15kV 1mA  Temp=%.0f°C",
	 gCaen.GetBoardTemp(), gCaen.GetSlotNumber());
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
  // VGEM
  if(m_cursor_x == 1 && m_cursor_y == 1){
    x = 25;
    y =  5;
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
  case KEY_UP:
    if(!m_lock && m_cursor_y > 0)
      m_cursor_y--;
    break;
  case KEY_DOWN:
    if(!m_lock &&
       m_cursor_y < NCmd + NCh - 1)
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
      }
    }
    // VGEM
    if(m_cursor_x == 1 && m_cursor_y == 1){
      auto input = GetChar();
      if(!input.empty()){
	long val = std::strtol(input.c_str(), nullptr, 10);
	m_vgem = (float)val;
	gCaen.SetChannelParam(1, "V0Set", m_vgem);
      }
    }
    // I0Set
    if(NCmd <= m_cursor_y){
      auto input = GetChar();
      if(!input.empty()){
	double val = std::strtod(input.c_str(), nullptr);
	gCaen.SetChannelParam(m_cursor_y - NCmd, "I0Set", val);
      }
    }
    break;
  case KEY_MOUSE:
    break;
  default:
    break;
  }
  m_lock |= !m_board_status | !m_mpod_status;
  return TRUE;
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
  while(Next() && gCaen.Update()){
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
