#ifndef PixTestPh_H
#define PixTestPh_H

#include "PixTest.hh"


class DLLEXPORT PixTestPh: public PixTest {
public:
  PixTestPh(PixSetup *, std::string);
  PixTestPh();
  virtual ~PixTestPh();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void bookHist(std::string); 
  
  void doTest(); 

private:

  int     fParNtrig; 
  std::string fParDAC; 
  int     fParLoDAC, fParHiDAC;
  std::string fParDAC2;
  int     fParDAC2Hi, fParDAC2Lo, fParDAC2Step;
  std::vector<std::pair<int, int> > fPIX; 

};
#endif
