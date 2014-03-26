#ifndef PIXTESTFACTORY_H
#define PIXTESTFACTORY_H

#include <iostream>

#include "PixTest.hh"
#include "PixSetup.hh"

class DLLEXPORT PixTestFactory {
public:

  static PixTestFactory*  instance(); 
  PixTest*                createTest(std::string, PixSetup *); 

protected: 
  PixTestFactory(); 
  ~PixTestFactory(); 

private:
  static PixTestFactory* fInstance; 

};




#endif
