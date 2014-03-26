// -- author: Daniel Pitzl
#ifndef PIXTESTDACSCANROC_H
#define PIXTESTDACSCANROC_H

#include "PixTest.hh"

class DLLEXPORT PixTestDacScanRoc: public PixTest {
public:
  PixTestDacScanRoc(PixSetup *, std::string);
  PixTestDacScanRoc();
  virtual ~PixTestDacScanRoc();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 
  
  void doTest(); 

private:

  int     fParNtrig; 
  std::string fParDAC; 
  int     fParLoDAC, fParHiDAC;

};
#endif
