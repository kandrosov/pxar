// -- author: Wolfram Erdmann
// Bump Bonding tests, really just a threshold map using cals

#include <sstream>   // parsing
#include <algorithm>  // std::find

#include "PixTestBBMap.hh"
#include "log.h"
#include "constants.h"   // roctypes

using namespace std;
using namespace pxar;

ClassImp(PixTestBBMap)

//------------------------------------------------------------------------------
PixTestBBMap::PixTestBBMap( PixSetup *a, std::string name )
: PixTest(a, name), fParNtrig(-1), fParVcalS(200),fPartest(0)
{
  PixTest::init();
  init();
  LOG(logDEBUG) << "PixTestBBMap ctor(PixSetup &a, string, TGTab *)";
}


//------------------------------------------------------------------------------
PixTestBBMap::PixTestBBMap() : PixTest()
{
  LOG(logDEBUG) << "PixTestBBMap ctor()";
}



//------------------------------------------------------------------------------
bool PixTestBBMap::setParameter( string parName, string sval )
{
  for( uint32_t i = 0; i < fParameters.size(); ++i ) {

    if( fParameters[i].first == parName ) {

      stringstream s(sval);

      if( !parName.compare( "Ntrig" ) ) { s >> fParNtrig; return true;}
      if( !parName.compare( "VcalS" ) ) { s >> fParVcalS; return true;}
      if( !parName.compare( "test" ) )  { s >> fPartest;  return true;}
    }
  }
  return false;
}

//------------------------------------------------------------------------------
void PixTestBBMap::init()
{
  LOG(logDEBUG) << "PixTestBBMap::init()";
  
  fDirectory = gFile->GetDirectory( fName.c_str() );
  if( !fDirectory ) {
    fDirectory = gFile->mkdir( fName.c_str() );
  }
  fDirectory->cd();
}

// ----------------------------------------------------------------------
void PixTestBBMap::setToolTips()
{
  fTestTip = string( "Bump Bonding Test = threshold map for CalS");
  fSummaryTip = string("module summary");
}


//------------------------------------------------------------------------------
PixTestBBMap::~PixTestBBMap()
{
  LOG(logDEBUG) << "PixTestBBMap dtor";
  std::list<TH1*>::iterator il;
  fDirectory->cd();
  for( il = fHistList.begin(); il != fHistList.end(); ++il ) {
    LOG(logINFO) << "Write out " << (*il)->GetName();
    (*il)->SetDirectory(fDirectory);
    (*il)->Write();
  }
}






//------------------------------------------------------------------------------
void PixTestBBMap::doTest()
{
  LOG(logINFO) << "PixTestBBMap::doTest() ntrig = " << fParNtrig
               << " VcalS = " << fParVcalS << endl;

  fDirectory->cd();
  fHistList.clear();
  PixTest::update();

  fApi->setDAC("ctrlreg", 4);      // high range
  fApi->_dut->testAllPixels(true); // all pix, all rocs
  fApi->_dut->maskAllPixels(false);

  
  int flag= FLAG_CALS;
  if (fPartest>0){ flag = 0;} // tests  
  bool subtractXtalk = false;
  
  if (subtractXtalk){ cout << "just to avoid compiler warnings" << endl;}
  
  // FIXME: range hardcoded for now, must avoid getting too close to the threshold until "other" pixels 
  // are disabled during the scan  
  vector<TH1*>  thrmaps = thrMaps("VthrComp", 0, 110, "VcalS", flag, fParNtrig); 

    

  vector<uint8_t> rocIds = fApi->_dut->getEnabledRocIDs(); 
  for (unsigned int idx = 0; idx < rocIds.size(); ++idx){
      
      unsigned int rocId = getIdFromIdx( idx );
      TH2D* rocmap = (TH2D*) thrmaps[idx];
      
      /*      
      if (subtractXtalk) {
        fHistList.push_back( rocmap0 );
        rocmap = (TH2D*) (mit->second)->Clone(Form("BB %2d diff",rocId));
        rocmap->Add( rocmap0, -1. );
        fHistOptions.insert(make_pair( rocmap, "colz" ));
        fHistList.push_back( rocmap );
      }
      */
      // fill distribution from map
      TH1D* hdist = bookTH1D( //new TH1D( 
        Form("CalS-threshold-distribution-%0d",rocId), 
        Form("CalS-threshold distribution %2d",rocId), 
        256, 0., 256.);
      
      for(int col=0; col<ROC_NUMCOLS; col++){
          for(int row=0; row<ROC_NUMROWS; row++){
              hdist->Fill( rocmap->GetBinContent( col+1, row+1 ) );
          }
      }
      fHistList.push_back( hdist );
  }
  
  TH2D * module =  (TH2D*) moduleMap("BB module map");
  fHistList.push_back( module);
  
  if (fHistList.size()>0){
    TH2D *h = (TH2D*)(*fHistList.begin());
    h->Draw(getHistOption(h).c_str());
    fDisplayedHist = find(fHistList.begin(), fHistList.end(), h);
    PixTest::update(); 
  }else{
    LOG(logDEBUG) << "no histogram produced, this is probably a bug";
  }
}
