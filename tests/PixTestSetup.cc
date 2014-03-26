#include <stdlib.h>     /* atof, atoi */
#include <algorithm>    // std::find
#include <iostream>
#include "PixTestSetup.hh"
#include "log.h"

#include <TH2.h>

using namespace std;
using namespace pxar;

// ----------------------------------------------------------------------
PixTestSetup::PixTestSetup(PixSetup *a, std::string name) : PixTest(a, name), fParNtrig(-1), fParVcal(-1) {
  PixTest::init();
  init(); 
  LOG(logDEBUG) << "PixTestSetup ctor(PixSetup &a, string, TGTab *)";
}


//----------------------------------------------------------
PixTestSetup::PixTestSetup() : PixTest() {
  LOG(logDEBUG) << "PixTestSetup ctor()";
}

// ----------------------------------------------------------------------
bool PixTestSetup::setParameter(string parName, string sval) {
  bool found(false);
  for (unsigned int i = 0; i < fParameters.size(); ++i) {
    if (fParameters[i].first == parName) {
      found = true; 

      LOG(logDEBUG) << "  ==> parName: " << parName;
      LOG(logDEBUG) << "  ==> sval:    " << sval;
      if (!parName.compare("Ntrig")) {
	fParNtrig = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  ==> setting fParNtrig to " << fParNtrig; 
	setToolTips();
      }
      if (!parName.compare("Ntests")) {
	fParNtests = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  ==> setting fParNtests to " << fParNtests; 
	setToolTips();
      }
      if (!parName.compare("Vcal")) {
	fParVcal = atoi(sval.c_str()); 
	LOG(logDEBUG) << "  ==> setting fParVcal to " << fParVcal; 
	setToolTips();
      }
      break;
    }
  }
  return found; 
}


// ----------------------------------------------------------------------
void PixTestSetup::init() {
  LOG(logDEBUG) << "PixTestSetup::init()";

  setToolTips();
  fDirectory = gFile->GetDirectory(fName.c_str()); 
  if (!fDirectory) {
    fDirectory = gFile->mkdir(fName.c_str()); 
  } 

}


// ----------------------------------------------------------------------
void PixTestSetup::setToolTips() {
  fTestTip    = string(Form("scan testboard parameter settings and check for valid readout\n")
		       + string("TO BE IMPLEMENTED!!"))
    ;
  fSummaryTip = string("summary plot to be implemented")
    ;
}


// ----------------------------------------------------------------------
void PixTestSetup::bookHist(string name) {

  fDirectory->cd(); 
  LOG(logDEBUG) << "nothing done with " << name;

}


//----------------------------------------------------------
PixTestSetup::~PixTestSetup() {
  LOG(logDEBUG) << "PixTestSetup dtor";
  std::list<TH1*>::iterator il; 
  fDirectory->cd(); 
  for (il = fHistList.begin(); il != fHistList.end(); ++il) {
    LOG(logDEBUG) << "Write out " << (*il)->GetName();
    (*il)->SetDirectory(fDirectory); 
    (*il)->Write(); 
  }
}


// ----------------------------------------------------------------------
void PixTestSetup::doTest() {
  fDirectory->cd();
  LOG(logINFO) << "PixTestSetup::doTest() ntrig = " << fParNtrig << " and ntests = " << fParNtests;
  //FIXME clearHist(); 

  bookHist("bla");
 
  vector<pair<string, double> > power_settings = fPixSetup->getConfigParameters()->getTbPowerSettings();
  vector<pair<uint16_t, uint8_t> > pg_setup = fPixSetup->getConfigParameters()->getTbPgSettings();;


  fApi->_dut->testAllPixels(true);
  fApi->_dut->maskAllPixels(false);

  int offset[] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
  const int NCLK(20), NOFF(9), NDESER(7); ;
  
  TH2D *h3[NDESER]; 

  for (int ideser = 0; ideser < NDESER; ++ideser) {
    h3[ideser] = new TH2D(Form("setupEff_deser%d", ideser), Form("setupEff_deser%d", ideser), NOFF, -1., 8., NCLK, 0., NCLK); 
    fHistList.push_back(h3[ideser]); 
  }

  unsigned int npix(0);
  for (unsigned int iroc = 0; iroc < fApi->_dut->getNEnabledRocs(); ++iroc) {
    npix += fApi->_dut->getNEnabledPixels(iroc); 
  }

  LOG(logINFO) << " total enabled ROCs: " << fApi->_dut->getNEnabledRocs() 
	       << " with total enabled pixels: " << npix;

  vector<pixel> results;
  for (int ideser = 0; ideser < NDESER; ++ideser) {
    for (int iclk = 0; iclk < NCLK; ++iclk) {
      for (int ioffset = 0; ioffset < NOFF; ++ioffset) {
	cout << "xxx> starting loop point: " << " deser160phase = " << ideser << " iclk = " << iclk << " offset = " << offset[ioffset] << endl;
	fPixSetup->getConfigParameters()->setTbParameter("clk", iclk); 
	fPixSetup->getConfigParameters()->setTbParameter("ctr", iclk); 
	fPixSetup->getConfigParameters()->setTbParameter("sda", iclk+15+offset[ioffset]); 
	fPixSetup->getConfigParameters()->setTbParameter("tin", iclk+5+offset[ioffset]); 
	fPixSetup->getConfigParameters()->setTbParameter("deser160phase", ideser); 
	
	vector<pair<string, uint8_t> > sig_delays = fPixSetup->getConfigParameters()->getTbSigDelays();
	fPixSetup->getConfigParameters()->dumpParameters(sig_delays); 
	
	fApi->initTestboard(sig_delays, power_settings, pg_setup);
	
	cout << "xxx> getEfficiencyMap: " << endl;
	for (int i = 0; i < fParNtests; ++i) {
	  results.clear();
	  results = fApi->getEfficiencyMap(0, fParNtrig);
	  cout << "clk = " << iclk << " offset = " << offset[ioffset] << " eff map size: " << results.size()  << endl;
	  if (npix == results.size()) h3[ideser]->Fill(offset[ioffset], iclk); 
	  if (0 == results.size()) break; // bail out for failures
	}
      }
    }
  }

  h3[0]->Draw("colz");
  fDisplayedHist = find(fHistList.begin(), fHistList.end(), h3[0]);
  PixTest::update(); 

  for (int ideser = 0; ideser < NDESER; ++ideser) {
    for (int ix = 0; ix < h3[ideser]->GetNbinsX(); ++ix) {
      for (int iy = 0; iy < h3[ideser]->GetNbinsY(); ++iy) {
	if (fParNtests == h3[ideser]->GetBinContent(ix+1, iy+1)) {
	  LOG(logINFO) << " Found 100% PixelAlive success rate for deser160phase = " << ideser 
		       << " CLK = " << h3[ideser]->GetYaxis()->GetBinCenter(iy+1) 
		       << " OFFSET = " << h3[ideser]->GetXaxis()->GetBinCenter(ix+1);
	}
      }
    }
  }
  LOG(logINFO) << "PixTestSetup::doTest() done";
}
