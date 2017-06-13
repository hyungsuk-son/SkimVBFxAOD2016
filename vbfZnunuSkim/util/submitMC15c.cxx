#include "xAODRootAccess/Init.h"
#include "EventLoop/Job.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoopGrid/PrunDriver.h"
#include "EventLoopGrid/GridDriver.h"
#include "EventLoopGrid/GridWorker.h"
#include <TSystem.h>
#include "SampleHandler/Sample.h"
#include "SampleHandler/SampleGrid.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ScanDir.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "SampleHandler/DiskListLocal.h"
#include "SampleHandler/MetaFields.h"
#include "SampleHandler/MetaObject.h"
#include <EventLoopAlgs/NTupleSvc.h>
#include <EventLoop/OutputStream.h>

#include "vbfZnunuSkim/vbfZnunuSkim.h"

int main( int argc, char* argv[] ) {

  // Take the submit directory from the input if provided:
  std::string submitDir = "submitDir";
  if( argc > 1 ) submitDir = argv[ 1 ];

  // Set up the job for xAOD access:
  xAOD::Init().ignore();

  // Construct the samples to run on:
  SH::SampleHandler sh;

  // use SampleHandler to scan all of the subdirectories of a directory for particular MC single file:
//  const char* inputFilePath = gSystem->ExpandPathName ("$ALRB_TutorialData/r6630/");
//  SH::ScanDir().sampleDepth(1).samplePattern("AOD.05352803._000031.pool.root.1").scan(sh, inputFilePath);
//  const char* inputFilePath = gSystem->ExpandPathName ("/afs/cern.ch/user/h/hson/Work/xAOD_data");
//  SH::DiskListLocal list (inputFilePath);
//  SH::scanDir (sh, list, "data15_13TeV.00271048.physics_Main.merge.AOD.f611_m1463._lb0408._0001.1"); // specifying one particular file for testing
  // If you want to use grid datasets the easiest option for discovery is scanRucio
//  SH::scanRucio (sh, "mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.DAOD_EXOT5.e3698_s2608_s2183_r7267_r6282_p2495");
  // If you know the name of all your grid datasets you can also skip the dq2-ls step and add the datasets directly
//  SH::addGrid (sh, "data15_13TeV.00281411.physics_Main.merge.AOD.f629_m1504");
//  SH::addGrid (sh, "data15_13TeV.00282784.physics_Main.merge.AOD.f640_m1511");

  // MC15c_13TeV (EXOT5)

  // EWK

  // Sherpa diboson samples, after the EventShape bugfix (which affected p2839)
  /*
  // Zmumu
  SH::addGrid (sh, "mc15_13TeV.364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5271_s2726_r7772_r7676_p2949");
  // Zee
  SH::addGrid (sh, "mc15_13TeV.364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5299_s2726_r7772_r7676_p2949");
  // Ztautau
  SH::addGrid (sh, "mc15_13TeV.364128.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364129.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364130.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364131.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364132.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364133.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364134.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364135.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364136.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364137.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364138.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5313_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364139.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5313_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364140.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364141.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5307_s2726_r7772_r7676_p2949");
  // Znunu
  SH::addGrid (sh, "mc15_13TeV.364142.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364143.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364144.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364145.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364146.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364147.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364148.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364149.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364150.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364152.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364153.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364155.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  // Wmunu
  SH::addGrid (sh, "mc15_13TeV.364156.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364157.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364158.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364159.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364160.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364161.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364162.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364163.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364165.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364166.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364167.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364168.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364169.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  // Wenu
  SH::addGrid (sh, "mc15_13TeV.364170.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364171.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364173.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364174.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364175.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364176.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364177.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364178.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364179.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364180.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364181.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364182.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364183.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  // Wtaunu
  SH::addGrid (sh, "mc15_13TeV.364184.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364185.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364186.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364187.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364188.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364189.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364190.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364191.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364192.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364193.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364194.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364195.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364196.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364197.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5340_s2726_r7772_r7676_p2949");
  */
  /*
  */


  // MC15c_13TeV (STDM4)

  // Sherpa diboson samples, after the EventShape bugfix (which affected p2839)
  // Zmumu
  SH::addGrid (sh, "mc15_13TeV.364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");


  // ttbar and single top
  // Multijet
  // Diboson

  // Set the name of the input TTree. It's always "CollectionTree"
  // for xAOD files.
  sh.setMetaString( "nc_tree", "CollectionTree" );
  sh.setMetaString("nc_grid_filter", "*AOD*");
//  sh.setMetaString("nc_grid_filter", "*");

  // Print what we found:
  sh.print();

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler( sh );
//  job.options()->setDouble (EL::Job::optMaxEvents, 500);
  //job.options()->setDouble (EL::Job::optRetries, 30);
  //job.options()->setDouble (EL::Job::optCacheSize, 10*1024*1024);
  //job.options()->setDouble (EL::Job::optCacheLearnEntries, 50);

/*  
  // For ntuple
  // define an output and an ntuple associated to that output
  EL::OutputStream output  ("myOutput");
  job.outputAdd (output);
  EL::NTupleSvc *ntuple = new EL::NTupleSvc ("myOutput");
  job.algsAdd (ntuple);
  //---------------------------------------------------------
*/

  // Add our analysis to the job:
  vbfZnunuSkim* alg = new vbfZnunuSkim();
  job.algsAdd( alg );

/*
  // For ntuple
  // Let your algorithm know the name of the output file
  alg->outputName = "myOutput"; // give the name of the output to our algorithm
  //-----------------------------------------------------------------------------
*/

  // Run the job using the local/direct driver:
//  EL::DirectDriver driver; //local
  EL::PrunDriver driver;  //grid
//  EL::GridDriver driver; //grid in the background

  driver.options()->setString("nc_outputSampleName", "user.hson.mc15c.13TeV.DAOD_STDM4_skim_v1_test3.06122017.%in:name[2]%.%in:name[6]%"); //For PrunDriver
//  driver.outputSampleName = "user.hson.gridtest1.11142015.%in:name[2]%.%in:name[6]%"; //For GridDriver
//  driver.options()->setDouble("nc_nFiles", 2); // FOR TESTING!
//  driver.options()->setDouble("nc_nFilesPerJob", 1);
//  driver.options()->setDouble(EL::Job::optGridNFilesPerJob, 1);
  driver.options()->setDouble(EL::Job::optGridNGBPerJob, 6);
  driver.options()->setDouble(EL::Job::optGridNGBPerMergeJob, 2);
//  driver.options()->setString("nc_excludedSite", "ANALY_INFN-NAPOLI-RECAS,ANALY_INFN-NAPOLI,ANALY_DESY-HH,ANALY_GRIF-IRFU,ANALY_AUSTRALIA,ANALY_SFU,ANALY_SCINET,ANALY_CPPM,ANALY_SiGNET,ANALY_LPC,ANALY_NSC,ANALY_CONNECT,ANALY_MWT2_SL6,ANALY_BU_ATLAS_Tier2_SL6,ANALY_wuppertalprod,ANALY_ARNES,ANALY_SLAC_SHORT_1HR,ANALY_SLAC,ANALY_RAL_SL6,ANALY_INFN-MILANO-ATLASC");
//  driver.options()->setString("nc_excludedSite", "ANALY_BNL_LONG,ANALY_LRZ,ANALY_FZK_SHORT,ANALY_SiGNET,ANALY_DESY-ZN,ANALY_FZK,ANALY_LPC,ANALY_TRIUMF,ANALY_GLASGOW_SL6");

  //driver.options()->setString("nc_excludedSite", "ANALY_BHAM_SL6,ANALY_BU_ATLAS_Tier2_SL6,ANALY_GOEGRID,ANALY_IHEP,ANALY_TR-10-ULAKBIM,ANALY_IL-TAU-HEP-CREAM,ANALY_MCGILL");
  //driver.options()->setString("nc_excludedSite", "ANALY_RAL_SL6");

//  driver.options()->setString("nc_site", "ANALY_CERN_SHORT,ANALY_CERN_SLC6,ANALY_PIC_SL6,ANALY_SARA"); // The Reflex dictionary build only works on a few sites
//  driver.options()->setString("nc_site", "ANALY_CERN_SLC6"); // The Reflex dictionary build only works on a few sites
//  driver.options()->setDouble(EL::Job::optGridMemory,10240); // 10 GB

//  driver.submit( job, submitDir );  // with monitoring
  driver.submitOnly(job, submitDir);  //without monitoring

  return 0;
}
