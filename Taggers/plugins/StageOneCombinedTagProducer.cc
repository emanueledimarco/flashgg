#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDMException.h"

#include "flashgg/DataFormats/interface/DiPhotonCandidate.h"
#include "flashgg/DataFormats/interface/StageOneCombinedTag.h"
#include "flashgg/DataFormats/interface/VBFMVAResult.h"
#include "flashgg/DataFormats/interface/VHhadMVAResult.h"
#include "flashgg/DataFormats/interface/VHhadACDNNResult.h"
#include "flashgg/DataFormats/interface/GluGluHMVAResult.h"
#include "DataFormats/Common/interface/RefToPtr.h"

#include "flashgg/DataFormats/interface/PDFWeightObject.h"

#include <TLorentzVector.h>
#include <vector>
#include <algorithm>
#include "TH2D.h"
#include "TFile.h"

using namespace std;
using namespace edm;

namespace flashgg {

    class StageOneCombinedTagProducer : public EDProducer
    {

    public:
        StageOneCombinedTagProducer( const ParameterSet & );

    private:
        void produce( Event &, const EventSetup & ) override;
        int chooseCategory( float );
        int chooseVBFCategory( float pbsm, float pbkg, float d0m );
        int chooseVHhadCategory( float dnnvh_bkg, float dnnvh_bsm );
        int computeStage1Kinematics( const StageOneCombinedTag );
        float setEffectivePtH( const GluGluHMVAResult, float );
        int setEffectiveNj( const GluGluHMVAResult, int );

        bool useMultiClass_;
        std::vector<double> rawDiphoBounds_, rawDijetBounds_, rawGghBounds_, rawVhHadBounds_, rawVbfpBSMBounds_, rawVbfpBKGBounds_, rawVbfpD0MBounds_, rawVhhaddnnBKGBounds_, rawVhhaddnnBSMBounds_;
        std::map<std::string, float> diphoBounds_, dijetBounds_, gghBounds_, vhHadBounds_;
        void constructBounds();

        std::vector<double> nloKfacVbfEdges_;
        std::vector<double> nloKfacVbfVals_;
        float getVBFKFactor(const StageOneCombinedTag tag_obj, std::vector<double> edges, std::vector<double> vals);

        edm::FileInPath nloKfacVHHadFile_;
        TH2D* nloKfacVHHad_;
        float getVHHadKFactor(const StageOneCombinedTag tag_obj);
        
        EDGetTokenT<View<DiPhotonCandidate> >      diPhotonToken_;
        EDGetTokenT<View<VBFMVAResult> >           vbfMvaResultToken_;
        EDGetTokenT<View<VHhadMVAResult> >           vhHadMvaResultToken_;
        EDGetTokenT<View<VHhadACDNNResult> >           vhHadACDNNResultToken_;
        EDGetTokenT<View<GluGluHMVAResult> >           gghMvaResultToken_;
        EDGetTokenT<View<DiPhotonMVAResult> >      mvaResultToken_;
        edm::EDGetTokenT<vector<flashgg::PDFWeightObject> > WeightToken_;

        string systLabel_;

        std::vector<edm::EDGetTokenT<View<flashgg::Jet> > > tokenJets_;
        std::vector<edm::InputTag> inputTagJets_;
        typedef std::vector<edm::Handle<edm::View<flashgg::Jet> > > JetCollectionVector;

    };

