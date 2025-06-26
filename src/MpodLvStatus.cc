// -*- C++ -*-

#include "MpodLvStatus.hh"

#include <stdexcept>

namespace
{
  const std::string top_dir = "http://urazato.monitor.k18br/mpod-ctrl/";
}

//_____________________________________________________________________________
bool
MpodLvStatus::CratePowerIsOn( void )
{
  std::string url = top_dir + "snmp.php?get=powerStatus";
  std::string command = "curl -s \"" + url + "\"";
  return Execute( command ) == "On";
}

//_____________________________________________________________________________
bool
MpodLvStatus::IsOk( void )
{
  return CratePowerIsOn();
}

//_____________________________________________________________________________
std::string
MpodLvStatus::Execute( const std::string& command )
{
  std::string ret;
  char buf[256];
  FILE* pipe = ::popen( command.c_str(), "r" );
  if( !pipe ){
    throw std::runtime_error( "[MpodLvStatus::Execute()] popen() failed" );
  }
  while( ::fgets( buf, sizeof(buf), pipe ) != nullptr ){
    ret += buf;
  }
  ::pclose( pipe );
  return ret;
}
