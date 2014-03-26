#ifndef PIXINITFUNC_H
#define PIXINITFUNC_H

#include "pxardllexport.h"

#include "TString.h"
#include "TObject.h"
#include "TH1.h"
#include "TF1.h"

#include <iostream>


class DLLEXPORT PixInitFunc {

public:

  PixInitFunc(); 
  ~PixInitFunc(); 

  void resetLimits(); 
  void limitPar(int ipar, double lo, double hi); 

  TF1* errScurve(TH1 *h); 


  double fLo, fHi;
  bool fLimit[20];
  double fLimitLo[20], fLimitHi[20]; 

};

#endif