    StageOneCombinedTagProducer::StageOneCombinedTagProducer( const ParameterSet &iConfig ) :
        diPhotonToken_( consumes<View<flashgg::DiPhotonCandidate> >( iConfig.getParameter<InputTag> ( "DiPhotonTag" ) ) ),
        vbfMvaResultToken_( consumes<View<flashgg::VBFMVAResult> >( iConfig.getParameter<InputTag> ( "VBFMVAResultTag" ) ) ),
        vhHadMvaResultToken_( consumes<View<flashgg::VHhadMVAResult> >( iConfig.getParameter<InputTag> ( "VHhadMVAResultTag" ) ) ),
        vhHadACDNNResultToken_( consumes<View<flashgg::VHhadACDNNResult> >( iConfig.getParameter<InputTag> ( "VHhadACDNNResultTag" ) ) ),
        gghMvaResultToken_( consumes<View<flashgg::GluGluHMVAResult> >( iConfig.getParameter<InputTag> ( "GluGluHMVAResultTag" ) ) ),
        mvaResultToken_( consumes<View<flashgg::DiPhotonMVAResult> >( iConfig.getParameter<InputTag> ( "MVAResultTag" ) ) ),
        WeightToken_( consumes<vector<flashgg::PDFWeightObject> >( iConfig.getUntrackedParameter<InputTag>( "WeightTag", InputTag( "flashggPDFWeightObject" ) ) ) ),
        systLabel_   ( iConfig.getParameter<string> ( "SystLabel" ) ),
        inputTagJets_ ( iConfig.getParameter<std::vector<edm::InputTag> >( "inputTagJets" ) )
    {
        for (unsigned i = 0 ; i < inputTagJets_.size() ; i++) {
            auto token = consumes<View<flashgg::Jet> >(inputTagJets_[i]);
            tokenJets_.push_back(token);
        }

        useMultiClass_  = iConfig.getParameter<bool> ("useMultiClass");
        rawDiphoBounds_ = iConfig.getParameter<std::vector<double> > ("rawDiphoBounds");
        rawDijetBounds_ = iConfig.getParameter<std::vector<double> > ("rawDijetBounds");
        rawVhHadBounds_ = iConfig.getParameter<std::vector<double> > ("rawVhHadBounds");
        rawGghBounds_   = iConfig.getParameter<std::vector<double> > ("rawGghBounds");
        rawVbfpBSMBounds_   = iConfig.getParameter<std::vector<double> > ("rawVbfpBSMBounds");
        rawVbfpBKGBounds_   = iConfig.getParameter<std::vector<double> > ("rawVbfpBKGBounds");
        rawVbfpD0MBounds_   = iConfig.getParameter<std::vector<double> > ("rawVbfpD0MBounds");
        rawVhhaddnnBKGBounds_ = iConfig.getParameter<std::vector<double> > ("rawVhhaddnnBKGBounds");
        rawVhhaddnnBSMBounds_ = iConfig.getParameter<std::vector<double> > ("rawVhhaddnnBSMBounds");
        
        nloKfacVbfEdges_ = iConfig.getParameter<std::vector<double> > ("nloKfacVbfEdges");
        nloKfacVbfVals_  = iConfig.getParameter<std::vector<double> > ("nloKfacVbfVals");

        // we are counting on ascending order - update this to give an error message or exception
        assert( is_sorted( rawVbfpBSMBounds_.begin(), rawVbfpBSMBounds_.end() ) );
        assert( is_sorted( rawVbfpBKGBounds_.begin(), rawVbfpBKGBounds_.end() ) );
        assert( is_sorted( rawVbfpD0MBounds_.begin(), rawVbfpD0MBounds_.end() ) );
        assert( is_sorted( nloKfacVbfEdges_.begin(), nloKfacVbfEdges_.end() ) );
        constructBounds();

        nloKfacVHHadFile_ = iConfig.getParameter<edm::FileInPath> ("nloKfacVHHadFile");
        TFile *f = TFile::Open(nloKfacVHHadFile_.fullPath().c_str());
        nloKfacVHHad_ = (TH2D*) f->Get("reweighting")->Clone();
        f->Close();
        delete f;
        
        produces<vector<DiPhotonTagBase> >();
    }

