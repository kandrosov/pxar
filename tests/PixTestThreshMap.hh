#ifndef PIXTESTTHRESH_H
#define PIXTESTTHRESH_H

#include "PixTest.hh"

#define ROCNUMROWS 80
#define ROCNUMCOLS 52

class DLLEXPORT PixTestThreshMap: public PixTest {
public:
  PixTestThreshMap(PixSetup *, std::string);
  PixTestThreshMap();
  virtual ~PixTestThreshMap();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 

  void doTest(); 

private:

  uint16_t fParNtrig;
  int 	   fParLoDAC; 
  int 	   fParHiDAC; 
  bool 	   bBumpBond;

  std::map<int, int> id2idx; // map the ROC ID onto the index of the ROC

};
#endif
