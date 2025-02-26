dipho_variables=[
    "dipho_sumpt                  := diPhoton.sumPt",
    "dipho_mass                   := diPhoton.mass",
    "dipho_leadPt                 := diPhoton.leadingPhoton.pt",
    "dipho_leadEta                := diPhoton.leadingPhoton.eta",
    "dipho_leadPhi                := diPhoton.leadingPhoton.phi",
    "dipho_leadE                  := diPhoton.leadingPhoton.energy",
    "dipho_lead_sieie             := diPhoton.leadingPhoton.sigmaIetaIeta",
    "dipho_lead_hoe               := diPhoton.leadingPhoton.hadronicOverEm",
    "dipho_lead_sigmaEoE          := diPhoton.leadingPhoton.sigEOverE",
    "dipho_lead_ptoM              := diPhoton.leadingPhoton.pt/diPhoton.mass",
    "dipho_leadR9                 := diPhoton.leadingPhoton.full5x5_r9",
    "dipho_leadIDMVA              := diPhoton.leadingView.phoIdMvaWrtChosenVtx",
    "dipho_lead_elveto            := diPhoton.leadingPhoton.passElectronVeto",
    "dipho_leadhasPixelSeed       := diPhoton.leadingPhoton.hasPixelSeed",
    "dipho_lead_prompt            := diPhoton.leadingPhoton.genMatchType",
    "dipho_subleadPt              := diPhoton.subLeadingPhoton.pt",
    "dipho_subleadEta             := diPhoton.subLeadingPhoton.eta",
    "dipho_subleadPhi             := diPhoton.subLeadingPhoton.phi",
    "dipho_subleadE               := diPhoton.subLeadingPhoton.energy",
    "dipho_sublead_sieie          := diPhoton.subLeadingPhoton.sigmaIetaIeta",
    "dipho_sublead_hoe            := diPhoton.subLeadingPhoton.hadronicOverEm",
    "dipho_sublead_sigmaEoE       := diPhoton.subLeadingPhoton.sigEOverE",
    "dipho_sublead_ptoM           := diPhoton.subLeadingPhoton.pt/diPhoton.mass",
    "dipho_subleadR9              := diPhoton.subLeadingPhoton.full5x5_r9",
    "dipho_subleadIDMVA           := diPhoton.subLeadingView.phoIdMvaWrtChosenVtx",
    "dipho_sublead_elveto         := diPhoton.subLeadingPhoton.passElectronVeto",
    "dipho_subleadhasPixelSeed    := diPhoton.subLeadingPhoton.hasPixelSeed",
    "dipho_sublead_prompt         := diPhoton.subLeadingPhoton.genMatchType",
    "dipho_mva                    := diPhotonMVA.result",
    "dipho_PToM                   := diPhoton.pt/diPhoton.mass",
    "sigmarv                      := diPhotonMVA.sigmarv",
    "sigmarvDecorr                := diPhotonMVA.decorrSigmarv",
    "sigmawv                      := diPhotonMVA.sigmawv",
    "CosPhi                       := diPhotonMVA.CosPhi",
    "vtxprob                      := diPhotonMVA.vtxprob",
    "pt                           := diPhoton.pt",
    "leadSCeta                    := diPhoton.leadingPhoton.superCluster.eta",
    "subleadSCeta                 := diPhoton.subLeadingPhoton.superCluster.eta"
]

ele_variables=[
    "n_ele        := electrons.size",
    "ele1_Charge  := ?(electrons.size>0)? electrons.at(0).charge           : 0",
    "ele1_Pt      := ?(electrons.size>0)? electrons.at(0).pt               : -999",
    "ele1_Eta     := ?(electrons.size>0)? electrons.at(0).eta              : -999",
    "ele1_Phi     := ?(electrons.size>0)? electrons.at(0).phi              : -999",
    "ele1_E       := ?(electrons.size>0)? electrons.at(0).energy           : -999",
    "ele1_EtaSC   := ?(electrons.size>0)? electrons.at(0).superCluster.eta : -999",
    "ele1_PhiSC   := ?(electrons.size>0)? electrons.at(0).superCluster.phi : -999",
    "ele2_Charge  := ?(electrons.size>1)? electrons.at(1).charge           : 0",
    "ele2_Pt      := ?(electrons.size>1)? electrons.at(1).pt               : -999",
    "ele2_Eta     := ?(electrons.size>1)? electrons.at(1).eta              : -999",
    "ele2_Phi     := ?(electrons.size>1)? electrons.at(1).phi              : -999",
    "ele2_E       := ?(electrons.size>1)? electrons.at(1).energy           : -999",
    "ele2_EtaSC   := ?(electrons.size>1)? electrons.at(1).superCluster.eta : -999",
    "ele2_PhiSC   := ?(electrons.size>1)? electrons.at(1).superCluster.phi : -999"
]

