// -- author: Daniel Pitzl
#ifndef PIXTESTDACSCANPIX_H
#define PIXTESTDACSCANPIX_H

#include "PixTest.hh"

class DLLEXPORT PixTestDacScanPix: public PixTest {
public:
  PixTestDacScanPix(PixSetup *, std::string);
  PixTestDacScanPix();
  virtual ~PixTestDacScanPix();
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
