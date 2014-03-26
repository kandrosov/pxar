#include <stdlib.h>     /* atof, atoi */
#include <algorithm>    // std::find
#include <iostream>
#include <fstream>

#include <TH1.h>
#include <TRandom.h>

#include "PixTestScurves.hh"
#include "PixUtil.hh"
#include "log.h"


using namespace std;
using namespace pxar;

// ----------------------------------------------------------------------
PixTestScurves::PixTestScurves(PixSetup *a, std::string name) : PixTest(a, name), fParDac(""), fParNtrig(-1), fParNpix(-1), fParDacLo(-1), fParDacHi(-1) {
  PixTest::init();
  init(); 
}


//----------------------------------------------------------
PixTestScurves::PixTestScurves() : PixTest() {
  //  LOG(logDEBUG) << "PixTestScurves ctor()";
}

// ----------------------------------------------------------------------
bool PixTestScurves::setParameter(string parName, string sval) {
  bool found(false);
  for (unsigned int i = 0; i < fParameters.size(); ++i) {
    if (fParameters[i].first == parName) {
      found = true; 
      sval.erase(remove(sval.begin(), sval.end(), ' '), sval.end());
      if (!parName.compare("Ntrig")) {
	fParNtrig = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  setting fParNtrig  ->" << fParNtrig << "<- from sval = " << sval;
      }
      if (!parName.compare("Npix")) {
	fParNpix = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  setting fParNpix  ->" << fParNpix << "<- from sval = " << sval;
      }
      if (!parName.compare("DAC")) {
	fParDac = sval;
	LOG(logDEBUG) << "  setting fParDac  ->" << fParDac << "<- from sval = " << sval;
      }
      if (!parName.compare("DacLo")) {
	fParDacLo = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  setting fParDacLo  ->" << fParDacLo << "<- from sval = " << sval;
      }
      if (!parName.compare("DacHi")) {
	fParDacHi = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  setting fParDacHi  ->" << fParDacHi << "<- from sval = " << sval;
      }

      setToolTips();
      break;
    }
  }
  
  return found; 
}


// ----------------------------------------------------------------------
void PixTestScurves::setToolTips() {
  fTestTip    = string(Form("measure and fit s-curves for DAC %s\n", fParDac.c_str())); 
  fSummaryTip = string("all ROCs are displayed side-by-side. Note the orientation:")
    + string("\nthe canvas bottom corresponds to the narrow module side with the cable")
    + string("\nexplanations for all plots: ")
    + string("thr_* shows the map of the s-curve thresholds")
    + string("\nsig_* shows the map of the s-curve widths")
    + string("\ndist_* shows the distribution/projections of the threshold and width maps")
    ;
}

// ----------------------------------------------------------------------
void PixTestScurves::init() {

  setToolTips(); 

  fDirectory = gFile->GetDirectory(fName.c_str()); 
  if (!fDirectory) {
    fDirectory = gFile->mkdir(fName.c_str()); 
  } 
  fDirectory->cd(); 

}

// ----------------------------------------------------------------------
void PixTestScurves::bookHist(string name) {
  fDirectory->cd(); 

  LOG(logDEBUG) << "nothing done with " << name;
  //  fHistList.clear();

}


//----------------------------------------------------------
PixTestScurves::~PixTestScurves() {
  LOG(logDEBUG) << "PixTestScurves dtor";
}


// ----------------------------------------------------------------------
void PixTestScurves::doTest() {
  if (fPixSetup->isDummy()) {
    dummyAnalysis(); 
    return;
  }

  fDirectory->cd();
  PixTest::update(); 
  LOG(logINFO) << "PixTestScurves::doTest() ntrig = " << fParNtrig 
	       << " using npix = " << fParNpix
	       << " scanning DAC " << fParDac 
	       << " from " << fParDacLo << " .. " << fParDacHi;

  fApi->_dut->testAllPixels(false);
  fApi->_dut->maskAllPixels(true);
  sparseRoc(fParNpix); 

  int RFLAG(7); 
  vector<TH1*> thr0 = scurveMaps(fParDac, "scurve"+fParDac, fParNtrig, fParDacLo, fParDacHi, RFLAG); 

  LOG(logINFO) << "PixTestScurves::doTest() done ";
  PixTest::update(); 
}


