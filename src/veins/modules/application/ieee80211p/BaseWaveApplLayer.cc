//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

// Adicionado (Minicurso_UFPI)
#include "modules/mobility/traci/TraCIMobility.h"

// Adicionado (Minicurso_UFPI)
using Veins::TraCIMobilityAccess;

const simsignalwrap_t BaseWaveApplLayer::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void BaseWaveApplLayer::initialize_default_veins_TraCI(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;
        //cout << findHost()->getFullName() << " Beacon offSet: " << offSet << endl; // betwen 0 and 1

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

//######################################### vehDist ###############################################################################################

void BaseWaveApplLayer::saveMessagesOnFile(WaveShortMessage* wsm, string fileName){
    myfile.open (fileName, std::ios_base::app); //Open file for just append

    //Send "strings" to be saved on the file
    myfile << "BeaconMessage from " << wsm->getSenderAddressTemporary() << " at " << simTime();
    myfile << " to " << wsm->getRecipientAddressTemporary() << endl;
    myfile << "wsm->getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
    myfile << "wsm->getName(): " << wsm->getName() << endl;
    myfile << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
    myfile << "wsm->getPriority(): " << wsm->getPriority() << endl;
    myfile << "wsm->getSerial(): " << wsm->getSerial() << endl;
    myfile << "wsm->getSecurityType(): " << wsm->getSecurityType() << endl;
    myfile << "wsm->getDataRate(): " << wsm->getDataRate() << endl;
    myfile << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
    myfile << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
    myfile << "wsm->getPsid(): " << wsm->getPsid() << endl;
    myfile << "wsm->getPsc(): " << wsm->getPsc() << endl;
    myfile << "wsm->getHeading(): " << wsm->getHeading() << endl;
    myfile << "wsm->getCategory(): " << wsm->getCategory() << endl;
    myfile << "wsm->getRoadId(): " << wsm->getRoadId() << endl;
    myfile << "wsm->getSenderSpeed(): " << wsm->getSenderSpeed() << endl;
    myfile << "wsm->getSenderAddressTemporary(): " << wsm->getSenderAddressTemporary() << endl;
    myfile << "wsm->getRecipientAddressTemporary(): " << wsm->getRecipientAddressTemporary() << endl;
    myfile << "wsm->getSource(): " << wsm->getSource() << endl;
    myfile << "wsm->getTarget(): " << wsm->getTarget() << endl;
    myfile << "findHost()->getFullName(): " << findHost()->getFullName() << endl;
    myfile << "wsm->getTargetPos(): " << wsm->getTargetPos() << endl;
    myfile << "wsm->getHopCount(): " << wsm->getHopCount() << endl;
    myfile << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
    myfile << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
    myfile << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
    myfile << "Time to generate and received: " << (simTime() - wsm->getTimestamp()) << endl;
    myfile << endl;

    myfile.close();
}

void BaseWaveApplLayer::openFileAndClose(string fileName, bool justForAppend, int ttlBeaconMessage, int countGenerateBeaconMessage ){
    if (justForAppend) {
        myfile.open(fileName, std::ios_base::app);
    } else {
        myfile.open(fileName);
    }
    printHeaderfileExecution(ttlBeaconMessage ,countGenerateBeaconMessage);
    myfile.close();
}

void BaseWaveApplLayer::printHeaderfileExecution(int ttlBeaconMessage, int countGenerateBeaconMessage){
    if (repeatNumber != 0) {
        myfile << endl;
    }
    myfile << "Exp: " << experimentNumber <<  " ######################################################";
    myfile << "#######################################################################################" << endl << endl;
    myfile << "Exp: " << experimentNumber << " ### Experiment: " << experimentNumber << " Execution: " << repeatNumber << " ttlBeaconMessage: " << ttlBeaconMessage;
    myfile << " countGenerateBeaconMessage: " << countGenerateBeaconMessage << endl << endl;
}

void BaseWaveApplLayer::generalInitializeVariables_executionByExperimentNumber(){
    source = findHost()->getFullName();
    beaconMessageHopLimit = par("beaconMessageHopLimit").longValue();
    stringTmp = ev.getConfig()->getConfigValue("seed-set");
    repeatNumber = atoi(stringTmp.c_str()); // number of execution (${repetition})

    experimentNumber = par("experimentNumber");
    if ((experimentNumber == 1) || (experimentNumber == 5)) {
        ttlBeaconMessage = par("ttlBeaconMessage_one").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_one");
    } else if ((experimentNumber == 2) || (experimentNumber == 6)) {
        ttlBeaconMessage = par("ttlBeaconMessage_one").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_two");
    } else if ((experimentNumber == 3) || (experimentNumber == 7)) {
        ttlBeaconMessage = par("ttlBeaconMessage_two").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_one");
    } else if ((experimentNumber == 4) || (experimentNumber == 8)) {
        ttlBeaconMessage = par("ttlBeaconMessage_two").doubleValue();
        countGenerateBeaconMessage = par("countGenerateBeaconMessage_two");
    } else {
        cout << "Error: Number of experiment not configured. Go to VehDist.cc line 146." << endl;
        exit(1); // Comets this line for use the values below
        ttlBeaconMessage = 60; // Just for don't left garbage value in this variable
        countGenerateBeaconMessage = 0; // Will not generate any message
    }
}
// end
//######################################### vehDist ##############################################################################################

void BaseWaveApplLayer::initialize_minicurso_UFPI_TraCI(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        //sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_minicurso);

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt); //parte modificada para o osdp e para o service_discovery
        }
    }
}

