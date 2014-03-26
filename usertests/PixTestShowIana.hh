// -- author: Wolfram Erdmann
#ifndef PIXTESTSHOWIANA_H
#define PIXTESTSHOWIANA_H

#include "PixTest.hh"

class DLLEXPORT PixTestShowIana: public PixTest {
public:
  PixTestShowIana(PixSetup *, std::string);
  PixTestShowIana();
  virtual ~PixTestShowIana();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 
  uint8_t readRocADC(uint8_t);

  void doTest(); 

};
#endif