// ----------------------------------------------------------------------
void PixTestScurves::dummyAnalysis() {
  string name("scurveVcal"); 
  TH2D *h2(0); 

  vector<string> afl; // aFewLines
  afl.push_back("32  93   0   0   0   0   0   0   0   0   0   1   0   3   3  11  12  13  22  25  35  37  48  49  50  50  50  50  50  50  50  50  50  50 ");
  afl.push_back("32  95   0   0   0   0   0   0   0   3   1   0   5   4   6  18  18  20  32  43  48  45  50  50  50  50  50  50  50  50  50  50  50  50 ");
  afl.push_back("32 101   0   0   0   0   0   0   0   0   0   0   1   2   7   9  17  21  22  36  39  35  45  43  48  49  50  50  50  50  50  50  50  50 ");
  afl.push_back("32  86   0   0   0   0   0   0   0   0   0   0   0   3   3   5  11   7  16  20  24  31  35  42  46  45  50  48  50  50  50  50  50  50 ");
  afl.push_back("32  99   0   0   0   0   0   0   0   0   0   2   1   2   6  14  19  24  31  37  41  45  46  46  48  50  50  50  50  50  50  50  50  50 ");
  afl.push_back("32  92   0   0   0   0   0   0   0   1   2   2   5   8   7  13  19  20  27  38  43  44  49  48  50  50  50  50  50  50  50  50  50  50 ");
  afl.push_back("32  98   0   0   0   0   0   0   0   0   0   0   2   0   3   7  21  27  35  39  39  46  49  48  50  50  50  50  50  50  50  50  50  50 ");
  afl.push_back("32  95   0   0   0   0   0   0   0   0   3   4   2   7  10  15  25  22  34  48  47  48  50  50  50  50  50  50  50  50  50  50  50  50 ");
  afl.push_back("32  93   0   0   0   0   1   0   0   0   1   3   2   3  11  13  14  26  35  34  40  46  40  50  50  50  50  50  50  50  50  50  50  50 ");
  int aflSize = afl.size();

  vector<uint8_t> rocIds = fApi->_dut->getEnabledRocIDs(); 
  ofstream OutputFile;
  string fname("SCurveData");

  for (unsigned int iroc = 0; iroc < rocIds.size(); ++iroc){
    fId2Idx.insert(make_pair(rocIds[iroc], iroc)); 
    h2 = bookTH2D(Form("%s_C%d", name.c_str(), iroc), Form("%s_C%d", name.c_str(), rocIds[iroc]), 52, 0., 52., 80, 0., 80.); 
    h2->SetMinimum(0.); 
    h2->SetDirectory(fDirectory); 
    setTitles(h2, "col", "row"); 
    fHistOptions.insert(make_pair(h2, "colz"));
    
    double x(0.); 
    int iline(0); 
    OutputFile.open(Form("%s/%s_C%d.dat", fPixSetup->getConfigParameters()->getDirectory().c_str(), fname.c_str(), iroc));
    OutputFile << "Mode 1" << endl;
    for (int ix = 0; ix < 52; ++ix) {
      for (int iy = 0; iy < 80; ++iy) {
	x = gRandom->Gaus(50., 8.);
	h2->SetBinContent(ix+1, iy+1, x); 
	iline = static_cast<int>(aflSize*gRandom->Rndm());
	OutputFile << afl[iline] << endl;
      }
    }
    OutputFile.close(); 
    fHistList.push_back(h2); 
  }

  TH2D *h = (TH2D*)(*fHistList.begin());
  h->Draw(getHistOption(h).c_str());
  fDisplayedHist = find(fHistList.begin(), fHistList.end(), h);
  PixTest::update(); 
  LOG(logINFO) << "PixTestScurves::dummyAnalysis() done";

}


// ----------------------------------------------------------------------
void PixTestScurves::output4moreweb() {
  // -- nothing required here
}
