// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#include "veins/modules/application/epidemic/epidemic.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

Define_Module(epidemic);

void epidemic::initialize(int stage) {
    BaseWaveApplLayer::initialize_veins_TraCI(stage);
    if (stage == 0) {
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        epidemicInitializeVariablesVeh();
    }
}

void epidemic::epidemicInitializeVariablesVeh() {
    vehInitializeValuesMfcv(traciVehicle->getTypeId(), mobility->getPositionAt(simTime() + 0.1)); // The same for Epidemic and VehDist
}

void epidemic::onBeacon(WaveShortMessage* wsm) {
    receivedOnBeaconEpidemic(wsm);

    // Another way to finish the simulation
    // Has the "sim-time-limit = 600s" from the ini file
    // and with endSimulation() you terminate in any time
    //if (simTime() > 100) {
    //    endSimulation();
    //}
}

void epidemic::onData(WaveShortMessage* wsm) {
    receivedOnDataEpidemic(wsm);
}

void epidemic::finish() {
    toFinishVeh();
}
