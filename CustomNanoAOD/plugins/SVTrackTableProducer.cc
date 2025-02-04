// This code is copied from 
// PhysicsTools/NanoAOD/plugins/VertexTableProducer.cc
// The main difference is that the original code takes VertexCompositePtrCandidate as SV source
// while this code take reco::Vertex
// In addition, some more variables are added

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexPrimitives/interface/ConvertToFromReco.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "DataFormats/Common/interface/ValueMap.h"

class SVTrackTableProducer : public edm::stream::EDProducer<> {
public:
  explicit SVTrackTableProducer(const edm::ParameterSet&);
  ~SVTrackTableProducer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;

  //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------

  const edm::EDGetTokenT<std::vector<reco::Vertex>> pvs_;
  const edm::EDGetTokenT<std::vector<reco::Vertex>> svs_;
  const edm::EDGetTokenT<reco::TrackCollection> tksrc_;
  const StringCutObjectSelector<reco::Vertex> svCut_;
  const std::string svName_;
  const std::string svDoc_;
  const double dlenMin_, dlenSigMin_;
  const bool storeCharge_;
  const std::string tkName_, tkbranchName_, tkdoc_;
  bool debug;
};

SVTrackTableProducer::SVTrackTableProducer(const edm::ParameterSet& params)
    : pvs_(consumes<std::vector<reco::Vertex>>(params.getParameter<edm::InputTag>("pvSrc"))),
      svs_(consumes<std::vector<reco::Vertex>>(params.getParameter<edm::InputTag>("svSrc"))),
      tksrc_(consumes<reco::TrackCollection>(params.getParameter<edm::InputTag>("tkSrc"))),
      svCut_(params.getParameter<std::string>("svCut"), true),
      svName_(params.getParameter<std::string>("svName")),
      svDoc_(params.getParameter<std::string>("svDoc")),
      dlenMin_(params.getParameter<double>("dlenMin")),
      dlenSigMin_(params.getParameter<double>("dlenSigMin")),
      storeCharge_(params.getParameter<bool>("storeCharge")),
      tkName_(params.getParameter<std::string>("tkName")),
      tkbranchName_(params.getParameter<std::string>("tkbranchName")),
      tkdoc_(params.getParameter<std::string>("tkdocString")),
      debug(params.getParameter<bool>("debug"))

{
  produces<nanoaod::FlatTable>("svs");
  produces<nanoaod::FlatTable>("tks");
}

SVTrackTableProducer::~SVTrackTableProducer() {
}


void SVTrackTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  edm::Handle<std::vector<reco::Vertex>> pvsIn;
  iEvent.getByToken(pvs_, pvsIn);

  edm::Handle<std::vector<reco::Vertex>> svsIn;
  iEvent.getByToken(svs_, svsIn);
  auto vertices = std::make_unique<std::vector<reco::Vertex>>();
  std::vector<float> x,y,z,dlen, dlenSig, pAngle, dxy, dxySig;
  std::vector<int> charge, nTrack;
  VertexDistance3D vdist;
  VertexDistanceXY vdistXY;

  size_t i = 0;
  const auto& PV0 = pvsIn->front();
  for (const auto& sv : *svsIn) {
    if (svCut_(sv)) {
      Measurement1D dl =
          vdist.distance(PV0, VertexState(RecoVertex::convertPos(sv.position()), RecoVertex::convertError(sv.error())));
      if (dl.value() > dlenMin_ and dl.significance() > dlenSigMin_) {
        x.push_back(sv.x());
        y.push_back(sv.y());
        z.push_back(sv.z());
        dlen.push_back(dl.value());
        dlenSig.push_back(dl.significance());
        reco::Vertex c = svsIn->at(i);
        vertices->push_back(c);
        double dx = (PV0.x() - sv.x()), dy = (PV0.y() - sv.y()), dz = (PV0.z() - sv.z());
        double pdotv = (dx * sv.p4().Px() + dy * sv.p4().Py() + dz * sv.p4().Pz()) / sqrt(sv.p4().P2()) / sqrt(dx * dx + dy * dy + dz * dz);
        pAngle.push_back(std::acos(pdotv));
        Measurement1D d2d = vdistXY.distance(
            PV0, VertexState(RecoVertex::convertPos(sv.position()), RecoVertex::convertError(sv.error())));
        dxy.push_back(d2d.value());
        dxySig.push_back(d2d.significance());
        nTrack.push_back(sv.tracksSize());

        if (storeCharge_) {
          int sum_charge = 0;
          for (auto v_tk = sv.tracks_begin(), vtke = sv.tracks_end(); v_tk != vtke; ++v_tk){
            sum_charge += (*v_tk)->charge();
          }
          charge.push_back(sum_charge);
        }
      }
    }
    i++;
  }

  auto svsTable = std::make_unique<nanoaod::FlatTable>(vertices->size(), svName_, false);
  // For SV we fill from here only stuff that cannot be created with the SimpleFlatTableProducer
  svsTable->addColumn<float>("x", x, "x position in cm", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>("y", y, "y position in cm", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>("z", z, "z position in cm", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>("dlen", dlen, "decay length in cm", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>("dlenSig", dlenSig, "decay length significance", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>("dxy", dxy, "2D decay length in cm", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>("dxySig", dxySig, "2D decay length significance", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<float>(
      "pAngle", pAngle, "pointing angle, i.e. acos(p_SV * (SV - PV)) ", nanoaod::FlatTable::FloatColumn, 10);
  svsTable->addColumn<int>("nTrack", nTrack, "number of tracks in the SV", nanoaod::FlatTable::IntColumn, 10);
  if (storeCharge_) {
    svsTable->addColumn<int>("charge", charge, "sum of the charge of the SV tracks", nanoaod::FlatTable::IntColumn, 10);
  }

  // Now vertex table is produced, let's make track tables
  const auto& tracks = iEvent.get(tksrc_);
  auto ntrack = tracks.size();
  auto tktab = std::make_unique<nanoaod::FlatTable>(ntrack, tkName_, false, true);

  std::vector<int> key(ntrack, -1);

  if (debug) {
    std::cout << "SVs " << vertices->size() << std::endl;
    for (size_t ivtx=0; ivtx<vertices->size(); ++ivtx) {
        const reco::Vertex& vtx = vertices->at(ivtx);
        std::cout << "reco vertex " << ivtx << " x: " << vtx.x() << " y: " << vtx.y() << " z: " << vtx.z()  << " tracks " << std::endl;
        for (auto v_tk = vtx.tracks_begin(), vtke = vtx.tracks_end(); v_tk != vtke; ++v_tk){
          std::cout << "  pt: " << (*v_tk)->pt() << " eta: " << (*v_tk)->eta() << " phi: " << (*v_tk)->phi() << std::endl;
        }
    }

  }

  for (size_t i = 0; i < ntrack; ++i) {
    const auto& tk = tracks.at(i);
    if (debug){
      std::cout << "reco track " << i << " pt " << tk.pt() << " eta " << tk.eta() << " phi " << tk.phi() << std::endl;
    }
    // match vertices by matching tracks
    // FIXME: This algorithm assumes tracks are not reused for different vertices, so it might be a problem when it is not the case
    int matched_vtx_idx = -1;
    for (size_t ivtx=0; ivtx<vertices->size(); ++ivtx) {
      const reco::Vertex& vtx = vertices->at(ivtx);
      double match_threshold = 1.1;
      // for each LLP, compare the matched tracks with tracks in the reco vertex 
      
      if (debug){
        std::cout << "reco vertex " << ivtx << " x: " << vtx.x() << " y: " << vtx.y() << " z: " << vtx.z()  << " tracks " << std::endl;
        for (auto v_tk = vtx.tracks_begin(), vtke = vtx.tracks_end(); v_tk != vtke; ++v_tk){
          std::cout << "  pt: " << (*v_tk)->pt() << " eta: " << (*v_tk)->eta() << " phi: " << (*v_tk)->phi() << std::endl;
        }
      }

      for (auto v_tk = vtx.tracks_begin(), vtke = vtx.tracks_end(); v_tk != vtke; ++v_tk){
        double dpt = fabs(tk.pt() - (*v_tk)->pt()) + 1;
        double deta = fabs(tk.eta() - (*v_tk)->eta()) + 1;
        double dphi = fabs(tk.phi() - (*v_tk)->phi()) + 1;
        if (dpt * deta * dphi < match_threshold){
          matched_vtx_idx = (int) ivtx;
          if (debug) {
            std::cout << "  track matched: " << std::endl;
            std::cout << "  |->  gen pt " << tk.pt() << " eta " << tk.eta() << " phi " << tk.phi() << std::endl;
            std::cout << "  --> reco pt " << (*v_tk)->pt() << " eta " << (*v_tk)->eta() << " phi " << (*v_tk)->phi() << std::endl;
          }
          break;
        }
      }
      if (matched_vtx_idx!=-1)
        break;
    }
    if (debug)
      std::cout << "track matched with vertex " << matched_vtx_idx << std::endl;
    key[i] = matched_vtx_idx;
  }
  tktab->addColumn<int>(tkbranchName_ + "Idx", key, "Index into secondary vertices list for " + tkdoc_, nanoaod::FlatTable::IntColumn);

  iEvent.put(std::move(svsTable), "svs");
  iEvent.put(std::move(tktab), "tks");
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void SVTrackTableProducer::beginStream(edm::StreamID) {}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void SVTrackTableProducer::endStream() {}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void SVTrackTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(SVTrackTableProducer);
