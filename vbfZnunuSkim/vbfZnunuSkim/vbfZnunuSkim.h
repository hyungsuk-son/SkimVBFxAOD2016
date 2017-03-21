#ifndef vbfZnunuSkim_vbfZnunuSkim_H
#define vbfZnunuSkim_vbfZnunuSkim_H

#include <EventLoop/Algorithm.h>

// Event Bookkeepers
#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"

// Root includes
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TTree.h>
#include <TFile.h>
#include <string>
#include <vector>
#include <map>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

// EDM
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

// Jet
#include "JetCalibTools/JetCalibrationTool.h"

// Metadata
#include "xAODMetaDataCnv/FileMetaDataTool.h"
#include "xAODTriggerCnv/TriggerMenuMetaDataTool.h"


class vbfZnunuSkim : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;


  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!

  xAOD::TEvent *m_event; //!
  bool m_isData; //!

  TFile *file_xAOD; //!
  TH1 *h_sumOfWeights; //!

  xAODMaker::FileMetaDataTool *m_fileMetaDataTool; //!
  xAODMaker::TriggerMenuMetaDataTool *m_triggerMenuMetaDataTool; //!

  TFile* fOut; //!
  float mcEventWeight; //!


  // Cut values
  float m_uncalibMonoJetPt; //!
  float m_monoJetPt; //!
  float m_leadingJetPt; //!
  float m_subleadingJetPt; //!
  float m_Mjj; //!

  // Jet
  JetCalibrationTool* m_jetCalibration; //!


  // this is a standard constructor
  vbfZnunuSkim ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // this is needed to distribute the algorithm to the workers
  ClassDef(vbfZnunuSkim, 1);
};

#endif