    void StageOneCombinedTagProducer::produce( Event &evt, const EventSetup & )
    {
        Handle<View<flashgg::DiPhotonCandidate> > diPhotons;
        evt.getByToken( diPhotonToken_, diPhotons );
        
        Handle<View<flashgg::DiPhotonMVAResult> > mvaResults;
        evt.getByToken( mvaResultToken_, mvaResults );
        
        Handle<View<flashgg::VBFMVAResult> > vbfMvaResults;
        evt.getByToken( vbfMvaResultToken_, vbfMvaResults );

        Handle<View<flashgg::VHhadMVAResult> > vhHadMvaResults;
        evt.getByToken( vhHadMvaResultToken_, vhHadMvaResults );

        Handle<View<flashgg::VHhadACDNNResult> > vhHadACDNNResults;
        evt.getByToken( vhHadACDNNResultToken_, vhHadACDNNResults );

        Handle<View<flashgg::GluGluHMVAResult> > gghMvaResults;
        evt.getByToken( gghMvaResultToken_, gghMvaResults );

        JetCollectionVector Jets( inputTagJets_.size() );
        for( size_t j = 0; j < inputTagJets_.size(); ++j ) {
            evt.getByToken( tokenJets_[j], Jets[j] );
        }

        std::unique_ptr<vector<DiPhotonTagBase> > stage1tags( new vector<DiPhotonTagBase> );

        // We are relying on corresponding sets - update this to give an error/exception
        assert( diPhotons->size() == vbfMvaResults->size() ); 
        assert( diPhotons->size() == vhHadMvaResults->size() );
        assert( diPhotons->size() == vhHadACDNNResults->size() ); 
        assert( diPhotons->size() == mvaResults->size() );

        for( unsigned int candIndex = 0; candIndex < diPhotons->size() ; candIndex++ ) {
            edm::Ptr<flashgg::VBFMVAResult>           vbf_mvares      = vbfMvaResults->ptrAt( candIndex );
            edm::Ptr<flashgg::VHhadMVAResult>         vhHad_mvares    = vhHadMvaResults->ptrAt( candIndex );
            edm::Ptr<flashgg::VHhadACDNNResult>       vhHadAC_dnnres  = vhHadACDNNResults->ptrAt( candIndex );
            edm::Ptr<flashgg::GluGluHMVAResult>       ggh_mvares      = gghMvaResults->ptrAt( candIndex );
            edm::Ptr<flashgg::DiPhotonMVAResult>      mvares          = mvaResults->ptrAt( candIndex );
            edm::Ptr<flashgg::DiPhotonCandidate>      dipho           = diPhotons->ptrAt( candIndex );
            
            StageOneCombinedTag stage1tag_obj( dipho, mvares, vbf_mvares, vhHad_mvares, vhHadAC_dnnres, ggh_mvares );
            stage1tag_obj.setDiPhotonIndex( candIndex );
            stage1tag_obj.setSystLabel( systLabel_ );
            stage1tag_obj.includeWeights( *dipho );

            if ( stage1tag_obj.VBFMVA().dijet_Mjj > 0. ) {

                // We don't want to include all the jet weights because btag weights are not relevant
                // So we do some tricky surgery instead, to get only the UnmatchedPUWeight
                float tagcorig = stage1tag_obj.centralWeight();
                float j1corig = stage1tag_obj.VBFMVA().leadJet_ptr->centralWeight();
                float j2corig = stage1tag_obj.VBFMVA().subleadJet_ptr->centralWeight();
                float j1cadjust = stage1tag_obj.VBFMVA().leadJet_ptr->weight("UnmatchedPUWeightCentral"); // stored without scaling to overall central
                float j2cadjust = stage1tag_obj.VBFMVA().subleadJet_ptr->weight("UnmatchedPUWeightCentral"); // stored without scaling to overall central
                float j1upadjust = 1.;
                float j2upadjust = 1.;
                float j1downadjust = 1.;
                float j2downadjust = 1.;
                float nlovbfkfac = getVBFKFactor(stage1tag_obj, nloKfacVbfEdges_, nloKfacVbfVals_);
                float nlovhhadkfac = getVHHadKFactor(stage1tag_obj);

                if (stage1tag_obj.VBFMVA().leadJet_ptr->hasWeight("UnmatchedPUWeightUp01sigma") ) {
                    j1upadjust = stage1tag_obj.VBFMVA().leadJet_ptr->weight("UnmatchedPUWeightUp01sigma")  / j1corig;
                    j1downadjust = stage1tag_obj.VBFMVA().leadJet_ptr->weight("UnmatchedPUWeightDown01sigma") / j1corig;
                }
                if (stage1tag_obj.VBFMVA().leadJet_ptr->hasWeight("UnmatchedPUWeightUp01sigma") ) {
                    j2upadjust = stage1tag_obj.VBFMVA().subleadJet_ptr->weight("UnmatchedPUWeightUp01sigma") / j2corig;
                    j2downadjust = stage1tag_obj.VBFMVA().subleadJet_ptr->weight("UnmatchedPUWeightDown01sigma") / j2corig;
                }
                if (false && systLabel_ == "") {
                    std::cout << "tagcorig j1cadjust j2cadjust j1upadjust j2upadjust j1downadjust j2downadjust ";
                    std::cout << tagcorig << " " << j1cadjust << " "<< j2cadjust << " "<< j1upadjust << " "<< j2upadjust << " "<< j1downadjust << " "<< j2downadjust << std::endl;
                }
                for (auto it = stage1tag_obj.weightListBegin() ; it != stage1tag_obj.weightListEnd(); it++) {
                    stage1tag_obj.setWeight(*it,stage1tag_obj.weight(*it) * j1cadjust * j2cadjust); 
                }
                if (stage1tag_obj.VBFMVA().leadJet_ptr->hasWeight("UnmatchedPUWeightUp01sigma") ) {
                    stage1tag_obj.setWeight("UnmatchedPUWeightUp01sigma", stage1tag_obj.centralWeight() * j1upadjust * j2upadjust );
                    stage1tag_obj.setWeight("UnmatchedPUWeightDown01sigma", stage1tag_obj.centralWeight() * j1downadjust * j2downadjust );
                }
                if (false && systLabel_ == "") {
                    for (auto it = stage1tag_obj.weightListBegin() ; it != stage1tag_obj.weightListEnd(); it++) {
                        std::cout << "SCZ Weight Debug " << *it << " " << stage1tag_obj.weight(*it) << std::endl;
                        
                    }                    
                }
                if (false && systLabel_ == "") {
                    std::cout << "NLO VBF kfactor = " << nlovbfkfac << std::endl;
                    std::cout << "NLO VH kfactor = " << nlovhhadkfac << std::endl;
                }
                stage1tag_obj.setWeight("vbfNLOweight", nlovbfkfac );
                stage1tag_obj.setWeight("vhhadNLOweight", nlovhhadkfac );
            }
            
            int chosenTag = computeStage1Kinematics( stage1tag_obj ); // choose category
            stage1tag_obj.setStage1recoTag( chosenTag ); // set the category in the object

            // saving the collection
            stage1tags->push_back( stage1tag_obj );
        }

        evt.put( std::move( stage1tags ) );
    }

    int StageOneCombinedTagProducer::chooseVBFCategory( float pbsm, float pbkg, float d0m ) {
        // should return 0 if mva above all the numbers, 1 if below the first, ..., boundaries.size()-N if below the Nth, ...
        if (pbsm<-100 || pbkg<-100 || d0m<-100) return -1;

        int n,m,o;
        for( n = 0 ; n < (int)rawVbfpBSMBounds_.size() ; n++ ) {
            if( (double)pbsm > rawVbfpBSMBounds_[rawVbfpBSMBounds_.size() - n - 1] ) break;
        }
        for( m = 0 ; m < (int)rawVbfpBKGBounds_.size() ; m++ ) {
            if( (double)pbkg > rawVbfpBKGBounds_[rawVbfpBKGBounds_.size() - m - 1] ) break;
        }
        for( o = 0 ; o < (int)rawVbfpD0MBounds_.size() ; o++ ) {
            if( (double)d0m > rawVbfpD0MBounds_[rawVbfpD0MBounds_.size() - o - 1] ) break;
        }

        int CAT = (int)((rawVbfpD0MBounds_.size()+1) * (rawVbfpBSMBounds_.size()+1) * m +
                        (rawVbfpBSMBounds_.size()+1) * o +
                        n);

        // std::cout << "pbkg = " << pbkg << "\td0m = " << d0m << "\tpbsm = " << pbsm
        //           << "m = " << m << "\to = " << o << "\tn = " << n 
        //           << " CAT = " << CAT << std::endl;

        return CAT;
    }

