// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#include "veins/modules/application/vehDist/vehDist.h"

using Veins::TraCIMobilityAccess;

Define_Module(vehDist);

void vehDist::initialize(int stage) {
    BaseWaveApplLayer::initialize_veins_TraCI(stage);
    if (stage == 0) { // traci - mobility, traci->getComand - traci, new traciVehice
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        vehDistInitializeVariablesVeh();
    }
}

void vehDist::vehDistInitializeVariablesVeh() {
    vehCreateUpdateRateTimeToSendEvent(); // Create Event to update the rateTimeToSend (Only VehDist)

    vehInitializeValuesVehDist(traciVehicle->getTypeId(), mobility->getPositionAt(simTime() + 0.1)); // The same for Epidemic and VehDist

    busPositionFunctionsRun(); // bus position function

    vehCreateEventTrySendBeaconMessage(); // Create one Event to try send messages in buffer (Only VehDist)
}

void vehDist::busPositionFunctionsRun() {
    routeId = traciVehicle->getRouteId();
    SrouteIDVehID.insert(make_pair(source, routeId));

    createBusPositionSaveEvent(); // Create event to generate a csv file with the bus position (if getBusPosition => true)
    busPosLoadFromFile(); // Load a csv file with bus position (if useBusPosition => true)
}

void vehDist::createBusPositionSaveEvent() {
    if (par("getBusPosition").boolValue()) {
        if (source.compare("car[0]") == 0) {
            string fileName = "results/busPosition.csv";
            myfile.open(fileName);
            myfile.close();
        }

        sendSaveBusPosition = new cMessage("Event get edges position", SendEvtSaveBusPosition);
        //cout << source << " at: " << simTime() << " schedule created SendEvtSaveEdgePosition to: "<< simTime() << endl;
        scheduleAt(simTime(), sendSaveBusPosition);
    }
}

void vehDist::busPosInsertByTimeMap() {
    if (par("getBusPosition").boolValue()) {
        if (vehCategory.compare("B") == 0) {
            int timeNow = int(simTime().dbl());
            struct busPosByTime structBusPosByTime;
            map <simtime_t, Coord> mapTimePos;
            Coord positionNow = mobility->getPositionAt(simTime());

            cout << "\ntraciVehicle->getRoadId(): " << traciVehicle->getRoadId();
            cout << "\ntraciVehicle->getRouteId(): " << traciVehicle->getRouteId();
            cout << "\ntraciVehicle->getLanePosition(): " << traciVehicle->getLanePosition();

            if (SposTimeBus.find(routeId) == SposTimeBus.end()) {
                mapTimePos.insert(make_pair(timeNow, positionNow));
                structBusPosByTime.timePos = mapTimePos;

                SposTimeBus.insert(make_pair(routeId, structBusPosByTime));
            } else {
                map <simtime_t, Coord>::iterator itTimePos;

                unordered_map <string, busPosByTime>::iterator itSp = SposTimeBus.find(routeId);
                structBusPosByTime = itSp->second;
                mapTimePos = structBusPosByTime.timePos;

                //positionBack = (mapTimePos.rbegin())->second;
                //positionBack = (--mapTimePos.end())->second;

                mapTimePos.insert(make_pair(timeNow, positionNow));

                structBusPosByTime.timePos = mapTimePos;
                SposTimeBus[routeId] = structBusPosByTime;
            }
        }
    }
}

void vehDist::saveBusPositionFile() {
    if (par("getBusPosition").boolValue()) {
        if (vehCategory.compare("B") == 0) {
            struct busPosByTime structBusPosByTime;
            map <simtime_t, Coord> mapTimePos;
            unordered_map <string, busPosByTime>::iterator itSp;

            string fileName = "results/busPosition.csv";
            myfile.open(fileName, std::ios_base::app);

            myfile << "\nrouteId position time\n" << routeId << "\n";
            itSp = SposTimeBus.find(routeId);
            structBusPosByTime = itSp->second;
            mapTimePos = structBusPosByTime.timePos;

            map <simtime_t, Coord>::iterator itTimePos;
            for (itTimePos = mapTimePos.begin(); itTimePos != mapTimePos.end(); itTimePos++){
                myfile << itTimePos->first << " " << itTimePos->second << "\n";
            }

            if (SnumVehicles.size() == 0) {
                myfile << "\n";
            }
            myfile.close();
        }
    }
}

void vehDist::busPosLoadFromFile() {
    if (source.compare("car[0]") == 0) {
        if (par("useBusPosition").boolValue()) {
            //cout << "\nLoading routes from of the bus";
            string fileInput, line;

            fileInput = par("fileBusPositionPath").stringValue();
            if (fileInput.compare("") == 0){
                cout << "\nbusPosition fileInput is empty - need pass the path to \"busPosition.csv\"\n\n";
                ASSERT2(0, "JBe - fileInput is empty - need pass the path to \"busPosition.csv\" -" );
            }

            freopen(fileInput.c_str(), "r", stdin);

            int countPos = 0;
            string timeString, routeIDTmp;
            Coord positionBus;
            size_t pos;
            simtime_t simTimeTmp;
            map <simtime_t, Coord> timePosTmp;

            while (getline(cin, line, '\n')) {
                //cout << "\nline: " << line;

                // (countPos == 0) // blank line
                // (countPos == 1) // routeId position time

                if (countPos == 2) {
                    timePosTmp.clear();
                    routeIDTmp = line;
                    //cout << "\n\nrouteIDTmp: " << routeIDTmp << "\n";
                } else if (countPos == 3) {
                    if (line.size() == 0) {
                        countPos = 3;
                    } else {
                        pos = line.find(' '); //line => 90 (2125.96,998.97,1.895)
                        timeString = line.substr(0, pos);
                        //cout << "\ntimeString: " << timeString;

                        string lineTmp = line.substr(pos + 1, std::string::npos);
                        //cout << "\nEdgePosition: " << lineTmp; // (2125.96,998.97,1.895)
                        pos = lineTmp.find(','); // (1787.24,
                        positionBus.x = atof(lineTmp.substr(1, pos - 1).c_str());
                        //cout << "\npositionStart.x: " << positionStart.x;

                        lineTmp = lineTmp.substr(pos +1, std::string::npos);
                        pos = lineTmp.find(','); // 998.97,1.895)
                        positionBus.y = atof(lineTmp.substr(0, pos).c_str());
                        //cout << "\npositionStart.y: " << positionStart.y;

                        lineTmp = lineTmp.substr(pos + 1, std::string::npos);
                        pos = lineTmp.find(','); // 1.895)
                        positionBus.z = atof(lineTmp.substr(0, lineTmp.size() - 1).c_str());
                        //cout << "\npositionStart.z: " << positionStart.z;

                        //cout << atoi(timeString.c_str()) << " " << positionBus << "\n";

                        simTimeTmp = atoi(timeString.c_str());
                        timePosTmp.insert(make_pair(simTimeTmp, positionBus));
                        countPos = 2;
                    }
                }
                countPos++;

               if (countPos == 4) {
                   struct busPosByTime structBusPosByTime;
                   structBusPosByTime.timePos = timePosTmp;

                   SposTimeBusLoaded.insert(make_pair(routeIDTmp, structBusPosByTime));
                   countPos = 1;
                   //cout << "\n" << routeIDTmp;
               }

            }
        }
    }
}

