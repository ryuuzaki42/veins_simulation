//
// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>
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

#include "veins/modules/application/vehDist/vehDist_rsu.h"

using Veins::AnnotationManagerAccess;

Define_Module(vehDist_rsu);

void vehDist_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_default_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
        sentMessage = false;

        rsuInitializeVariables();
    }
}

void vehDist_rsu::rsuInitializeVariables() {
    generalInitializeVariables_executionByExperimentNumber();

    restartFilesResult();
    //cout << " " << source << " entered in the scenario" << endl;
}

void vehDist_rsu::restartFilesResult() {
    stringTmp = getFolderResult(experimentSendbyDSCR);

    fileMessagesBroadcast = fileMessagesUnicast = fileMessagesCount = stringTmp + source;

    fileMessagesBroadcast += "_Broadcast_Messages.r";
    fileMessagesUnicast += "_Messages_Received.r";
    fileMessagesCount += "_Count_Messages_Received.r";

    //fileMessagesDrop and fileMessagesGenerated not used yet to RSU

    if ((myId == 0) && (repeatNumber == 0)) { //Open a new file (blank)
        if (experimentNumber == 1) { // maxSpeed 15 m/s
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the spped to 15 m/s, command: " << comand << endl;
        } else if (experimentNumber == 5){  // maxSpeed 25 m/s
            string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' vehDist.rou.xml";
            system(comand.c_str());
            cout << endl << "Change the spped to 25 m/s, command: " << comand << endl;
        }

        stringTmp = "mkdir -p " + stringTmp + " > /dev/null";
        cout << endl << "Created the folder, command: \"" << stringTmp << "\"" << endl;
        cout << "repeatNumber: " << repeatNumber << endl;
        system(stringTmp.c_str()); //create a folder results

        openFileAndClose(fileMessagesBroadcast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, false, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, false, ttlBeaconMessage, countGenerateBeaconMessage);
    } else { // (repeatNumber != 0)) // for just append
        openFileAndClose(fileMessagesBroadcast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesUnicast, true, ttlBeaconMessage, countGenerateBeaconMessage);
        openFileAndClose(fileMessagesCount, true, ttlBeaconMessage, countGenerateBeaconMessage);
    }
}

void vehDist_rsu::handleLowerMsg(cMessage* msg) {
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (wsm->getType() == 1) {
        onBeaconStatus(wsm);
    }
    else if (wsm->getType() == 2) {
        onBeaconMessage(wsm);
    }
    else{
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
        cout << "unknown message (" << wsm->getName() << ")  received\n";
        exit(1);
    }
    delete(msg);
}

void vehDist_rsu::onBeaconStatus(WaveShortMessage* wsm) {
}

void vehDist_rsu::onBeaconMessage(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) {
        findHost()->bubble("Received Message");
        saveMessagesOnFile(wsm, fileMessagesUnicast);

        messagesReceivedMeasuring(wsm);
    } else {
        saveMessagesOnFile(wsm, fileMessagesBroadcast);
    }
}

void vehDist_rsu::messagesReceivedMeasuring(WaveShortMessage* wsm) {
    string wsmData = wsm->getWsmData();
    simtime_t timeToArrived = (simTime() - wsm->getTimestamp());
    unsigned short int countHops = (beaconMessageHopLimit - wsm->getHopCount());
    map<string, struct messages>::iterator it = messagesReceived.find(wsm->getGlobalMessageIdentificaton());

    if (it != messagesReceived.end()) {
        it->second.copyMessage++;

        it->second.hops += ", " + to_string(countHops);
        it->second.sumHops += countHops;
        if (countHops > it->second.maxHop) {
            it->second.maxHop = countHops;
        }
        if (countHops < it->second.minHop) {
             it->second.minHop = countHops;
        }

        it->second.sumTimeRecived += timeToArrived;
        it->second.times += ", " + timeToArrived.str();

        if (wsmData.size() > 42){ // WSMData generated by car[x] and carry by [ T,
            it->second.wsmData += " & " + wsmData.substr(42);
        }
        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        it->second.countT += count(wsmData.begin(), wsmData.end(), 'T');
        it->second.countP += count(wsmData.begin(), wsmData.end(), 'P');
    } else{
        struct messages m;
        m.copyMessage = 1;
        m.wsmData = wsmData;
        m.hops = to_string(countHops);
        m.maxHop = m.minHop = m.sumHops = countHops;

        m.sumTimeRecived = timeToArrived;
        m.times = timeToArrived.str();

        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        m.countT = count(wsmData.begin(), wsmData.end(), 'T');
        m.countP = count(wsmData.begin(), wsmData.end(), 'P');

        messagesReceived.insert(make_pair(wsm->getGlobalMessageIdentificaton(), m));
    }
}