    int StageOneCombinedTagProducer::chooseVHhadCategory( float dnnvh_bkg, float dnnvh_bsm ) {
        //0 = Bkg, 1 = SM1, 2 = SM2, 3 = SM3, 4 = BSM1, 5 = BSM2
        int cat = 0;

        if( (double)dnnvh_bkg < 0.0 || (double)dnnvh_bsm < 0.0 ){
            cat = -1;
        } else if( (double)dnnvh_bkg < rawVhhaddnnBKGBounds_[0] && (double)dnnvh_bsm < rawVhhaddnnBSMBounds_[0] ){
            cat = 1;
        } else if( (double)dnnvh_bkg > rawVhhaddnnBKGBounds_[0] && (double)dnnvh_bkg < rawVhhaddnnBKGBounds_[1] && (double)dnnvh_bsm < rawVhhaddnnBSMBounds_[1] ){ 
            cat = 2;
        } else if( (double)dnnvh_bkg > rawVhhaddnnBKGBounds_[1] && (double)dnnvh_bkg < rawVhhaddnnBKGBounds_[2] && (double)dnnvh_bsm < rawVhhaddnnBSMBounds_[2] ){ 
            cat = 3;
        } else if( (double)dnnvh_bkg < rawVhhaddnnBKGBounds_[3] && (double)dnnvh_bsm > rawVhhaddnnBSMBounds_[3] ){ 
            cat = 4;
        } else if( (double)dnnvh_bkg < rawVhhaddnnBKGBounds_[4] && (double)dnnvh_bsm > rawVhhaddnnBSMBounds_[4] && ((double)dnnvh_bkg > rawVhhaddnnBKGBounds_[3] || (double)dnnvh_bsm < rawVhhaddnnBSMBounds_[3]) ){ 
            cat = 5;
        }

        return cat;
    }