void vehDist::onBeacon(WaveShortMessage* wsm) {
    unordered_map <string, WaveShortMessage>::iterator itStatusNeighbors = beaconStatusNeighbors.find(wsm->getSource());
    if (itStatusNeighbors != beaconStatusNeighbors.end()) { // Update the beaconStatus
        itStatusNeighbors->second = *wsm;
    } else {
        if (SuseBeaconStatusBufferSize) {
            if (beaconStatusNeighbors.size() > SbeaconStatusBufferSize) {
                removeOldestInputBeaconStatus();
            }
        }
        beaconStatusNeighbors.insert(make_pair(wsm->getSource(), *wsm));
        sendMessageToOneNeighborTarget(wsm->getSource()); // Look in buffer it has messages for this new vehNeighbor

        sendMessageDeliveryBuffer(wsm->getSource());
    }
    //printBeaconStatusNeighbors();
}

void vehDist::onData(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) { // Verify if this is the recipient of the message
        //saveMessagesOnFile(wsm, SfileMessagesUnicastVeh);

        // Real scenario
        //if (source.compare(wsm->getTarget()) == 0) { // Message to this vehicle
        //
        //} else ...

        if (wsm->getOnlyDelivery()) {
            cout << "vehCategory: " << vehCategory << "\n";
            cout << source << " with one message ToDelivery: " << wsm->getOnlyDelivery() << " target: " << wsm->getTarget() << " at: " << simTime() << endl;

            busMsgToDelivery += source + string(" msg: ") + wsm->getGlobalMessageIdentificaton() + string(" at: ") + to_string(simTime().dbl());
            busMsgToDelivery += string(" to ") + wsm->getTarget() + string("\n");
        }

        bool insert = sendOneNewMessageToOneNeighborTarget(*wsm); // Look in neigborStatus buffer if has the target of this message
        if (insert) {
            if (wsm->getOnlyDelivery()) {
                map <string, WaveShortMessage> messagesTmp;
                messagesToDelivery msgToDeliveryTmp;

                if (messagesBufferToDelivery.find(wsm->getTarget()) == messagesBufferToDelivery.end()) {
                    messagesTmp.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));

                    msgToDeliveryTmp.messages = messagesTmp;

                    messagesBufferToDelivery.insert(make_pair(wsm->getTarget(), msgToDeliveryTmp));
                } else {
                    msgToDeliveryTmp = messagesBufferToDelivery[wsm->getTarget()];
                    messagesTmp = msgToDeliveryTmp.messages;

                    if (messagesTmp.find(wsm->getGlobalMessageIdentificaton()) == messagesTmp.end()) {
                        messagesTmp.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
                        msgToDeliveryTmp.messages = messagesTmp;
                        messagesBufferToDelivery[wsm->getTarget()] = msgToDeliveryTmp;
                    }
                }
            } else {
                // test if message has been delivered to the target before.
                if (find(messagesDelivered.begin(), messagesDelivered.end(), wsm->getGlobalMessageIdentificaton()) == messagesDelivered.end()) {
                    if (wsm->getHopCount() > 0) {
                        if (messagesBufferVehDist.find(wsm->getGlobalMessageIdentificaton()) == messagesBufferVehDist.end()) { //verify if the message isn't in the buffer
                            cout << source << " saving message from " << wsm->getSenderAddressTemporary() << " with Id: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
                            string wsmDataTmp = wsm->getWsmData();
                            if (wsmDataTmp.size() < 42) { // WSMData generated by car[x] and carry by [ T, P
                                wsmDataTmp += " and carry by (";
                            } else {
                                wsmDataTmp += ", ";
                            }
                            wsmDataTmp += vehCategory;
                            wsm->setWsmData(wsmDataTmp.c_str());

                            if (messagesBufferVehDist.size() > SmessageBufferSize) {
                                removeOldestInputBeaconMessage();
                            }

                            messagesBufferVehDist.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm)); // Add the message in the vehicle buffer
                            msgBufferUse++;
                            messagesOrderReceivedVehDist.push_back(wsm->getGlobalMessageIdentificaton());

                            //colorCarryMessageVehDist(messagesBufferVehDist);
                            //printMessagesBuffer();
                        } else {
                            cout << source << " message (" << wsm->getGlobalMessageIdentificaton() << ") is on the buffer at: " << simTime() << endl;
                        }
                    } else { // wsm->getHopCount() <= 0
                        cout << "JBe - Error: received message with hopCount <= 0, hopCount: " << wsm->getHopCount() << " at: " << simTime() << " - " << endl;
                        ASSERT2(0, "JBe - Error: received message with hopCount <= 0 -");
                    }
                } else {
                    cout << source << " message with Id: " << wsm->getGlobalMessageIdentificaton() << " from " << wsm->getSenderAddressTemporary() << " has been delivered to the target before" << endl;
                }
            }
        }
    }/*else { // Message to another vehicle
        cout << "Saving broadcast message from: " << wsm->getSenderAddressTemporary() << " to " << source << endl;
        saveMessagesOnFile(wsm, fileMessagesBroadcast);
    }*/
    //printMessagesBuffer();
}

void vehDist::removeOldestInputBeaconMessage() {
    //printMessagesBuffer();

    if (!messagesBufferVehDist.empty()) {
        unsigned short int typeRemoved = 0;
        unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBufferVehDist.begin();
        simtime_t minTime = itMessage->second.getTimestamp();
        string idMessage = itMessage->first;
        itMessage++;

        for (; itMessage != messagesBufferVehDist.end(); itMessage++) {
            if (minTime > itMessage->second.getTimestamp()) {
                minTime = itMessage->second.getTimestamp();
                idMessage = itMessage->first;
            }
        }

        if (simTime() > (minTime + SttlMessage)) {
            cout << source << " remove one message (" << idMessage << ") by time, minTime: " << minTime << " at: " << simTime();
            cout << " ttlMessage: " << SttlMessage << endl;
            typeRemoved = 3; // by ttl (1 buffer, 2 copy, 3 time)
        } else if (messagesBufferVehDist.size() > SmessageBufferSize) {
            //cout << source << " remove one message (" << idMessage << ") by space, MessageBufferVehDist.size(): " << messagesBufferVehDist.size();
            //cout << " at: " << simTime() << " SmessageBufferSize: " << SmessageBufferSize << endl;
            typeRemoved = 1; // by buffer (1 buffer, 2 copy, 3 time)
        }

        if (typeRemoved != 0) {
            insertMessageDropVeh(idMessage, typeRemoved, messagesBufferVehDist[idMessage].getTimestamp()); // Removed by the value of tyRemoved (1 buffer, 2 copy, 3 time)
            messagesBufferVehDist.erase(idMessage);
            auto itMsg = find(messagesOrderReceivedVehDist.begin(), messagesOrderReceivedVehDist.end(), idMessage);
            messagesOrderReceivedVehDist.erase(itMsg);

            //colorCarryMessageVehDist(messagesBufferVehDist);
            removeOldestInputBeaconMessage();
        }
    } /*else {
        cout << "messagesBuffer from " << source << " is empty now" << endl;
    }*/
}

