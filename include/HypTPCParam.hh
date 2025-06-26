// -*- C++ -*-

#ifndef HYPTPC_PARAM_HH
#define HYPTPC_PARAM_HH

//_____________________________________________________________________________
static const double mm =  1.;
static const double cm = 10.;
// height
static const double PadPlane    =   0.0*mm;
static const double GemTop      =   6.0*mm + PadPlane;
static const double GatingGrid  =   4.2*mm + GemTop;
static const double TargetLevel = 300.0*mm + GatingGrid;
static const double Cathode     = 550.0*mm + GatingGrid;

#endif
