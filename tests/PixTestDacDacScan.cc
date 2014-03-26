#include <stdlib.h>     /* atof, atoi */
#include <algorithm>    // std::find
#include <iostream>
#include "PixTestDacDacScan.hh"
#include "log.h"

#include <TH2.h>

using namespace std;
using namespace pxar;

// ----------------------------------------------------------------------
PixTestDacDacScan::PixTestDacDacScan(PixSetup *a, std::string name) : 
PixTest(a, name), fParNtrig(-1), fParDAC1("nada"), fParDAC2("nada"), fParLoDAC1(-1), fParHiDAC1(-1), fParLoDAC2(-1), fParHiDAC2(-1) {
  PixTest::init();
  init(); 
  LOG(logDEBUG) << "PixTestDacDacScan ctor(PixSetup &a, string, TGTab *)";
}


//----------------------------------------------------------
PixTestDacDacScan::PixTestDacDacScan() : PixTest() {
  LOG(logDEBUG) << "PixTestDacDacScan ctor()";
}

// ----------------------------------------------------------------------
bool PixTestDacDacScan::setParameter(string parName, string sval) {
  bool found(false);
  string str1, str2; 
  string::size_type s1;
  int pixc, pixr; 
  for (unsigned int i = 0; i < fParameters.size(); ++i) {
    if (fParameters[i].first == parName) {
      found = true; 
      sval.erase(remove(sval.begin(), sval.end(), ' '), sval.end());
      if (!parName.compare("Ntrig")) {
	fParNtrig = atoi(sval.c_str()); 
	setToolTips();
      }
      if (!parName.compare("DAC1")) {
	fParDAC1 = sval; 
	setToolTips();
      }
      if (!parName.compare("DAC2")) {
	fParDAC2 = sval; 
	setToolTips();
      }
      if (!parName.compare("DAC1LO")) {
	fParLoDAC1 = atoi(sval.c_str()); 
	setToolTips();
      }
      if (!parName.compare("DAC1HI")) {
	fParHiDAC1 = atoi(sval.c_str()); 
	setToolTips();
      }
      if (!parName.compare("DAC2LO")) {
	fParLoDAC2 = atoi(sval.c_str()); 
	setToolTips();
      }
      if (!parName.compare("DAC2HI")) {
	fParHiDAC2 = atoi(sval.c_str()); 
	setToolTips();
      }
      if (!parName.compare("PIX")) {
	s1 = sval.find(","); 
	if (string::npos != s1) {
	  str1 = sval.substr(0, s1); 
	  pixc = atoi(str1.c_str()); 
	  str2 = sval.substr(s1+1); 
	  pixr = atoi(str2.c_str()); 
	  fPIX.push_back(make_pair(pixc, pixr)); 
	} else {
	  fPIX.push_back(make_pair(-1, -1)); 
	}
      }
      if (!parName.compare("PIX1")) {
	s1 = sval.find(","); 
	if (string::npos != s1) {
	  str1 = sval.substr(0, s1); 
	  pixc = atoi(str1.c_str()); 
	  str2 = sval.substr(s1+1); 
	  pixr = atoi(str2.c_str()); 
	  fPIX.push_back(make_pair(pixc, pixr)); 
	} else {
	  fPIX.push_back(make_pair(-1, -1)); 
	}
      }
      if (!parName.compare("PIX2")) {
	s1 = sval.find(","); 
	if (string::npos != s1) {
	  str1 = sval.substr(0, s1); 
	  pixc = atoi(str1.c_str()); 
	  str2 = sval.substr(s1+1); 
	  pixr = atoi(str2.c_str()); 
	  fPIX.push_back(make_pair(pixc, pixr)); 
	} else {
	  fPIX.push_back(make_pair(-1, -1)); 
	}
      }
      if (!parName.compare("PIX3")) {
	s1 = sval.find(","); 
	if (string::npos != s1) {
	  str1 = sval.substr(0, s1); 
	  pixc = atoi(str1.c_str()); 
	  str2 = sval.substr(s1+1); 
	  pixr = atoi(str2.c_str()); 
	  fPIX.push_back(make_pair(pixc, pixr)); 
	} else {
	  fPIX.push_back(make_pair(-1, -1)); 
	}
      }
      if (!parName.compare("PIX4")) {
	s1 = sval.find(","); 
	if (string::npos != s1) {
	  str1 = sval.substr(0, s1); 
	  pixc = atoi(str1.c_str()); 
	  str2 = sval.substr(s1+1); 
	  pixr = atoi(str2.c_str()); 
	  fPIX.push_back(make_pair(pixc, pixr)); 
	} else {
	  fPIX.push_back(make_pair(-1, -1)); 
	}
      }

      // FIXME: remove/update from fPIX if the user removes via the GUI!

      break;
    }
  }
  
  return found; 
}


