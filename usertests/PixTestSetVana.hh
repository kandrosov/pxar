// -- author: Daniel Pitzl
#ifndef PIXTESTSETVANA_H
#define PIXTESTSETVANA_H

#include "PixTest.hh"

class DLLEXPORT PixTestSetVana: public PixTest
{
public:
  PixTestSetVana(PixSetup *, std::string);
  PixTestSetVana();
  virtual ~PixTestSetVana();
  virtual bool setParameter(std::string parName, std::string sval);
  void init();
  void bookHist(std::string);

  void doTest();

private:

  int fTargetIa; // [mA/ROC]

};
#endif
