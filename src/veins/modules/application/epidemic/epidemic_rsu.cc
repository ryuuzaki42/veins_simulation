// Copyright (C) 2015-2017 João Batista <joao.b@usp.br>

#include "veins/modules/application/epidemic/epidemic_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(epidemic_rsu);

void epidemic_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);

        epidemicInitializeVariablesRsu();
    }
}

void epidemic_rsu::epidemicInitializeVariablesRsu() {
    rsuInitializeValuesMfcv(mobi->getInitialPositionFromIniFileRSU());
}

void epidemic_rsu::onBeacon(WaveShortMessage* wsm) {
    receivedOnBeaconEpidemic(wsm);
}

void epidemic_rsu::onData(WaveShortMessage* wsm) {
    receivedOnDataEpidemic(wsm);
}

void epidemic_rsu::finish() {
    toFinishRSU();
}