// ----------------------------------------------------------------------
void PixTestDacDacScan::init() {
  LOG(logDEBUG) << "PixTestDacDacScan::init()";

  setToolTips(); 
  fDirectory = gFile->GetDirectory(fName.c_str()); 
  if (!fDirectory) {
    fDirectory = gFile->mkdir(fName.c_str()); 
  } 
  fDirectory->cd(); 

}


// ----------------------------------------------------------------------
void PixTestDacDacScan::setToolTips() {
  fTestTip    = string(Form("scan the two DACs %s vs %s and ",  fParDAC1.c_str(), fParDAC2.c_str()))
    + string("determine the number of hits for each setting")
    + string("\nNOTE: There is currently a limitation that the total number of scanned points is less than 2^14!")
    ;
  fSummaryTip = string("summary plot to be implemented")
    ;

}

// ----------------------------------------------------------------------
void PixTestDacDacScan::bookHist(string name) {
  fDirectory->cd(); 
  LOG(logDEBUG) << "nothing done with " << name;

}


//----------------------------------------------------------
PixTestDacDacScan::~PixTestDacDacScan() {
  LOG(logDEBUG) << "PixTestDacDacScan dtor";
}


// ----------------------------------------------------------------------
void PixTestDacDacScan::doTest() {
  fDirectory->cd();
  PixTest::update(); 
  LOG(logDEBUG) << "dac scan " << fParDAC1 << " vs. " << fParDAC2 << " ntrig = " << fParNtrig << " for npixels = " << fPIX.size(); 

  string name = fParDAC1 + string(":") + fParDAC2; 
  bookHist(name);
  string::size_type s1 = name.find(":"); 
  string dac1 = name.substr(0, s1); 
  string dac2 = name.substr(s1+1); 
  LOG(logDEBUG) << "PixTestDacDacScan for dacs ->" << dac1 << "<- and ->" << dac2 << "<-";
  
  TH2D *h2(0);
  map<string, TH2D*> maps;
  vector<uint8_t> rocIds = fApi->_dut->getEnabledRocIDs(); 
  for (unsigned int iroc = 0; iroc < rocIds.size(); ++iroc){
    for (unsigned int ip = 0; ip < fPIX.size(); ++ip) {
      h2 = bookTH2D(Form("nhits_%s_%s_c%d_r%d_C%d", dac1.c_str(), dac2.c_str(), fPIX[ip].first, fPIX[ip].second, rocIds[iroc]), 
		    Form("nhits_%s_%s_c%d_r%d_C%d", dac1.c_str(), dac2.c_str(), fPIX[ip].first, fPIX[ip].second, rocIds[iroc]), 
		    256, 0., 256., 256, 0., 256.); 
      h2->SetMinimum(0.); 
      setTitles(h2, dac1.c_str(), dac2.c_str()); 
      fHistList.push_back(h2);
      fHistOptions.insert(make_pair(h2, "colz")); 
      maps.insert(make_pair(Form("nhits_%s_%s_c%d_r%d_C%d", 
				 dac1.c_str(), dac2.c_str(), fPIX[ip].first, fPIX[ip].second, rocIds[iroc]), h2)); 
    }
    
  }

  fApi->_dut->testAllPixels(false);
  fApi->_dut->maskAllPixels(true);
  vector<pair<uint8_t, pair<uint8_t, vector<pixel> > > >  rresults, results;
  for (unsigned int i = 0; i < fPIX.size(); ++i) {
    if (fPIX[i].first > -1)  {
      fApi->_dut->testPixel(fPIX[i].first, fPIX[i].second, true);
      fApi->_dut->maskPixel(fPIX[i].first, fPIX[i].second, false);
      rresults = fApi->getEfficiencyVsDACDAC(fParDAC1, fParLoDAC1, fParHiDAC1, 
					     fParDAC2, fParLoDAC2, fParHiDAC2, 0, fParNtrig);
      copy(rresults.begin(), rresults.end(), back_inserter(results)); 

      fApi->_dut->testPixel(fPIX[i].first, fPIX[i].second, false);
      fApi->_dut->maskPixel(fPIX[i].first, fPIX[i].second, true);
    }
  }

  TH2D *h(0); 
  for (unsigned int iroc = 0; iroc < rocIds.size(); ++iroc){
    for (unsigned int i = 0; i < results.size(); ++i) {
      pair<uint8_t, pair<uint8_t, vector<pixel> > > v = results[i];
      int idac1 = v.first; 
      pair<uint8_t, vector<pixel> > w = v.second;      
      int idac2 = w.first;
      vector<pixel> wpix = w.second;

      for (unsigned ipix = 0; ipix < wpix.size(); ++ipix) {
	if (wpix[ipix].roc_id == rocIds[iroc]) {
	  h = maps[Form("nhits_%s_%s_c%d_r%d_C%d", 
			fParDAC1.c_str(), fParDAC2.c_str(), wpix[ipix].column, wpix[ipix].row, rocIds[iroc])];
	  if (h) {
	    h->Fill(idac1, idac2, wpix[ipix].value); 
	  } else {
	    LOG(logDEBUG) << "XX did not find " 
			  << Form("nhits_%s_%s_c%d_r%d_C%d", 
				  fParDAC1.c_str(), fParDAC2.c_str(), wpix[ipix].column, wpix[ipix].row, rocIds[iroc]);
	  }

	}
      }

    }

    fDisplayedHist = find(fHistList.begin(), fHistList.end(), h);
    if (h) h->Draw(getHistOption(h).c_str());
    PixTest::update(); 
  }

}


