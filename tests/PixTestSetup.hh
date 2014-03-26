#ifndef PIXTESTSETUP_H
#define PIXTESTSETUP_H

#include "PixTest.hh"

class DLLEXPORT PixTestSetup: public PixTest {
public:
  PixTestSetup(PixSetup *, std::string);
  PixTestSetup();
  virtual ~PixTestSetup();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 

  void doTest(); 

private:

  int     fParNtrig; 
  int     fParNtests;
  int     fParVcal; 

};
#endif