void vehDist::removeOldestInputBeaconStatus() {
    //printBeaconStatusNeighbors();

    if (!beaconStatusNeighbors.empty()) {
        unordered_map <string, WaveShortMessage>::iterator itBeaconStatus;
        itBeaconStatus = beaconStatusNeighbors.begin();
        simtime_t minTime = itBeaconStatus->second.getTimestamp();
        string idBeacon = itBeaconStatus->first;
        itBeaconStatus++;

        for (; itBeaconStatus != beaconStatusNeighbors.end(); itBeaconStatus++) {
            if (minTime > itBeaconStatus->second.getTimestamp()) {
                minTime = itBeaconStatus->second.getTimestamp();
                idBeacon = itBeaconStatus->first;
            }
        }

        if (simTime() > (minTime + SttlBeaconStatus)) {
            //cout << source << " remove one beaconStatus (" << idBeacon << ") by time, minTime: " << minTime << " at: " << simTime();
            //cout << " SttlBeaconStatus: " << SttlBeaconStatus << endl;
            beaconStatusNeighbors.erase(idBeacon);
            removeOldestInputBeaconStatus();
        } else if (SuseBeaconStatusBufferSize) {
            if (beaconStatusNeighbors.size() > SbeaconStatusBufferSize) {
                //cout << source << " remove one beaconStatus (" << idBeacon << ") by space, beaconStatusNeighbors.size(): " << beaconStatusNeighbors.size();
                //cout << " at: " << simTime() << " SbeaconStatusBufferSize: " << SbeaconStatusBufferSize << endl;
                beaconStatusNeighbors.erase(idBeacon);
                removeOldestInputBeaconStatus();
            }
        }
    } /*else {
        cout << "beaconStatusNeighbors from " << source << " is empty now" << endl;
    }*/
}

void vehDist::sendBeaconMessage() {
    //cout << source << " try to send message at: " << simTime() << endl;
//############################################################# run many times
    removeOldestInputBeaconStatus();
    removeOldestInputBeaconMessage();

//    unordered_map<string, WaveShortMessage>::iterator itbeaconStatus;
//    for (itbeaconStatus = beaconStatusNeighbors.begin(); itbeaconStatus != beaconStatusNeighbors.end(); itbeaconStatus++){
//        sendMessageToOneNeighborTarget(itbeaconStatus->second.getSource()); // Look in buffer it has messages for this vehicle neighbor
//    }
//############################################################# run many times

    if (messageToSend >= messagesOrderReceivedVehDist.size()) {
        messageToSend = 0;
    }

    //printMessagesBuffer();
    trySendBeaconMessage();
    //printMessagesBuffer();
    messageToSend++; // Move to next message

    if (!SuseRateTimeToSend) {
        //cout << source << " schedule useRateTimeToSend: false the sendBeaconMessageEvt at: " << simTime() << " to: " << (simTime() + par("normalTimeSendMessage").doubleValue()) << endl;
        scheduleAt((simTime() + par("normalTimeSendMessage").doubleValue()), sendBeaconMessageEvt);
    } else {
        if (messageToSend >= messagesOrderReceivedVehDist.size()) {
            if (simTime() > timeToFinishLastStartSend) {
                scheduleAt((simTime() + double(rateTimeToSend)/1000), sendBeaconMessageEvt);
                //cout << source << " 1_schedule at: " << simTime() << " to: " << (simTime() + double(rateTimeToSend)/1000);
            } else {
                scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
                //cout << source << " 2_schedule at: " << simTime() << " to: " << timeToFinishLastStartSend;
            }

            timeToFinishLastStartSend += double(rateTimeToSendLimitTime)/1000;
        } else {
            scheduleAt((simTime() + double(rateTimeToSend)/1000), sendBeaconMessageEvt);
            //cout << source << " 3_schedule at: " << simTime() << " to: " << (simTime() + double(rateTimeToSend)/1000) << " rateTimeToSend: " << rateTimeToSend;
        }
        //cout << " timeToFinishLastStartSend: " << timeToFinishLastStartSend << endl;
        //cout << "                               " << source << " expSendbyDSCR: " << SexpSendbyDSCR << " at: " << simTime() << endl << endl;
    }
}

void vehDist::trySendBeaconMessage() {
    if (!messagesBufferVehDist.empty()) {
        cout << source << " messagesBuffer with "<< messagesBufferVehDist.size() << " message(s) at: " << simTime() << endl;
        // Test with hopCount 0, send only

        if (!beaconStatusNeighbors.empty()) {
            //printBeaconStatusNeighbors();
            // The same range to the RSU and veh

            msgOnlyDelivery = false;
            string idMessage, rcvId = source;
            idMessage = messagesOrderReceivedVehDist[messageToSend];
            if (messagesBufferVehDist[idMessage].getHopCount() > 1) {
                rcvId = neighborWithShortestDistanceToTarge(idMessage);
            } else {
                rcvId = neighborWithShortestDistanceToTargeOnlyDelivery(idMessage);
            }

            string catVeh;
            if (source.compare(rcvId) != 0) {
                unordered_map <string, WaveShortMessage>::iterator itBeaconN;
                itBeaconN = beaconStatusNeighbors.find(rcvId);
                catVeh = itBeaconN->second.getCategory();

                cout << source << " chose the vehicle " << rcvId << " in the expSendbyDSCR " << SexpSendbyDSCR << " to be a next hop to the " << idMessage << " message" << endl;
                cout << "    " << source << " send message to " << rcvId << " with category " << catVeh << " at: "<< simTime() << endl;
                cout << "    MessageToSend: " << messageToSend << endl;
                cout << "    MessageID: " << idMessage << endl;
                cout << "    Source: " << messagesBufferVehDist[idMessage].getSource() << endl;
                cout << "    Message content: " << messagesBufferVehDist[idMessage].getWsmData() << endl;
                cout << "    Target: " << messagesBufferVehDist[idMessage].getTarget() << endl;
                cout << "    TargetPos: " << messagesBufferVehDist[idMessage].getTargetPos() << endl;
                cout << "    Timestamp: " << messagesBufferVehDist[idMessage].getTimestamp() << endl;
                cout << "    HopCount: " << messagesBufferVehDist[idMessage].getHopCount() << endl << endl;

                WaveShortMessage wsmTmpSend = messagesBufferVehDist[idMessage];
                // toDeliveryMsg is set when send to a bus (expSend13)
                wsmTmpSend.setOnlyDelivery(msgOnlyDelivery);

                sendWSM(updateBeaconMessageWSM(wsmTmpSend.dup(), rcvId));
                ScountMsgPacketSend++;

                if ((!SallowMessageCopy) || (msgOnlyDelivery)) {
                    cout << source << " send the message " << idMessage << " and removing (message copy not allow) at: "  << simTime() << endl;

                    insertMessageDropVeh(idMessage, 2, messagesBufferVehDist[idMessage].getTimestamp()); // Removed by the value of tyRemoved (1 buffer, 2 copy, 3 time)

                    messagesBufferVehDist.erase(idMessage);
                    messageToSend--;

                    auto it = find(messagesOrderReceivedVehDist.begin(), messagesOrderReceivedVehDist.end(), idMessage);
                    messagesOrderReceivedVehDist.erase(it);
                    //colorCarryMessageVehDist(messagesBufferVehDist);
                }

                if (SuseMessagesSendLog) {
                    if (messagesSendLog.empty()) {
                        messagesSendLog.insert(make_pair(idMessage, rcvId));
                    } else {
                        messagesSendLog[idMessage] += ", " + rcvId;
                    }
                }
            } else {
                cout << source << " not send any message" << endl << endl;
            }
        } else {
            cout << source << " beaconNeighbors on sendDataMessage is empty now" << endl << endl;
        }
    } /*else {
        cout << source << " messagesBuffer is empty at: " << simTime() << endl;
    }*/
}

