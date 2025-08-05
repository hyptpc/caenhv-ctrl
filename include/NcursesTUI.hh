// -*- C++ -*-

#ifndef NCURSES_TUI_HH
#define NCURSES_TUI_HH

#include <deque>
#include <string>
#include <vector>

//_____________________________________________________________________________
class NcursesTUI
{
public:
  static NcursesTUI& GetInstance();
  ~NcursesTUI();

private:
  NcursesTUI();
  NcursesTUI(const NcursesTUI&);
  NcursesTUI& operator=(const NcursesTUI&);

private:
  enum ECommand {
    CMD_CATHODE,
    CMD_GEM,
    CMD_GATE
  };
  struct Command
  {
    ECommand    index;
    std::string label;
    Command(ECommand i, std::string l)
      : index(i), label(l)
    {}
    void operator ()(bool s);
  };

private:
  enum EColorPair {
    CP_DEFAULT,
    CP_RED,
    CP_GREEN,
    CP_YELLOW,
    CP_BLUE,
    CP_WHITE,
    CP_CURSOR,
    CP_TOPBAR,
    CP_RED_REV,
    CP_GREEN_REV,
    CP_YELLOW_REV,
    CP_BLUE_REV,
    CP_MAGENTA_REV,
    CP_WHITE_REV,
    NCOLORPAIR
  };
  bool                 m_lock;
  std::vector<Command> m_command_list;
  std::deque<bool>     m_status_list;
  int                  m_cursor_x;
  int                  m_cursor_y;
  int                  m_key;
  bool                 m_board_status;
  bool                 m_mpod_status;
  float                m_drift_field; // [V/cm]
  float                m_vcat; // [V]
  float                m_vfield; // [V]
  float                m_vgem; // [V]
  float                m_vgate_plus_diff; // [V]
  float                m_vgate_minus_diff; // [V]

public:
  void   Run();

private:
  void        Clear();
  void        DrawChannelList();
  void        DrawCommandList();
  void        DrawDebug();
  void        DrawKeyList();
  void        DrawTopBar();
  std::string GetChar();
  void        GotoXY(int x, int y);
  bool        Next();
  int         Printf(const char* fmt, ...);
  int         Puts(const std::string& str);
};

//_____________________________________________________________________________
inline NcursesTUI&
NcursesTUI::GetInstance()
{
  static NcursesTUI s_instance;
  return s_instance;
}

#endif
