// -- author: Wolfram Erdmann
#ifndef PIXTESTBBMAP_H
#define PIXTESTBBMAP_H

#include "PixTest.hh"

class DLLEXPORT PixTestBBMap: public PixTest {
public:
  PixTestBBMap(PixSetup *, std::string);
  PixTestBBMap();
  virtual ~PixTestBBMap();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  
  


  void doTest(); 

private:
  std::vector<uint8_t> rocIds;  // convenience
  int          fParNtrig; 
  int          fParVcalS;   // vcal dac value of the cals probe signal
  int          fPartest;    // direct vcal value for test runs, active if >0

  ClassDef(PixTestBBMap, 1); 

};
#endif