string vehDist::chooseCategory_RandomNumber1to100(unsigned short int percentP, string vehIdP, string vehIdT) {
    unsigned short int valRand = rand() % 100 + 1;
    //cout << "valRand " << valRand;
    if (valRand <= percentP) {
        //cout << " return vehIdP" << endl;
        return vehIdP;
    } else { // valRand > percentP
        //cout << " return vehIdT" << endl;
        return vehIdT;
    }
}

string vehDist::neighborWithShortestDistanceToTargeOnlyDelivery(string idMessage) {
    unordered_map <string, WaveShortMessage>::iterator itBeaconNeighbors;

    for (itBeaconNeighbors = beaconStatusNeighbors.begin(); itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
        if (strcmp(itBeaconNeighbors->second.getSource(), messagesBufferVehDist[idMessage].getTarget()) == 0) {
            cout << source << " found target, message " << idMessage << " target " << messagesBufferVehDist[idMessage].getTarget() << " at: " << simTime() << endl;
            return itBeaconNeighbors->second.getSource();
        }
    }
    return source;
}
string vehDist::neighborWithShortestDistanceToTarge(string idMessage) {
    string category, neighborCategory, neighborId;
    Coord targetPos, senderPosPrevious, senderPosNow;
    shortestDistance sD;
    unsigned short int meetFirstCategory, meetSecondCategory;
    double neighborDistanceBefore, neighborDistanceNow;
    unordered_map <string, shortestDistance> vehShortestDistanceToTarget;
    unordered_map <string, WaveShortMessage>::iterator itBeaconNeighbors;

    meetFirstCategory = meetSecondCategory = 0;
    bool insert;

    targetPos = messagesBufferVehDist[idMessage].getTargetPos();

    cout << source << " meet with " << beaconStatusNeighbors.size() << " neighbors options to the send the message " << idMessage << endl;
    for (itBeaconNeighbors = beaconStatusNeighbors.begin(); itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
        neighborCategory = itBeaconNeighbors->second.getCategory();
        neighborId = itBeaconNeighbors->first;

        if (neighborCategory.compare("rsu") != 0) { // When use more than one rsu, will ignore the RSU in the neighborID
            if (!messagesSendLog.empty() && (messagesSendLog[idMessage].find(neighborId) != std::string::npos)) {
                cout << source << " has been send a message " << idMessage << " to " << neighborId << endl;
            } else {
                if (SexpSendbyDSCR == 13) {
                    if (neighborCategory.compare("B") == 0) {
                        cout << "\n\n" << source << " whit neighbor: "<< neighborId << " category: " << neighborCategory;
                        cout << " msg to: " << messagesBufferVehDist[idMessage].getTarget() << " with pos: " << targetPos;
                        msgOnlyDelivery = busRouteDiffTarget(neighborId, targetPos);
                        if (msgOnlyDelivery) {
                            cout << "send by busTestEdgeRoute to "<< neighborId << "\n\n";
                            return neighborId;
                        }
                        cout << "\n\n";
                    }
                }

                senderPosPrevious = itBeaconNeighbors->second.getSenderPosPrevious();
                senderPosNow = itBeaconNeighbors->second.getSenderPos();

                neighborDistanceBefore = traci->getDistance(senderPosPrevious, targetPos, false);
                neighborDistanceNow = traci->getDistance(senderPosNow, targetPos, false);

                cout << "    1337 " << neighborId << " cat: " << neighborCategory <<  " DistB: " << neighborDistanceBefore << " DistN: " << neighborDistanceNow << endl;

                insert = true;
                if (SusePathHistory) { // True will check if the vehicle is closing to target (message target destination)
                    if (neighborDistanceNow > neighborDistanceBefore) { // Test if is closing to target
                        insert = false;
                        //cout << itBeaconNeighbors->first << " going to another direction" << endl;
                    }
                }

                if (insert) {
                    sD.categoryVeh = neighborCategory;
                    sD.distanceToTargetNow = neighborDistanceNow;
                    sD.distanceToTargetBefore = neighborDistanceBefore;
                    sD.senderPos = senderPosNow;
                    sD.speedVeh = itBeaconNeighbors->second.getSenderSpeed();
                    sD.rateTimeToSendVeh = itBeaconNeighbors->second.getRateTimeToSend();

                    if (sD.categoryVeh.compare(SfirstCategory) == 0) { // P
                        sD.distanceToTargetCategory = sD.distanceToTargetNow; // equal to * 1
                        meetFirstCategory = 1;
                    } else if (sD.categoryVeh.compare(SsecondCategory) == 0) { // B or T
                        sD.distanceToTargetCategory = sD.distanceToTargetNow * 0.90;
                        meetSecondCategory = 1;
                    } else {
                        cout << endl << "JBe - Error category unknown - " << source << " category: " << sD.categoryVeh << endl;
                        cout << "firstCategory: " << SfirstCategory << " secondCategory: " << SsecondCategory << endl;
                        ASSERT2(0, "JBe - Error category unknown -");
                    }

                    sD.decisionValueDistanceSpeed = sD.distanceToTargetNow - (sD.speedVeh);
                    sD.decisionValueDistanceRateTimeToSend = sD.distanceToTargetNow + (double(sD.rateTimeToSendVeh)/100);
                    sD.decisionValueDistanceSpeedRateTimeToSend = sD.distanceToTargetNow - (sD.speedVeh) + (double(sD.rateTimeToSendVeh)/100);

                    // Distance = [0 - 125] - 720 m
                    // vehicle Speed = 0 - (16.67/25) - 84 m/s
                    // rateTimeToSend = 100 to 5000 ms
                    // DecisonValueDS = distance - speed
                    // DecisonValueDSCR = distance - speed + rateTimeToSend/100 (0.1 * 10)

                    vehShortestDistanceToTarget.insert(make_pair(neighborId, sD));
                } else {
                    cout << "\n\n    " << neighborId << " not insert in the vehShortestDistanceToTarget\n\n";
                }

                if ((meetFirstCategory == 1) && (meetSecondCategory == 1)) {
                    ScountTwoCategoryN++;
                    //cout << endl << "Meet of vehicles of two categories" << endl;
                } else if ((meetFirstCategory == 1) || (meetSecondCategory == 1)) {
                    ScountMeetN++;
                }
            }
        }
    }

    printVehShortestDistanceToTarget(vehShortestDistanceToTarget, idMessage);

    if (SusePathHistory) {
        if (vehShortestDistanceToTarget.empty()) {
            cout << source << " vehShortestDistanceToTarget is empty. SusePathHistory is true\n";
            cout << "Select the more closest to target to take good use of this meet";

            itBeaconNeighbors = beaconStatusNeighbors.begin();
            neighborId = itBeaconNeighbors->first;
            neighborDistanceBefore = traci->getDistance(itBeaconNeighbors->second.getSenderPos(), targetPos, false);
            itBeaconNeighbors++;

            for (; itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
                neighborDistanceNow = traci->getDistance(itBeaconNeighbors->second.getSenderPos(), targetPos, false);

                if (neighborDistanceBefore > neighborDistanceNow) {
                    neighborDistanceBefore = neighborDistanceNow;
                    neighborId = itBeaconNeighbors->first;
                }
            }

            cout << "\nThe vehicle more closest is " << neighborId << endl;
            return neighborId;
        }
    }

    neighborId = source;
    if (!vehShortestDistanceToTarget.empty()) {
        switch (SexpSendbyDSCR) {
            case 1:
                neighborId = chooseByDistance(vehShortestDistanceToTarget);
                break;
            case 12:
                neighborId = chooseByDistance_Speed(vehShortestDistanceToTarget);
                break;
            case 13:
                //neighborId = chooseByDistance_CategoryA(vehShortestDistanceToTarget, SpercentP);

                //neighborId = chooseByDistance_CategoryB(vehShortestDistanceToTarget, SpercentP);

                neighborId = chooseByDistance(vehShortestDistanceToTarget);
                break;
            case 14:
                neighborId = chooseByDistance_RateTimeToSend(vehShortestDistanceToTarget);
                break;
            case 123:
                neighborId = chooseByDistance_Speed_Category(vehShortestDistanceToTarget, SpercentP);
                break;
////######################################################################################################
//            case 124:
//                neighborId = chooseByDistance_Speed_RateTimeToSend(vehShortestDistanceToTarget, SpercentP);
//                break;
//            case 134:
//                neighborId = chooseByDistance_Category_RateTimeToSend(vehShortestDistanceToTarget, SpercentP);
//                break;
////######################################################################################################
            case 1234:
                neighborId = chooseByDistance_Speed_Category_RateTimeToSend(vehShortestDistanceToTarget, SpercentP);
                break;
            default:
                cout << "JBe - Error! expSendbyDSCR: " << SexpSendbyDSCR << "not defined, class in vehDist.cc -";
                ASSERT2(0, "JBe - Error expSendbyDSCR value not defined -");
            }
    }

    return neighborId;
}

