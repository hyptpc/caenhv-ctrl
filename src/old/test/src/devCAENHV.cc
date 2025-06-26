#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

#define UNIX

#include "CAENHVWrapper.h"

enum E_CAEN_STATUS
  {
    kOn                   = 0,
    kRampingUp            = 1,
    kRampingDown          = 2,
    kOverCurrent          = 3,
    kOverVoltage          = 4,
    kUnderVoltage         = 5,
    kExternalTrip         = 6,
    kMaxV                 = 7,
    kExternalDisable      = 8,
    kInternalTrip         = 9,
    kUnplugged            = 10,
    kReserved             = 11,
    kOverVolageProtection = 12,
    kPowerFail            = 13,
    kTemperatureError     = 14,
    N_CAEN_STATUS         = 15
  };

namespace{
  int sysHndl = -1;
}

void
SetChParam(int slot, const std::string& chname, unsigned short ch, float val)
{
  unsigned short chlist[] = { ch };
  unsigned int uintlist[] = { val };
  float floatlist[] = { val };
  if( chname == "Pw" )
    CAENHV_SetChParam(sysHndl, slot, chname.c_str(), 1, chlist, uintlist);
  else
    CAENHV_SetChParam(sysHndl, slot, chname.c_str(), 1, chlist, floatlist);
}

int
main( int argc, char* argv[] )
{
  const std::string board = "SY5527";
  char host[] = "192.168.1.8";
  int slot_buf = 1;
  int ch_max_buf = 6;

  const int slot = slot_buf;
  const int ch_max = ch_max_buf>24 ? 24 : ch_max_buf;

  CAENHV_SYSTEM_TYPE_t sysType;
  int link = LINKTYPE_TCPIP;
  char UserName[] = "admin";
  char Passwd[] = "admin";

  if( board == "SY1527" ) sysType = SY1527;
  else if( board == "SY2527" ) sysType = SY2527;
  else if( board == "SY4527" ) sysType = SY4527;
  else if( board == "SY5527" ) sysType = SY5527;
  else{
    std::cerr << "CAEN HV: unknown borad type " << board << std::endl;
    return 0;
  }

  if(sysHndl==-1){
    int ret = CAENHV_InitSystem(sysType, link, host, UserName, Passwd, &sysHndl);
    if(ret != CAENHV_OK){
      printf("\nCAENHV_InitSystem: %s (num. %d)\n\n", CAENHV_GetError(sysHndl), ret);
      return 0;
    }
  }

  unsigned short slotList[1] = { (unsigned short)slot };
  unsigned short ChList[24] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23
  };
  float V0Set[24] = {};
  float I0Set[24] = {};
  float VMon[24] = {};
  float IMon[24] = {};
  unsigned int Pw[24] = {};
  unsigned int Status[24] = {};
  unsigned int BdStatus = 0;

  CAENHV_GetBdParam(sysHndl, slot, slotList, "BdStatus", &BdStatus);
  CAENHV_GetChParam(sysHndl, slot, "V0Set", ch_max, ChList, V0Set);
  CAENHV_GetChParam(sysHndl, slot, "I0Set", ch_max, ChList, I0Set);
  CAENHV_GetChParam(sysHndl, slot, "VMon", ch_max, ChList, VMon);
  CAENHV_GetChParam(sysHndl, slot, "IMon", ch_max, ChList, IMon);
  CAENHV_GetChParam(sysHndl, slot, "Pw", ch_max, ChList, Pw);
  CAENHV_GetChParam(sysHndl, slot, "Status", ch_max, ChList, Status);

  // {
  //   int ret = CAENHV_DeinitSystem(sysHndl);
  //   if( ret != CAENHV_OK ){
  //     printf("CAENHV_DeinitSystem: %s (num. %d)\n\n",
  // 	     CAENHV_GetError(sysHndl), ret);
  //     return 0;
  //   }
  // }

  float* ptr = new float[256];
  int ndata=0;

  ptr[ndata++] = BdStatus;

  for(int i=0;i<ch_max;i++){
    printf("CH:%2d  V0Set:%6.1f  I0Set:%6.1f  VMon:%6.1f  IMon:%6.1f  Pw:%s Status: %u\n",
    	   i, V0Set[i], I0Set[i], VMon[i], IMon[i], Pw[i]? "ON":"OFF", Status[i]);
    ptr[ndata++] = V0Set[i];
    ptr[ndata++] = I0Set[i];
    ptr[ndata++] = VMon[i];
    ptr[ndata++] = IMon[i];
    ptr[ndata++] = (float)Pw[i];
    ptr[ndata++] = (float)Status[i];
  }

  SetChParam(slot, "V0Set", 3, 400.);
  SetChParam(slot, "Pw", 3, 0);

  return 0;
}