void BaseWaveApplLayer::initialize_epidemic(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        //define the minimum slide window length among contacts to send new version of summary vector
        sendSummaryVectorInterval = par("sendSummaryVectorInterval").longValue();
        //define the maximum buffer size (in number of messages) that a node is willing to allocate for epidemic messages.
        maximumEpidemicBufferSize = par("maximumEpidemicBufferSize").longValue();
        //define the maximum number of hopes that a message can be forward before reach the target
        hopCount = par("hopCount").longValue();

        //cout << "sendSummaryVectorInterval: " << sendSummaryVectorInterval << endl;
        //cout << "maximumEpidemicBufferSize: " << maximumEpidemicBufferSize << endl;
        //cout << "hopCount: " << hopCount << endl;
        //cout << "uniform(0,1):" << uniform(0,1) << endl;
        //cout << "rand() % 100 + 1:" << (rand() % 100 + 1) << endl;

        //hopsToDeliverSignal = registerSignal("numberHops");
        //messageArrivalSignal = registerSignal("delay");
        //messageArrivalSignal = registerSignal("messageReceived");

        //sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_epidemic);
        //std::cout << "sendBeaconEvt: " << sendBeaconEvt->getName() << " SEND_BEACON_EVT: " << SEND_BEACON_EVT << " SERVICE_PROVIDER: " << SERVICE_PROVIDER << std::endl;

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        //std::cout << "Eu sou o " << findHost()->getFullName() << " myMac: " << myMac <<  " myMacInteger: " << MACToInteger() << std::endl;

        //if(strcmp(findHost()->getFullName(), "rsu[0]")==0){
        //    targetAddress = MACToInteger();
        //    assert(targetAddress);
        //    ///std::cout << "Eu sou o " << findHost()->getFullName() << " myMac: " << myMac <<  " myMacInteger: " << MACToInteger() << std::endl;
        //    //cout << "O target foi definido para: " << targetAddress << endl;
        //}
        //else{
        //    //targetAddress = findHost()->get
        //}

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

void BaseWaveApplLayer::initialize_mfcv_epidemic(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage==0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
        assert(myMac);

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        //define the minimum slide window length among contacts to send new version of summary vector
        sendSummaryVectorInterval = par("sendSummaryVectorInterval").longValue();
        //define the maximum buffer size (in number of messages) that a node is willing to allocate for mfcv_epidemic messages.
        maximumMfcvEpidemicBufferSize = par("maximumMfcvEpidemicBufferSize").longValue();
        //define the maximum number of hopes that a message can be forward before reach the target
        hopCount = par("hopCount").longValue();

        //cout << "sendSummaryVectorInterval: " << sendSummaryVectorInterval << endl;
        //cout << "maximumMfcvEpidemicBufferSize: " << maximumMfcvEpidemicBufferSize << endl;
        //cout << "hopCount: " << hopCount << endl;
        //cout << "uniform(0,1):" << uniform(0,1) << endl;
        //cout << "rand() % 100 + 1:" << (rand() % 100 + 1) << endl;

        //hopsToDeliverSignal = registerSignal("numberHops");
        //messageArrivalSignal = registerSignal("delay");
        //messageArrivalSignal = registerSignal("messageReceived");

        //sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_mfcv_epidemic);
        //std::cout << "sendBeaconEvt: " << sendBeaconEvt->getName() << " SEND_BEACON_EVT: " << SEND_BEACON_EVT << " SERVICE_PROVIDER: " << SERVICE_PROVIDER << std::endl;

        //simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        //std::cout << "Eu sou o " << findHost()->getFullName() << " myMac: " << myMac <<  " myMacInteger: " << MACToInteger() << std::endl;

        //if(strcmp(findHost()->getFullName(), "rsu[0]")==0){
        //    targetAddress = MACToInteger();
        //    assert(targetAddress);
        //    ///std::cout << "Eu sou o " << findHost()->getFullName() << " myMac: " << myMac <<  " myMacInteger: " << MACToInteger() << std::endl;
        //    //cout << "O target foi definido para: " << targetAddress << endl;
        //}
        //else{
        //    //targetAddress = findHost()->get
        //}

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

//WaveShortMessage*  BaseWaveApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
WaveShortMessage*  BaseWaveApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);

    switch (channel) {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon_minicurso") {
        // Change Minicurso_UFPI
        wsm->setRoadId(TraCIMobilityAccess().get(getParentModule()) ->getRoadId().c_str());
        wsm->setSenderSpeed(TraCIMobilityAccess(). get(getParentModule())->getSpeed());
        //DBG << "\n ttt" << wsm->getSenderSpeed() << " ttt\n";
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }else if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

//WaveShortMessage*  BaseWaveApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, std::string rcvId, int serial) {
WaveShortMessage*  BaseWaveApplLayer::prepareWSM_epidemic(std::string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(MACToInteger());
    wsm->setRecipientAddress(rcvId);
    //wsm->setSource(source);
    //wsm->setTarget(target);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }
    return wsm;
}

unsigned int BaseWaveApplLayer::MACToInteger(){
    unsigned int macInt;
    std::stringstream ss;
    ss << std::hex << myMac;
    ss >> macInt;
    return macInt;
}

void BaseWaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
}