void vehDist::printVehShortestDistanceToTarget(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, string idMessage) {
    if (!vehShortestDistanceToTarget.empty()) {
        cout << "\nMeet with " << vehShortestDistanceToTarget.size() << " options to a send " << idMessage << " message" << endl;
        cout << "Source vehicle information" << endl;
        cout << "    Id(veh): " << source << endl;
        cout << "    Message Id: " << messagesBufferVehDist[idMessage].getGlobalMessageIdentificaton() << endl;
        cout << "    Category: " << vehCategory << endl;
        cout << "    Distance: " << traci->getDistance(curPosition, messagesBufferVehDist[idMessage].getTargetPos(), false) << endl;
        cout << "    Speed: " << mobility->getSpeed() << endl;
        cout << "    Position: " << mobility->getCurrentPosition() << endl;
        cout << "    RateTimeToSend: " << rateTimeToSend << endl;

        cout << endl << "Printing vehShortestDistanceToTarget to " << source << " at: " << simTime() << endl;
        unordered_map <string, shortestDistance>::iterator itShortestDistance;
        for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
            cout << "    Id(veh): " << itShortestDistance->first << endl;
            cout << "    Position: " << itShortestDistance->second.senderPos << endl;
            cout << "    Category: " << itShortestDistance->second.categoryVeh << endl;
            cout << "    DistanceBefore: " << itShortestDistance->second.distanceToTargetBefore << endl;
            cout << "    DistanceNow:    " << itShortestDistance->second.distanceToTargetNow << endl;
            cout << "    Speed: " << itShortestDistance->second.speedVeh << endl;
            cout << "    RateTimeToSend: " << itShortestDistance->second.rateTimeToSendVeh << endl;
            cout << "    DistanceCategory: " << itShortestDistance->second.distanceToTargetCategory << endl;
            cout << "    DecisionValueDistanceSpeed: " << itShortestDistance->second.decisionValueDistanceSpeed << endl;
            cout << "    decisionValueDistanceRateTimeToSend: " << itShortestDistance->second.decisionValueDistanceRateTimeToSend << endl;
            cout << "    DecisionValueDistanceSpeedRateTimeToSend: " << itShortestDistance->second.decisionValueDistanceSpeedRateTimeToSend << endl << endl;
        }
    } else {
        cout << endl << "vehShortestDistanceToTarget to " << source << " at: " << simTime() << " vehShortestDistanceToTarget is empty" << endl << endl;
    }
}

