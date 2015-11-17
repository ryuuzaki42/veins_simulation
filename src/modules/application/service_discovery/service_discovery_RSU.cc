//
// Copyright (C) 2014 Luz Marina Santos
//

#include "application/service_discovery/service_discovery_RSU.h"

using Veins::AnnotationManagerAccess;

Define_Module(service_discovery_RSU);

void service_discovery_RSU::initialize(int stage) {
    serv1 = par("serv1").longValue();
    serv2 = par("serv2").longValue();
    BaseWaveApplLayer::initialize_service_discovery(stage);
    std::cout << "Starting services " << serv1 << " - "<< serv2 << std::endl;
    setServices();
     if (stage == 0) {
         t=simTime();
        sent_adv.setName("SentAdvertisements");
        sendServiceEvt = new cMessage("service evt", SERVICE_EVT);
        scheduleAt(simTime() + par("serviceInterval").doubleValue(), sendServiceEvt);
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
    }
}

void service_discovery_RSU::onBeacon(WaveShortMessage* wsm) {

}

void service_discovery_RSU::onData(WaveShortMessage* wsm) {

}

void service_discovery_RSU::handleSelfMsg(cMessage* msg) {
      switch (msg->getKind()) {
         case SEND_BEACON_EVT: {
             sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
             scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
             break;
         }
         case SERVICE_EVT: {
             int x=0;
             for (int i=serv1-1; i< serv2; i++) {
                   simtime_t eed = simTime();
                   sent_adv.record(eed);
                   sendMessage(service[x].c_str());
                   x++;}
             //if (simTime()-t <= 1500)
                  scheduleAt(simTime() + par("serviceInterval").doubleValue(), sendServiceEvt);
             //break;
         }
         default: {
             if (msg)
                 DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
                 break;
         }
     }
}

void service_discovery_RSU::sendMessage(const char* serv) {
    sentMessage = true;
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    //Message of type data
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    //add the next line
    wsm->setWsmData(serv);
    sendWSM(wsm);
}

void service_discovery_RSU::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void service_discovery_RSU::setServices() {
     int x=0,y=0;
     for (int i=serv1-1; i< serv2; i++){
        std::stringstream ss,ss2;
        ss << i;
        ss2 << y;
        service[x]="service"+ss2.str()+" description"+ss.str()+" address"+ss.str();
        std::cout << service[x].c_str() << std::endl;
        x++;
        y++;
        if (y==5) y=0;
    }
}
