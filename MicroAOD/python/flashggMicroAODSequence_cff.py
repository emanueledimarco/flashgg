import FWCore.ParameterSet.Config as cms
from flashgg.MicroAOD.flashggTkVtxMap_cfi import flashggVertexMapUnique,flashggVertexMapNonUnique,flashggVertexMapForCHS,flashggVertexMapForPUPPI

from flashgg.MicroAOD.flashggPhotons_cfi import flashggPhotons
from flashgg.MicroAOD.flashggRandomizedPhotonProducer_cff import flashggRandomizedPhotons
from flashgg.MicroAOD.flashggDiPhotons_cfi import flashggDiPhotons

from flashgg.MicroAOD.flashggJets_cfi import flashggFinalJets,flashggFinalPuppiJets
from flashgg.MicroAOD.flashggElectrons_cfi import flashggElectrons
from flashgg.MicroAOD.flashggMuons_cfi import flashggMuons
from flashgg.MicroAOD.flashggSecondaryVertices_cfi import flashggSVFlavourTagInfos, flashggSVs

from flashgg.MicroAOD.flashggLeptonSelectors_cff import flashggSelectedMuons,flashggSelectedElectrons
from flashgg.MicroAOD.flashggMicroAODGenSequence_cff import *

eventCount = cms.EDProducer("EventCountProducer")
weightsCount = cms.EDProducer("WeightsCountProducer",
                              generator=cms.InputTag("generator"),
                              pileupInfo=cms.InputTag("slimmedAddPileupInfo"),
                              doObsPileup=cms.untracked.bool(True),
                              minObsPileup=cms.double(-0.5),
                              maxObsPileup=cms.double(100.5),
                              nbinsObsPileup=cms.int32(101),
                              doTruePileup=cms.untracked.bool(True),
                              minTruePileup=cms.double(-0.5),
                              maxTruePileup=cms.double(100.5),
                              nbinsTruePileup=cms.int32(101),
                              )

flashggDiPhotonFilterSequence = cms.Sequence()
flashggMuonFilterSequence = cms.Sequence()

flashggMicroAODSequence = cms.Sequence( eventCount+weightsCount
                                       +flashggVertexMapUnique+flashggVertexMapNonUnique
                                       +flashggMicroAODGenSequence
                                       +flashggPhotons * flashggRandomizedPhotons * flashggDiPhotons
                                       +flashggDiPhotonFilterSequence
                                       +flashggElectrons*flashggSelectedElectrons
                                       +flashggMuons*flashggSelectedMuons
                                       +flashggMuonFilterSequence
                                       +flashggVertexMapForCHS*flashggFinalJets
                                       +flashggVertexMapForPUPPI*flashggFinalPuppiJets
                                       +flashggSVFlavourTagInfos*flashggSVs
                                        )