string vehDist::chooseByDistance(unordered_map <string, shortestDistance> vehShortestDistanceToTarget) {
    unordered_map <string, shortestDistance>::iterator itShortestDistance;
    double distanceToTarget, shortestDistanceToTarget;

    string vehId = source;
    shortestDistanceToTarget = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceToTarget = itShortestDistance->second.distanceToTargetNow;
        if (shortestDistanceToTarget > distanceToTarget) {
            shortestDistanceToTarget = distanceToTarget;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chooseByDistance_Speed(unordered_map <string, shortestDistance> vehShortestDistanceToTarget) {
    unordered_map <string, shortestDistance>::iterator itShortestDistance;
    double distanceSpeedValue, shortestDistanceSpeedValue;

    string vehId = source;
    shortestDistanceSpeedValue = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceSpeedValue = itShortestDistance->second.decisionValueDistanceSpeed;
        if (shortestDistanceSpeedValue > distanceSpeedValue) {
            shortestDistanceSpeedValue = distanceSpeedValue;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chooseByDistance_CategoryA(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    unordered_map <string, shortestDistance>::iterator itShortestDistance;
    double distanceToTarget, shortestDistanceT, shortestDistanceP;
    string category, vehIdP, vehIdT;

    vehIdP = vehIdT = source;
    shortestDistanceP = shortestDistanceT = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        category = itShortestDistance->second.categoryVeh;
        distanceToTarget = itShortestDistance->second.distanceToTargetNow;
        if (category.compare(SfirstCategory) == 0) {
            if (shortestDistanceP > distanceToTarget) {
                shortestDistanceP = distanceToTarget;
                vehIdP = itShortestDistance->first;
            }
        } else if (category.compare(SsecondCategory) == 0) {
            if (shortestDistanceT > distanceToTarget) {
                shortestDistanceT = distanceToTarget;
                vehIdT = itShortestDistance->first;
            }
        }
    }

    if (shortestDistanceP == DBL_MAX) {
        return vehIdT;
    } else if (shortestDistanceT == DBL_MAX) {
        return vehIdP;
    }

    if (int(shortestDistanceT) > int(shortestDistanceP)) {
        ScountMeetPshortestT++;
        return chooseCategory_RandomNumber1to100(percentP, vehIdP, vehIdT);
    } else {
        return vehIdT;
    }
}

string vehDist::chooseByDistance_CategoryB(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    string vehId = source;
    double distanceToTargetCategory, shortestDistanceCategory;
    shortestDistanceCategory = DBL_MAX;

    unordered_map <string, shortestDistance>::iterator itShortestDistance = vehShortestDistanceToTarget.begin();
    for (; itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceToTargetCategory = itShortestDistance->second.distanceToTargetCategory;

        if (shortestDistanceCategory > distanceToTargetCategory) {
            shortestDistanceCategory = distanceToTargetCategory;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chooseByDistance_RateTimeToSend(unordered_map <string, shortestDistance> vehShortestDistanceToTarget) {
    unordered_map <string, shortestDistance>::iterator itShortestDistance;
    double distanceRateTimeToSendValue, shortestDistanceRateTimeToSendValue;
    string vehId = source;

    shortestDistanceRateTimeToSendValue = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        distanceRateTimeToSendValue = itShortestDistance->second.decisionValueDistanceRateTimeToSend;
        if (shortestDistanceRateTimeToSendValue > distanceRateTimeToSendValue) {
            shortestDistanceRateTimeToSendValue = distanceRateTimeToSendValue;
            vehId = itShortestDistance->first;
        }
    }
    return vehId;
}

string vehDist::chooseByDistance_Speed_Category(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    unordered_map <string, shortestDistance>::iterator itShortestDistance;
    double distanceSpeedValue, shortestDistanceT, shortestDistanceP;
    string category, vehIdP, vehIdT;

    vehIdP = vehIdT = source;
    shortestDistanceP = shortestDistanceT = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        category = itShortestDistance->second.categoryVeh;
        distanceSpeedValue = itShortestDistance->second.decisionValueDistanceSpeed;
        if (category.compare(SfirstCategory) == 0) {
            if (shortestDistanceP > distanceSpeedValue) {
                shortestDistanceP = distanceSpeedValue;
                vehIdP = itShortestDistance->first;
            }
        } else if (category.compare(SsecondCategory) == 0) {
            if (shortestDistanceT > distanceSpeedValue) {
                shortestDistanceT = distanceSpeedValue;
                vehIdT = itShortestDistance->first;
            }
        }
    }

    if (shortestDistanceP == DBL_MAX) {
        return vehIdT;
    } else if (shortestDistanceT == DBL_MAX) {
        return vehIdP;
    }

    if (int(shortestDistanceT) > int(shortestDistanceP)) {
        ScountMeetPshortestT++;
        return chooseCategory_RandomNumber1to100(percentP, vehIdP, vehIdT);
    } else {
        return vehIdT;
    }
}

string vehDist::chooseByDistance_Speed_Category_RateTimeToSend(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP) {
    unordered_map <string, shortestDistance>::iterator itShortestDistance;
    double valueDSCR, shortestDistanceT, shortestDistanceP;
    string category, vehIdP, vehIdT;

    vehIdP = vehIdT = source;
    shortestDistanceP = shortestDistanceT = DBL_MAX;
    for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
        category = itShortestDistance->second.categoryVeh;
        valueDSCR = itShortestDistance->second.decisionValueDistanceSpeedRateTimeToSend;
        if (category.compare(SfirstCategory) == 0) {
            if (shortestDistanceP > valueDSCR) {
                shortestDistanceP = valueDSCR;
                vehIdP = itShortestDistance->first;
            }
        } else if (category.compare(SsecondCategory) == 0) {
            if (shortestDistanceT > valueDSCR) {
                shortestDistanceT = valueDSCR;
                vehIdT = itShortestDistance->first;
            }
        }
    }

    if (shortestDistanceP == DBL_MAX) {
        return vehIdT;
    } else if (shortestDistanceT == DBL_MAX) {
        return vehIdP;
    }

    if (int(shortestDistanceT) > int(shortestDistanceP)) {
        ScountMeetPshortestT++;
        return chooseCategory_RandomNumber1to100(percentP, vehIdP, vehIdT);
    } else {
        return vehIdT;
    }
}

void vehDist::finish() {
    toFinishVeh();

    if (vehCategory.compare("B") == 0) {
        myfile.open(SfileMessagesCountRsu, std::ios_base::app);
        cout << "\nbusMsgToDelivery from " << source << "\n" << busMsgToDelivery << "\n";
        myfile << "\nbusMsgToDelivery from " << source << "\n" << busMsgToDelivery << "\n";
        myfile.close();
    }

    saveBusPositionFile();
}

void vehDist::sendMessageToOneNeighborTarget(string beaconSource) {
    unsigned short int countMessage = messagesBufferVehDist.size();
    unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBufferVehDist.begin();
    string idMessage;
    while (countMessage > 0) {
        if (beaconSource.compare(itMessage->second.getTarget()) == 0) {
            idMessage = itMessage->second.getGlobalMessageIdentificaton();
            cout << source << " sending MessageToOneNeighborTarget: " << idMessage << " to: " << beaconSource << " and removing from messagesBuffer" << endl;

            sendWSM(updateBeaconMessageWSM(itMessage->second.dup(), beaconSource));
            ScountMsgPacketSend++;
            messagesDelivered.push_back(idMessage);

            if (countMessage == 1) {
                countMessage = 0;
            } else {
                countMessage--;
                itMessage++;
            }

            messagesBufferVehDist.erase(idMessage);
            auto it = find(messagesOrderReceivedVehDist.begin(), messagesOrderReceivedVehDist.end(), idMessage);
            if (it != messagesOrderReceivedVehDist.end()) {
                messagesOrderReceivedVehDist.erase(it);
            } else {
                cout << endl << source << "JBe - Error in messagesOrderReceived, need to have the same entries as messagesBuffer -" << endl;
                ASSERT2(0, "JBe - Error in messagesOrderReceived, need to have the same entries as messagesBuffer -");
            }
            //colorCarryMessageVehDist(messagesBufferVehDist);
        } else {
            countMessage--;
            itMessage++;
        }
    }
}

void vehDist::sendMessageDeliveryBuffer(string beaconSource) {
    if (messagesBufferToDelivery.find(beaconSource) != messagesBufferToDelivery.end()) {
        messagesToDelivery msgToDeliveryTmp = messagesBufferToDelivery.find(beaconSource)->second;
        map <string, WaveShortMessage> messagesTmp = msgToDeliveryTmp.messages;

        unsigned short int countMessage = messagesTmp.size();
        map <string, WaveShortMessage>::iterator itMessage = messagesTmp.begin();
        string idMessage;

        while (countMessage > 0) {
            if (beaconSource.compare(itMessage->second.getTarget()) == 0) {
                idMessage = itMessage->second.getGlobalMessageIdentificaton();

                sendWSM(updateBeaconMessageWSM(itMessage->second.dup(), beaconSource));
                ScountMsgPacketSend++;
                messagesDelivered.push_back(idMessage);

                if (countMessage == 1) {
                    countMessage = 0;
                } else {
                    countMessage--;
                    itMessage++;
                }

                messagesTmp.erase(idMessage);
                //colorCarryMessageVehDist(messagesBufferToDelivery);
            } else {
                countMessage--;
                itMessage++;
            }
        }

        msgToDeliveryTmp.messages = messagesTmp;
        messagesBufferToDelivery[beaconSource] = msgToDeliveryTmp;
    }
}

bool vehDist::sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm) {
    unordered_map <string, WaveShortMessage>::iterator itBeaconStatus = beaconStatusNeighbors.begin();
    string targetMessage = wsm.getTarget();

    for (itBeaconStatus = beaconStatusNeighbors.begin(); itBeaconStatus != beaconStatusNeighbors.end(); itBeaconStatus++) {
        if (targetMessage.compare(itBeaconStatus->first) == 0) { // Test if this neighbor is the target of the last message received
            cout << source << " sending OneNewMessageToOneNeighborTarget: " << wsm.getGlobalMessageIdentificaton() << " to: " << itBeaconStatus->first << endl;

            sendWSM(updateBeaconMessageWSM(wsm.dup(), itBeaconStatus->first));
            ScountMsgPacketSend++;
            messagesDelivered.push_back(wsm.getGlobalMessageIdentificaton());

            return false;
        }
    }
    return true;
}

void vehDist::vehCreateUpdateRateTimeToSendEvent() {
    rateTimeToSend = 2500; // Initial rateTimeToSend #2500 - Send in: 2500 ms
    rateTimeToSendDistanceControl = 10; // Equal to 10 m in 1 s
    rateTimeToSendLimitTime = 5; // #5
    rateTimeToSendLimitTime = rateTimeToSendLimitTime * 1000; // #5000 // Limit that rateTimeToSend can be (one message by 5000 ms), value must be (bufferMessage limit) * rateTimeToSend, in this case 50 * 100 = 5000
    rateTimeToSendUpdateTime = 1; // Will update every 1 s

    sendUpdateRateTimeToSendVeh = new cMessage("Event update rateTimeToSend vehicle", SendEvtUpdateRateTimeToSendVeh);
    //cout << source << " at: " << simTime() << " schedule created UpdateRateTimeToSend to: "<< (simTime() + rateTimeToSendUpdateTime) << endl;

    scheduleAt((simTime() + rateTimeToSendUpdateTime), sendUpdateRateTimeToSendVeh);
}

void vehDist::vehCreateEventTrySendBeaconMessage() {
    if (SvehDistCreateEventGenerateMessage) {
        sendBeaconMessageEvt = new cMessage("Event send beacon message", SendEvtBeaconMessage);
        timeToFinishLastStartSend = simTime() + vehOffSet;
        messageToSend = 0; // messagesOrderReceived.front();

        //cout << source << " at: "<< simTime() << " schedule created SendBeaconMessage to: "<< timeToFinishLastStartSend << endl;
        scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
        timeToFinishLastStartSend += double(rateTimeToSendLimitTime)/1000; // /1000 because works with s instead ms
    }
}

WaveShortMessage* vehDist::prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial) {
    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);
    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setSerial(serial);
    wsm->setTimestamp(simTime());
    wsm->setSource(source.c_str());

    // beaconStatus don't need
    //wsm->setRecipientAddressTemporary();  // => "BROADCAST"
    //wsm->setSenderAddressTemporary(source);
    //wsm->setTarget(); // => "BROADCAST"

    wsm->setSenderSpeed(mobility->getSpeed());
    wsm->setCategory(vehCategory.c_str());
    wsm->setSenderPos(curPosition);
    if (simTime() < StimeToUpdatePosition) {
        wsm->setSenderPosPrevious(mobility->getPositionAt(0.1));
    } else {
        wsm->setSenderPosPrevious(mobility->getPositionAt(simTime() - StimeToUpdatePosition));
    }
    wsm->setRateTimeToSend(rateTimeToSend);

//    wsm->setHeading(getVehHeading4()); //wsm->setHeading(getVehHeading8()); // heading 1 to 4 or 1 to 8
//    wsm->setRoadId(mobility->getRoadId().c_str());

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << endl;
    return wsm;
}