void BaseWaveApplLayer::handlePositionUpdate(cObject* obj) {
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

void BaseWaveApplLayer::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon") {
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "data") {
        onData(wsm);
    }
    else if (std::string(wsm->getName()) == "beacon_minicurso") {
        onBeacon(wsm);
    }
    else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}

void BaseWaveApplLayer::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case SEND_BEACON_EVT_minicurso: {
            sendWSM(prepareWSM("beacon_minicurso", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        case SEND_BEACON_EVT_epidemic: {
            //prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
            //I our implementation, if rcvId = BROADCAST then we are broadcasting beacons. Otherwise, this parameter must be instantiated with the receiver address
            sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
//        case SEND_BEACON_EVT_mfcv_epidemic: {
//            //prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
//            //I our implementation, if rcvId = BROADCAST then we are broadcasting beacons. Otherwise, this parameter must be instantiated with the receiver address
//            sendWSM(prepareWSM_mfcv_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, BROADCAST, -1));
//            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
//            break;
//        }
        default: {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}

void BaseWaveApplLayer::sendWSM(WaveShortMessage* wsm) {
    sendDelayedDown(wsm,individualOffset);
}

void BaseWaveApplLayer::finish() {
    if (sendBeaconEvt->isScheduled()) {
        cancelAndDelete(sendBeaconEvt);
    }else {
        delete sendBeaconEvt;
    }
    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}

BaseWaveApplLayer::~BaseWaveApplLayer() {

}