// ----------------------------------------------------------------------
TH1* PixTestDacDacScan::moduleMap(string histname) {
  LOG(logDEBUG) << "PixTestDacScan::moduleMap histname: " << histname; 
  TH1* h0 = (*fDisplayedHist);
  if (!h0->InheritsFrom(TH2::Class())) {
    return 0; 
  }

  TH2D *h1 = (TH2D*)h0; 
  string h1name = h1->GetName();
  string::size_type s1 = h1name.rfind("_c"); 
  string barename = h1name.substr(0, s1);
  LOG(logDEBUG) << "h1->GetName() = " << h1name << " -> " << barename; 

  TH2D *h(0);
  string hname;
  int cycle(-1), ic(-1), ir(-1); 
  vector<uint8_t> rocIds = fApi->_dut->getEnabledRocIDs(); 
  for (unsigned int iroc = 0; iroc < rocIds.size(); ++iroc){

    h1  = bookTH2D(Form("%s_C%d", barename.c_str(), rocIds[iroc]), Form("%s_C%d", barename.c_str(), rocIds[iroc]), 
		   h0->GetNbinsX(), h0->GetXaxis()->GetBinLowEdge(1), h0->GetXaxis()->GetBinLowEdge(h0->GetNbinsX()+1), 
		   h0->GetNbinsY(), h0->GetYaxis()->GetBinLowEdge(1), h0->GetYaxis()->GetBinLowEdge(h0->GetNbinsY()+1)); 
    fHistOptions.insert(make_pair(h1, "colz"));
    if (0 == iroc) cycle = -1 + histCycle(Form("%s_C%d", barename.c_str(), rocIds[iroc])); 

    list<TH1*>::iterator hbeg = fHistList.begin();
    list<TH1*>::iterator hend = fHistList.end();
    cout << "Search for: " << Form("%s", barename.c_str()) << endl;
    for (list<TH1*>::iterator il = hbeg; il != hend; ++il) {
      h = dynamic_cast<TH2D*>(*il);
      hname = h->GetName();
      cout << "  looking at " << hname << endl;
      if (string::npos != hname.find(Form("%s", barename.c_str()))) {
	if (string::npos != hname.rfind(Form("_C%d", rocIds[iroc]))) {
	  if (string::npos != hname.rfind(Form("_V%d", cycle))) {
	    for (int ix = 1; ix < h->GetNbinsX(); ++ix) {
	      for (int iy = 1; iy < h->GetNbinsX(); ++iy) {
		h1->Fill(h->GetBinCenter(ix), h->GetBinCenter(iy), h->GetBinContent(ix, iy)); 
	      }
	    }
	  }
	}
      }
    }
    fHistList.push_back(h1); 
  }

  fDisplayedHist = find(fHistList.begin(), fHistList.end(), h1);

  if (h1) h1->Draw(getHistOption(h1).c_str());
  update(); 
  return h1; 
}
