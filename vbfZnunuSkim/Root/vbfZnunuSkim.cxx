#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <vbfZnunuSkim/vbfZnunuSkim.h>

#include "EventLoop/OutputStream.h"

// Infrastructure include(s):
#include "xAODRootAccess/tools/Message.h"

// ASG status code check
#include <AsgTools/MessageCheck.h>

// EDM includes:
#include "xAODEventInfo/EventInfo.h"
#include "xAODBase/IParticle.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODBase/IParticleHelpers.h"
#include "AthContainers/ConstDataVector.h"

#include "PATInterfaces/CorrectionCode.h" // to check the return correction code status of tools
#include "xAODCore/ShallowAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCore/AuxContainerBase.h"


struct DescendingPt:std::function<bool(const xAOD::IParticle*, const xAOD::IParticle*)> {
  bool operator()(const xAOD::IParticle* l, const xAOD::IParticle* r)  const {
    return l->pt() > r->pt();
  }
};




// this is needed to distribute the algorithm to the workers
ClassImp(vbfZnunuSkim)



vbfZnunuSkim :: vbfZnunuSkim ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode vbfZnunuSkim :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  job.useXAOD ();
  xAOD::Init().ignore(); // call before opening first file

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  ANA_CHECK(xAOD::Init());



  // Output
  EL::OutputStream out_xAOD ("mini-xAOD", "xAOD");
  job.outputAdd (out_xAOD);

  //EL::OutputStream out_tree ("output_tree");
  //job.outputAdd (out_tree);

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  h_sumOfWeights = new TH1D("h_sumOfWeights", "MetaData_EventCount", 6, 0.5, 6.5);
  //h_sumOfWeights -> GetXaxis() -> SetBinLabel(1, "sumOfWeights DxAOD");
  //h_sumOfWeights -> GetXaxis() -> SetBinLabel(2, "sumOfWeightsSquared DxAOD");
  //h_sumOfWeights -> GetXaxis() -> SetBinLabel(3, "nEvents DxAOD");
  h_sumOfWeights -> GetXaxis() -> SetBinLabel(1, "sumOfWeights initial");
  h_sumOfWeights -> GetXaxis() -> SetBinLabel(2, "sumOfWeightsSquared initial");
  h_sumOfWeights -> GetXaxis() -> SetBinLabel(3, "nEvents initial");
  wk()->addOutput (h_sumOfWeights);

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed

  // get TEvent and TStore - must be done here b/c we need to retrieve CutBookkeepers container from TEvent!
  m_event = wk()->xaodEvent();

  //----------------------------
  // Event information
  //--------------------------- 
  const xAOD::EventInfo* eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info collection in initialise. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  // check if the event is data or MC
  // (many tools are applied either to data or MC)
  m_isData = true;
  // check if the event is MC
  if(eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ){
    m_isData = false; // can do something with this later
  }


  // Event Bookkeepers
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/AnalysisMetadata#Luminosity_Bookkeepers

  // get the MetaData tree once a new file is opened, with
  TTree *MetaData = dynamic_cast<TTree*>(wk()->inputFile()->Get("MetaData"));
  if (!MetaData) {
    Error("fileExecute()", "MetaData not found! Exiting.");
    return EL::StatusCode::FAILURE;
  }
  MetaData->LoadTree(0);

  //check if file is from a DxAOD
  bool m_isDerivation = !MetaData->GetBranch("StreamAOD");

  if(!m_isData && m_isDerivation){

    /*
    // check for corruption
    const xAOD::CutBookkeeperContainer* incompleteCBC = nullptr;
    if(!m_event->retrieveMetaInput(incompleteCBC, "IncompleteCutBookkeepers").isSuccess()){
      Error("initializeEvent()","Failed to retrieve IncompleteCutBookkeepers from MetaData! Exiting.");
      return EL::StatusCode::FAILURE;
    }
    if ( incompleteCBC->size() != 0 ) {
      Error("initializeEvent()","Found incomplete Bookkeepers! Check file for corruption.");
      return EL::StatusCode::FAILURE;
    }
    */

    // Now, let's find the actual information
    const xAOD::CutBookkeeperContainer* completeCBC = 0;
    if(!m_event->retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()){
      Error("initializeEvent()","Failed to retrieve CutBookkeepers from MetaData! Exiting.");
      return EL::StatusCode::FAILURE;
    }

    /*
    // First, let's find the smallest cycle number,
    // i.e., the original first processing step/cycle
    int minCycle = 10000;
    for ( auto cbk : *completeCBC ) {
      if ( ! cbk->name().empty()  && minCycle > cbk->cycle() ){ minCycle = cbk->cycle(); }
    }

    // Now, let's actually find the right one that contains all the needed info...
    const xAOD::CutBookkeeper* allEventsCBK=0;
    const xAOD::CutBookkeeper* DxAODEventsCBK=0;
    std::string derivationName = "EXOT5Kernel"; //need to replace by appropriate name
    int maxCycle = -1;
    for (const auto& cbk: *completeCBC) {
      if (cbk->cycle() > maxCycle && cbk->name() == "AllExecutedEvents" && cbk->inputStream() == "StreamAOD") {
        allEventsCBK = cbk;
        maxCycle = cbk->cycle();
      }
      if ( cbk->name() == derivationName){
        DxAODEventsCBK = cbk;
      }
      }
      */

    // Now, let's actually find the right one that contains all the needed info...
    const xAOD::CutBookkeeper* allEventsCBK=0;
    int maxcycle = -1;
    for ( auto cbk : *completeCBC ) { 
      if ( cbk->name() == "AllExecutedEvents" && cbk->inputStream() == "StreamAOD" && cbk->cycle() > maxcycle){
        maxcycle = cbk->cycle();
        allEventsCBK = cbk;
      }
    }


    uint64_t nEventsProcessed  = allEventsCBK->nAcceptedEvents();
    double sumOfWeights        = allEventsCBK->sumOfEventWeights();
    double sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();

    /*
    uint64_t nEventsDxAOD           = DxAODEventsCBK->nAcceptedEvents();
    double sumOfWeightsDxAOD        = DxAODEventsCBK->sumOfEventWeights();
    double sumOfWeightsSquaredDxAOD = DxAODEventsCBK->sumOfEventWeightsSquared();
    */



    //Info("execute()", " Event # = %llu, sumOfweights = %f, mcEventWeight = %f", eventInfo->eventNumber(), sumOfWeights, eventInfo->mcEventWeight());
    //Info("execute()", " Event # = %llu, nEventsProcessed = %d, sumOfweights = %f, sumOfWeightsSquared = %f", eventInfo->eventNumber(), nEventsProcessed, sumOfWeights, sumOfWeightsSquared);

    //h_sumOfWeights -> Fill(1, sumOfWeightsDxAOD);
    //h_sumOfWeights -> Fill(2, sumOfWeightsSquaredDxAOD);
    //h_sumOfWeights -> Fill(3, nEventsDxAOD);
    h_sumOfWeights -> Fill(1, sumOfWeights);
    h_sumOfWeights -> Fill(2, sumOfWeightsSquared);
    h_sumOfWeights -> Fill(3, nEventsProcessed);

    //Info("execute()", " Event # = %llu, sumOfWeights/nEventsProcessed = %f", eventInfo->eventNumber(), sumOfWeights/double(nEventsProcessed));
  }
  
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  //----------------------------
  // Event information
  //--------------------------- 
  const xAOD::EventInfo* eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info collection in initialise. Exiting." );
    return EL::StatusCode::FAILURE;
  }


  // Cut values
  m_uncalibMonoJetPt = 100000.;
  m_monoJetPt = 100000.;
  m_leadingJetPt = 70000.;
  m_subleadingJetPt = 40000.;
  m_Mjj = 180000.;


  // output file
  file_xAOD = wk()->getOutputFile ("mini-xAOD");
  ANA_CHECK(m_event->writeTo(file_xAOD));

  // check if the event is data or MC
  // (many tools are applied either to data or MC)
  m_isData = true;
  // check if the event is MC
  if(eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ){
    m_isData = false; // can do something with this later
  }

  // tools to store the meta data in the output mini-xAOD
  m_fileMetaDataTool = new xAODMaker::FileMetaDataTool("FileMetaDataTool");
  ANA_CHECK(m_fileMetaDataTool->initialize());

  m_triggerMenuMetaDataTool = new xAODMaker::TriggerMenuMetaDataTool("TriggerMenuMetaDataTool");
  ANA_CHECK(m_triggerMenuMetaDataTool->initialize());


  // JES Calibration (https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetEtmissRecommendations2016)
  const std::string name_JetCalibTools = "JetCalibTools";
  std::string jetAlgo = "AntiKt4EMTopo"; //String describing your jet collection, for example AntiKt4EMTopo or AntiKt4LCTopo
  std::string config = "JES_data2016_data2015_Recommendation_Dec2016.config"; //Path to global config used to initialize the tool
  std::string calibSeq = "JetArea_Residual_Origin_EtaJES_GSC"; //String describing the calibration sequence to apply
  if (m_isData) calibSeq += "_Insitu";
  //Call the constructor. The default constructor can also be used if the arguments are set with python configuration instead
  //Initialize the tool
  m_jetCalibration = new JetCalibrationTool(name_JetCalibTools.c_str());
  ANA_CHECK(m_jetCalibration->setProperty("JetCollection", jetAlgo.c_str()));
  ANA_CHECK(m_jetCalibration->setProperty("ConfigFile", config.c_str()));
  ANA_CHECK(m_jetCalibration->setProperty("CalibSequence", calibSeq.c_str()));
  ANA_CHECK(m_jetCalibration->setProperty("IsData", m_isData));
  ANA_CHECK(m_jetCalibration->initialize());




  if (!m_isData) {
    m_event->setAuxItemList("AntiKt4EMTopoJetsAux.", "pt.eta.phi.m.constituentLinks.constituentWeights.ConstituentScale.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.JetEMScaleMomentum_phi.JetEMScaleMomentum_m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.InputType.AlgorithmType.SizeParameter.btaggingLink.EnergyPerSampling.NumTrkPt500.SumPtTrkPt500.EMFrac.Width.LeadingClusterSecondR.Mu12.N90Constituents.NegativeE.NumTrkPt1000.OotFracClusters10.OotFracClusters5.OriginCorrected.OriginVertex.PartonTruthLabelID.PileupCorrected.PlanarFlow.Sphericity.Split12.Split23.Split34.SumPtTrkPt1000.Tau1.Tau1_wta.Tau2.Tau2_wta.Tau3.Tau3_wta.ThrustMaj.ThrustMin.Timing.TrackWidthPt1000.TrackWidthPt500.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T.WidthPhi.ZCut12.ZCut23.ZCut34.ActiveArea.ActiveArea4vec_eta.ActiveArea4vec_m.ActiveArea4vec_phi.ActiveArea4vec_pt.Angularity.Aplanarity.AverageLArQF.BchCorrCell.CentroidR.Charge.ConeExclBHadronsFinal.ConeExclCHadronsFinal.ConeExclTausFinal.ConeTruthLabelID.DetectorEta.Dip12.Dip13.Dip23.DipExcl12.ECF1.ECF2.ECF3.ECPSFraction.FoxWolfram0.FoxWolfram1.FoxWolfram2.FoxWolfram3.FoxWolfram4.FracSamplingMax.FracSamplingMaxIndex.GhostAntiKt2TrackJet.GhostAntiKt2TrackJetCount.GhostAntiKt2TrackJetPt.GhostAntiKt3TrackJet.GhostAntiKt3TrackJetCount.GhostAntiKt3TrackJetPt.GhostAntiKt4TrackJet.GhostAntiKt4TrackJetCount.GhostAntiKt4TrackJetPt.GhostBHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostBHadronsInitial.GhostBHadronsInitialCount.GhostBHadronsInitialPt.GhostBQuarksFinal.GhostBQuarksFinalCount.GhostBQuarksFinalPt.GhostCHadronsFinal.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostCHadronsInitial.GhostCHadronsInitialCount.GhostCHadronsInitialPt.GhostCQuarksFinal.GhostCQuarksFinalCount.GhostCQuarksFinalPt.GhostHBosons.GhostHBosonsCount.GhostHBosonsPt.GhostMuonSegment.GhostMuonSegmentCount.GhostPartons.GhostPartonsCount.GhostPartonsPt.GhostTQuarksFinal.GhostTQuarksFinalCount.GhostTQuarksFinalPt.GhostTausFinal.GhostTausFinalCount.GhostTausFinalPt.GhostTrack.GhostTrackCount.GhostTrackPt.GhostTruth.GhostTruthAssociationFraction.GhostTruthAssociationLink.GhostTruthCount.GhostTruthPt.GhostWBosons.GhostWBosonsCount.GhostWBosonsPt.GhostZBosons.GhostZBosonsCount.GhostZBosonsPt.HECFrac.HECQuality.HadronConeExclTruthLabelID.HighestJVFVtx.IsoDelta2SumPt.IsoDelta3SumPt.JVF.JetGhostArea.JetLCScaleMomentum_eta.JetLCScaleMomentum_m.JetLCScaleMomentum_phi.JetLCScaleMomentum_pt.JetOriginConstitScaleMomentum_eta.JetOriginConstitScaleMomentum_m.JetOriginConstitScaleMomentum_phi.JetOriginConstitScaleMomentum_pt.JetPileupScaleMomentum_eta.JetPileupScaleMomentum_m.JetPileupScaleMomentum_phi.JetPileupScaleMomentum_pt.Jvt.JvtJvfcorr.JvtRpt.KtDR.LArBadHVEnergyFrac.LArBadHVNCell.LArQuality.LeadingClusterCenterLambda.LeadingClusterPt.LeadingClusterSecondLambda"); 
    m_event->setAuxItemList("MuonsAux.", "pt.eta.phi.muonSegmentLinks.charge.EnergyLoss.numberOfPrecisionHoleLayers.numberOfPrecisionLayers.truthParticleLink.etcone20.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone30.topoetcone40.truthOrigin.truthType.author.inDetTrackParticleLink.muonType.CaloLRLikelihood.CaloMuonIDTag.DFCommonGoodMuon.EnergyLossSigma.MeasEnergyLoss.MeasEnergyLossSigma.ParamEnergyLoss.ParamEnergyLossSigmaMinus.ParamEnergyLossSigmaPlus.clusterLink.combinedTrackOutBoundsPrecisionHits.combinedTrackParticleLink.energyLossType.etcone30.etcone40.extendedClosePrecisionHits.extendedLargeHits.extendedLargeHoles.extendedOutBoundsPrecisionHits.extendedSmallHits.extendedSmallHoles.extrapolatedMuonSpectrometerTrackParticleLink.innerClosePrecisionHits.innerLargeHits.innerLargeHoles.innerOutBoundsPrecisionHits.innerSmallHits.innerSmallHoles.isEndcapGoodLayers.isSmallGoodSectors.middleClosePrecisionHits.middleLargeHits.middleLargeHoles.middleOutBoundsPrecisionHits.middleSmallHits.middleSmallHoles.momentumBalanceSignificance.muonSpectrometerTrackParticleLink.numberOfGoodPrecisionLayers.outerClosePrecisionHits.outerLargeHits.outerLargeHoles.outerOutBoundsPrecisionHits.outerSmallHits.outerSmallHoles.quality"); 
    m_event->setAuxItemList("ElectronsAux.",
        "trackParticleLinks.pt.eta.phi.m.charge.truthParticleLink.deltaPhiRescaled2.e233.e237.e277.e2tsts1.emaxs1.emins1.etcone20.etcone20ptCorrection.etcone30ptCorrection.etcone40ptCorrection.ethad.ethad1.f1.f3.f3core.fracs1.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone20ptCorrection.topoetcone30.topoetcone30ptCorrection.topoetcone40.topoetcone40ptCorrection.truthOrigin.truthType.weta1.weta2.wtots1.DFCommonElectronsIsEMLoose.DFCommonElectronsIsEMMedium.DFCommonElectronsIsEMTight.DFCommonElectronsLHLoose.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsML.DeltaE.Eratio.LHLoose.Loose.Medium.OQ.Reta.Rhad.Rhad1.Rphi.Tight.author.caloClusterLinks.deltaEta1.deltaPhi2"); 
    m_event->setAuxItemList("EventInfoAux.", "runNumber.eventNumber.eventTypeBitmask.averageInteractionsPerCrossing.beamPosSigmaX.beamPosSigmaY.beamPosSigmaXY.mcChannelNumber.mcEventWeights");
    m_event->setAuxItemList("PhotonsAux.", "pt.eta.phi.m.truthParticleLink.e233.e237.e277.e2tsts1.emaxs1.emins1.etcone20ptCorrection.etcone30ptCorrection.etcone40ptCorrection.ethad.ethad1.f1.f3.fracs1.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone20ptCorrection.topoetcone30.topoetcone30ptCorrection.topoetcone40.topoetcone40ptCorrection.truthOrigin.truthType.weta1.weta2.wtots1.DeltaE.Eratio.Loose.OQ.Reta.Rhad.Rhad1.Rphi.Tight.author.caloClusterLinks.DFCommonPhotonsIsEMLoose.DFCommonPhotonsIsEMTight.vertexLinks"); 
    m_event->setAuxItemList("TauJetsAux.",  "pt.eta.phi.m.trackLinks.wideTrackLinks.otherTrackLinks.jetLink.vertexLink.secondaryVertexLink.hadronicPFOLinks.shotPFOLinks.chargedPFOLinks.neutralPFOLinks.pi0PFOLinks.protoChargedPFOLinks.protoNeutralPFOLinks.protoPi0PFOLinks.charge.truthParticleLink.BDTJetScore.BDTEleScore.isTauFlags.IsTruthMatched.truthJetLink");
  }
  else {
    m_event->setAuxItemList("AntiKt4EMTopoJetsAux.",
        "pt.eta.phi.m.constituentLinks.constituentWeights.ConstituentScale.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.JetEMScaleMomentum_phi.JetEMScaleMomentum_m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.InputType.AlgorithmType.SizeParameter.btaggingLink.EnergyPerSampling.NumTrkPt500.SumPtTrkPt500.EMFrac.Width.ActiveArea.ActiveArea4vec_eta.ActiveArea4vec_m.ActiveArea4vec_phi.ActiveArea4vec_pt.Angularity.Aplanarity.AverageLArQF.BchCorrCell.CentroidR.Charge.DetectorEta.Dip12.Dip13.Dip23.DipExcl12.ECF1.ECF2.ECF3.ECPSFraction.FoxWolfram0.FoxWolfram1.FoxWolfram2.FoxWolfram3.FoxWolfram4.FracSamplingMax.FracSamplingMaxIndex.GhostAntiKt2TrackJet.GhostAntiKt2TrackJetCount.GhostAntiKt2TrackJetPt.GhostAntiKt3TrackJet.GhostAntiKt3TrackJetCount.GhostAntiKt3TrackJetPt.GhostAntiKt4TrackJet.GhostAntiKt4TrackJetCount.GhostAntiKt4TrackJetPt.GhostMuonSegment.GhostMuonSegmentCount.GhostTrack.GhostTrackCount.GhostTrackPt.HECFrac.HECQuality.HighestJVFVtx.IsoDelta2SumPt.IsoDelta3SumPt.JVF.JetGhostArea.JetLCScaleMomentum_eta.JetLCScaleMomentum_m.JetLCScaleMomentum_phi.JetLCScaleMomentum_pt.JetOriginConstitScaleMomentum_eta.JetOriginConstitScaleMomentum_m.JetOriginConstitScaleMomentum_phi.JetOriginConstitScaleMomentum_pt.JetPileupScaleMomentum_eta.JetPileupScaleMomentum_m.JetPileupScaleMomentum_phi.JetPileupScaleMomentum_pt.Jvt.JvtJvfcorr.JvtRpt.KtDR.LArBadHVEnergyFrac.LArBadHVNCell.LArQuality.LeadingClusterCenterLambda.LeadingClusterPt.LeadingClusterSecondLambda.LeadingClusterSecondR.Mu12.N90Constituents.NegativeE.NumTrkPt1000.OotFracClusters10.OotFracClusters5.OriginCorrected.OriginVertex.PileupCorrected.PlanarFlow.Sphericity.Split12.Split23.Split34.SumPtTrkPt1000.Tau1.Tau1_wta.Tau2.Tau2_wta.Tau3.Tau3_wta.ThrustMaj.ThrustMin.Timing.TrackWidthPt1000.TrackWidthPt500.WidthPhi.ZCut12.ZCut23.ZCut34"); 
    m_event->setAuxItemList("MuonsAux.",
        "pt.eta.phi.muonSegmentLinks.charge.EnergyLoss.numberOfPrecisionHoleLayers.numberOfPrecisionLayers.author.etcone20.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone30.topoetcone40.inDetTrackParticleLink.muonType.quality.truthOrigin.truthParticleLink.truthType.CaloLRLikelihood.CaloMuonIDTag.DFCommonGoodMuon.EnergyLossSigma.MeasEnergyLoss.MeasEnergyLossSigma.ParamEnergyLoss.ParamEnergyLossSigmaMinus.ParamEnergyLossSigmaPlus.clusterLink.combinedTrackOutBoundsPrecisionHits.combinedTrackParticleLink.energyLossType.etcone30.etcone40.extendedClosePrecisionHits.extendedLargeHits.extendedLargeHoles.extendedOutBoundsPrecisionHits.extendedSmallHits.extendedSmallHoles.extrapolatedMuonSpectrometerTrackParticleLink.innerClosePrecisionHits.innerLargeHits.innerLargeHoles.innerOutBoundsPrecisionHits.innerSmallHits.innerSmallHoles.isEndcapGoodLayers.isSmallGoodSectors.middleClosePrecisionHits.middleLargeHits.middleLargeHoles.middleOutBoundsPrecisionHits.middleSmallHits.middleSmallHoles.momentumBalanceSignificance.muonSpectrometerTrackParticleLink.numberOfGoodPrecisionLayers.outerClosePrecisionHits.outerLargeHits.outerLargeHoles.outerOutBoundsPrecisionHits.outerSmallHits.outerSmallHoles");
    m_event->setAuxItemList("ElectronsAux.",
        "trackParticleLinks.pt.eta.phi.m.charge.DFCommonElectronsIsEMLoose.DFCommonElectronsIsEMMedium.DFCommonElectronsIsEMTight.DFCommonElectronsLHLoose.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsML.DeltaE.Eratio.LHLoose.Loose.Medium.OQ.Reta.Rhad.Rhad1.Rphi.Tight.author.caloClusterLinks.deltaEta1.deltaPhi2.deltaPhiRescaled2.e233.e237.e277.e2tsts1.emaxs1.emins1.etcone20.etcone20ptCorrection.etcone30ptCorrection.etcone40ptCorrection.ethad.ethad1.f1.f3.f3core.fracs1.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone20ptCorrection.topoetcone30.topoetcone30ptCorrection.topoetcone40.topoetcone40ptCorrection.weta1.weta2.wtots1");
    m_event->setAuxItemList("EventInfoAux.", "runNumber.eventNumber.lumiBlock.eventTypeBitmask.averageInteractionsPerCrossing.sctFlags.larFlags.tileFlags.coreFlags.beamPosSigmaX.beamPosSigmaY.beamPosSigmaXY");
    m_event->setAuxItemList("PhotonsAux.", 
        "pt.eta.phi.m.DeltaE.Eratio.Loose.OQ.Reta.Rhad.Rhad1.Rphi.Tight.author.caloClusterLinks.e233.e237.e277.e2tsts1.emaxs1.emins1.etcone20ptCorrection.etcone30ptCorrection.etcone40ptCorrection.ethad.ethad1.f1.f3.fracs1.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone20ptCorrection.topoetcone30.topoetcone30ptCorrection.topoetcone40.topoetcone40ptCorrection.weta1.weta2.wtots1.DFCommonPhotonsIsEMLoose.DFCommonPhotonsIsEMTight.vertexLinks"); 
    m_event->setAuxItemList("TauJetsAux.",
        "pt.eta.phi.m.trackLinks.wideTrackLinks.otherTrackLinks.jetLink.vertexLink.secondaryVertexLink.hadronicPFOLinks.shotPFOLinks.chargedPFOLinks.neutralPFOLinks.pi0PFOLinks.protoChargedPFOLinks.protoNeutralPFOLinks.protoPi0PFOLinks.charge.BDTEleScore.BDTJetScore.isTauFlags"); 
  }
  m_event->setAuxItemList("Kt4EMTopoEventShapeAux.", "Density");
  m_event->setAuxItemList("CombinedMuonTrackParticlesAux.",  "phi.numberOfSCTHoles.numberOfSCTDeadSensors.d0.z0.theta.numberOfTRTHits.qOverP.numberOfTRTOutliers.definingParametersCovMatrix.vz.numberOfPixelHits.numberOfPixelHoles.chiSquared.numberOfPixelDeadSensors.numberDoF.numberOfSCTHits");
  m_event->setAuxItemList("ExtrapolatedMuonTrackParticlesAux.", "phi.qOverP.theta.definingParametersCovMatrix");
  m_event->setAuxItemList("GSFTrackParticlesAux.", "phi.d0.definingParametersCovMatrix.numberOfInnermostPixelLayerHits.numberOfPixelDeadSensors.numberOfPixelHits.numberOfSCTDeadSensors.numberOfSCTHits.numberOfTRTHits.numberOfTRTOutliers.qOverP.theta.vz.z0.eProbabilityHT.expectInnermostPixelLayerHit.expectNextToInnermostPixelLayerHit.numberOfInnermostPixelLayerOutliers.numberOfNextToInnermostPixelLayerHits.numberOfNextToInnermostPixelLayerOutliers.numberOfTRTHighThresholdHits.numberOfTRTHighThresholdOutliers.numberOfTRTXenonHits.parameterPX.parameterPY.parameterPZ.parameterPosition.parameterX"); 
  m_event->setAuxItemList("GSFConversionVerticesAux.", "trackParticleLinks.px.py.pz.pt2.x.y.z.pt1");
  m_event->setAuxItemList("InDetTrackParticlesAux.",
      "phi.d0.definingParametersCovMatrix.numberOfPixelDeadSensors.numberOfPixelHits.numberOfPixelHoles.numberOfSCTDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfTRTHits.numberOfTRTOutliers.qOverP.theta.vz.z0");
  m_event->setAuxItemList("egammaClustersAux.", "PHICALOFRAME.calE.calEta.calPhi.e_sampl.eta_sampl.ETACALOFRAME");
  m_event->setAuxItemList("PrimaryVerticesAux.", "trackParticleLinks.z.vertexType");
  m_event->setAuxItemList("TruthEventsAux.", "truthParticleLinks.PDGID1.PDGID2.Q.X1.X2");
  m_event->setAuxItemList("MET_TruthAux.", "name.mpx.mpy.sumet.source");
  m_event->setAuxItemList("AntiKt4TruthJetsAux.", "pt.eta.phi.m.constituentLinks.constituentWeights.ConstituentScale.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.InputType.AlgorithmType.SizeParameter.Width.Mu12.PartonTruthLabelID.PlanarFlow.Sphericity.Split12.Split23.Split34.Tau1.Tau1_wta.Tau2.Tau2_wta.Tau3.Tau3_wta.ThrustMaj.ThrustMin.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T.WidthPhi.ZCut12.ZCut23.ZCut34.Angularity.Aplanarity.ConeExclBHadronsFinal.ConeExclCHadronsFinal.ConeExclTausFinal.ConeTruthLabelID.Dip12.Dip13.Dip23.DipExcl12.ECF1.ECF2.ECF3.FoxWolfram0.FoxWolfram1.FoxWolfram2.FoxWolfram3.FoxWolfram4.GhostBHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostBHadronsInitial.GhostBHadronsInitialCount.GhostBHadronsInitialPt.GhostBQuarksFinal.GhostBQuarksFinalCount.GhostBQuarksFinalPt.GhostCHadronsFinal.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostCHadronsInitial.GhostCHadronsInitialCount.GhostCHadronsInitialPt.GhostCQuarksFinal.GhostCQuarksFinalCount.GhostCQuarksFinalPt.GhostHBosons.GhostHBosonsCount.GhostHBosonsPt.GhostPartons.GhostPartonsCount.GhostPartonsPt.GhostTQuarksFinal.GhostTQuarksFinalCount.GhostTQuarksFinalPt.GhostTausFinal.GhostTausFinalCount.GhostTausFinalPt.GhostWBosons.GhostWBosonsCount.GhostWBosonsPt.GhostZBosons.GhostZBosonsCount.GhostZBosonsPt.HadronConeExclTruthLabelID.IsoDelta2SumPt.IsoDelta3SumPt.JetGhostArea.KtDR");
  m_event->setAuxItemList("METAssoc_AntiKt4EMTopoAux.", "jetLink.cale.calkey.calpx.calpy.calpz.calsumpt.isMisc.jettrke.jettrkpx.jettrkpy.jettrkpz.jettrksumpt.objectLinks.overlapIndices.overlapTypes.trke.trkkey.trkpx.trkpy.trkpz.trksumpt");
  m_event->setAuxItemList("MET_Core_AntiKt4EMTopoAux.", "name.mpx.mpy.source.sumet");
  m_event->setAuxItemList("xTrigDecisionAux.", "bgCode.tav.tap.tbp.lvl2PassedRaw.efPassedRaw.lvl2PassedThrough.efPassedThrough.lvl2Prescaled.efPrescaled.lvl2Resurrected.efResurrected");
  m_event->setAuxItemList("TrigNavigationAux.", "serialized");
  m_event->setAuxItemList("HLT_xAOD__MuonContainer_MuonEFInfoAux.", "pt.eta.phi");
  m_event->setAuxItemList("HLT_xAOD__ElectronContainer_egamma_ElectronsAux.", "pt.eta.phi");
  m_event->setAuxItemList("BTagging_AntiKt4EMTopo.", "MV2c20_discriminant");




  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  //----------------------------
  // Event information
  //--------------------------- 
  const xAOD::EventInfo* eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info collection in execute. Exiting." );
    return EL::StatusCode::FAILURE;
  }


  bool passUncalibMonojetCut = false;
  bool passRecoJetCuts = false;


  //------------
  // JETS
  //------------

  static std::string jetType = "AntiKt4EMTopoJets";

  /// full copy 
  // get jet container of interest
  const xAOD::JetContainer* m_jets(0);
  if ( !m_event->retrieve( m_jets, jetType ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Failed to retrieve Jet container. Exiting." );
    return EL::StatusCode::FAILURE;
  }


  // Create the new container
  xAOD::JetContainer* m_recoJet = new xAOD::JetContainer(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Jet>

  /// shallow copy for jet calibration tool
  // create a shallow copy of the jets container for MET building
  std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > jet_shallowCopy = xAOD::shallowCopyContainer( *m_jets );
  xAOD::JetContainer* jetSC = jet_shallowCopy.first;

  // iterate over our shallow copy
  for (const auto& jets : *jetSC) { // C++11 shortcut
    //Info("execute()", "  original jet pt = %.2f GeV", jets->pt() * 0.001);

    // According to https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetEtmissRecommendationsMC15

    if (jets->pt() > 100000.) passUncalibMonojetCut = true;

    // JES calibration
    if ( !m_jetCalibration->applyCalibration(*jets).isSuccess() ){
      Error("execute()", "Failed to apply calibration to Jet objects. Exiting." );
      return EL::StatusCode::FAILURE;
      }
      
    //Info("execute()", "  calibrated jet pt = %.2f GeV", jets->pt() * 0.001);

    xAOD::Jet* newJet = new xAOD::Jet();
    newJet->makePrivateStore(*jets);
    m_recoJet->push_back( newJet );

  } // end for loop over shallow copied jets



  // Record the objects into the output xAOD:
  jet_shallowCopy.second->setShallowIO( false ); // true = shallow copy, false = deep copy
                                                                          // if true should have something like this line somewhere:
                                                                          // event->copy("AntiKt4EMTopoJets");
  ANA_CHECK(m_event->record( jet_shallowCopy.first, "ShallowCopiedJets" ));
  ANA_CHECK(m_event->record( jet_shallowCopy.second, "ShallowCopiedJetsAux." ));

  // Delete shallow copy containers
  // If you plan to write it out to an output xAOD you can give ownership to TEvent which will then handle deletion for you.
  //delete jet_shallowCopy.first;
  //delete jet_shallowCopy.second;


  // Sort recoJets
  if (m_recoJet->size() > 1) std::partial_sort(m_recoJet->begin(), m_recoJet->begin()+2, m_recoJet->end(), DescendingPt());
  //if (m_recoJet->size() > 1) std::sort(m_recoJet->begin(), m_recoJet->end(), DescendingPt());
  
  float mjj = 0;
  if (m_recoJet->size() > 1) {
    TLorentzVector jet1 = m_recoJet->at(0)->p4();
    TLorentzVector jet2 = m_recoJet->at(1)->p4();
    auto dijet = jet1 + jet2;
    mjj = dijet.M();
  }

  if ((m_recoJet->size() > 0 && m_recoJet->at(0)->pt() > m_monoJetPt) || (m_recoJet->size() > 1 && m_recoJet->at(0)->pt() > m_leadingJetPt && m_recoJet->at(1)->pt() > m_subleadingJetPt && mjj > m_Mjj)) passRecoJetCuts = true;

  // Delete copy containers
  delete m_recoJet;

  //bool acceptEvent = passUncalibMonojetCut || passRecoJetCuts;
  bool acceptEvent = passRecoJetCuts;
  if (!acceptEvent){
    return EL::StatusCode::SUCCESS; // go to next event
  }


  if (!m_isData) {
    ANA_CHECK(m_event->copy("TruthEvents"));
    ANA_CHECK(m_event->copy("AntiKt4TruthJets"));
    ANA_CHECK(m_event->copy("MET_Truth"));
    ANA_CHECK(m_event->copy("EXOT5TruthNeutrinos"));
    ANA_CHECK(m_event->copy("EXOT5TruthMuons"));
    ANA_CHECK(m_event->copy("EXOT5TruthElectrons"));
    ANA_CHECK(m_event->copy("TruthTaus"));
    ANA_CHECK(m_event->copy("TruthParticles"));
  }

  ANA_CHECK(m_event->copy("EventInfo"));
  ANA_CHECK(m_event->copy("PrimaryVertices"));
  ANA_CHECK(m_event->copy("Kt4EMTopoEventShape"));
  //ANA_CHECK(m_event->copy("AntiKt4EMTopoJets"));
  ANA_CHECK(m_event->copy("Muons"));
  ANA_CHECK(m_event->copy("Electrons"));
  ANA_CHECK(m_event->copy("Photons"));
  ANA_CHECK(m_event->copy("TauJets"));
  ANA_CHECK(m_event->copy("METAssoc_AntiKt4EMTopo"));
  ANA_CHECK(m_event->copy("MET_Core_AntiKt4EMTopo"));
  ANA_CHECK(m_event->copy("egammaClusters"));
  ANA_CHECK(m_event->copy("GSFTrackParticles"));
  ANA_CHECK(m_event->copy("GSFConversionVertices"));
  ANA_CHECK(m_event->copy("InDetTrackParticles"));
  ANA_CHECK(m_event->copy("CombinedMuonTrackParticles"));
  ANA_CHECK(m_event->copy("ExtrapolatedMuonTrackParticles"));
  ANA_CHECK(m_event->copy("xTrigDecision"));
  ANA_CHECK(m_event->copy("TrigNavigation"));
  ANA_CHECK(m_event->copy("TrigConfKeys"));
  ANA_CHECK(m_event->copy("HLT_xAOD__MuonContainer_MuonEFInfo"));
  ANA_CHECK(m_event->copy("HLT_xAOD__ElectronContainer_egamma_Electrons"));
  ANA_CHECK(m_event->copy("BTagging_AntiKt4EMTopo"));
  ANA_CHECK(m_event->copy("MET_Track"));

  m_event->fill();



  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  file_xAOD->cd();
  h_sumOfWeights->Write();
  ANA_CHECK(m_event->finishWritingTo( file_xAOD ));

  // File Meta Data Tool
  if(m_fileMetaDataTool){
    delete m_fileMetaDataTool;
    m_fileMetaDataTool = 0;
  }

  // Trigger Menu Meta Data Tool
  if(m_triggerMenuMetaDataTool){
    delete m_triggerMenuMetaDataTool;
    m_triggerMenuMetaDataTool = 0;
  }

  /// JES Calibration
  if(m_jetCalibration){
    delete m_jetCalibration;
    m_jetCalibration = 0;
  }


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode vbfZnunuSkim :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}
