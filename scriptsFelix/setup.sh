#!/bin/bash
export SCRAM_ARCH=slc7_amd64_gcc700
source /cvmfs/cms.cern.ch/cmsset_default.sh

if [ -r CMSSW_10_6_30/src ] ; then
  echo release CMSSW_10_6_30 already exists
else
  echo creating release CMSSW_10_6_30
  scram p CMSSW CMSSW_10_6_30
fi
cd CMSSW_10_6_30/src
eval `scram runtime -sh`
scram b

cmsDriver.py  --python_filename testPREMIX-cfg.py --eventcontent PREMIXRAW --customise Configuration/DataProcessing/Utils.addMonitoring --datatier GEN-SIM-DIGI --fileout file:testPREMIX.root --pileup_input "dbs:/Neutrino_E-10_gun/RunIISummer20ULPrePremix-UL18_106X_upgrade2018_realistic_v11_L1v1-v2/PREMIX" --conditions 106X_upgrade2018_realistic_v11_L1v1 --step DIGI,DATAMIX,L1,DIGI2RAW --procModifiers premix_stage2 --geometry DB:Extended --filein file:testLHEGENSIM.root --datamix PreMix --era Run2_2018 --runUnscheduled --no_exec --mc -n 5000

cd ../..
if [ -r CMSSW_10_2_16_UL/src ] ; then
  echo release CMSSW_10_2_16_UL already exists
else
  echo creating release CMSSW_10_2_16_UL
  scram p CMSSW CMSSW_10_2_16_UL
fi
cd CMSSW_10_2_16_UL/src
eval `scram runtime -sh`
scram b

cmsDriver.py  --python_filename testHLT-cfg.py --eventcontent RAWSIM --customise Configuration/DataProcessing/Utils.addMonitoring --datatier GEN-SIM-RAW --fileout file:testHLT.root --conditions 102X_upgrade2018_realistic_v15 --customise_commands 'process.source.bypassVersionCheck = cms.untracked.bool(True)' --step HLT:2018v32 --geometry DB:Extended --filein file:testPREMIX.root --era Run2_2018 --no_exec --mc -n 5000

cd ../..
cd CMSSW_10_6_30/src
eval `scramv1 runtime -sh`

cmsDriver.py  --python_filename testAODSIM-cfg.py --eventcontent AODSIM --customise Configuration/DataProcessing/Utils.addMonitoring --datatier AODSIM --fileout file:testAODSIM.root --conditions 106X_upgrade2018_realistic_v11_L1v1 --step RAW2DIGI,L1Reco,RECO,RECOSIM,EI --geometry DB:Extended --filein file:testHLT.root --era Run2_2018 --runUnscheduled --no_exec --mc -n 5000

cmsDriver.py  --python_filename testMINIAODSIM-cfg.py --eventcontent MINIAODSIM --customise Configuration/DataProcessing/Utils.addMonitoring --datatier MINIAODSIM --fileout file:testMINIAODSIM.root --conditions 106X_upgrade2018_realistic_v16_L1v1 --step PAT --procModifiers run2_miniAOD_UL --geometry DB:Extended --filein file:testAODSIM.root --era Run2_2018 --runUnscheduled --no_exec --mc -n 5000

cmsDriver.py  --python_filename testNANOAODSIM-cfg.py --eventcontent NANOEDMAODSIM --customise Configuration/DataProcessing/Utils.addMonitoring --datatier NANOAODSIM --fileout file:testNANOAODSIM.root --conditions 106X_upgrade2018_realistic_v16_L1v1 --step NANO --filein file:testMINIAODSIM.root --era Run2_2018,run2_nanoAOD_106Xv2 --no_exec --mc -n 5000

cd ../..
cp CMSSW_10_6_30/src/testPREMIX-cfg.py testPREMIX-cfg.py
cp CMSSW_10_2_16_UL/src/testHLT-cfg.py testHLT-cfg.py
cp CMSSW_10_6_30/src/testAODSIM-cfg.py testAODSIM-cfg.py
cp CMSSW_10_6_30/src/testMINIAODSIM-cfg.py testMINIAODSIM-cfg.py
cp CMSSW_10_6_30/src/testNANOAODSIM-cfg.py testNANOAODSIM-cfg.py

rm -r CMSSW_10_6_30
rm -r CMSSW_10_2_16_UL

#cp SUS-RunIISummer19UL17-stop500-dm15-br100-ctau99_miniAOD_FullSim_$3.root /eos/user/l/lang/LLStop/Stop500_DM15/Full/