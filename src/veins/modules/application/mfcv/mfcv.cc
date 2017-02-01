// Copyright (C) 2015-2017 João Batista <joao.b@usp.br>

#include "veins/modules/application/mfcv/mfcv.h"

using Veins::TraCIMobilityAccess;

Define_Module(mfcv);

void mfcv::initialize(int stage) {
    BaseWaveApplLayer::initialize_veins_TraCI(stage);
    if (stage == 0) { // traci - mobility, traci->getComand - traci, new traciVehice
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        mfcvInitializeVariablesVeh();
    }
}

void mfcv::mfcvInitializeVariablesVeh() {
    vehCreateUpdateRateTimeToSendEvent(); // Create Event to update the rateTimeToSend (Only mfcv)

    vehInitializeValuesMfcv(traciVehicle->getTypeId(), mobility->getPositionAt(simTime() + 0.1)); // The same for Epidemic and mfcv

    busPositionFunctionsRun(); // bus position function

    loadLaneNames(); // load lane names

    vehCreateEventTrySendBeaconMessage(); // Create one Event to try send messages in buffer (Only mfcv)

    createEvtTaxiChangeRoute();
}

void mfcv::createEvtTaxiChangeRoute() {
    if (vehCategory.compare("T") == 0 ) {
        taxiRoadIdNow = traciVehicle->getRoadId();
        sendTaxiChangeRoute = new cMessage("Event update rateTimeToSend vehicle", SendEvtTaxiChangeRoute);
        //cout << source << " at: " << simTime() << " schedule created sendTaxiChangeRoute to: "<< (simTime() + 1) << endl;

        scheduleAt(simTime(), sendTaxiChangeRoute);
    }
}

int mfcv::selectLaneNamePositionId() {
    uniform_int_distribution <int> dist(0, (SlaneNameLoaded.size() - 1)); // generate a value, e.g, dist(0, 10), will be 0, 1, ... 10
    return dist(mtSelectLaneName);
}

void mfcv::tryChangeRouteUntilSuccess() {
    cout << "\nPlannedRoadIds before";
    printPlannedRoadIds(traciVehicle->getPlannedRoadIds());

    bool resultChangeRoute;
    int positionId;
    string newTarget;

    while (!resultChangeRoute) {
        positionId = selectLaneNamePositionId();
        newTarget = SlaneNameLoaded.at(positionId);
        cout << "\nnewTarget test: " << newTarget << " - SlaneNameLoaded.at(" << positionId << ")";

        resultChangeRoute = traciVehicle->newRoute_JB(newTarget.c_str());
        cout << " - resultChangeRoute status: " << boolToString(resultChangeRoute);

        if (resultChangeRoute) {
            cout << "\nRoute changed - New target is: " << newTarget;

            cout << "\n\nPlannedRoadIds after";
            printPlannedRoadIds(traciVehicle->getPlannedRoadIds());
        } else {
            cout << "\nRoute not changed\n";
        }
    }
}

void mfcv::printPlannedRoadIds(list<string> plannedRoadIds) {
    cout << "\n" << source << " printPlannedRoadIds" << " at: " << simTime();
    cout << " routeId: " << traciVehicle->getRouteId() << " roadId: " << mobility->getRoadId();

    cout << "\nPlannedRoadIds: ";
    list<string>::iterator itPlannedRoadIds = plannedRoadIds.begin();
    for(itPlannedRoadIds = plannedRoadIds.begin(); itPlannedRoadIds != plannedRoadIds.end(); ++itPlannedRoadIds) {
        cout << *itPlannedRoadIds << " ";
    }
    cout << "\n";
}

void mfcv::busPositionFunctionsRun() {
    routeId = traciVehicle->getRouteId();
    SrouteIdVehId.insert(make_pair(source, routeId));
    msgUseOnlyDeliveryBuffer = 0;

    createBusPositionSaveEvent(); // Create event to generate a csv file with the bus position (if getBusPosition => true)
    busPosLoadFromFile(); // Load a csv file with bus position (if useBusPosition => true)
}

