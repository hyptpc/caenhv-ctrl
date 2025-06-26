// -*- C++ -*-

#ifndef MPOD_LV_STATUS_HH
#define MPOD_LV_STATUS_HH

#include <string>

//_____________________________________________________________________________
class MpodLvStatus
{
public:
  static bool       CratePowerIsOn( void );
  static bool       IsOk( void );

private:
  static std::string Execute( const std::string& command );
};

#endif
