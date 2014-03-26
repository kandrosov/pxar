// -- author: Daniel Pitzl
#ifndef PIXTESTDACSCANPH_H
#define PIXTESTDACSCANPH_H

#include "PixTest.hh"

class DLLEXPORT PixTestDacScanPh: public PixTest {
public:
  PixTestDacScanPh(PixSetup *, std::string);
  PixTestDacScanPh();
  virtual ~PixTestDacScanPh();
  virtual bool setParameter(std::string parName, std::string sval);
  void init();
  void setToolTips();
  void bookHist(std::string);

  void doTest();

private:

  int     fParNtrig;
  std::string fParDAC;
  int     fParLoDAC, fParHiDAC;
  int     fParCals;
  std::vector<std::pair<int, int> > fPIX;

};
#endif
