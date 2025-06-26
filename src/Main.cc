// -*- C++ -*-

#include "CaenControl.hh"
#include "NcursesTUI.hh"

//_____________________________________________________________________________
int
main(int argc, char* argv[])
{
  auto& gCaen = CaenControl::GetInstance();
  gCaen.SetSystemType(SY5527);
  gCaen.SetLinkType(LINKTYPE_TCPIP);
  gCaen.SetHostName("192.168.20.58");
  gCaen.SetUserName("admin");
  gCaen.SetPassword("admin");
  gCaen.SetSlotNumber(1);
  gCaen.SetMaxChannel(6);
  if(!gCaen.Initialize())
    return EXIT_FAILURE;

  auto& gTUI = NcursesTUI::GetInstance();
  gTUI.Run();

  gCaen.Finalize();
  return EXIT_SUCCESS;
}
