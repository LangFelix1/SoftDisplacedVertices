"""
This produces flat table of displaced vertices
"""

import FWCore.ParameterSet.Config as cms


SVTrackTable = cms.EDProducer("SVTrackTableProducer",
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    svSrc = cms.InputTag("IVFSecondaryVerticesSoftDV"),
    svCut = cms.string(""), 
    dlenMin = cms.double(0),
    dlenSigMin = cms.double(0),
    svName = cms.string("SDVSecVtx"),
    svDoc = cms.string("Table of displaced vertices"),
    storeCharge = cms.bool(True),
    tkSrc = cms.InputTag("VertexTracksFilter","seed"),
    tkName = cms.string('SDVTrack'),
    tkbranchName = cms.string('SDVSecVtx'),
    tkdocString = cms.string('Vertex matching to tracks'),
    debug = cms.bool(True),

)
