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
  // If you want to use grid datasets the easiest option for discovery is scanDQ2
//  SH::scanDQ2 (sh, "data15_13TeV.00270816.physics_Main.merge.AOD.f611_m1463");
  //SH::scanRucio (sh, "mc15_13TeV.364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  // If you know the name of all your grid datasets you can also skip the dq2-ls step and add the datasets directly


  // Retrieve dataset from the list file
  // https://atlaswww.hep.anl.gov/asc/WebSVN/filedetails.php?repname=dijets&path=%2Fcode%2Ftrunk%2Futil%2FrunDijetResonance.cxx
  //std::string myTagName = "mc15c.13TeV.Sherpa.DAOD_STDM4_skim_v3.02222018"; // MC15c (STDM4 derivation)
  //std::string myTagName = "mc15c.13TeV.Sherpa.DAOD_EXOT5_skim_v3.02222018"; // MC15c (EXOT5 derivation)
  std::string myTagName = "data15.13TeV.DAOD_EXOT5_skim_v3.02242018"; // Data 15 (EXOT5 derivation)
  std::string containerName;
  std::vector< std::string > outputContainerNames; //for grid only

  //std::ifstream inFile( "/cluster/home/h/s/hson02/atlas09Home/Work/vbfZinvSkim/vbfZnunuSkim/util/mc15c_Sherpa_STDM4_dataset.txt" ); // MC15c (STDM4 derivation)
  //std::ifstream inFile( "/cluster/home/h/s/hson02/atlas09Home/Work/vbfZinvSkim/vbfZnunuSkim/util/mc15c_Sherpa_EXOT5_dataset.txt" ); // MC15c (EXOT5 derivation)
  std::ifstream inFile( "/cluster/home/h/s/hson02/atlas09Home/Work/vbfZinvSkim/vbfZnunuSkim/util/data15_EXOT5_grlCovered.txt" ); // MC15c (EXOT5 derivation)
  while(std::getline(inFile, containerName) ){
    if (containerName.size() > 1 && containerName.find("#") != 0 ){
      std::cout << "Adding container : " << containerName << std::endl;
      SH::addGrid( sh, containerName);
      //Add output container name to file of containers
      // follows grid format: "user.hson."+myTagName+".%in:name[2]%.%in:name[6]%"

      // ex) mc15_13TeV.364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949
      int namePosition1 = 0;
      int namePosition2 = 0;
      namePosition1 = containerName.find_first_of(".", namePosition1);
      namePosition2 = containerName.find_last_of(".")+1;
      std::string datasetNum = containerName.substr(namePosition1, 8); // ex) .364151.
      std::string tagName = containerName.substr(namePosition2); // ex) e5308_s2726_r7772_r7676_p2949

      outputContainerNames.push_back( ("user.hson."+myTagName+datasetNum+tagName+"_mini-xAOD.root/") );
      std::cout << "Printing output container name : " << "user.hson."+myTagName+datasetNum+tagName+"_mini-xAOD.root/" << std::endl;
    }
  }



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
  //job.options()->setDouble (EL::Job::optMaxEvents, 500);
  //job.options()->setDouble (EL::Job::optRetries, 30);
  // Use TTreeCache to precache data files to speed up analysis
  //job.options()->setDouble (EL::Job::optCacheSize, 10*1024*1024);
  //job.options()->setDouble (EL::Job::optCacheLearnEntries, 20);

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

  driver.options()->setString("nc_outputSampleName", "user.hson."+myTagName+".%in:name[2]%.%in:name[6]%"); //For PrunDriver
//  driver.outputSampleName = "user.hson.gridtest1.11142015.%in:name[2]%.%in:name[6]%"; //For GridDriver
  //driver.options()->setDouble("nc_nFiles", 1); // FOR TESTING!
  //driver.options()->setDouble("nc_nFilesPerJob", 1);
  //driver.options()->setString(EL::Job::optGridMergeOutput, "true");
  //driver.options()->setDouble(EL::Job::optGridMemory,10240); // 10 GB
  //driver.options()->setDouble(EL::Job::optGridNFilesPerJob, 2);
//  driver.options()->setDouble(EL::Job::optGridMaxNFilesPerJob, 3);
  driver.options()->setDouble(EL::Job::optGridNGBPerJob, 6);
  driver.options()->setDouble(EL::Job::optGridNGBPerMergeJob, 3);
//  driver.options()->setString("nc_excludedSite", "ANALY_SCINET,ANALY_VICTORIA,ANALY_CERN_CLOUD,ANALY_IN2P3-CC,ANALY_LAPP,ANALY_CONNECT_SHORT,ANALY_SFU,ANALY_CONNECT,ANALY_RAL_SL6,ANALY_GRIF-LPNHE,ANALY_HU_ATLAS_Tier2,ANALY_OU_OCHEP_SWT2,ANALY_IFIC,ANALY_ECDF_SL6");
//  driver.options()->setString("nc_excludedSite", "ANALY_INFN-NAPOLI-RECAS,ANALY_INFN-NAPOLI,ANALY_DESY-HH,ANALY_GRIF-IRFU,ANALY_AUSTRALIA,ANALY_SFU,ANALY_SCINET,ANALY_CPPM,ANALY_SiGNET,ANALY_LPC,ANALY_NSC,ANALY_CONNECT,ANALY_MWT2_SL6,ANALY_BU_ATLAS_Tier2_SL6,ANALY_wuppertalprod,ANALY_ARNES,ANALY_SLAC_SHORT_1HR,ANALY_SLAC,ANALY_RAL_SL6,ANALY_INFN-MILANO-ATLASC");
//  driver.options()->setString("nc_excludedSite", "ANALY_TRIUMF");
//  driver.options()->setString("nc_site", "ANALY_CERN_SHORT,ANALY_CERN_SLC6,ANALY_PIC_SL6,ANALY_SARA"); // The Reflex dictionary build only works on a few sites
//  driver.options()->setString("nc_site", "ANALY_CERN_SLC6"); // The Reflex dictionary build only works on a few sites

//  driver.submit( job, submitDir );  // with monitoring
  driver.submitOnly(job, submitDir);  //without monitoring


  // For grid, save list of ouput containers to the submission directory
  // https://atlaswww.hep.anl.gov/asc/WebSVN/filedetails.php?repname=dijets&path=%2Fcode%2Ftrunk%2Futil%2FrunDijetResonance.cxx
  std::ofstream fileList((submitDir+"/outputContainers.txt"), std::ios_base::out);
  for( unsigned int iCont=0; iCont < outputContainerNames.size(); ++iCont){
    fileList << outputContainerNames.at(iCont)+"\n";
  }
  fileList.close();


  return 0;
}
