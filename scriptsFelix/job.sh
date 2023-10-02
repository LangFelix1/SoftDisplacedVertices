#!/bin/bash

#voms-proxy-init --voms cms --out $(pwd)/voms_proxy.txt --hours 24
#export X509_USER_PROXY=$(pwd)/voms_proxy.txt
export SCRAM_ARCH=slc7_amd64_gcc700

source /cvmfs/cms.cern.ch/cmsset_default.sh

SEED=$(($(date +%N) % 100 + 1))
echo simulating with seed = $SEED

mkdir $$
cd $$

if [ -r CMSSW_10_6_30/src ] ; then
  echo release CMSSW_10_6_30 already exists
else
  echo creating release CMSSW_10_6_30
  scram p CMSSW CMSSW_10_6_30
fi
cd CMSSW_10_6_30/src
cp -r ../../../Configuration/ .
eval `scram runtime -sh`
scram b

#LHE,GEN-SIM
cmsDriver.py Configuration/GenProduction/python/test-fragment.py --python_filename testLHEGENSIM-$SEED-cfg.py --eventcontent RAWSIM,LHE --customise Configuration/DataProcessing/Utils.addMonitoring --datatier GEN-SIM,LHE --fileout file:testLHEGENSIM.root --conditions 106X_upgrade2018_realistic_v4 --beamspot Realistic25ns13TeVEarly2018Collision --step LHE,GEN,SIM --geometry DB:Extended --era Run2_2018 --no_exec --mc --customise_commands process.RandomNumberGeneratorService.externalLHEProducer.initialSeed="int(${SEED})" -n 5000
cmsRun testLHEGENSIM-$SEED-cfg.py

#PREMIX
cp ../../../testPREMIX-cfg.py .
cmsRun testPREMIX-cfg.py

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

cp ../../CMSSW_10_6_30/src/testPREMIX.root .

#HLT
cp ../../../testHLT-cfg.py .
cmsRun testHLT-cfg.py

cd ../..
cd CMSSW_10_6_30/src
eval `scramv1 runtime -sh`

cp ../../CMSSW_10_2_16_UL/src/testHLT.root .

#AODSIM (with IVF1)
cp ../../../testAODSIM-IVF1-cfg.py .
cmsRun testAODSIM-IVF1-cfg.py

#MINIAOD
cp ../../../testMINIAODSIM-cfg.py .
cmsRun testMINIAODSIM-cfg.py

#NANOAOD
cp ../../../testNANOAODSIM-cfg.py .
cmsRun testNANOAODSIM-cfg.py

#cp SUS-RunIISummer19UL17-stop500-dm15-br100-ctau99_miniAOD_FullSim_$3.root /eos/user/l/lang/LLStop/Stop500_DM15/Full/