void mfcv::createBusPositionSaveEvent() {
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

void mfcv::busPosInsertByTimeMap() {
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

void mfcv::saveBusPositionFile() {
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

void mfcv::busPosLoadFromFile() {
    if (source.compare("car[0]") == 0) {
        if (par("useBusPosition").boolValue()) {
            cout << "\n\nLoading routes from of the bus";
            string fileInput, line;

            fileInput = par("fileBusPositionPath").stringValue();
            if (fileInput.compare("") == 0) {
                cout << "\nbusPosition fileInput is empty - need pass the path to \"busPosition.csv\"\n\n";
                ASSERT2(0, "JBe - fileInput is empty - need pass the path to \"busPosition.csv\" -" );
            }

            ifstream inputIfstream(fileInput.c_str());
            cout << "\nfileInput: " << fileInput << "\n";

            int countPos = 0;
            string timeString, routeIdTmp;
            Coord positionBus;
            size_t pos;
            simtime_t simTimeTmp;
            map <simtime_t, Coord> timePosTmp;

            while (getline(inputIfstream, line, '\n')) {
                //cout << "\nline: " << line;

                // (countPos == 0) // blank line
                // (countPos == 1) // routeId position time

                if (countPos == 2) {
                    timePosTmp.clear();
                    routeIdTmp = line;
                    //cout << "\n\nrouteIdTmp: " << routeIdTmp << "\n";
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

                   SposTimeBusLoaded.insert(make_pair(routeIdTmp, structBusPosByTime));
                   countPos = 1;
                   cout << "\nBus route loaded: " << routeIdTmp;
               }

            }
        }
    }
}

void mfcv::loadLaneNames() {
    if (source.compare("car[0]") == 0) {
        cout << "\n\nLoading lane name from of the taxi";
        string fileInput, line;

        fileInput = par("fileLaneNamesPath").stringValue();
        if (fileInput.compare("") == 0) {
            cout << "\nLane name fileInput is empty - need pass the path to \"laneName.csv\"\n\n";
            ASSERT2(0, "JBe - fileInput is empty - need pass the path to \"laneName.csv.csv\" -" );
        }

        ifstream inputIfstream(fileInput.c_str());
        cout << "\nfileInput: " << fileInput << "\n";

        while (getline(inputIfstream, line)) {
            //cout << "\nline: " << line;
            SlaneNameLoaded.push_back(line);
        }
    }
}

void mfcv::onBeacon(WaveShortMessage* wsm) {
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

void mfcv::onData(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) { // Verify if this is the recipient of the message
        //saveMessagesOnFile(wsm, SfileMessagesUnicastVeh);

        // Real scenario
        //if (source.compare(wsm->getTarget()) == 0) { // Message to this vehicle
        //
        //} else ...

        bool insert = sendOneNewMessageToOneNeighborTarget(*wsm); // Look in neigborStatus buffer if has the target of this message
        if (insert) {
            if (wsm->getOnlyDelivery()) {
                cout << "vehCategory: " << vehCategory << "\n";
                cout << source << " with one message ToDelivery: " << wsm->getOnlyDelivery() << " target: " << wsm->getTarget() << " at: " << simTime() << endl;

                busMsgToDelivery += source + string(" msg: ") + wsm->getGlobalMessageIdentificaton() + string(" at: ") + to_string(simTime().dbl());
                busMsgToDelivery += string(" to ") + wsm->getTarget() + string("\n");

                if (messagesBufferOnlyDelivery.find(wsm->getTarget()) == messagesBufferOnlyDelivery.end()) {
                    messagesBufferOnlyDelivery.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));

                    if (msgUseOnlyDeliveryBuffer < messagesBufferOnlyDelivery.size()) {
                        msgUseOnlyDeliveryBuffer = messagesBufferOnlyDelivery.size();
                    }
                }
            } else {
                // test if message has been delivered to the target before.
                if (find(messagesDelivered.begin(), messagesDelivered.end(), wsm->getGlobalMessageIdentificaton()) == messagesDelivered.end()) {
                    if (wsm->getHopCount() > 0) {
                        if (messagesBufferMfcv.find(wsm->getGlobalMessageIdentificaton()) == messagesBufferMfcv.end()) { //verify if the message isn't in the buffer
                            cout << source << " saving message from " << wsm->getSenderAddressTemporary() << " with Id: " << wsm->getGlobalMessageIdentificaton() << " at: " << simTime() << endl;
                            string wsmDataTmp = wsm->getWsmData();
                            if (wsmDataTmp.size() < 42) { // WSMData generated by car[x] and carry by [ T, P
                                wsmDataTmp += " and carry by (";
                            } else {
                                wsmDataTmp += ", ";
                            }
                            wsmDataTmp += vehCategory;
                            wsm->setWsmData(wsmDataTmp.c_str());

                            if (messagesBufferMfcv.size() > SmessageBufferSize) {
                                removeOldestInputBeaconMessage();
                            }

                            messagesBufferMfcv.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm)); // Add the message in the vehicle buffer
                            msgBufferUse++;
                            messagesOrderReceivedMfcv.push_back(wsm->getGlobalMessageIdentificaton());

                            //colorCarryMessageMfcv(messagesBufferMfcv);
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

void mfcv::removeOldestInputBeaconMessage() {
    //printMessagesBuffer();

    if (!messagesBufferMfcv.empty()) {
        unsigned short int typeRemoved = 0;
        unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBufferMfcv.begin();
        simtime_t minTime = itMessage->second.getTimestamp();
        string idMessage = itMessage->first;
        itMessage++;

        for (; itMessage != messagesBufferMfcv.end(); itMessage++) {
            if (minTime > itMessage->second.getTimestamp()) {
                minTime = itMessage->second.getTimestamp();
                idMessage = itMessage->first;
            }
        }

        if (simTime() > (minTime + SttlMessage)) {
            cout << source << " remove one message (" << idMessage << ") by time, minTime: " << minTime << " at: " << simTime();
            cout << " ttlMessage: " << SttlMessage << endl;
            typeRemoved = 3; // by ttl (1 buffer, 2 copy, 3 time)
        } else if (messagesBufferMfcv.size() > SmessageBufferSize) {
            //cout << source << " remove one message (" << idMessage << ") by space, MessageBuffermfcv.size(): " << messagesBufferMfcv.size();
            //cout << " at: " << simTime() << " SmessageBufferSize: " << SmessageBufferSize << endl;
            typeRemoved = 1; // by buffer (1 buffer, 2 copy, 3 time)
        }

        if (typeRemoved != 0) {
            insertMessageDropVeh(idMessage, typeRemoved, messagesBufferMfcv[idMessage].getTimestamp()); // Removed by the value of tyRemoved (1 buffer, 2 copy, 3 time)
            messagesBufferMfcv.erase(idMessage);
            auto itMsg = find(messagesOrderReceivedMfcv.begin(), messagesOrderReceivedMfcv.end(), idMessage);
            messagesOrderReceivedMfcv.erase(itMsg);

            //colorCarryMessageMfcv(messagesBufferMfcv);
            removeOldestInputBeaconMessage();
        }
    } /*else {
        cout << "messagesBuffer from " << source << " is empty now" << endl;
    }*/
}

void mfcv::removeOldestInputBeaconStatus() {
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

void mfcv::sendBeaconMessage() {
    //cout << source << " try to send message at: " << simTime() << endl;
//############################################################# run many times
    removeOldestInputBeaconStatus();
    removeOldestInputBeaconMessage();

//    unordered_map<string, WaveShortMessage>::iterator itbeaconStatus;
//    for (itbeaconStatus = beaconStatusNeighbors.begin(); itbeaconStatus != beaconStatusNeighbors.end(); itbeaconStatus++){
//        sendMessageToOneNeighborTarget(itbeaconStatus->second.getSource()); // Look in buffer it has messages for this vehicle neighbor
//    }
//############################################################# run many times

    if (messageToSend >= messagesOrderReceivedMfcv.size()) {
        messageToSend = 0;
    }

    //printMessagesBuffer();
    trySendBeaconMessage();
    //printMessagesBuffer();
    messageToSend++; // Move to next message
    simtime_t schedleNext = simTime() + SnormalTimeSendMessage/2 + double(rateTimeToSend)/1000;

    //cout << "normalTimeSendMessage: " << SnormalTimeSendMessage << " rateTimeToSend: " << double(rateTimeToSend)/1000 << "s";
    //cout << "\nschedleNext: " << schedleNext << "\n";
    if (!SuseRateTimeToSend) {
        //cout << source << " schedule useRateTimeToSend: false the sendBeaconMessageEvt at: " << simTime() << " to: " << (simTime() + SnormalTimeSendMessage) << endl;
        scheduleAt((simTime() + SnormalTimeSendMessage), sendBeaconMessageEvt);
    } else {
        if (messageToSend >= messagesOrderReceivedMfcv.size()) {
            if (simTime() > timeToFinishLastStartSend) {
                scheduleAt(schedleNext, sendBeaconMessageEvt);
                //cout << source << " 1_schedule at: " << simTime() << " to: " << schedleNext;
            } else {
                scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
                //cout << source << " 2_schedule at: " << simTime() << " to: " << timeToFinishLastStartSend;
            }

            timeToFinishLastStartSend += (double(rateTimeToSend)/1000 + SnormalTimeSendMessage) * 2;
        } else {
            scheduleAt(schedleNext, sendBeaconMessageEvt);
            //cout << source << " 3_schedule at: " << simTime() << " to: " << schedleNext << " rateTimeToSend: " << rateTimeToSend;
        }
        //cout << " timeToFinishLastStartSend: " << timeToFinishLastStartSend;
        //cout << "\n                               " << source << " expSendbyDSCR: " << SexpSendbyDSCR << " at: " << simTime() << "\n\n";
    }
}

void mfcv::trySendBeaconMessage() {
    if (!messagesBufferMfcv.empty()) {
        cout << source << " messagesBuffer with "<< messagesBufferMfcv.size() << " message(s) at: " << simTime() << endl;
        // Test with hopCount 0, send only

        if (!beaconStatusNeighbors.empty()) {
            //printBeaconStatusNeighbors();
            // The same range to the RSU and veh

            msgOnlyDeliveryFunctionResult = 0;
            string idMessage, rcvId = source;
            idMessage = messagesOrderReceivedMfcv[messageToSend];
            if (messagesBufferMfcv[idMessage].getHopCount() > 1) {
                rcvId = neighborWithShortestDistanceToTarget(idMessage);
            } else {
                rcvId = neighborWithShortestDistanceToTargetOnlyDelivery(idMessage);
            }

            string catVeh;
            if (source.compare(rcvId) != 0) {
                unordered_map <string, WaveShortMessage>::iterator itBeaconN;
                itBeaconN = beaconStatusNeighbors.find(rcvId);
                catVeh = itBeaconN->second.getCategory();

                cout << source << " chose the vehicle " << rcvId << " in the expSendbyDSCR " << SexpSendbyDSCR << " to be a next hop to the " << idMessage << " message" << endl;
                cout << "    " << source << " send message to " << rcvId << " with category " << catVeh << " at: "<< simTime() << endl;
                cout << "    MessageToSend: " << messageToSend << endl;
                cout << "    MessageId: " << idMessage << endl;
                cout << "    Source: " << messagesBufferMfcv[idMessage].getSource() << endl;
                cout << "    Message content: " << messagesBufferMfcv[idMessage].getWsmData() << endl;
                cout << "    Target: " << messagesBufferMfcv[idMessage].getTarget() << endl;
                cout << "    TargetPos: " << messagesBufferMfcv[idMessage].getTargetPos() << endl;
                cout << "    Timestamp: " << messagesBufferMfcv[idMessage].getTimestamp() << endl;
                cout << "    HopCount: " << messagesBufferMfcv[idMessage].getHopCount() << endl << endl;

                WaveShortMessage wsmTmpSend = messagesBufferMfcv[idMessage];
                // toDeliveryMsg is set when send to a bus (expSend13)
                if (msgOnlyDeliveryFunctionResult == 1) {
                    wsmTmpSend.setOnlyDelivery(true);
                }

                sendWSM(updateBeaconMessageWSM(wsmTmpSend.dup(), rcvId));
                ScountMsgPacketSend++;

                if ((!SallowMessageCopy) || (msgOnlyDeliveryFunctionResult == 1)) {
                    cout << source << " send the message " << idMessage << " and removing (message copy not allow) at: "  << simTime() << endl;

                    insertMessageDropVeh(idMessage, 2, messagesBufferMfcv[idMessage].getTimestamp()); // Removed by the value of tyRemoved (1 buffer, 2 copy, 3 time)

                    messagesBufferMfcv.erase(idMessage);
                    messageToSend--;

                    auto it = find(messagesOrderReceivedMfcv.begin(), messagesOrderReceivedMfcv.end(), idMessage);
                    messagesOrderReceivedMfcv.erase(it);
                    //colorCarryMessageMfcv(messagesBufferMfcv);
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

string mfcv::neighborWithShortestDistanceToTargetOnlyDelivery(string idMessage) {
    unordered_map <string, WaveShortMessage>::iterator itBeaconNeighbors;

    for (itBeaconNeighbors = beaconStatusNeighbors.begin(); itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
        if (strcmp(itBeaconNeighbors->second.getSource(), messagesBufferMfcv[idMessage].getTarget()) == 0) {
            cout << source << " found target, message " << idMessage << " target " << messagesBufferMfcv[idMessage].getTarget() << " at: " << simTime() << endl;
            return itBeaconNeighbors->second.getSource();
        }
    }
    return source;
}

bool mfcv::has3inSexpSendbyDSCR() { //true if has 3 (3 meaning category in the experiment)
    switch (SexpSendbyDSCR) {
        case 13: return true;
        case 23: return true;
        case 34: return true;

        case 123: return true;
        case 134: return true;
        case 234: return true;

        case 1234: return true;
    }
    return false;
}

string mfcv::neighborWithShortestDistanceToTarget(string idMessage) {
    string category, neighborCategory, neighborId;
    Coord targetPos, senderPosPrevious, senderPosNow;
    shortestDistance sD;
    unsigned short int meet1Cat, meet2Cat, meet3Cat;
    double neighborDistanceBefore, neighborDistanceNow;
    double rateTimeToSendPart, speedPart;

    unordered_map <string, shortestDistance> vehShortestDistanceToTarget;
    unordered_map <string, WaveShortMessage>::iterator itBeaconNeighbors;

    meet1Cat = meet2Cat = meet3Cat = 0;
    bool insert;

    double localVehDistanceNow = traci->getDistance(curPosition, targetPos, false);
    targetPos = messagesBufferMfcv[idMessage].getTargetPos();

    cout << source << " meet with " << beaconStatusNeighbors.size() << " neighbors options to the send the message " << idMessage << endl;
    for (itBeaconNeighbors = beaconStatusNeighbors.begin(); itBeaconNeighbors != beaconStatusNeighbors.end(); itBeaconNeighbors++) {
        neighborCategory = itBeaconNeighbors->second.getCategory();
        neighborId = itBeaconNeighbors->first;

        if (neighborCategory.compare("rsu") != 0) { // When use more than one rsu, will ignore the RSU in the neighborId
            if (!messagesSendLog.empty() && (messagesSendLog[idMessage].find(neighborId) != std::string::npos)) {
                cout << source << " has been send a message " << idMessage << " to " << neighborId << endl;
            } else {
                if(has3inSexpSendbyDSCR()) {
                    if (neighborCategory.compare("B") == 0) {
                        cout << "\n\n" << source << " whit neighbor: "<< neighborId << " category: " << neighborCategory;
                        cout << " msg to: " << messagesBufferMfcv[idMessage].getTarget() << " with pos: " << targetPos;
                        if (!itBeaconNeighbors->second.getBufferMessageOnlyDeliveryFull()) {
                            msgOnlyDeliveryFunctionResult = busRouteDiffTarget(neighborId, targetPos, localVehDistanceNow);
                            if (msgOnlyDeliveryFunctionResult == 1) {
                                cout << "send by busTestEdgeRoute to "<< neighborId << "\n\n";
                                ScountMessageOnlyDeliveryBus++;

                                if ((meet1Cat + meet2Cat + meet3Cat) <= 1) {
                                    ScountMeetJustOneCategory++;
                                } else if ((meet1Cat + meet2Cat + meet3Cat) == 2) {
                                    ScountMeetTwoCategory++;
                                }
                                return neighborId;
                            }
                            cout << "\n\n";
                        } else {
                            cout << "itBeaconNeighbors->second.getBufferMessageOnlyDeliveryFull() << Full Buffer" << endl;
                        }
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
                    sD.distanceToTargetBefore = neighborDistanceBefore;
                    sD.senderPos = senderPosNow;
                    sD.speedVeh = itBeaconNeighbors->second.getSenderSpeed();
                    sD.rateTimeToSendVeh = itBeaconNeighbors->second.getRateTimeToSend();

                    speedPart = sD.speedVeh * SspeedModDecision;
                    rateTimeToSendPart = double(sD.rateTimeToSendVeh) * SrateTimeToSendModDecision;

                    sD.Distance = neighborDistanceNow; // 1
                    sD.DistanceSpeed = sD.Distance + speedPart; // 12

                    // 13
                    if (sD.categoryVeh.compare(SfirstCategoryPrivateCar) == 0) { // Private car
                        sD.DistanceCategory = sD.Distance;
                        meet1Cat = 1;
                    } else if (sD.categoryVeh.compare(SsecondCategoryBus) == 0) { // Bus
                        if (msgOnlyDeliveryFunctionResult == 2) {
                            sD.DistanceCategory = sD.Distance * SbusValueCategoryGoingTarget;
                        } else {
                            sD.DistanceCategory = sD.Distance;
                        }
                        meet2Cat = 1;
                    } else if (sD.categoryVeh.compare(SthirdCategoryTaxi) == 0) { // Taxi
                        sD.DistanceCategory = sD.Distance * StaxiValueCategory;
                        meet3Cat = 1;
                    } else {
                        cout << endl << "JBe - Error category unknown - " << source << " category: " << sD.categoryVeh << endl;
                        cout << "\nSfirstCategoryPrivateCar: " << SfirstCategoryPrivateCar;
                        cout << "\nSsecondCategoryBus: " << SsecondCategoryBus;
                        cout << "\nSthirdCategoryTaxi: " << SthirdCategoryTaxi << endl;
                        ASSERT2(0, "JBe - Error category unknown -");
                    }

                    sD.DistanceRateTimeToSend = sD.Distance + rateTimeToSendPart; // 14
                    sD.DistanceSpeedCategory = sD.DistanceCategory + speedPart; // 123
                    sD.DistanceSpeedRateTimeToSend = sD.DistanceSpeed + rateTimeToSendPart; // 124
                    sD.DistanceCategoryRateTimeToSend = sD.DistanceCategory + rateTimeToSendPart; // 134
                    sD.DistanceSpeedCategoryRateTimeToSend = sD.DistanceSpeedCategory + rateTimeToSendPart; // 1234

                    // Distance = [0 - 125] - 720 m
                    // vehicle Speed = 0 - (16.67/25) - 84 m/s
                    // rateTimeToSend = 100 to 5000 ms

                    // Category - distanceToTargetNow
                    //     Private car - D * 1
                    //     Bus         - 1ª Test route - If <= 250 msg.target - send.OnlyDelivery(True)
                    //                 - 2ª Test route - if <= 80% msg.target - D * StaxiValueCategory
                    //                 - 3ª Test route - D * 1
                    //     Taxi        - D * SbusValueCategoryGoingTarget

                    // DecisonValueDS = distance - speed
                    // DecisonValueDCR = distanceCategory + rateTimeToSend/100 (0.1 * 10)

                    vehShortestDistanceToTarget.insert(make_pair(neighborId, sD));
                } else {
                    cout << "\n\n    " << neighborId << " not insert in the vehShortestDistanceToTarget\n\n";
                }
            }
        }
    }

    if ((meet1Cat + meet2Cat + meet3Cat) == 1) {
        ScountMeetJustOneCategory++;
    } else if ((meet1Cat + meet2Cat + meet3Cat) == 2) {
        ScountMeetTwoCategory++;
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

    return selectVehIdWithSmallValueBySexpSendbyDSCR(vehShortestDistanceToTarget);
}

void mfcv::printVehShortestDistanceToTarget(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, string idMessage) {
    if (!vehShortestDistanceToTarget.empty()) {
        cout << "\nMeet with " << vehShortestDistanceToTarget.size() << " options to a send " << idMessage << " message" << endl;
        cout << "Source vehicle information" << endl;
        cout << "    Id(veh): " << source << endl;
        cout << "    Message Id: " << messagesBufferMfcv[idMessage].getGlobalMessageIdentificaton() << endl;
        cout << "    Category: " << vehCategory << endl;
        cout << "    Distance: " << traci->getDistance(curPosition, messagesBufferMfcv[idMessage].getTargetPos(), false) << endl;
        cout << "    Speed: " << mobility->getSpeed() << endl;
        cout << "    Position: " << mobility->getCurrentPosition() << endl;
        cout << "    RateTimeToSend: " << rateTimeToSend << endl;

        cout << "\nPrinting vehShortestDistanceToTarget to " << source << " at: " << simTime() << "\n\n";
        unordered_map <string, shortestDistance>::iterator itShortestDistance;
        for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
            cout << "\n    Id(veh): " << itShortestDistance->first;
            cout << "\n    Position: " << itShortestDistance->second.senderPos;
            cout << "\n    Category: " << itShortestDistance->second.categoryVeh;
            cout << "\n    DistanceBefore: " << itShortestDistance->second.distanceToTargetBefore;
            cout << "\n    Speed: " << itShortestDistance->second.speedVeh;
            cout << "\n    RateTimeToSend: " << itShortestDistance->second.rateTimeToSendVeh;

            cout << "\n       1 Distance: " << itShortestDistance->second.Distance;
            cout << "\n      12 DistanceSpeed: " << itShortestDistance->second.DistanceSpeed;
            cout << "\n      13 DistanceCategory: " << itShortestDistance->second.DistanceCategory;
            cout << "\n      14 DistanceRateTimeToSend: " << itShortestDistance->second.DistanceRateTimeToSend;
            cout << "\n     123 DistanceSpeedCategory: " << itShortestDistance->second.DistanceSpeedCategory;
            cout << "\n     124 DistanceSpeedRateTimeToSend: " << itShortestDistance->second.DistanceSpeedRateTimeToSend;
            cout << "\n     134 DistanceCategoryRateTimeToSend: " << itShortestDistance->second.DistanceCategoryRateTimeToSend;
            cout << "\n    1234 DistanceSpeedCategoryRateTimeToSend: " << itShortestDistance->second.DistanceSpeedCategoryRateTimeToSend << "\n\n";
        }
    } else {
        cout << endl << "vehShortestDistanceToTarget to " << source << " at: " << simTime() << " vehShortestDistanceToTarget is empty" << endl << endl;
    }
}

string mfcv::selectVehIdWithSmallValueBySexpSendbyDSCR(unordered_map <string, shortestDistance> vehShortestDistanceToTarget) {
    string vehId = source;
    if (!vehShortestDistanceToTarget.empty()) {
        unordered_map <string, shortestDistance>::iterator itShortestDistance;
        double valueToTest, smallValueFound;

        smallValueFound = DBL_MAX;
        for (itShortestDistance = vehShortestDistanceToTarget.begin(); itShortestDistance != vehShortestDistanceToTarget.end(); itShortestDistance++) {
            // 1: Distance - 2: Speed - 3: Category - 4: RateTimeToSend
            switch (SexpSendbyDSCR) {
                case 1: // Distance
                    valueToTest = itShortestDistance->second.Distance;
                    break;
                case 12: //DistanceSpeed
                    valueToTest = itShortestDistance->second.DistanceSpeed;
                    break;
                case 13: // DistanceCategory
                    valueToTest = itShortestDistance->second.DistanceCategory;
                    break;
                case 14: // DistanceRateTimeToSend
                    valueToTest = itShortestDistance->second.DistanceRateTimeToSend;
                    break;
                case 123: // DistanceSpeedCategory
                    valueToTest = itShortestDistance->second.DistanceSpeedCategory;
                    break;
                case 124: // DistanceSpeedRateTimeToSend
                    valueToTest = itShortestDistance->second.DistanceSpeedRateTimeToSend;
                    break;
                case 134: // DistanceCategoryRateTimeToSend
                    valueToTest = itShortestDistance->second.DistanceCategoryRateTimeToSend;
                    break;
                case 1234: // DistanceSpeedCategoryRateTimeToSend
                    valueToTest = itShortestDistance->second.DistanceSpeedCategoryRateTimeToSend;
                    break;
                default:
                    cout << "JBe - Error! expSendbyDSCR: " << SexpSendbyDSCR << " not defined, class in mfcv.cc -";
                    ASSERT2(0, "JBe - Error expSendbyDSCR value not defined -");
            }

            if (smallValueFound > valueToTest) {
                smallValueFound = valueToTest;
                vehId = itShortestDistance->first;
            }
        }
    }
    return vehId;
}

void mfcv::finish() {
    toFinishVeh();

    if (vehCategory.compare("B") == 0) {
        myfile.open(SfileMessagesCountRsu, std::ios_base::app);
        cout << "\nbusMsgToDelivery from " << source << "\n" << busMsgToDelivery << "\n";
        myfile << "\nbusMsgToDelivery from " << source << "\n" << busMsgToDelivery << "\n";
        myfile.close();
    }

    cout << source << " msgUseOnlyDeliveryBuffer: " << msgUseOnlyDeliveryBuffer << " - end route at: " << simTime() << endl;
    SmsgUseOnlyDeliveryBufferGeneral += msgUseOnlyDeliveryBuffer;
    saveBusPositionFile();
}

void mfcv::sendMessageToOneNeighborTarget(string beaconSource) {
    unsigned short int countMessage = messagesBufferMfcv.size();
    unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBufferMfcv.begin();
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

            messagesBufferMfcv.erase(idMessage);
            auto it = find(messagesOrderReceivedMfcv.begin(), messagesOrderReceivedMfcv.end(), idMessage);
            if (it != messagesOrderReceivedMfcv.end()) {
                messagesOrderReceivedMfcv.erase(it);
            } else {
                cout << endl << source << "JBe - Error in messagesOrderReceived, need to have the same entries as messagesBuffer -" << endl;
                ASSERT2(0, "JBe - Error in messagesOrderReceived, need to have the same entries as messagesBuffer -");
            }
            //colorCarryMessageMfcv(messagesBufferMfcv);
        } else {
            countMessage--;
            itMessage++;
        }
    }
}

void mfcv::sendMessageDeliveryBuffer(string beaconSource) {
    unordered_map <string, WaveShortMessage>::iterator itMessage = messagesBufferOnlyDelivery.begin();
    unsigned short int countMessage = messagesBufferOnlyDelivery.size();
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

            messagesBufferOnlyDelivery.erase(idMessage);
            //colorCarryMessageMfcv(messagesBufferToDelivery);
        } else {
            countMessage--;
            itMessage++;
        }
    }
}

bool mfcv::sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm) {
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

void mfcv::vehCreateUpdateRateTimeToSendEvent() {
    rateTimeToSend = 2500; // Initial rateTimeToSend #2500 - Send in: 2500 ms
    rateTimeToSendDistanceControl = 10; // Equal to 10 m in 1 s
    rateTimeToSendLimitTime = 5; // #5
    rateTimeToSendLimitTime = rateTimeToSendLimitTime * 1000; // #5000 // Limit that rateTimeToSend can be (one message by 5000 ms), value must be (bufferMessage limit) * rateTimeToSend, in this case 50 * 100 = 5000
    rateTimeToSendUpdateTime = 1; // Will update every 1 s

    sendUpdateRateTimeToSendVeh = new cMessage("Event update rateTimeToSend vehicle", SendEvtUpdateRateTimeToSendVeh);
    //cout << source << " at: " << simTime() << " schedule created UpdateRateTimeToSend to: "<< (simTime() + rateTimeToSendUpdateTime) << endl;

    scheduleAt((simTime() + rateTimeToSendUpdateTime), sendUpdateRateTimeToSendVeh);
}

void mfcv::vehCreateEventTrySendBeaconMessage() {
    if (SmfcvCreateEventGenerateMessage) {
        sendBeaconMessageEvt = new cMessage("Event send beacon message", SendEvtBeaconMessage);
        timeToFinishLastStartSend = simTime() + vehOffSet;
        messageToSend = 0; // messagesOrderReceived.front();

        //cout << source << " at: "<< simTime() << " schedule created SendBeaconMessage to: "<< timeToFinishLastStartSend << endl;
        scheduleAt(timeToFinishLastStartSend, sendBeaconMessageEvt);
        timeToFinishLastStartSend += (double(rateTimeToSend)/1000 + SnormalTimeSendMessage) * 2; // /1000 because works with s instead ms
    }
}

WaveShortMessage* mfcv::prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial) {
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

    if (vehCategory.compare("B") == 0) { // It is a bus
        if (messagesBufferOnlyDelivery.size() >= SbufferMessageOnlyDeliveryLimit) {
            wsm->setBufferMessageOnlyDeliveryFull(true);

            cout << source << " SlimitbufferMsgOnlyDevivery: " << SbufferMessageOnlyDeliveryLimit << " - Full" << endl;
            cout << "messagesBufferOnlyDelivery.size(): " << messagesBufferOnlyDelivery.size() << " at: " << simTime() << endl;
        } else {
            wsm->setBufferMessageOnlyDeliveryFull(false);
        }
    } else { // It is another category of vehicle
        wsm->setBufferMessageOnlyDeliveryFull(false);
    }

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << endl;
    return wsm;
}

WaveShortMessage* mfcv::updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId) {
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

unsigned short int mfcv::busRouteDiffTarget(string busId, Coord targetPos, double localVehDistanceNow) {
    if (par("useBusPosition").boolValue()) {
        string routeIdTmp;
        if (SrouteIdVehId.find(busId) == SrouteIdVehId.end()) {
            cout << "\nJBe - SrouteIdVehId.find(" << busId << ") == SrouteIdVehId.end()";
            ASSERT2(0, "JBe - SrouteIdVehId.find(busId) == SrouteIdVehId.end()");
        } else {
            routeIdTmp = SrouteIdVehId.find(busId)->second;
        }

        struct busPosByTime structBusPosByTime;
        if (SposTimeBusLoaded.find(routeIdTmp) == SposTimeBusLoaded.end()) {
            cout << "\nSposTimeBusLoaded.find(" << routeIdTmp << ") == SposTimeBusLoaded.end()";
            ASSERT2(0, "SposTimeBusLoaded.find(routeIdTmp) == SposTimeBusLoaded.end()");
        } else {
            structBusPosByTime = SposTimeBusLoaded.find(routeIdTmp)->second;
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

        cout << "\nrouteIdTmp: " << routeIdTmp;
        cout << "\ntargetPos: " << targetPos;
        cout << "\ntimeSmallDist: " << timeSmallDist;
        cout << "\nsmallCoord: " << smallCoord;
        cout << "\nsmallDist : " << smallDist << "\n";

        if (smallDist <= 250) {
            return 1; // Send to bus as onlyDelivery
        } else if (smallDist <= (localVehDistanceNow * SbusPercentageRouteGoTarget)) { // Small distance less than "SbusPercentageRouteGoTarget" % of now to the target
            return 2; // Use bus values as "busValueCategoryGoingTarget"
        }
    }
    return 0; // Equal a private car
}

void mfcv::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT_mfcv: { // Call prepareBeaconStatusWSM local to the veh
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
        case SendEvtTaxiChangeRoute: {
            list<string> plannedRoadIds = traciVehicle->getPlannedRoadIds();
            cout << "\n\n" << source << " Category: " << vehCategory << " plannedRoadIds.size() == " << plannedRoadIds.size() << " at: " << simTime();

            string tmpTaxiRoadIdNow = traciVehicle->getRoadId();
            if (count(tmpTaxiRoadIdNow.begin(), tmpTaxiRoadIdNow.end(), ':') == 0) { // To ignore the conjunctions (start with :)
                taxiRoadIdNow = tmpTaxiRoadIdNow; // Update the roadId
            }
            cout << " traciVehicle->getRoadId(): " << traciVehicle->getRoadId() << " taxiRoadIdNow: " << taxiRoadIdNow << endl;

            list<string>::iterator itPlannedRoadIds = plannedRoadIds.begin();
            string tmpTaxiRoute = *itPlannedRoadIds;
            itPlannedRoadIds++;
            for(; itPlannedRoadIds != plannedRoadIds.end(); ++itPlannedRoadIds) {
                tmpTaxiRoute += " " + *itPlannedRoadIds;
            }

            int posInti = tmpTaxiRoute.find(taxiRoadIdNow);
            string tmpTaxiSubRoute = tmpTaxiRoute.substr((posInti), tmpTaxiRoute.size());

            cout << "\ntmpTaxiRoute: \"" << tmpTaxiRoute << "\"";
            cout << "\ntmpTaxiSubRoute: \"" << tmpTaxiSubRoute << "\"\n";

            if (count(tmpTaxiSubRoute.begin(), tmpTaxiSubRoute.end(), ' ') == 0) { // Has only 1 lane to in the route
                tryChangeRouteUntilSuccess();
            }

            scheduleAt(simTime() + 1, sendTaxiChangeRoute);
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

void mfcv::vehUpdateRateTimeToSend() {
    //cout << source << " rateTimeToSend: " << rateTimeToSend;
    unsigned short int distance = traci->getDistance(mobility->getPositionAt(simTime() - rateTimeToSendUpdateTime), curPosition, false);

    if (distance >= rateTimeToSendDistanceControl) {
        if(rateTimeToSend > 100) { // Lower limit
            rateTimeToSend -= 100;
        }
    } else {
        if (rateTimeToSend < rateTimeToSendLimitTime) { // Upper limit
            rateTimeToSend += 100;
        }
    }
    //cout << " updated to: " << rateTimeToSend << " at: " << simTime() << " by: " <<  distance << " traveled [" << mobility->getPositionAt(simTime() - rateTimeToSendUpdateTime) << " " << curPosition << "]" << endl;
}

void mfcv::printMessagesBuffer() {
    if (!messagesBufferMfcv.empty()) {
        cout << endl << "messagesBuffer from " << source << " at: " << simTime() << " position: " << curPosition << endl;
        unordered_map <string, WaveShortMessage>::iterator itMessage;
        for (itMessage = messagesBufferMfcv.begin(); itMessage != messagesBufferMfcv.end(); itMessage++) {
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

void mfcv::printBeaconStatusNeighbors() {
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
unsigned short int mfcv::getVehHeading4() {
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

unsigned short int mfcv::getVehHeading8() {
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