void vehDist_rsu::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            //sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            sendWSM(prepareBeaconStatusWSM("beaconStatus", beaconLengthBits, type_CCH, beaconPriority, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        default: {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}

WaveShortMessage* vehDist_rsu::prepareBeaconStatusWSM(std::string name, int lengthBits, t_channel channel, int priority, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->setType(1); // Beacon of Status
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setSerial(serial);
    wsm->setTimestamp(simTime());
    wsm->setSenderPos(curPosition);
    wsm->setSenderPosPrevious(curPosition); // RSU can't move
    wsm->setSource(source.c_str());

    //beacon don't need
    //wsm->setRecipientAddressString(); => "BROADCAST"
    // wsm->setSenderAddressTemporary();
    //wsm->setTarget(); => "BROADCAST"
    // wsm->setTimeToSend();

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    return wsm;
}

void vehDist_rsu::finish() {
    printCountMessagesReceived();
    if (experimentNumber == 8) { // maxSpeed 15 m/s
        string comand = "sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' vehDist.rou.xml";
        system(comand.c_str());
        cout << endl << "Setting speed back to default (15 m/s), command: " << comand << endl;
    }
}

void vehDist_rsu::printCountMessagesReceived() {
    myfile.open (fileMessagesCount, std::ios_base::app);

    if (!messagesReceived.empty()) {
        myfile << "messagesReceived from " << source << endl;

        float avgGeneralHopsMessage = 0;
        SimTime avgGeneralCopyMessageReceived = 0;
        SimTime avgGeneralTimeMessageReceived = 0;

        unsigned short int countP = 0;
        unsigned short int countT = 0;
        map<string, struct messages>::iterator it;
        for (it = messagesReceived.begin(); it != messagesReceived.end(); it++) {
            myfile << endl << "## Message ID: " << it->first << endl;
            myfile << "Count received: " << it->second.copyMessage << endl;
            avgGeneralCopyMessageReceived += it->second.copyMessage;

            myfile << it->second.wsmData << endl;
            myfile << "Hops: " << it->second.hops << endl;
            myfile << "Sum hops: " << it->second.sumHops << endl;
            avgGeneralHopsMessage += it->second.sumHops;
            myfile << "Average hops: " << (it->second.sumHops/it->second.copyMessage) << endl;
            myfile << "Max hop: " << it->second.maxHop << endl;
            myfile << "Min hop: " << it->second.minHop << endl;

            myfile << "Times: " << it->second.times << endl;
            myfile << "Sum times: " << it->second.sumTimeRecived << endl;
            avgGeneralTimeMessageReceived += it->second.sumTimeRecived;
            myfile << "Average time to received: " << (it->second.sumTimeRecived/it->second.copyMessage) << endl;

            myfile << "Category T count: " << it->second.countT << endl;
            countT += it->second.countT;
            myfile << "Category P count: " << it->second.countP << endl;
            countP += it->second.countP;
        }

        // TODO: XX geradas, mas só (XX - 4) recebidas
        avgGeneralHopsMessage /= messagesReceived.size();
        avgGeneralTimeMessageReceived /= messagesReceived.size();
        avgGeneralCopyMessageReceived /= messagesReceived.size();

        myfile << endl << "Exp: " << experimentNumber << " ### Count Messages Received: " << messagesReceived.size() << endl;
        myfile << "Exp: " << experimentNumber << " ### avg time to receive: " << avgGeneralTimeMessageReceived << endl;
        myfile << "Exp: " << experimentNumber << " ### avg copy received: " << avgGeneralCopyMessageReceived << endl;
        myfile << "Exp: " << experimentNumber << " ### avg hops to received: " << avgGeneralHopsMessage << endl;

        myfile << "Exp: " << experimentNumber << " ### Category T general: " << countT << endl;
        myfile << "Exp: " << experimentNumber << " ### Category P general: " << countP << endl << endl;;
    } else {
        myfile << "messagesReceived from " << source << " is empty" << endl;
    }
    myfile.close();
}

// #####################################################################################################
void vehDist_rsu::onBeacon(WaveShortMessage* wsm) {
}

void vehDist_rsu::onData(WaveShortMessage* wsm) {
}