    int StageOneCombinedTagProducer::computeStage1Kinematics( const StageOneCombinedTag tag_obj ) 
    {
        int chosenTag_ = DiPhotonTagBase::stage1recoTag::LOGICERROR;
        float ptH = tag_obj.diPhoton()->pt();
        unsigned int nJ = 0;
        float mjj = 0.;
        //float ptHjj = 0.; // used in the STXS analysis (substituted by VBF AC categorization)
        float mvaScore = tag_obj.diPhotonMVA().transformedMvaValue(); // maps output score from TMVA back to XGBoost original
        float dijetScore = tag_obj.VBFMVA().prob_VBF_value();
        float gghScore = tag_obj.VBFMVA().prob_ggH_value();
        //float vhHadScore = tag_obj.VHhadMVA().transformedMvaValue( tag_obj.VHhadMVA().VHhadMVAValue() );
        float leadMvaScore = tag_obj.diPhotonMVA().leadmva;
        float subleadMvaScore = tag_obj.diPhotonMVA().subleadmva;
        float leadPToM = tag_obj.diPhotonMVA().leadptom;
        float subleadPToM = tag_obj.diPhotonMVA().subleadptom;
        float dnn_pbsm = tag_obj.VBFMVA().dnnprob_bsm_value();
        float dnn_pbkg = tag_obj.VBFMVA().dnnprob_bkg_value();
        float d0m = tag_obj.VBFMVA().mela_D0minus_value();
        int vbfcat = chooseVBFCategory(dnn_pbsm, dnn_pbkg, d0m);
        float vhhaddnn_bkg = tag_obj.VHhadACDNN().dnnvh_bkg_value();
        float vhhaddnn_bsm = tag_obj.VHhadACDNN().dnnvh_bsm_value();
        int vhhadcat = chooseVHhadCategory(vhhaddnn_bkg, vhhaddnn_bsm);
        edm::Ptr<flashgg::Jet> j0 = tag_obj.VBFMVA().leadJet_ptr; 
        edm::Ptr<flashgg::Jet> j1 = tag_obj.VBFMVA().subleadJet_ptr;
        
        if ( !j0.isNull() ) {
            if ( j0->pt() > 30. ) { nJ += 1; }
        }
        if ( !j1.isNull() ) {
            if ( j1->pt() > 30. ) { nJ += 1; }
        }

        if ( nJ >= 2 ) {
            mjj = ( j0->p4() + j1->p4() ).mass();
            //ptHjj = ( j0->p4() + j1->p4() + tag_obj.diPhoton()->p4() ).pt();
        }

        if ( useMultiClass_ && mjj < 350. ) {
            ptH = setEffectivePtH( tag_obj.GluGluHMVA(), ptH );
            nJ  = setEffectiveNj(  tag_obj.GluGluHMVA(), nJ );
        }

        // below assign the categories for ggH & VBF based on diphoton MVA score and dijet MVA score - boundaries taken from metaConditions
        if (nJ == 0) {
            if ( ptH > 650. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_GT650_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_GT650_Tag0;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 450. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_450_650_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_450_650_Tag0;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 300. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_300_450_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_300_450_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_PTH_300_450_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_300_450_Tag1;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 200. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_200_300_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_200_300_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_PTH_200_300_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_200_300_Tag1;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 10. ) {
                if (mvaScore > diphoBounds_["RECO_0J_PTH_GT10_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_0J_PTH_GT10_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_0J_PTH_GT10_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_0J_PTH_GT10_Tag1;
                }
                else if (mvaScore > diphoBounds_["RECO_0J_PTH_GT10_Tag2"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_0J_PTH_GT10_Tag2;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            }
            else {
                if (mvaScore > diphoBounds_["RECO_0J_PTH_0_10_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_0J_PTH_0_10_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_0J_PTH_0_10_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_0J_PTH_0_10_Tag1;
                }
                else if (mvaScore > diphoBounds_["RECO_0J_PTH_0_10_Tag2"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_0J_PTH_0_10_Tag2;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            }
        } else if ( nJ == 1 ) {
            if ( ptH > 650. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_GT650_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_GT650_Tag0;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 450. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_450_650_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_450_650_Tag0;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 300. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_300_450_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_300_450_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_PTH_300_450_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_300_450_Tag1;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 200. ) {
                if (mvaScore > diphoBounds_["RECO_PTH_200_300_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_200_300_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_PTH_200_300_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_200_300_Tag1;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 120. ) {
                if (mvaScore > diphoBounds_["RECO_1J_PTH_120_200_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_120_200_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_1J_PTH_120_200_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_120_200_Tag1;
                }
                else if (mvaScore > diphoBounds_["RECO_1J_PTH_120_200_Tag2"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_120_200_Tag2;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else if ( ptH > 60. ) {
                if (mvaScore > diphoBounds_["RECO_1J_PTH_60_120_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_60_120_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_1J_PTH_60_120_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_60_120_Tag1;
                }
                else if (mvaScore > diphoBounds_["RECO_1J_PTH_60_120_Tag2"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_60_120_Tag2;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            } else {
                if (mvaScore > diphoBounds_["RECO_1J_PTH_0_60_Tag0"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_0_60_Tag0;
                }
                else if (mvaScore > diphoBounds_["RECO_1J_PTH_0_60_Tag1"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_0_60_Tag1;
                }
                else if (mvaScore > diphoBounds_["RECO_1J_PTH_0_60_Tag2"]) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_1J_PTH_0_60_Tag2;
                }
                else { 
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                }
            }
        } else { // 2 jets
            bool reProcess = false;
            if ( mjj > 350. && j0->p4().pt() > 40. && j1->p4().pt() > 30. && leadMvaScore > -0.2 && subleadMvaScore > -0.2 ) { //cuts optimised using data-driven dijet BDT plus new diphoton BDT
                if ( vbfcat == 1 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACGGH_Tag0;
                }
                else if ( vbfcat == 3 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACGGH_Tag1;
                }
                else if ( vbfcat == 5 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVBFSM_Tag0;
                }
                else if ( vbfcat == 6 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVBFBSM_Tag0;
                }
                else if ( vbfcat == 7 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVBFBSM_Tag1;
                }
                else {
                    reProcess = true;
                }
            }
            else if ( mjj > 0. && mjj < 250. && j0->p4().pt() > 30. && j1->p4().pt() > 30. && abs(j0->p4().eta()) < 2.4 && abs(j1->p4().eta()) < 2.4 && leadMvaScore > 0.0 && subleadMvaScore > 0.0 ) { //cuts for VH hadronic
                if ( vhhadcat == 1 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVHHADSM_Tag0;
                }
                else if ( vhhadcat == 2 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVHHADSM_Tag1;
                }
                else if ( vhhadcat == 3 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVHHADSM_Tag2;
                }
                else if ( vhhadcat == 4 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVHHADBSM_Tag0;
                }
                else if ( vhhadcat == 5 ) {
                    chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFTOPO_ACVHHADBSM_Tag1;
                }
                else { 
                    reProcess = true;
                }
            }
            else {
               reProcess = true;
            }
            if ( reProcess ) {
                if ( mjj > 350. && j0->p4().pt() > 40. && j1->p4().pt() > 30. && leadMvaScore > -0.2 && subleadMvaScore > -0.2 ) { //cuts optimised using data-driven dijet BDT plus new diphoton BDT
                    if ( dijetScore < dijetBounds_["RECO_VBFLIKEGGH_Tag0"] && gghScore > gghBounds_["RECO_VBFLIKEGGH_Tag0"]  && mvaScore > diphoBounds_["RECO_VBFLIKEGGH_Tag0"]) {
                            chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFLIKEGGH_Tag0;
                    } else if ( dijetScore < dijetBounds_["RECO_VBFLIKEGGH_Tag1"] && gghScore > gghBounds_["RECO_VBFLIKEGGH_Tag1"]  && mvaScore > diphoBounds_["RECO_VBFLIKEGGH_Tag1"]) {
                            chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_VBFLIKEGGH_Tag1;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else if ( ptH > 650. ) {
                    if (mvaScore > diphoBounds_["RECO_PTH_GT650_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_GT650_Tag0;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else if ( ptH > 450. ) {
                    if (mvaScore > diphoBounds_["RECO_PTH_450_650_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_450_650_Tag0;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else if ( ptH > 300. ) {
                    if (mvaScore > diphoBounds_["RECO_PTH_300_450_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_300_450_Tag0;
                    }
                    else if (mvaScore > diphoBounds_["RECO_PTH_300_450_Tag1"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_300_450_Tag1;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else if ( ptH > 200. ) {
                    if (mvaScore > diphoBounds_["RECO_PTH_200_300_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_200_300_Tag0;
                    }
                    else if (mvaScore > diphoBounds_["RECO_PTH_200_300_Tag1"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_PTH_200_300_Tag1;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else if ( ptH > 120. ) {
                    if (mvaScore > diphoBounds_["RECO_GE2J_PTH_120_200_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_120_200_Tag0;
                    }
                    else if (mvaScore > diphoBounds_["RECO_GE2J_PTH_120_200_Tag1"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_120_200_Tag1;
                    }
                    else if (mvaScore > diphoBounds_["RECO_GE2J_PTH_120_200_Tag2"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_120_200_Tag2;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else if ( ptH > 60. ) {
                    if (mvaScore > diphoBounds_["RECO_GE2J_PTH_60_120_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_60_120_Tag0;
                    }
                    else if (mvaScore > diphoBounds_["RECO_GE2J_PTH_60_120_Tag1"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_60_120_Tag1;
                    }
                    else if (mvaScore > diphoBounds_["RECO_GE2J_PTH_60_120_Tag2"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_60_120_Tag2;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                } else {
                    if (mvaScore > diphoBounds_["RECO_GE2J_PTH_0_60_Tag0"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_0_60_Tag0;
                    }
                    else if (mvaScore > diphoBounds_["RECO_GE2J_PTH_0_60_Tag1"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_0_60_Tag1;
                    }
                    else if (mvaScore > diphoBounds_["RECO_GE2J_PTH_0_60_Tag2"]) {
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::RECO_GE2J_PTH_0_60_Tag2;
                    }
                    else { 
                        chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
                    }
                }
            }
        }
        // reject events not passing the scaled pT cuts
        if ( leadPToM < 1./3. || subleadPToM < 1./4. ) {
            chosenTag_ = DiPhotonTagBase::stage1recoTag::NOTAG;
        }
        
        //finally pass back the chosen tag
        return chosenTag_;
    }

    float StageOneCombinedTagProducer::setEffectivePtH( const GluGluHMVAResult res, float ptH ) 
    {
        float val = -999.;
        float maxProb = -1.;
        if ( res.prob_0J_PTH_0_10() > maxProb ) {
            val = 5.;
            maxProb = res.prob_0J_PTH_0_10();
        }
        if ( res.prob_0J_PTH_GT10() > maxProb ) {
            val = 15.;
            maxProb = res.prob_0J_PTH_GT10();
        }
        if ( res.prob_1J_PTH_0_60() > maxProb ) {
            val = 30.;
            maxProb = res.prob_1J_PTH_0_60();
        }
        if ( res.prob_1J_PTH_60_120() > maxProb ) {
            val = 90.;
            maxProb = res.prob_1J_PTH_60_120();
        }
        if ( res.prob_1J_PTH_120_200() > maxProb ) {
            val = 160.;
            maxProb = res.prob_1J_PTH_120_200();
        }
        if ( res.prob_GE2J_MJJ_0_350_PTH_0_60() > maxProb ) {
            val = 30.;
            maxProb = res.prob_GE2J_MJJ_0_350_PTH_0_60();
        }
        if ( res.prob_GE2J_MJJ_0_350_PTH_60_120() > maxProb ) {
            val = 90.;
            maxProb = res.prob_GE2J_MJJ_0_350_PTH_60_120();
        }
        if ( res.prob_GE2J_MJJ_0_350_PTH_120_200() > maxProb ) {
            val = 160.;
            maxProb = res.prob_GE2J_MJJ_0_350_PTH_120_200();
        }
        if ( res.prob_PTH_GT200() > maxProb ) {
            if (ptH < 200.) { val = 250.; }
            else { val = ptH; }
            maxProb = res.prob_PTH_GT200();
        }
        return val;
    }

    int StageOneCombinedTagProducer::setEffectiveNj( const GluGluHMVAResult res, int nJ ) 
    {
        int val = -1;
        float maxProb = -1.;
        if ( res.prob_0J_PTH_0_10() > maxProb ) {
            val = 0;
            maxProb = res.prob_0J_PTH_0_10();
        }
        if ( res.prob_0J_PTH_GT10() > maxProb ) {
            val = 0;
            maxProb = res.prob_0J_PTH_GT10();
        }
        if ( res.prob_1J_PTH_0_60() > maxProb ) {
            val = 1;
            maxProb = res.prob_1J_PTH_0_60();
        }
        if ( res.prob_1J_PTH_60_120() > maxProb ) {
            val = 1;
            maxProb = res.prob_1J_PTH_60_120();
        }
        if ( res.prob_1J_PTH_120_200() > maxProb ) {
            val = 1;
            maxProb = res.prob_1J_PTH_120_200();
        }
        if ( res.prob_GE2J_MJJ_0_350_PTH_0_60() > maxProb ) {
            val = 2;
            maxProb = res.prob_GE2J_MJJ_0_350_PTH_0_60();
        }
        if ( res.prob_GE2J_MJJ_0_350_PTH_60_120() > maxProb ) {
            val = 2;
            maxProb = res.prob_GE2J_MJJ_0_350_PTH_60_120();
        }
        if ( res.prob_GE2J_MJJ_0_350_PTH_120_200() > maxProb ) {
            val = 2;
            maxProb = res.prob_GE2J_MJJ_0_350_PTH_120_200();
        }
        if ( res.prob_PTH_GT200() > maxProb ) {
            val = nJ;
            maxProb = res.prob_PTH_GT200();
        }
        return val;
    }

    void StageOneCombinedTagProducer::constructBounds() 
    {
        diphoBounds_["RECO_0J_PTH_0_10_Tag0"] = rawDiphoBounds_[0];
        diphoBounds_["RECO_0J_PTH_0_10_Tag1"] = rawDiphoBounds_[1];
        diphoBounds_["RECO_0J_PTH_0_10_Tag2"] = rawDiphoBounds_[2];
        diphoBounds_["RECO_0J_PTH_GT10_Tag0"] = rawDiphoBounds_[3];
        diphoBounds_["RECO_0J_PTH_GT10_Tag1"] = rawDiphoBounds_[4];
        diphoBounds_["RECO_0J_PTH_GT10_Tag2"] = rawDiphoBounds_[5];
        diphoBounds_["RECO_1J_PTH_0_60_Tag0"] = rawDiphoBounds_[6];
        diphoBounds_["RECO_1J_PTH_0_60_Tag1"] = rawDiphoBounds_[7];
        diphoBounds_["RECO_1J_PTH_0_60_Tag2"] = rawDiphoBounds_[8];
        diphoBounds_["RECO_1J_PTH_60_120_Tag0"] = rawDiphoBounds_[9];
        diphoBounds_["RECO_1J_PTH_60_120_Tag1"] = rawDiphoBounds_[10];
        diphoBounds_["RECO_1J_PTH_60_120_Tag2"] = rawDiphoBounds_[11];
        diphoBounds_["RECO_1J_PTH_120_200_Tag0"] = rawDiphoBounds_[12];
        diphoBounds_["RECO_1J_PTH_120_200_Tag1"] = rawDiphoBounds_[13];
        diphoBounds_["RECO_1J_PTH_120_200_Tag2"] = rawDiphoBounds_[14];
        diphoBounds_["RECO_GE2J_PTH_0_60_Tag0"] = rawDiphoBounds_[15];
        diphoBounds_["RECO_GE2J_PTH_0_60_Tag1"] = rawDiphoBounds_[16];
        diphoBounds_["RECO_GE2J_PTH_0_60_Tag2"] = rawDiphoBounds_[17];
        diphoBounds_["RECO_GE2J_PTH_60_120_Tag0"] = rawDiphoBounds_[18];
        diphoBounds_["RECO_GE2J_PTH_60_120_Tag1"] = rawDiphoBounds_[19];
        diphoBounds_["RECO_GE2J_PTH_60_120_Tag2"] = rawDiphoBounds_[20];
        diphoBounds_["RECO_GE2J_PTH_120_200_Tag0"] = rawDiphoBounds_[21];
        diphoBounds_["RECO_GE2J_PTH_120_200_Tag1"] = rawDiphoBounds_[22];
        diphoBounds_["RECO_GE2J_PTH_120_200_Tag2"] = rawDiphoBounds_[23];
        diphoBounds_["RECO_PTH_200_300_Tag0"] = rawDiphoBounds_[24];
        diphoBounds_["RECO_PTH_200_300_Tag1"] = rawDiphoBounds_[25];
        diphoBounds_["RECO_PTH_300_450_Tag0"] = rawDiphoBounds_[26];
        diphoBounds_["RECO_PTH_300_450_Tag1"] = rawDiphoBounds_[27];
        diphoBounds_["RECO_PTH_450_650_Tag0"] = rawDiphoBounds_[28];
        diphoBounds_["RECO_PTH_GT650_Tag0"]  = rawDiphoBounds_[29];
        diphoBounds_["RECO_VBFTOPO_VHHAD_Tag0"] = rawDiphoBounds_[30];
        diphoBounds_["RECO_VBFTOPO_VHHAD_Tag1"] = rawDiphoBounds_[31];
        diphoBounds_["RECO_VBFTOPO_JET3VETO_LOWMJJ_Tag0"] = rawDiphoBounds_[32];
        diphoBounds_["RECO_VBFTOPO_JET3VETO_LOWMJJ_Tag1"] = rawDiphoBounds_[33];
        diphoBounds_["RECO_VBFTOPO_JET3VETO_HIGHMJJ_Tag0"] = rawDiphoBounds_[34];
        diphoBounds_["RECO_VBFTOPO_JET3VETO_HIGHMJJ_Tag1"] = rawDiphoBounds_[35];
        diphoBounds_["RECO_VBFTOPO_JET3_LOWMJJ_Tag0"] = rawDiphoBounds_[36];
        diphoBounds_["RECO_VBFTOPO_JET3_LOWMJJ_Tag1"] = rawDiphoBounds_[37];
        diphoBounds_["RECO_VBFTOPO_JET3_HIGHMJJ_Tag0"] = rawDiphoBounds_[38];
        diphoBounds_["RECO_VBFTOPO_JET3_HIGHMJJ_Tag1"] = rawDiphoBounds_[39];
        diphoBounds_["RECO_VBFTOPO_BSM_Tag0"] = rawDiphoBounds_[40];
        diphoBounds_["RECO_VBFTOPO_BSM_Tag1"] = rawDiphoBounds_[41];
        diphoBounds_["RECO_VBFLIKEGGH_Tag0"] = rawDiphoBounds_[42];
        diphoBounds_["RECO_VBFLIKEGGH_Tag1"] = rawDiphoBounds_[43];

        dijetBounds_["RECO_VBFTOPO_JET3VETO_LOWMJJ_Tag0"] = rawDijetBounds_[0];
        dijetBounds_["RECO_VBFTOPO_JET3VETO_LOWMJJ_Tag1"] = rawDijetBounds_[1];
        dijetBounds_["RECO_VBFTOPO_JET3VETO_HIGHMJJ_Tag0"] = rawDijetBounds_[2];
        dijetBounds_["RECO_VBFTOPO_JET3VETO_HIGHMJJ_Tag1"] = rawDijetBounds_[3];
        dijetBounds_["RECO_VBFTOPO_JET3_LOWMJJ_Tag0"] = rawDijetBounds_[4];
        dijetBounds_["RECO_VBFTOPO_JET3_LOWMJJ_Tag1"] = rawDijetBounds_[5];
        dijetBounds_["RECO_VBFTOPO_JET3_HIGHMJJ_Tag0"] = rawDijetBounds_[6];
        dijetBounds_["RECO_VBFTOPO_JET3_HIGHMJJ_Tag1"] = rawDijetBounds_[7];
        dijetBounds_["RECO_VBFTOPO_BSM_Tag0"] = rawDijetBounds_[8];
        dijetBounds_["RECO_VBFTOPO_BSM_Tag1"] = rawDijetBounds_[9];
        dijetBounds_["RECO_VBFLIKEGGH_Tag0"] = rawDijetBounds_[10];
        dijetBounds_["RECO_VBFLIKEGGH_Tag1"] = rawDijetBounds_[11];

        gghBounds_["RECO_VBFTOPO_JET3VETO_LOWMJJ_Tag0"] = rawGghBounds_[0];
        gghBounds_["RECO_VBFTOPO_JET3VETO_LOWMJJ_Tag1"] = rawGghBounds_[1];
        gghBounds_["RECO_VBFTOPO_JET3VETO_HIGHMJJ_Tag0"] = rawGghBounds_[2];
        gghBounds_["RECO_VBFTOPO_JET3VETO_HIGHMJJ_Tag1"] = rawGghBounds_[3];
        gghBounds_["RECO_VBFTOPO_JET3_LOWMJJ_Tag0"] = rawGghBounds_[4];
        gghBounds_["RECO_VBFTOPO_JET3_LOWMJJ_Tag1"] = rawGghBounds_[5];
        gghBounds_["RECO_VBFTOPO_JET3_HIGHMJJ_Tag0"] = rawGghBounds_[6];
        gghBounds_["RECO_VBFTOPO_JET3_HIGHMJJ_Tag1"] = rawGghBounds_[7];
        gghBounds_["RECO_VBFTOPO_BSM_Tag0"] = rawGghBounds_[8];
        gghBounds_["RECO_VBFTOPO_BSM_Tag1"] = rawGghBounds_[9];
        gghBounds_["RECO_VBFLIKEGGH_Tag0"] = rawGghBounds_[10];
        gghBounds_["RECO_VBFLIKEGGH_Tag1"] = rawGghBounds_[11];

        vhHadBounds_["RECO_VBFTOPO_VHHAD_Tag0"] = rawVhHadBounds_[0];
        vhHadBounds_["RECO_VBFTOPO_VHHAD_Tag1"] = rawVhHadBounds_[1];
    }


    float StageOneCombinedTagProducer::getVBFKFactor(const StageOneCombinedTag tag_obj, std::vector<double> edges, std::vector<double> vals)
    {
        float ptH = tag_obj.diPhoton()->pt();
        int b;
        for( b = 0; b < (int)edges.size(); b++ ) {
            if( ptH > edges[edges.size() - b - 1] ) break;
        }
        float kfactor = vals[edges.size() - b];
        return kfactor;
    }

    float StageOneCombinedTagProducer::getVHHadKFactor(const StageOneCombinedTag tag_obj)
    {
        float x = tag_obj.VHhadACDNN().dnnvh_bkg_value();
        float y = tag_obj.VHhadACDNN().dnnvh_bsm_value();

        int ibinx = nloKfacVHHad_->GetXaxis()->FindBin(x);
        int ibiny = nloKfacVHHad_->GetYaxis()->FindBin(y);
        if (ibinx <= 0) ibinx = 1;
        if (ibinx > nloKfacVHHad_->GetNbinsX()) ibinx = nloKfacVHHad_->GetNbinsX();
        if (ibiny <= 0) ibiny = 1;
        if (ibiny > nloKfacVHHad_->GetNbinsY()) ibiny = nloKfacVHHad_->GetNbinsY();
        
        return min(nloKfacVHHad_->GetBinContent(ibinx,ibiny),2.);
    }
}


typedef flashgg::StageOneCombinedTagProducer FlashggStageOneCombinedTagProducer;
DEFINE_FWK_MODULE( FlashggStageOneCombinedTagProducer );
// Local Variables:
// mode:c++
// indent-tabs-mode:nil
// tab-width:4
// c-basic-offset:4
// End:
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

