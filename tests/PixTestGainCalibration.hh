#ifndef PIXTESTGAINCALIBRATION_H
#define PIXTESTGAINCALIBRATION_H

#include "PixTest.hh"

class DLLEXPORT PixTestGainCalibration: public PixTest {
public:
  PixTestGainCalibration(PixSetup *a, std::string name);
  PixTestGainCalibration();
  virtual ~PixTestGainCalibration();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();

  void doTest(); 
  
};
#endif
