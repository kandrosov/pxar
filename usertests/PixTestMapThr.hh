// -- author: Daniel Pitzl
#ifndef PIXTESTMAPTHR_H
#define PIXTESTMAPTHR_H

#include "PixTest.hh"

class DLLEXPORT PixTestMapThr: public PixTest {
public:
  PixTestMapThr(PixSetup *, std::string);
  PixTestMapThr();
  virtual ~PixTestMapThr();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 

  void doTest(); 

private:

  int     fParNtrig; 

};
#endif