WaveShortMessage* vehDist::updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId) {
    wsm->setSenderAddressTemporary(source.c_str());
    wsm->setRecipientAddressTemporary(rcvId.c_str());
    wsm->setCategory(vehCategory.c_str());
    wsm->setHopCount(wsm->getHopCount() -1);

//    wsm->setRoadId(mobility->getRoadId().c_str());
//    wsm->setSenderSpeed(mobility->getSpeed());
//    wsm->setSenderPos(curPosition);
//    if (simTime() < StimeToUpdatePosition) {
//        wsm->setSenderPosPrevious(mobility->getPositionAt(0.1));
//    } else {
//        wsm->setSenderPosPrevious(mobility->getPositionAt(simTime() - StimeToUpdatePosition));
//    }
//    wsm->setHeading(getVehHeading4()); //wsm->setHeading(getVehHeading8());

    return wsm;
}

bool vehDist::busRouteDiffTarget(string busID, Coord targetPos) {
    if (par("useBusPosition").boolValue()) {
        string routeIDTmp;
        if (SrouteIDVehID.find(busID) == SrouteIDVehID.end()) {
            cout << "\nJBe - SrouteIDVehID.find(" << busID << ") == SrouteIDVehID.end()";
            ASSERT2(0, "JBe - SrouteIDVehID.find(busID) == SrouteIDVehID.end()");
        } else {
            routeIDTmp = SrouteIDVehID.find(busID)->second;
        }

        struct busPosByTime structBusPosByTime;
        if (SposTimeBusLoaded.find(routeIDTmp) == SposTimeBusLoaded.end()) {
            cout << "\nSposTimeBusLoaded.find(" << routeIDTmp << ") == SposTimeBusLoaded.end()";
            ASSERT2(0, "SposTimeBusLoaded.find(routeIDTmp) == SposTimeBusLoaded.end()");
        } else {
            structBusPosByTime = SposTimeBusLoaded.find(routeIDTmp)->second;
        }

        double distNow, smallDist;
        map <simtime_t, Coord> timePosTmp = structBusPosByTime.timePos;
        int timeNow = int(simTime().dbl());

        map <simtime_t, Coord>::iterator itTimePos;
        if (timePosTmp.find(timeNow) == timePosTmp.end()) {
            cout << "\ntimePosTmp.find(" << timeNow << ") == timePosTmp.end() - return false";
            return false;
        } else {
            itTimePos = timePosTmp.find(timeNow);
        }

        Coord smallCoord = itTimePos->second;
        smallDist = traci->getDistance(smallCoord, targetPos, false);
        int timeSmallDist = int(itTimePos->first.dbl());
        itTimePos++;

        //cout <<"\nsimTime: " << simTime() << "\n";
        for(; itTimePos != timePosTmp.end(); itTimePos++) {
            distNow = traci->getDistance(itTimePos->second, targetPos, false);

            //cout << "time: " << itTimePos->first << " distNow: " << distNow << " smallDist: " << smallDist << " \n";

            if (distNow < smallDist) {
                //cout << "\ntraci->getDistance" << itTimePos->second << ", "<<  targetPos << ", false)";
                //cout << "\n\ndistNow: " << distNow << "\nsmallDist: " << smallDist;

                smallDist = distNow;
                timeSmallDist = int(itTimePos->first.dbl());
                smallCoord = itTimePos->second;
            }
        }

        cout << "\nrouteIDTmp: " << routeIDTmp;
        cout << "\ntargetPos: " << targetPos;
        cout << "\ntimeSmallDist: " << timeSmallDist;
        cout << "\nsmallCoord: " << smallCoord;
        cout << "\nsmallDist : " << smallDist << "\n";

        if (smallDist <= 250){
            return true;
        }
    }
    return false;
}