mu_variables=[
    "n_mu       := muons.size",
    "mu1_Charge := ?(muons.size>0)? muons.at(0).charge : 0",
    "mu1_Pt     := ?(muons.size>0)? muons.at(0).pt     : -999",
    "mu1_Eta    := ?(muons.size>0)? muons.at(0).eta    : -999",
    "mu1_Phi    := ?(muons.size>0)? muons.at(0).phi    : -999",
    "mu1_E      := ?(muons.size>0)? muons.at(0).energy : -999",
    "mu2_Charge := ?(muons.size>1)? muons.at(1).charge : 0",
    "mu2_Pt     := ?(muons.size>1)? muons.at(1).pt     : -999",
    "mu2_Eta    := ?(muons.size>1)? muons.at(1).eta    : -999",
    "mu2_Phi    := ?(muons.size>1)? muons.at(1).phi    : -999",
    "mu2_E      := ?(muons.size>1)? muons.at(1).energy : -999"
]

jet_variables=[
    "n_jets      := jets.size"
]

njet = 15
for i in range(njet):
    jet_variables.append( "jet%d_Pt  := ?(jets.size>%d)? jets.at(%d).pt      : -999" % (i+1, i, i))
    jet_variables.append( "jet%d_Eta := ?(jets.size>%d)? jets.at(%d).eta     : -999" % (i+1, i, i))
    jet_variables.append( "jet%d_Phi := ?(jets.size>%d)? jets.at(%d).phi     : -999" % (i+1, i, i))
    jet_variables.append( "jet%d_E   := ?(jets.size>%d)? jets.at(%d).energy  : -999" % (i+1, i, i))
    jet_variables.append( "jet%d_deepbtag   := ?(jets.size>%d)? jets.at(%d).bDiscriminator('pfDeepCSVJetTags:probb') + jets.at(%d).bDiscriminator('pfDeepCSVJetTags:probbb') : -999" % (i+1, i, i, i) )

met_variables=[
    "met_Pt      := met.getCorPt()",
    "met_Phi     := met.getCorPhi()",
    "met_sumEt   := met.sumEt()",
    "met_Sig     := met.mEtSig()",
    "met_RealSig := met.significance()"
]

extra_variables=[
    "minDeltaPhiJetMet := MinDeltaPhiJetMet()",
    "maxJetDeepCSV     := MaxJetDeepCSV()"
]

gen_variables=[
    "hasZ    := associatedZ()",
    "hasW    := associatedW()",
    "VhasNeu := VhasNeutrinos()",
    "VhasLep := VhasLeptons()",
    "VhasHad := VhasHadrons()",
    "Vpt     := Vpt()"
]


stxs_vars = [
                'vh_mva :=  VHmva()'
]

whAnomVars = [
    'WHiggs0MToGG_MVA := WHiggs0MToGG_mva()',
    'WHiggs0PHToGG_MVA := WHiggs0PHToGG_mva()',
    'WHiggs0L1ToGG_MVA := WHiggs0L1ToGG_mva()',
]
zhAnomVars = [
    'ZHiggs0MToGG_MVA := ZHiggs0MToGG_mva()',
    'ZHiggs0PHToGG_MVA := ZHiggs0PHToGG_mva()',
    'ZHiggs0L1ToGG_MVA := ZHiggs0L1ToGG_mva()',
]

VHMET_vars = [
    'vhmet_mva    :=VHMetMVA()',
    'ACMVAfa3d0ZH :=ACMVAfa3d0ZH()',
    'ACMVAfa2d0ZH :=ACMVAfa2d0ZH()',
    'ACMVAfL1d0ZH :=ACMVAfL1d0ZH()'
]

wh_anom_dumper_vars=  stxs_vars + whAnomVars
zh_anom_dumper_vars=  stxs_vars + zhAnomVars