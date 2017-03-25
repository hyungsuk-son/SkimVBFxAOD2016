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
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"

// Jet
#include "JetCalibTools/JetCalibrationTool.h"

// Muon
#include "MuonMomentumCorrections/MuonCalibrationAndSmearingTool.h"

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


  // Cut values
  // Jet
  float m_uncalibMonoJetPt; //!
  float m_monoJetPt; //!
  float m_leadingJetPt; //!
  float m_subleadingJetPt; //!
  float m_Mjj; //!
  // Lepton
  float m_muonPtCut; //!

  // Jet
  JetCalibrationTool* m_jetCalibration; //!

  // Muon
  CP::MuonCalibrationAndSmearingTool *m_muonCalibrationAndSmearingTool; //!

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