void vehDist::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT_vehDist: { // Call prepareBeaconStatusWSM local to the veh
            sendWSM(prepareBeaconStatusWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            ScountBeaconSend++;
            break;
        }
        case SendEvtBeaconMessage: {
            sendBeaconMessage();
            break;
        }
        case SendEvtUpdateRateTimeToSendVeh: {
            vehUpdateRateTimeToSend();
            scheduleAt(simTime() + rateTimeToSendUpdateTime, sendUpdateRateTimeToSendVeh);
            break;
        }
        case SendEvtSaveBusPosition: {
            busPosInsertByTimeMap();
            scheduleAt(simTime() + 1, sendSaveBusPosition);
            break;
        }
        default: {
            BaseWaveApplLayer::handleSelfMsg(msg);
            break;
        }
    }
}

void vehDist::vehUpdateRateTimeToSend() {
    //cout << source << " rateTimeToSend: " << rateTimeToSend;
    unsigned short int distance = traci->getDistance(mobility->getPositionAt(simTime() - rateTimeToSendUpdateTime), curPosition, false);

    if (distance >= rateTimeToSendDistanceControl) {
        if(rateTimeToSend > 100) { // Inferior limit
            rateTimeToSend -= 100;
        }
    } else {
        if (rateTimeToSend < rateTimeToSendLimitTime) { // Upper limit
            rateTimeToSend += 100;
        }
    }
    //cout << " updated to: " << rateTimeToSend << " at: " << simTime() << " by: " <<  distance << " traveled [" << mobility->getPositionAt(simTime() - rateTimeToSendUpdateTime) << " " << curPosition << "]" << endl;
}

void vehDist::printMessagesBuffer() {
    if (!messagesBufferVehDist.empty()) {
        cout << endl << "messagesBuffer from " << source << " at: " << simTime() << " position: " << curPosition << endl;
        unordered_map <string, WaveShortMessage>::iterator itMessage;
        for (itMessage = messagesBufferVehDist.begin(); itMessage != messagesBufferVehDist.end(); itMessage++) {
            cout << "    Id(message): " << itMessage->second.getGlobalMessageIdentificaton() << endl;
            cout << "    WsmData: " << itMessage->second.getWsmData() << endl;
            cout << "    Source: " << itMessage->second.getSource() << endl;
            cout << "    Target: " << itMessage->second.getTarget() << endl;
            cout << "    Timestamp: " << itMessage->second.getTimestamp() << endl;
            cout << "    HopCount: " << itMessage->second.getHopCount() << endl << endl << endl;
        }
    } else {
        cout << endl << "messagesBuffer from " << source << " is empty now: " << simTime() << " position: " << curPosition << endl << endl;
    }
}

void vehDist::printBeaconStatusNeighbors() {
    if (!beaconStatusNeighbors.empty()) {
        cout << endl << "beaconStatusNeighbors from " << source << " at: " << simTime() << " position: " << curPosition << endl;
        unordered_map <string, WaveShortMessage>::iterator itBeaconStatus;
        for (itBeaconStatus = beaconStatusNeighbors.begin(); itBeaconStatus != beaconStatusNeighbors.end(); itBeaconStatus++) {
            cout << "    Id(vehicle): " << itBeaconStatus->first << endl;
            cout << "    PositionPrevious: " << itBeaconStatus->second.getSenderPosPrevious() << endl;
            cout << "    Position: " << itBeaconStatus->second.getSenderPos() << endl;
            cout << "    Speed: " << itBeaconStatus->second.getSenderSpeed() << endl;
            cout << "    Category: " << itBeaconStatus->second.getCategory() << endl;
            cout << "    RoadId: " << itBeaconStatus->second.getRoadId() << endl;
            cout << "    Heading: " << itBeaconStatus->second.getHeading() << endl;
            cout << "    Timestamp: " << itBeaconStatus->second.getTimestamp() << endl;
            cout << "    RateTimeToSend: " << itBeaconStatus->second.getRateTimeToSend() << endl << endl;
        }
    } else {
        cout << endl << "beaconStatusNeighbors from " << source << " is empty now: " << simTime() << " position: " << curPosition << endl;
    }
}

// Not used ###############################################################
unsigned short int vehDist::getVehHeading4() {
    // marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
    // marcospaiva.com.br/localizacao.htm

    double angle;
    if (mobility->getAngleRad() < 0) { // radians are negative, so degrees negative
        angle = (((mobility->getAngleRad() + 2 * M_PI ) * 180)/ M_PI);
    } else { // radians are positive, so degrees positive
        angle = ((mobility->getAngleRad() * 180) / M_PI);
    }

    if ((angle >= 315 && angle < 360) || (angle >= 0 && angle < 45)) {
        return 1; // L or E => 0º
    } else if (angle >= 45 && angle < 135) {
        return 2; // N => 90º
    } else if (angle >= 135  && angle < 225) {
        return 3; // O or W => 180º
    } else if (angle >= 225  && angle < 315) {
        return 4; // S => 270º
    } else {
        return 9; // Error
    }
}

unsigned short int vehDist::getVehHeading8() {
    // marcospaiva.com.br/images/rosa_dos_ventos%2002.GIF
    // marcospaiva.com.br/localizacao.htm

    double angle;
    if (mobility->getAngleRad() < 0) { // radians are negative, so degrees negative
        angle = (((mobility->getAngleRad() + 2 * M_PI ) * 180)/ M_PI);
    } else { // radians are positive, so degrees positive
        angle = ((mobility->getAngleRad() * 180) / M_PI);
    }

    if ((angle >= 337.5 && angle < 360) || (angle >= 0 && angle < 22.5)) {
        return 1; // L or E => 0º
    } else if (angle >= 22.5 && angle < 67.5) {
        return 2; // NE => 45º
    } else if (angle >= 67.5  && angle < 112.5) {
        return 3; // N => 90º
    } else if (angle >= 112.5  && angle < 157.5) {
        return 4; // NO => 135º
    } else if (angle >= 157.5  && angle < 202.5) {
        return 5; // O or W => 180º
    } else if (angle >= 202.5  && angle < 247.5) {
        return 6; // SO => 225º
    } else if (angle >= 247.5  && angle < 292.5) {
        return 7; // S => 270º
    } else if (angle >= 292.5  && angle < 337.5) {
        return 8; // SE => 315º
    } else {
        return 9; // Error
    }
}
