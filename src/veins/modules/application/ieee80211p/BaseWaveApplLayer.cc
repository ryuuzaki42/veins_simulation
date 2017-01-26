// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

const simsignalwrap_t BaseWaveApplLayer::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void BaseWaveApplLayer::initialize_veins_TraCI(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage == 0) {
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

        SbeaconTypeInitialize = par("beaconTypeInitialize");

        if (SbeaconTypeInitialize == 1) { // 1 MFCV
            sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_mfcv);
        } else if (SbeaconTypeInitialize == 2) { // 2 epidemic
            sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT_epidemic);
        } else if (SbeaconTypeInitialize == 0) { // 0 default
            sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        } else {
            cout << "JBe - beaconTypeInitialize is unknown -" << SbeaconTypeInitialize << endl;
            ASSERT2(0, "JBe - beaconTypeInitialize is unknown -");
        }

        // Simulate asynchronous channel access
        double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;
        //cout << findHost()->getFullName() << " Beacon offSet: " << offSet << endl; // Between 0 and 1

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
    }
}

//######################################### MFCV - begin #######################################################################
void BaseWaveApplLayer::saveMessagesOnFile(WaveShortMessage* wsm, string fileName) {
    myfile.open(fileName, std::ios_base::app); // Open file for just append

    //Send "strings" to be saved on the file
    myfile << "BeaconMessage from " << wsm->getSenderAddressTemporary() << " at: " << simTime();
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
    myfile << "wsm->getBufferMessageOnlyDeliveryFull(): " << wsm->getBufferMessageOnlyDeliveryFull() << endl;
    myfile << "Time to generate and received: " << (simTime() - wsm->getTimestamp()) << endl << endl;

    myfile.close();
}

void BaseWaveApplLayer::vehInitializeValuesMfcv(string category, Coord position) {
    generalInitializeVariables_executionByExpNumberMfcv();

    curPosition = position;
    vehOffSet = double(myId)/1000; // Simulate asynchronous channel access. Values between 0.001, 0.002
    SnumVehicles.push_back(source);
    cout << endl << "Count of vehicle in the scenario at: " << simTime() << " - " << SnumVehicles.size() << endl;
    ScountVehicleAll++;

    if (category.find("taxi") != std::string::npos) {
        vehCategory = 'T'; // Set 'T' to "taxi"
    } else if (category.find("passenger") != std::string::npos) {
        vehCategory = 'P'; // Set 'P' to "passenger"
    } else if (category.find("ignoring") != std::string::npos) {
        vehCategory = 'P'; // Set 'P' to "ignoring"
    } else if (category.find("bus") != std::string::npos) {
        vehCategory = 'B'; // Set 'B' to "bus"
    } else {
        cout << "JBe - Initial category: " << category << endl;
        ASSERT2(0, "JBe - Initial category is unknown - ");
    }

    if (vehCategory.compare("B") == 0) {
        ScountVehicleBus++;
    } else if (vehCategory.compare("T") == 0) {
        ScountVehicleTaxi++;
    }

    WaveShortMessage wsmTmp;
    wsmTmp.setTimestamp(simTime());
    wsmTmp.setCategory(vehCategory.c_str());
    SvehScenario.insert(make_pair(source, wsmTmp));

    restartFilesResultVeh(SprojectInfo, position);

    vehGenerateMessageBeginVeh(vehOffSet); // Create Event to generate messages by the vehicles

    cout << endl << source;
    if (par("beaconTypeInitialize").longValue() == 2) {
        cout << " (MACint: " << MACToInteger() << ")";
    }
    cout << " category: " << vehCategory << " (initial: " << category << ") entered in the ";
    cout << "scenario (position: " << curPosition << ") at: " << simTime() << " whit OffSet: " << vehOffSet << "\n\n";

    if (par("getTraffic").boolValue()) {
        insertVehTraffic(); // Create or not the vehicle traffic in one .csv file
    }
}
void BaseWaveApplLayer::insertVehTraffic() {
    traffic traf;
    traf.source = source;
    traf.entryTime = simTime();

    SvehTraffic.insert(make_pair(insertZeroIntReturnString(myId), traf));
}

string BaseWaveApplLayer::insertZeroIntReturnString(int value) {
    string stringValue;

    if (value < 10){
        stringValue = "000" + to_string(value);
    } else if (value < 100) {
        stringValue = "00" + to_string(value);
    } else if (value < 1000) {
        stringValue = '0' + to_string(value);
    } else {
        stringValue = to_string(value);
    }

    return stringValue;
}

void BaseWaveApplLayer::rsuInitializeValuesMfcv(Coord position) {
    generalInitializeVariables_executionByExpNumberMfcv();

    restartFilesResultRSU();

    rsuSelectVehGenerateMessageBegin();

    if (SrsuPositions.find(myId) == SrsuPositions.end()) {
        if ((myId < 3) && (myId >= 0)) {
            SrsuPositions.insert(make_pair(myId, position));
            SresultMsgReceived.countRsuTarget = SrsuPositions.size();
        } else {
            cout << source << " JBe - configuration to another RSU not implemented - myId: " << myId << endl;
            ASSERT2(0, "JBe - configuration to another RSU not implemented -");
        }
    } else {
        cout << source << " JBe - rsu already inserted in the rsuPosition - myId: " << myId << endl;
        ASSERT2(0, "JBe - rsu already inserted in the rsuPosition -");
    }

   cout << endl << source;
   if (par("beaconTypeInitialize").longValue() == 2) {
       cout << " (MACint: " << MACToInteger() << ")";
   }
   cout << " entered in the scenario at: " << simTime() << " in the position: " << SrsuPositions[myId] << "\n\n";

   if (source.compare("rsu[0]") == 0) {
       SresultMsgReceived.avgCopyMessageReceived = 0;
       SresultMsgReceived.avgHopsMessage = 0;
       SresultMsgReceived.avgTimeMessageReceived = 0;
       SresultMsgReceived.countFirstCategoryPrivateCar = 0;
       SresultMsgReceived.countSecondCategoryBus = 0;
       SresultMsgReceived.countThirdCategoryTaxi = 0;
       SresultMsgReceived.messageHopDiffZero = 0;
       SresultMsgReceived.messageHopEqualZero = 0;
       SresultMsgReceived.totalCopyReceived = 0;
       SresultMsgReceived.totalMessageReceived = 0;
       SresultMsgReceived.avgHopsFirstReceived = 0;
     }
}

void BaseWaveApplLayer::openFileAndClose(string fileName, bool justForAppend) {
    if (justForAppend) {
        myfile.open(fileName, std::ios_base::app);
    } else {
        myfile.open(fileName);
    }

    printHeaderfileExecution();
    myfile.close();
}

void BaseWaveApplLayer::printHeaderfileExecution() {
    myfile << endl;
    string expSendbyDSCRText = insertZeroIntReturnString(SexpSendbyDSCR);;

    myfile << "Exp: " << SexpNumber << " expSendbyDSCR: " << expSendbyDSCRText.c_str() << " ################";
    myfile << "##########################################################################" << endl;
    myfile << "Exp: " << SexpNumber << " expSendbyDSCR: " << expSendbyDSCRText.c_str() << " ### ExpNumber: " << SexpNumber << " RepeatNumber: " << SrepeatNumber;
    myfile << " ttlMessage: " << SttlMessage << " countGenerateMessage: " << ScountGenerateMessage << endl;
}

void BaseWaveApplLayer::generalInitializeVariables_executionByExpNumberMfcv() {
    source = findHost()->getFullName();
    msgBufferUse = 0;

    // Epidemic variables
    lastTimeSendLocalSummaryVector = nodesRecentlySendLocalSummaryVector = 0;

    countToDeliveryMsgLocal = 0;
    if (source.compare("rsu[0]") == 0) { // Static values inside of the if, that are the same for vehicle(s) and RSU(s)
        if (par("disableCout").boolValue()) {
            cout.setstate(std::ios_base::failbit); // "Disable" the cout
            //cout.clear(); // "Enable" the cout
        }

        SfirstCategoryPrivateCar = 'P';
        SsecondCategoryBus = 'B';
        SthirdCategoryTaxi = 'T';

        SbufferMessageOnlyDeliveryLimit = par("bufferMessageOnlyDeliveryLimit");
        ScountToDeliveryMsg = SmsgUseOnlyDeliveryBufferGeneral = 0;
        SsimulationTimeLimit = atoi(ev.getConfig()->getConfigValue("sim-time-limit"));
        SmessageBufferSize = par("messageBufferSize"); // Define the maximum buffer size (in number of messages) that a node is willing to allocate
        SmessageHopLimit = par("messageHopLimit").longValue();
        SrepeatNumber = atoi(ev.getConfig()->getConfigValue("seed-set")); // Number of execution (${repetition})
        SexpSendbyDSCR = par("expSendbyDSCR").longValue();
        SsendSummaryVectorInterval = par("sendSummaryVectorInterval").longValue(); // For Epidemic, Define the minimum slide window length among contacts to send new version of summary vector

        SallowMessageCopy = par("allowMessageCopy").boolValue();
        SselectFromAllVehicles = par("selectFromAllVehicles").boolValue();
        SusePathHistory = par("usePathHistory").boolValue(); // User or not path history when will send a message
        SuseMessagesSendLog = par("useMessagesSendLog").boolValue();
        SmfcvCreateEventGenerateMessage = par("mfcvCreateEventGenerateMessage").boolValue();
        SuseRateTimeToSend = par("useRateTimeToSend").boolValue();

        SttlBeaconStatus = par("ttlBeaconStatus");
        SbeaconStatusBufferSize = par("beaconStatusBufferSize");
        SuseBeaconStatusBufferSize = par("useBeaconStatusBufferSize");
        StimeToUpdatePosition = par("vehTimeUpdatePosition");
        StimeLimitGenerateMessage = par("timeLimitGenerateMessage");
        SvehTimeLimitToAcceptGenerateMgs = par("vehTimeLimitToAcceptGenerateMgs");

        SexpNumber = par("expNumber").longValue();
        if ((SexpNumber == 1) || (SexpNumber == 5)) {
            SttlMessage = par("ttlMessage_one").longValue();
            ScountGenerateMessage = par("countGenerateMessage_one").longValue();
        } else if ((SexpNumber == 2) || (SexpNumber == 6)) {
            SttlMessage = par("ttlMessage_one").longValue();
            ScountGenerateMessage = par("countGenerateMessage_two").longValue();
        } else if ((SexpNumber == 3) || (SexpNumber == 7)) {
            SttlMessage = par("ttlMessage_two").longValue();
            ScountGenerateMessage = par("countGenerateMessage_one").longValue();
        } else if ((SexpNumber == 4) || (SexpNumber == 8)) {
            SttlMessage = par("ttlMessage_two").longValue();
            ScountGenerateMessage = par("countGenerateMessage_two").longValue();
        } else {
            cout << "Error: Number of experiment not configured - SexpNumber" << SexpNumber << " class BaseWaveApplLayer.cc" << endl;
            ASSERT2(0, "JBe - Error: Number of experiment not configured -");
        }

        ScountMesssageDrop = ScountMsgPacketSend = SmsgBufferUseGeneral = ScountVehicleAll = ScountVehicleBus = 0;
        ScountBeaconSend = ScountSummaryVectorSend = ScountRequestMessageVectorSend = ScountVehicleTaxi= 0;
        SmsgDroppedbyTTL = SmsgDroppedbyCopy = SmsgDroppedbyBuffer = 0;
        ScountMeetTotal = ScountMeetTwoCategory = ScountMeetJustOneCategory = ScountMessageOnlyDeliveryBus = 0;
        SmessageId = 1;

        // Initialize random seed (Seed the RNG) # Inside of IF because must be executed one time (the seed is "static")
        mtSelectVehicleGenarateMessage.seed(SrepeatNumber); // Instead another value, for make the experiment more reproducible, so seed = reapeatNumber
        mtTargetMessageSelect.seed(SrepeatNumber); // Used to select a random target
        mtSelectLaneName.seed(SrepeatNumber); // Used to select lane name

        // To run with different routes files use only one seed set 1 to a the seed
        // Not use rand anymore //srand(SrepeatNumber + 1); // repeatNumber + 1, because srand(0) == srand(1)

        string texTmp = "\nExp: " + to_string(SexpNumber) + "_ ";
        SprojectInfo = texTmp;
        SprojectInfo += texTmp + "Project information";

        SprojectInfo += texTmp + "SfirstCategoryPrivateCar:_ \"" + SfirstCategoryPrivateCar + "\"";
        SprojectInfo += texTmp + "SsecondCategoryBus:_ \"" + SsecondCategoryBus + "\"";
        SprojectInfo += texTmp + "SThirdCategoryTaxi:_ \"" + SthirdCategoryTaxi + "\"";
        SprojectInfo += texTmp + "beaconTypeInitialize:_ " + boolToString(SbeaconTypeInitialize);
        SprojectInfo += texTmp + "mfcvCreateEventGenerateMessage:_ " + boolToString(SmfcvCreateEventGenerateMessage);
        SprojectInfo += texTmp + "Experiment:_ " + to_string(SexpNumber);
        SprojectInfo += texTmp + "repeatNumber:_ " + to_string(SrepeatNumber);
        SprojectInfo += texTmp + "ttlMessage:_ " + to_string(SttlMessage) + " s";
        SprojectInfo += texTmp + "countGenerateMessage:_ " + to_string(ScountGenerateMessage);
        SprojectInfo += texTmp + "allowMessageCopy:_ " + boolToString(SallowMessageCopy);
        SprojectInfo += texTmp + "selectFromAllVehicles:_ " + boolToString(SselectFromAllVehicles);
        SprojectInfo += texTmp + "timeLimitGenerateMessage:_ " + to_string(StimeLimitGenerateMessage) + " s";
        SprojectInfo += texTmp + "messageHopLimit:_ " + to_string(SmessageHopLimit);
        SprojectInfo += texTmp + "messageBufferSize:_ " + to_string(SmessageBufferSize);
        SprojectInfo += texTmp + "expSendbyDSCR:_ " + to_string(SexpSendbyDSCR);
        SprojectInfo += texTmp;
        SprojectInfo += texTmp + "beaconLengthBits:_ " + to_string(beaconLengthBits);
        SprojectInfo += texTmp + "headerLength:_ " + to_string(headerLength);
        SprojectInfo += texTmp + "dataLengthBits:_ " + to_string(dataLengthBits);

        SprojectInfo += texTmp;
        if (SbeaconTypeInitialize == 1) {
            SprojectInfo += texTmp + "useRateTimeToSend:_ " + boolToString(SuseRateTimeToSend);
            if (!SuseRateTimeToSend) {
                stringstream ss;
                ss << fixed << setprecision(1) << par("normalTimeSendMessage").doubleValue();
                SprojectInfo += texTmp + "normalTimeSendMessage:_ " + ss.str() + " s";
            }

            SprojectInfo += texTmp + "ttlBeaconStatus:_ " + to_string(SttlBeaconStatus) + " s";
            SprojectInfo += texTmp + "beaconStatusBufferSize:_ " + to_string(SbeaconStatusBufferSize);
            SprojectInfo += texTmp + "useBeaconStatusBufferSize:_ " + boolToString(SuseBeaconStatusBufferSize);
            SprojectInfo += texTmp + "usePathHistory:_ " + boolToString(SusePathHistory);
            SprojectInfo += texTmp + "useMessagesSendLog:_ " + boolToString(SuseMessagesSendLog);
            SprojectInfo += texTmp + "timeToUpdatePosition:_ " + to_string(StimeToUpdatePosition) + " s";
            SprojectInfo += texTmp + "bufferMessageOnlyDeliveryLimit:_ " + to_string(SbufferMessageOnlyDeliveryLimit);
        } else if (SbeaconTypeInitialize == 2) {
            SprojectInfo += texTmp + "sendSummaryVectorInterval:_ " + to_string(SsendSummaryVectorInterval) + " s";
        } else {
            cout << "JBe - beaconTypeInitialize is unknown -" << SbeaconTypeInitialize << endl;
            ASSERT2(0, "JBe - beaconTypeInitialize is unknown -");
        }

        SprojectInfo += getCFGVAR();

        cout << endl << SprojectInfo << endl;
    }
}

string BaseWaveApplLayer::getCFGVAR() { // Variables from omnetpp-4.6/include/cconfiguration.h
    int trafficGranularitySum = par("trafficGranularitySum");

    string texTmp = "\nExp: " + to_string(SexpNumber) + "_ ";

    string getAll = texTmp;

    getAll += texTmp + "simulationLimit:_ " + to_string(SsimulationTimeLimit) + string("s");
    getAll += texTmp + "trafficGranularitySum:_ " + to_string(trafficGranularitySum) + string("s");

    Veins::TraCIScenarioManagerLaunchd* traciSMLget = Veins::TraCIScenarioManagerLaunchdAccess().get();

    double penetrationRateValueScenario = traciSMLget->par("penetrationRate").doubleValue();
    penetrationRateValueScenario *= 100;
    getAll += texTmp + "penetrationRateValueScenario:_ " + to_string(int(penetrationRateValueScenario)) + string("%");

    int seedScenarioManager = traciSMLget->par("seed");
    getAll += texTmp + "seedScenario (*.manager.seed):_ " + to_string(seedScenarioManager) + string("_if -1 will use CFGVAR_RUNNUMBER");

    cConfigurationEx* configExGet = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx();

    string seedScenarioManager2 = configExGet->getVariable(CFGVAR_RUNNUMBER);
    getAll += texTmp + "seedScenarioManager2 (*.manager.seed):_ " + seedScenarioManager2 + string("_CFGVAR_RUNNUMBER value");

    string seedNumberSet = ev.getConfig()->getConfigValue("seed-set");
    getAll += texTmp + "seedScenario (seed-set):_ " + to_string(atoi(seedNumberSet.c_str()));

    string folderStartScenario = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    getAll += texTmp + "Folder scenario start:_ " + folderStartScenario;

    cXMLElement* launchConfigScenario = traciSMLget->par("launchConfig").xmlValue();
    string folderScenarioLaunch = launchConfigScenario->getSourceLocation();
    getAll += texTmp + "folderScenarioLaunch:_ " + folderScenarioLaunch;

    getAll += texTmp;
    getAll += texTmp + "runid(General-Run-YMD-Hour-PID) :_ " + configExGet->getVariable(CFGVAR_RUNID);
    getAll += texTmp + "inifile:_ " + configExGet->getVariable(CFGVAR_INIFILE);
    getAll += texTmp + "configname:_ " + configExGet->getVariable(CFGVAR_CONFIGNAME);
    getAll += texTmp + "runnumber:_ " + configExGet->getVariable(CFGVAR_RUNNUMBER);
    getAll += texTmp + "network:_ " + configExGet->getVariable(CFGVAR_NETWORK);
    getAll += texTmp + "experiment:_ " + configExGet->getVariable(CFGVAR_EXPERIMENT);
    getAll += texTmp + "measurement:_ " + configExGet->getVariable(CFGVAR_MEASUREMENT);
    getAll += texTmp + "replication:_ " + configExGet->getVariable(CFGVAR_REPLICATION);
    //getAll += texTmp + "processid:_ " + configExGet->getVariable(CFGVAR_PROCESSID);
    //getAll += texTmp + "datetime:_ " + configExGet->getVariable(CFGVAR_DATETIME);
    getAll += texTmp + "resultdir:_ " + configExGet->getVariable(CFGVAR_RESULTDIR);
    getAll += texTmp + "repetition:_ " + configExGet->getVariable(CFGVAR_REPETITION);
    getAll += texTmp + "seedset:_ " + configExGet->getVariable(CFGVAR_SEEDSET);
    getAll += texTmp + "iterationvars - without $repetition:_ " + configExGet->getVariable(CFGVAR_ITERATIONVARS);;
    getAll += texTmp + "iterationvars2 - with $repetition:_ " + configExGet->getVariable(CFGVAR_ITERATIONVARS2);
    getAll += texTmp + "\n";

    return getAll;
}

string BaseWaveApplLayer::boolToString(bool value) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

string BaseWaveApplLayer::getFolderResultMfcv(unsigned short int expSendbyDSCR) {
    string expSendbyDSCRText;
    switch (expSendbyDSCR) {
        case 1:
            expSendbyDSCRText = "0001_chooseByDistance";
            break;
        case 12:
            expSendbyDSCRText = "0012_chooseByDistance_Speed";
            break;
        case 13:
            expSendbyDSCRText = "0013_chooseByDistance_Category";
            break;
        case 134:
            expSendbyDSCRText = "0134_chooseByDistance_Category_RateTimeToSend";
            break;
        case 99:
            expSendbyDSCRText = "0099_epidemic";
            break;
        default:
            cout << "JBe - Error, expSendbyDSCR: " << expSendbyDSCR << " not defined, class in BaseWaveApplLayer.cc";
            ASSERT2(0, "JBe - Error expSendbyDSCR value not defined -");
    }

    string resultFolderPart = "results/";
    if (SbeaconTypeInitialize == 1) {
        resultFolderPart += "mfcv_resultsEnd_";
    } else if (SbeaconTypeInitialize == 2) {
        resultFolderPart += "epidemic_resultsEnd_";
    } else {
        cout << "JBe - beaconTypeInitialize is unknown -" << SbeaconTypeInitialize << endl;
        ASSERT2(0, "JBe - beaconTypeInitialize is unknown -");
    }

    unsigned short int expPartOneOrTwo = par("expPart_one_or_two");
    resultFolderPart += to_string(expPartOneOrTwo) + "/" + expSendbyDSCRText + "/" + "E" + to_string(SexpNumber) + "_";
    resultFolderPart += to_string(SttlMessage) + "_" + to_string(ScountGenerateMessage) +"/";

    string seedNumber = ev.getConfig()->getConfigValue("seed-set");
    resultFolderPart += "run_" + seedNumber + "/";
    return resultFolderPart;
}

void BaseWaveApplLayer::restartFilesResultRSU() {
    bool justAppend = false; // Open a new file (blank)
    if (myId == 0) {
        SresultFolder = getFolderResultMfcv(SexpSendbyDSCR);

        SfileMessagesCountRsu = SresultFolder + "rsu_" + "Count_Messages_Received.r";
        SfileMessagesGeneratedVehRsu = SresultFolder + "VehRsu_Messages_Generated.r";

        string commandCreateFolder = "mkdir -p " + SresultFolder + " 2> /dev/null";
        cout << endl << "Created the folder, command: \"" << commandCreateFolder << "\"" << endl;
        cout << "repeatNumber: " << SrepeatNumber << endl;
        system(commandCreateFolder.c_str()); // Try create a folder to save the results

        openFileAndClose(SfileMessagesCountRsu, justAppend);
        openFileAndClose(SfileMessagesGeneratedVehRsu, justAppend);

        if (par("getTraffic").boolValue()) {
            getTrafficEvtMethodCheck = new cMessage("get traffic evt", SendGetTrafficMethodCheck);

            //map <string, cModule*> vehInsideSmap =  Veins::TraCIScenarioManagerLaunchdAccess().get()->getManagedHosts();

            int countVehScenario = Veins::TraCIScenarioManagerLaunchdAccess().get()->getManagedHosts().size();
            SvehGetTrafficMethodCheck.insert(make_pair(int(simTime().dbl()), countVehScenario));

            scheduleAt(simTime() + par("trafficGranularitySum"), getTrafficEvtMethodCheck);
        }
    } //else { // repeatNumber != 0 just append
//        justAppend = true;
//    }

    fileMessagesBroadcastRsu = fileMessagesUnicastRsu = SresultFolder + source + "_";

    fileMessagesUnicastRsu += "Messages_Received.r";
    fileMessagesBroadcastRsu += "Broadcast_Messages.r";

    openFileAndClose(fileMessagesUnicastRsu, justAppend);
    openFileAndClose(fileMessagesBroadcastRsu, justAppend);
}

void BaseWaveApplLayer::restartFilesResultVeh(string projectInfo, Coord initialPos) {
    if (myId == 0) {
        SfileMessagesUnicastVeh = SfileMessagesDropVeh = SresultFolder;
        SfileMessagesUnicastVeh += "Veh_Unicast_Messages.r";
        SfileMessagesDropVeh += "Veh_Messages_Drop.r";

        bool justAppend = false;
        openFileAndClose(SfileMessagesUnicastVeh, justAppend);
        openFileAndClose(SfileMessagesDropVeh, justAppend);

        myfile.open(SfileMessagesDropVeh, std::ios_base::app);
        myfile << projectInfo << endl;
        myfile.close();
    }

    //saveVehStartPositionVeh(SresultFolder, initialPos); // Save the start position of vehicle. Just for test of the seed
}

void BaseWaveApplLayer::saveVehStartPositionVeh(string fileNameLocation, Coord initialPos) {
    fileNameLocation += "Veh_Position_Initialize.r";
    if (myId == 0) {
        myfile.open(fileNameLocation);

        printHeaderfileExecution();
        myfile << "Start Position Vehicles" << endl;
    } else {
        myfile.open(fileNameLocation, std::ios_base::app);
    }

    myfile << source << " : " << initialPos << endl;
    myfile.close();
}

void BaseWaveApplLayer::vehGenerateMessageBeginVeh(double vehOffSet) {
    if (SmfcvCreateEventGenerateMessage) { // Create Event to generate messages by the vehicles
        sendGenerateMessageEvt = new cMessage("Event generate beacon message", SendEvtGenerateMessage);
        //cout << source << " at: " << simTime() << " schedule created SendEvtGenerateMessage to: "<< (simTime() + vehOffSet) << endl;
        scheduleAt((simTime() + vehOffSet), sendGenerateMessageEvt);
    } else { // All vehicle that enter will generate only one message
        generateMessageMfcvAndEpidemic();
    }
}

void BaseWaveApplLayer::rsuSelectVehGenerateMessageBegin() {
    if (myId == 0) { // Only the rsu[0] select vehicles to generate messages
        if (SmfcvCreateEventGenerateMessage) { // Create Event to select vehicles to generate messages
            sendSelectVehGenerateMessageEvt = new cMessage("Event generate beacon message", SendEvtSelectVehGenerateMessage);
            //cout << source << " at: " << simTime() << " schedule created SendEvtSelectVehGenerateMessage to: "<< simTime() << endl;
            scheduleAt((simTime() + par("timeStartGenerateMessage")), sendSelectVehGenerateMessageEvt); // + 30 of warm-up time
        }
    }
}

void BaseWaveApplLayer::vehGenerateBeaconMessageAfterBeginVeh() {
    auto itVeh = find(SvehGenerateMessage.begin(), SvehGenerateMessage.end(), source);
    if (itVeh != SvehGenerateMessage.end()) { // If have "vehNumber" on buffer, will generate one message
        generateMessageMfcvAndEpidemic();

        SvehGenerateMessage.erase(itVeh);
        vehGenerateBeaconMessageAfterBeginVeh(); // To try generate another message, if the vehicle was choosen one time
    }
}

void BaseWaveApplLayer::selectTarget() { // Set the target node to who the message has to be delivered
    uniform_int_distribution <int> dist(0, (SrsuPositions.size() - 1)); // generate a value, e.g, dist(0, 10), will be 0, 1, ... 10
    int targetRsuId = dist(mtTargetMessageSelect);

    if (SrsuPositions.find(targetRsuId) != SrsuPositions.end()) {
        target = "rsu[" + to_string(targetRsuId) + "]";
        target_x = SrsuPositions[targetRsuId].x;
        target_y = SrsuPositions[targetRsuId].y;
    } else {
        cout << source << " JBe - rsu is not inserted in the rsuPosition - myId: " << myId << " targetRsuId: "<< targetRsuId << endl;
        ASSERT2(0, "JBe - rsu is not inserted in the rsuPosition -");
    }
}

void BaseWaveApplLayer::generateMessageMfcvAndEpidemic() {
    WaveShortMessage* wsm = new WaveShortMessage("data");
    wsm->addBitLength(headerLength);
    wsm->addBitLength(dataLengthBits);
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    switch (channel) {
        case type_SCH: // Will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
            wsm->setChannelNumber(Channels::SCH1);
            break;
        case type_CCH:
            wsm->setChannelNumber(Channels::CCH);
            break;
    }
    wsm->setPsid(0);
    wsm->setPriority(dataPriority);
    wsm->setWsmVersion(1);
    wsm->setSerial(2);
    wsm->setTimestamp(simTime());

    wsm->setSource(source.c_str());
    selectTarget(); // target = rsu[x] (x =1, 2, 3 etc) and respective position
    wsm->setTarget(target.c_str());

    // Not used in messages
    // wsm->setSenderPosPrevious()
    // wsm->setSenderPos()
    // wsm->setRateTimeToSend()
    // wsm->setRoadId()

    wsm->setHopCount(SmessageHopLimit + 1); // Is beaconMessageHopLimit+1 because hops count is equals to routes in the path, not hops.
    string wsmDataTmp = "WSMData generated by " + source;
    wsm->setWsmData(wsmDataTmp.c_str());

    myfile.open(SfileMessagesGeneratedVehRsu, std::ios_base::app); // Save info (Id and vehicle generate) on fileMessagesGenerated
    myfile << "                                                                    ";

    wsm->setGlobalMessageIdentificaton(insertZeroIntReturnString(SmessageId).c_str());

    myfile << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " to " << target << " " << Coord(target_x, target_y, 3) << " at: " << simTime() << endl;
    cout << "### " << source << " generated the message ID: " << wsm->getGlobalMessageIdentificaton() << " to " << target << " " << Coord(target_x, target_y, 3) << " at: " << simTime() << endl;
    myfile.close();

    if (SbeaconTypeInitialize == 1) { // MFCV
        wsm->setTargetPos(Coord(target_x, target_y, 3));
        wsm->setSenderAddressTemporary(source.c_str());
        // wsm->setRecipientAddressTemporary("Initial"); // Set when will send

        messagesBufferMfcv.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm)); // Adding the message on the buffer
        messagesOrderReceivedMfcv.push_back(wsm->getGlobalMessageIdentificaton());

        //colorCarryMessageMfcv(messagesBufferMfcv); // Change the range-color in the vehicle (GUI)
    } else if (SbeaconTypeInitialize == 2) { // Epidemic
        wsm->setSenderAddress(MACToInteger());
        // wsm.setRecipientAddress(); // Set when will send

        wsm->setSummaryVector(false);
        wsm->setRequestMessages(false);

        epidemicLocalMessageBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
        epidemicLocalSummaryVector.insert(make_pair(wsm->getGlobalMessageIdentificaton(), true));

        //colorCarryMessageMfcv(epidemicLocalMessageBuffer);
    } else {
        cout << "JBe - beaconTypeInitialize is unknown -" << SbeaconTypeInitialize << endl;
        ASSERT2(0, "JBe - beaconTypeInitialize is unknown -");
    }

    msgBufferUse++;
    SmessageId++;
}

void BaseWaveApplLayer::printCountBeaconMessagesDropVeh() {
    myfile.open (SfileMessagesDropVeh, std::ios_base::app);

    myfile << endl << "messagesDrop from " << source << endl;
    unsigned short int messageDropbyOneVeh = 0;
    map <string, struct messagesDropStruct>::iterator itMessageDrop;
    for (itMessageDrop = messagesDropVeh.begin(); itMessageDrop != messagesDropVeh.end(); itMessageDrop++) {
        myfile << "Message Id: " << itMessageDrop->first << endl;
        myfile << "By Buffer: " << itMessageDrop->second.byBuffer << endl;
        myfile << "By Time: " << itMessageDrop->second.byTime << endl;
        myfile << "By Copy: " << itMessageDrop->second.byCopy << endl;
        myfile << "TimeGenerate: " << itMessageDrop->second.timeGenerate << endl;
        myfile << "TimeDroped: " << itMessageDrop->second.timeDroped << endl;
        myfile << "TimeDifference: " << itMessageDrop->second.timeDifference << endl << endl;
        messageDropbyOneVeh += itMessageDrop->second.byBuffer + itMessageDrop->second.byTime;
        SmsgDroppedbyTTL += itMessageDrop->second.byTime;
        SmsgDroppedbyCopy += itMessageDrop->second.byCopy;
        SmsgDroppedbyBuffer += itMessageDrop->second.byBuffer;
    }

    ScountMesssageDrop += messageDropbyOneVeh;
    myfile << "### " << source << " dropped: " << messageDropbyOneVeh << endl;
    myfile << "### " << source << " use message buffer: " << msgBufferUse << endl;
    SmsgBufferUseGeneral += msgBufferUse;

    if (SnumVehicles.size() == 1) {
        string textTmp = "\nExp: " + to_string(SexpNumber) + " ###veh ";

        myfile << textTmp + "Final count messages drop: " << ScountMesssageDrop;
        myfile << textTmp + "Final count message dropped by buffer: " << SmsgDroppedbyBuffer;
        myfile << textTmp + "Final count message dropped by copy (Only valid if copy of message are not allowed): " << SmsgDroppedbyCopy;
        myfile << textTmp + "Final count message dropped by ttl: " << SmsgDroppedbyTTL;
        myfile << textTmp + "Final average buffer use: " << double(SmsgBufferUseGeneral)/ScountVehicleAll;
        myfile << textTmp;
        myfile << textTmp + "Count of vehicle in the scenario (all time): " << ScountVehicleAll;
        myfile << textTmp + "Count of vehicle Bus in the scenario (all time): " << ScountVehicleBus;
        myfile << textTmp + "Count of vehicle Taxi in the scenario (all time): " << ScountVehicleTaxi;
        myfile << textTmp;
        myfile << textTmp + "MFCV";
        myfile << textTmp + "        Count total of meetings: " << ScountMeetTotal;
        myfile << textTmp + "        Count total of meetings: " << ScountMeetTotal;
        myfile << textTmp + "        Count meetings with three category: " << (ScountMeetTotal - (ScountMeetTwoCategory + ScountMeetJustOneCategory));
        myfile << textTmp + "        Count meetings with two category: " << ScountMeetTwoCategory;
        myfile << textTmp + "        Count meetings with just one category: " << ScountMeetJustOneCategory;
        myfile << textTmp;
        myfile << textTmp + "        Count message OnlyDelivery sent to bus: " << ScountMessageOnlyDeliveryBus;
        myfile << textTmp;
        myfile << textTmp + "Count beacon send: " << ScountBeaconSend;
        myfile << textTmp + "Final count packets messages send: " << ScountMsgPacketSend;
        myfile << textTmp + "        Epidemic count SummaryVectorSend: " << ScountSummaryVectorSend;
        myfile << textTmp + "        Epidemic count RequestMessageVectorSend: " << ScountRequestMessageVectorSend << endl;
        myfile << textTmp + "        Epidemic (full) is the sum of those 3: " << (ScountMsgPacketSend + ScountSummaryVectorSend + ScountRequestMessageVectorSend) << endl;
    }
    myfile.close();
}

void BaseWaveApplLayer::toFinishVeh() {
    printCountBeaconMessagesDropVeh();

    auto itVeh = find(SnumVehicles.begin(), SnumVehicles.end(), source);
    if (itVeh != SnumVehicles.end()) {
        SnumVehicles.erase(itVeh);
        cout << endl << "Count of vehicle in the scenario at: " << simTime() << " - " << SnumVehicles.size() << endl;
        SvehScenario.erase(source);
    } else {
        cout << "JBe - Error in mfcv::numVehicles, need to have the same entries as the number of vehicles" << endl;
        ASSERT2(0, "JBe - Error in mfcv::numVehicles, need to have the same entries as the number of vehicles -");
    }

    if (SvehCategoryCount.find(vehCategory) == SvehCategoryCount.end()) {
        SvehCategoryCount.insert(make_pair(vehCategory, 1));
    }else {
        SvehCategoryCount[vehCategory]++;
    }

    if (par("getTraffic").boolValue()) {
        string myIdString = insertZeroIntReturnString(myId);
        if (SvehTraffic.find(myIdString) != SvehTraffic.end()) {
            SvehTraffic[myIdString].exitTime = simTime();
        } else {
            cout << "JBe - Error in mfcv::numVehicles, need to have the same entries as the number of vehicles" << endl;
            ASSERT2(0, "JBe - Error in mfcv::numVehicles, need to have the same entries as the number of vehicles -");
        }

        printVehTraffic();
    }
}

void BaseWaveApplLayer::printVehTraffic() {
    if (SnumVehicles.size() == 0) {
        int trafficGranularitySum = par("trafficGranularitySum");

        int countValue = SsimulationTimeLimit/trafficGranularitySum + 1; // e.g., 600/60 + 1 => 11, or 310/30 +1 = 11

        int valueEntry[countValue], valueExit[countValue];
        for (int i = 0; i < countValue; i++) {
            valueEntry[i] = 0;
            valueExit[i] = 0;
        }

        string fileName="results/vehTraffic_simLimt" + to_string(SsimulationTimeLimit) + "s_gran_"+ to_string(trafficGranularitySum) + "s_.csv";
        myfile.open(fileName);
        myfile << endl << "_This CVS file is separated by underscore\n\n\n";

        myfile << SprojectInfo;

        double sumTimeSecond, sumTimeMin, diffTimeVeh;
        sumTimeSecond = sumTimeMin = 0;

        int numValue = 1;
        myfile << "numValue_myId_Source_entryTime (s)_exitTime (s)_timeInsideScenario (s)_timeInsideScenarioIMin (min)" << endl;
        map <string, struct traffic>::iterator itVehTraffic;
        for (itVehTraffic = SvehTraffic.begin(); itVehTraffic != SvehTraffic.end(); itVehTraffic++) {
            diffTimeVeh = itVehTraffic->second.exitTime.dbl() - itVehTraffic->second.entryTime.dbl();

            sumTimeSecond += diffTimeVeh;
            sumTimeMin += (diffTimeVeh/60);

            myfile << numValue << "_" << itVehTraffic->first << "_" << itVehTraffic->second.source << "_";
            myfile << itVehTraffic->second.entryTime << "_" << itVehTraffic->second.exitTime << "_";
            myfile << diffTimeVeh << "_" << (diffTimeVeh/60) << endl;

            //valueEntry
            int tmpCount = trafficGranularitySum;
            int vectorIndex = 0;
            bool flagContinue = true;
            while (flagContinue) {
                if (itVehTraffic->second.entryTime < tmpCount) {
                    valueEntry[vectorIndex]++;
                    flagContinue = false;
                } else {
                    vectorIndex++;
                    tmpCount += trafficGranularitySum;
                }
            }

            //valueExit
            tmpCount = trafficGranularitySum;
            vectorIndex = 0;
            flagContinue = true;
            while (flagContinue) {
                if (itVehTraffic->second.exitTime < tmpCount) {
                    valueExit[vectorIndex]++;
                    flagContinue = false;
                } else {
                    vectorIndex++;
                    tmpCount += trafficGranularitySum;
                }
            }

            numValue++;
        }

        int j, sumValuesEntry, sumValuesExit;
        j = trafficGranularitySum;
        sumValuesEntry = sumValuesExit = 0;

        myfile << endl << endl << endl;
        myfile << "Time (s)_Time (min)_Inside Scenario (count veh)_Entry (count veh)_Exit (count veh)" << endl;
        myfile << "0_0_0_0_0" << endl;

        for (int i = 0; i < countValue; i++) {
            sumValuesEntry += valueEntry[i];
            sumValuesExit += valueExit[i];

            myfile << j << "_" << double(j)/60 << "_" << (sumValuesEntry - sumValuesExit);
            myfile << "_" << sumValuesEntry << "_" << sumValuesExit << endl;

            j += trafficGranularitySum;
        }

        myfile << endl << endl << endl;
        myfile << "Total of vehicles_" << SvehTraffic.size() << endl;
        myfile << "sumTimeSecond_" << sumTimeSecond << " s" << endl;
        myfile << "average Second_" << sumTimeSecond/SvehTraffic.size() << " s" << endl;
        myfile << "sumTimeMin_" << sumTimeMin << " min" << endl;
        myfile << "average Min_" << sumTimeMin/SvehTraffic.size() << " min" << endl;

        myfile.close();
    }
}

void BaseWaveApplLayer::colorCarryMessageMfcv(unordered_map <string, WaveShortMessage> bufferOfMessages) {
    if (!bufferOfMessages.empty()) {
        unordered_map <string, WaveShortMessage>::iterator itMessage = bufferOfMessages.begin();
        for (unsigned int i = 0; i < bufferOfMessages.size(); i++) {
            if (source.compare(itMessage->second.getSource()) == 0) {
                findHost()->getDisplayString().updateWith("r=12,green"); // Has message(s) with himself was generated
                return; // Most important messages generated by himself
            } else {
                findHost()->getDisplayString().updateWith("r=12,blue"); // Has (only) message(s) with another was generated
            }
            itMessage++;
        }
    } else {
        findHost()->getDisplayString().updateWith("r=0"); // Remove the range color
    }
}

int BaseWaveApplLayer::SelectRandomVehiclePositionId() {
    if (SnumVehicles.size() < 1){
        cout << "JBe - error SnumVehicles.size() < 1" << endl;
        ASSERT2(0, "JBe - error SnumVehicles.size() < 1");
    }

    uniform_int_distribution <int> dist(0, (SnumVehicles.size() - 1)); // generate a value, e.g, dist(0, 10), will be 0, 1, ... 10
    return dist(mtSelectVehicleGenarateMessage);
}

void BaseWaveApplLayer::selectVehGenerateMessage() {
    if (myId == 0) { // Only rsu[0] will/can select the vehicle to generate messages
        myfile.open(SfileMessagesGeneratedVehRsu, std::ios_base::app); // To save info (Id and vehicle generate) on fileMessagesGenerated
        cout << source << " at: " << simTime() << " in selectVehGenerateMessage, timeLimitGenerateMessage: " << StimeLimitGenerateMessage << endl;

        if (simTime() <= StimeLimitGenerateMessage) { // Modify the generate message and test StimeLimitGenerateMessage
            unsigned short int vehSelected, trySelectVeh;
            trySelectVeh = 0;

            for (unsigned short int i = 0; i < ScountGenerateMessage;) { // select vehicle to generate messages
                vehSelected = SelectRandomVehiclePositionId(); // return a number from 0 to (SnumVehicles.size() - 1), the index vector
                string vehSelectedId = SnumVehicles[vehSelected]; // Get the vehicle name

                // Vehicle can be choosen to generate more than one message?
                auto itVehSelected = find(SvehGenerateMessage.begin(), SvehGenerateMessage.end(), vehSelectedId);
                if (itVehSelected == SvehGenerateMessage.end()) {
                    // If SselectFromAllVehicles false, will test if vehicle are less than 60 s in the scenario
                    if ((SselectFromAllVehicles) || (SvehScenario[vehSelectedId].getTimestamp() + SvehTimeLimitToAcceptGenerateMgs) >= simTime()) {
                        SvehGenerateMessage.push_back(vehSelectedId);
                        cout << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                        myfile << source << " selected the " << vehSelectedId << " to generate a message at: " << simTime() << endl;
                        i++;
                    } else {
                        cout << endl << source << " selected " << vehSelectedId << " to generate " << SmessageId;
                        cout << " message, but has Timestamp: " << SvehScenario[vehSelectedId] <<" at: " << simTime() << endl;
                        trySelectVeh++;
                    }
                } else {
                    trySelectVeh++;
                }

                if (trySelectVeh > (SvehScenario.size() * 4)) {
                    cout << endl << "JBe - Error loop in select vehicle to generate message, class BaseWaveApplLayer.cc";
                    cout << endl << "trySelectVeh: " << trySelectVeh << " mfcv::vehScenario.size(): " << SvehScenario.size() << endl;
                    ASSERT2(0, "JBe - Error loop in select vehicle to generate message -");
                }
            }
        } else {
            cout << endl << source << " time is more than StimeLimitGenerateMessage: " << StimeLimitGenerateMessage << " to generate a message at: " << simTime() << endl;
        }

        myfile.close();
    }
}

void BaseWaveApplLayer::insertMessageDropVeh(string messageId, unsigned short int type, simtime_t timeGenarted) {
    if (messagesDropVeh.find(messageId) == messagesDropVeh.end()) {
        struct messagesDropStruct mD_tmp;
        mD_tmp.byBuffer = mD_tmp.byTime = mD_tmp.byCopy = 0;

        if (type == 1) { // by buffer limit
            mD_tmp.byBuffer = 1;
        } else if (type == 2) { // By copy not allow
            mD_tmp.byCopy = 1;
        } else if (type == 3) { // by TTL/time limit
            mD_tmp.byTime = 1;
        }

        mD_tmp.timeGenerate = timeGenarted;
        mD_tmp.timeDroped = simTime();
        mD_tmp.timeDifference = mD_tmp.timeDroped - mD_tmp.timeGenerate;

        messagesDropVeh.insert(make_pair(messageId, mD_tmp));
    } else {
        if (type == 1) { // Increment the number of byBuffer (limit)
            messagesDropVeh[messageId].byBuffer++;
        } else if (type == 2) {  // Increment the number of byCopy (limit)
            messagesDropVeh[messageId].byCopy++;
        } else if (type == 3) { // Increment the number of byTime (limit)
            messagesDropVeh[messageId].byTime++;
        }
    }
}

void BaseWaveApplLayer::printCountMessagesReceivedRSU() {
    myfile.open(SfileMessagesCountRsu, std::ios_base::app);

    string textTmp = "Exp: " + to_string(SexpNumber) + " expSendbyDSCR: " + to_string(SexpSendbyDSCR) + " ### " + source + " ";
    if (!messagesReceivedRSU.empty()) {
        SimTime avgGeneralTimeMessageReceived;
        unsigned int countFirstCategoryPrivateCar, countSecondCategoryBus, countThirdCategoryTaxi;
        double avgGeneralHopsMessage, avgGeneralCopyMessageReceived, avgGeneralFirstHopsReceived;

        avgGeneralTimeMessageReceived = 0;
        countFirstCategoryPrivateCar = countSecondCategoryBus = countThirdCategoryTaxi = 0;

        avgGeneralHopsMessage = avgGeneralCopyMessageReceived = 0;
        map <string, struct messages>::iterator itMessagesReceived;
        for (itMessagesReceived = messagesReceivedRSU.begin(); itMessagesReceived != messagesReceivedRSU.end(); itMessagesReceived++) {
            myfile << endl << "## Message ID: " << itMessagesReceived->first << endl;
            myfile << "Count received: " << itMessagesReceived->second.countCopyMessage << endl;
            avgGeneralCopyMessageReceived += itMessagesReceived->second.countCopyMessage;

            myfile << "First received source: " << itMessagesReceived->second.firstSource << endl;
            myfile << "Count first hop: " << itMessagesReceived->second.firstHopReceived << endl;
            avgGeneralFirstHopsReceived += itMessagesReceived->second.firstHopReceived;

            myfile << "Message timestamp generate: " << itMessagesReceived->second.timeGenerate << endl;
            myfile << "wsmData: " << itMessagesReceived->second.wsmData << endl;
            myfile << "Hops: " << itMessagesReceived->second.hops << endl;
            myfile << "Sum hops: " << itMessagesReceived->second.sumHops << endl;
            myfile << "Avg hops: " << double(itMessagesReceived->second.sumHops)/itMessagesReceived->second.countCopyMessage << endl;
            avgGeneralHopsMessage += double(itMessagesReceived->second.sumHops)/itMessagesReceived->second.countCopyMessage;

            myfile << "Max hop: " << itMessagesReceived->second.maxHop << endl;
            myfile << "Min hop: " << itMessagesReceived->second.minHop << endl;

            myfile << "Times: " << itMessagesReceived->second.times << endl;
            myfile << "Sum times: " << itMessagesReceived->second.sumTimeRecived << endl;
            avgGeneralTimeMessageReceived += itMessagesReceived->second.sumTimeRecived;
            myfile << "Average time to received: " << (itMessagesReceived->second.sumTimeRecived/itMessagesReceived->second.countCopyMessage) << endl;

            myfile << "Count first (" << SfirstCategoryPrivateCar << ") category: " << itMessagesReceived->second.countFirstCategoryPrivateCar << endl;
            countFirstCategoryPrivateCar += itMessagesReceived->second.countFirstCategoryPrivateCar;
            myfile << "Count second (" << SsecondCategoryBus << ") category: " << itMessagesReceived->second.countSecondCategoryBus << endl;
            countSecondCategoryBus += itMessagesReceived->second.countSecondCategoryBus;
            myfile << "Count third (" << SthirdCategoryTaxi << ") category: " << itMessagesReceived->second.countThirdCategoryTaxi << endl;
            countThirdCategoryTaxi += itMessagesReceived->second.countThirdCategoryTaxi;
        }

        unsigned short int messageCountHopZero = 0;
        string messageHopCountZeroIds, messageHopCountDifferentZeroIds;
        messageHopCountZeroIds = messageHopCountDifferentZeroIds = "";
        for (itMessagesReceived = messagesReceivedRSU.begin(); itMessagesReceived != messagesReceivedRSU.end(); itMessagesReceived++) {
            if (itMessagesReceived->second.sumHops == 0) {
                messageHopCountZeroIds += itMessagesReceived->first + ", ";
                messageCountHopZero++;
            } else {
                messageHopCountDifferentZeroIds += itMessagesReceived->first + ", ";
            }
        }

        myfile << "\nMessages received with hop count equal to zero:\n";
        myfile << messageHopCountZeroIds << endl;

        myfile <<"\nMessages received with hop count different of zero:\n";
        myfile << messageHopCountDifferentZeroIds << endl;

        avgGeneralFirstHopsReceived /= messagesReceivedRSU.size();
        avgGeneralHopsMessage /= messagesReceivedRSU.size();
        avgGeneralTimeMessageReceived /= avgGeneralCopyMessageReceived;

        double percentMsgReceived = double(messagesReceivedRSU.size() * 100 )/(SmessageId - 1);

        myfile << endl << endl << textTmp << endl;
        myfile << textTmp << "Count messages received: " << messagesReceivedRSU.size();
        myfile << " of " << (SmessageId - 1) << " or " << percentMsgReceived << " % received" << endl;
        SresultMsgReceived.totalMessageReceived += messagesReceivedRSU.size();

        myfile << textTmp << endl;
        myfile << textTmp << "Count copy message received: " << avgGeneralCopyMessageReceived << endl;
        SresultMsgReceived.totalCopyReceived += avgGeneralCopyMessageReceived;

        myfile << textTmp << "Count messages with hop count equal of zero received: " << messageCountHopZero << endl;
        SresultMsgReceived.messageHopEqualZero += messageCountHopZero;

        myfile << textTmp << "Count messages with hop count different of zero Received: " << (messagesReceivedRSU.size() - messageCountHopZero) << endl;
        SresultMsgReceived.messageHopDiffZero += (messagesReceivedRSU.size() - messageCountHopZero);

        myfile << textTmp << endl;
        myfile << textTmp << "Average time to receive: " << avgGeneralTimeMessageReceived << endl;
        SresultMsgReceived.avgTimeMessageReceived += avgGeneralTimeMessageReceived.dbl();

        avgGeneralCopyMessageReceived /= messagesReceivedRSU.size();
        myfile << textTmp << "Average copy received: " << avgGeneralCopyMessageReceived << endl;
        SresultMsgReceived.avgCopyMessageReceived += avgGeneralCopyMessageReceived;

        myfile << textTmp << "Average first hops to received: " << avgGeneralFirstHopsReceived << endl;
        SresultMsgReceived.avgHopsFirstReceived += avgGeneralFirstHopsReceived;

        myfile << textTmp << "Average hops to received: " << avgGeneralHopsMessage << endl;
        SresultMsgReceived.avgHopsMessage += avgGeneralHopsMessage;

        myfile << textTmp << endl;
        myfile << textTmp << "counttoDeliveryMsgLocal: " << countToDeliveryMsgLocal << endl;

        myfile << textTmp << endl;
        myfile << textTmp << "Count hops by category " << SfirstCategoryPrivateCar << " general: " << countFirstCategoryPrivateCar << endl;
        SresultMsgReceived.countFirstCategoryPrivateCar += countFirstCategoryPrivateCar;

        myfile << textTmp << "Count hops by category " << SsecondCategoryBus << " general: " << countSecondCategoryBus << endl;
        SresultMsgReceived.countSecondCategoryBus += countSecondCategoryBus;

        myfile << textTmp << "Count hops by category " << SthirdCategoryTaxi << " general: " << countThirdCategoryTaxi << endl;
        SresultMsgReceived.countThirdCategoryTaxi += countThirdCategoryTaxi;
    } else {
        myfile << endl << endl << textTmp << endl;
        myfile << textTmp << "Count messages received: " << 0 << endl;
        myfile << textTmp << "messagesReceived from " << source << " is empty" << endl;
    }

    if (SrsuPositions.size() == 1) { // SnumVehicles.size() == 0
        string textTmp = "Exp: " + to_string(SexpNumber) + " expSendbyDSCR: " + to_string(SexpSendbyDSCR) + " ### rsu[*] ";
        myfile << endl << endl << textTmp << endl;
        myfile << textTmp << "Total of messages received: " << SresultMsgReceived.totalMessageReceived;
        double percentMsgReceived = double(SresultMsgReceived.totalMessageReceived * 100 )/(SmessageId - 1);
        myfile << " of " << (SmessageId - 1) << " or " << percentMsgReceived << " % received" << endl;
        myfile << textTmp << endl;
        myfile << textTmp << "Total count copy message received: " << SresultMsgReceived.totalCopyReceived << endl;
        myfile << textTmp << "Total count messages with hop count equal of zero received: " << SresultMsgReceived.messageHopEqualZero << endl;
        myfile << textTmp << "Total count messages with hop count different of zero Received: " << SresultMsgReceived.messageHopDiffZero << endl;
        myfile << textTmp << endl;
        myfile << textTmp << "Total average time to receive: " << SresultMsgReceived.avgTimeMessageReceived/SresultMsgReceived.countRsuTarget << endl;
        myfile << textTmp << "Total average copy received: " << SresultMsgReceived.avgCopyMessageReceived/SresultMsgReceived.countRsuTarget << endl;
        myfile << textTmp << "Total average first hops to received: " << SresultMsgReceived.avgHopsFirstReceived/SresultMsgReceived.countRsuTarget << endl;
        myfile << textTmp << "Total average hops to received: " << SresultMsgReceived.avgHopsMessage/SresultMsgReceived.countRsuTarget << endl;
        myfile << textTmp << endl;
        myfile << textTmp << "Total count hops by category " << SfirstCategoryPrivateCar << " general: " << SresultMsgReceived.countFirstCategoryPrivateCar << endl;
        myfile << textTmp << "Total count hops by category " << SsecondCategoryBus << " general: " << SresultMsgReceived.countSecondCategoryBus << endl;
        myfile << textTmp << "Total count hops by category " << SthirdCategoryTaxi << " general: " << SresultMsgReceived.countThirdCategoryTaxi << endl;
        myfile << textTmp << endl;
        myfile << textTmp << "counttoDeliveryMsg: " << ScountToDeliveryMsg << endl;
        myfile << textTmp << endl;
        myfile << textTmp << "## Count of vehicle by category in the scenario\n";
        for (auto& x: SvehCategoryCount) {
            myfile << textTmp << "    Category: " << x.first << " count: " << x.second << endl;
        }

        myfile << textTmp << endl;
        myfile << textTmp << "msgUseOnlyDeliveryBufferGeneral total: " << SmsgUseOnlyDeliveryBufferGeneral;

        if (ScountVehicleBus == 0) {
            ScountVehicleBus = 1;
            if (SmsgUseOnlyDeliveryBufferGeneral != 0) {
                cout << "JBe - (msgUseOnlyDeliveryBufferGeneral != 0) with (ScountVehicleBus == 0) at: " << simTime() << endl;
                ASSERT2(0, "JBe - (msgUseOnlyDeliveryBufferGeneral != 0) with (ScountVehicleBus == 0) - ");
            }
        }
        myfile << textTmp << "msgUseOnlyDeliveryBufferGeneral avg: " << double(SmsgUseOnlyDeliveryBufferGeneral)/ScountVehicleBus;
        myfile << textTmp << endl;

        myfile << textTmp << endl;
        myfile << textTmp << "This simulation run terminated correctly" << endl;
        myfile << textTmp << endl;
    }

    myfile.close();
}

void BaseWaveApplLayer::messagesReceivedMeasuringRSU(WaveShortMessage* wsm) {
    string wsmData = wsm->getWsmData();
    simtime_t timeToArrived = (simTime() - wsm->getTimestamp());
    unsigned short int countHops = (SmessageHopLimit - wsm->getHopCount());
    map <string, struct messages>::iterator itMessagesReceived = messagesReceivedRSU.find(wsm->getGlobalMessageIdentificaton());

    if (itMessagesReceived != messagesReceivedRSU.end()) {
        itMessagesReceived->second.countCopyMessage++;

        itMessagesReceived->second.hops += ", " + to_string(countHops);
        itMessagesReceived->second.sumHops += countHops;
        if (countHops > itMessagesReceived->second.maxHop) {
            itMessagesReceived->second.maxHop = countHops;
        }
        if (countHops < itMessagesReceived->second.minHop) {
            itMessagesReceived->second.minHop = countHops;
        }

        itMessagesReceived->second.sumTimeRecived += timeToArrived;
        itMessagesReceived->second.times += ", " + timeToArrived.str();

        if (wsmData.size() > 42) { // WSMData generated by car[x] and carry by [ T,
            itMessagesReceived->second.wsmData += " & " + wsmData.substr(42); // To check
        }

        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        itMessagesReceived->second.countFirstCategoryPrivateCar += count(wsmData.begin(), wsmData.end(), SfirstCategoryPrivateCar[0]);
        itMessagesReceived->second.countSecondCategoryBus += count(wsmData.begin(), wsmData.end(), SsecondCategoryBus[0]);
        itMessagesReceived->second.countThirdCategoryTaxi += count(wsmData.begin(), wsmData.end(), SthirdCategoryTaxi[0]);
    } else {
        struct messages msg;
        msg.firstSource = wsm->getSource();
        msg.firstHopReceived = countHops;
        msg.countCopyMessage = 1;
        msg.wsmData = wsmData;
        msg.hops = to_string(countHops);
        msg.maxHop = msg.minHop = msg.sumHops = countHops;
        msg.timeGenerate = wsm->getTimestamp();

        msg.sumTimeRecived = timeToArrived;
        msg.times = timeToArrived.str();

        // Be aware, don't use the category identification as a value insert in the wsmData in the begin
        msg.countFirstCategoryPrivateCar = count(wsmData.begin(), wsmData.end(), SfirstCategoryPrivateCar[0]);
        msg.countSecondCategoryBus = count(wsmData.begin(), wsmData.end(), SsecondCategoryBus[0]);
        msg.countThirdCategoryTaxi = count(wsmData.begin(), wsmData.end(), SthirdCategoryTaxi[0]);

        messagesReceivedRSU.insert(make_pair(wsm->getGlobalMessageIdentificaton(), msg));
    }
}

void BaseWaveApplLayer::toFinishRSU() {
    printCountMessagesReceivedRSU();

    if (SrsuPositions.find(myId) != SrsuPositions.end()) {
        SrsuPositions.erase(myId);
    } else {
        cout << "JBe - rsu not inserted in the rsuPosition - myId:" << myId << " source: " << source << endl;
        ASSERT2(0, "JBe - rsu not inserted in the rsuPosition - ");
    }

    if (SrsuPositions.size() == 0) {
        printVehTrafficMethodCheck();
    }
}

void BaseWaveApplLayer::printVehTrafficMethodCheck() {
    if (par("getTraffic").boolValue()) {
        int trafficGranularitySum = par("trafficGranularitySum");

        string fileName="results/vehTraffic_simLimt" + to_string(SsimulationTimeLimit) + "s_gran_"+ to_string(trafficGranularitySum) + "s_MethodCheck.csv";
        myfile.open(fileName);
        myfile << endl << "_This CVS file is separated by underscore\n\n\n";

        myfile << "Time (s)_" << "Time (min)_" << "Inside Scenario (count veh)\n";

        map <simtime_t, int>::iterator itSvehGetTraffic;
        for (itSvehGetTraffic = SvehGetTrafficMethodCheck.begin(); itSvehGetTraffic != SvehGetTrafficMethodCheck.end(); itSvehGetTraffic++) {
            myfile << itSvehGetTraffic->first << "_"<< itSvehGetTraffic->first/60 << "_" << itSvehGetTraffic->second << endl;
        }

        myfile.close();
    }
}
//######################################### MFCV - end #########################################################################

//WaveShortMessage*  BaseWaveApplLayer::prepareWSM(string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
WaveShortMessage*  BaseWaveApplLayer::prepareWSM(string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    wsm->setSource(findHost()->getFullName());

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << endl;
    } else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << endl;
    }
    return wsm;
}

//############################################ Epidemic - begin ################################################################

WaveShortMessage* BaseWaveApplLayer::prepareWSM_epidemic(string name, int lengthBits, t_channel channel, int priority, unsigned int rcvId, int serial) {
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
    wsm->setTimestamp(simTime());

    wsm->setSenderAddress(MACToInteger());
    wsm->setRecipientAddress(rcvId);

    wsm->setSource(source.c_str());
    wsm->setTarget(target.c_str());

    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << endl;
    }else if (name == "data") {
        DBG << "Creating Data with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << endl;
    }
    return wsm;
}

unsigned int BaseWaveApplLayer::MACToInteger(){
    unsigned int macInt;
    stringstream ss;
    ss << std::hex << myMac;
    ss >> macInt;
    return macInt;
}

void BaseWaveApplLayer::printWaveShortMessage(WaveShortMessage* wsm) {
    cout << endl << findHost()->getFullName() << " print wsm message at: "<< simTime() << endl;
    cout << "wsm->getBitLength(): " << wsm->getBitLength() << endl;
    cout << "wsm->getCategory(): " << wsm->getCategory() << endl;
    cout << "wsm->getChannelNumber(): " << wsm->getChannelNumber() << endl;
    cout << "wsm->getGlobalMessageIdentificaton(): " << wsm->getGlobalMessageIdentificaton() << endl;
    cout << "wsm->getHopCount(): " << wsm->getHopCount() << endl << endl;
    cout << "wsm->getName(): " << wsm->getName() << endl;
    cout << "wsm->getPriority(): " << wsm->getPriority() << endl;
    cout << "wsm->getPsid(): " << wsm->getPsid() << endl;
    cout << "wsm->getRateTimeToSend(): " << wsm->getRateTimeToSend() << endl;
    cout << "wsm->getRecipientAddress(): " << wsm->getRecipientAddress() << endl;
    cout << "wsm->getRecipientAddressTemporary(): " << wsm->getRecipientAddressTemporary() << endl;
    cout << "wsm->getRequestMessages(): " << boolToString(wsm->getRequestMessages()) << endl;
    cout << "wsm->getRoadId(): " << wsm->getRoadId() << endl;
    cout << "wsm->getSenderAddress(): " << wsm->getSenderAddress() << endl;
    cout << "wsm->getSenderAddressTemporary(): " << wsm->getSenderAddressTemporary() << endl;
    cout << "wsm->getSenderPos(): " << wsm->getSenderPos() << endl;
    cout << "wsm->getSenderPosPrevious(): " << wsm->getSenderPosPrevious() << endl;
    cout << "wsm->getSerial(): " << wsm->getSerial() << endl;
    cout << "wsm->getSource(): " << wsm->getSource() << endl;
    cout << "wsm->getSummaryVector(): " << boolToString(wsm->getSummaryVector()) << endl;
    cout << "wsm->getTarget(): " << wsm->getTarget() << endl;
    cout << "wsm->getTimestamp(): " << wsm->getTimestamp() << endl;
    cout << "wsm->getWsmData(): " << wsm->getWsmData() << endl;
    cout << "wsm->getWsmLength(): " << wsm->getWsmLength() << endl;
    cout << "wsm->getWsmVersion(): " << wsm->getWsmVersion() << endl;
}

//Method used to initiate the anti-entropy session sending the epidemicLocalSummaryVector
void BaseWaveApplLayer::sendLocalSummaryVector(unsigned int rcvId) {
    string idMessage;
    unsigned short int countMessage = epidemicLocalMessageBuffer.size();
    unordered_map <string , WaveShortMessage>::iterator itMsg = epidemicLocalMessageBuffer.begin();
    //printEpidemicLocalMessageBuffer();
    while (countMessage > 0) {
        if ((itMsg->second.getTimestamp() + SttlMessage) < simTime()) {
            idMessage = itMsg->second.getGlobalMessageIdentificaton();

            if (countMessage == 1) {
                countMessage = 0;
            } else {
                countMessage--;
                itMsg++;
            }

            insertMessageDropVeh(idMessage, 3, itMsg->second.getTimestamp()); // Removed by the value of tyRemoved (1 buffer, 2 hop, 3 time)

            epidemicLocalMessageBuffer.erase(idMessage);
            epidemicLocalSummaryVector.erase(idMessage);
            //colorCarryMessageMfcv(epidemicLocalMessageBuffer);
        } else {
            countMessage--;
            itMsg++;
        }
    }

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, rcvId, 2);

    wsm->setSummaryVector(true);
    wsm->setRequestMessages(false);

    wsm->setWsmData(getLocalSummaryVectorData().c_str()); // Put the summary vector here, on data wsm field

    sendWSM(wsm); // Sending the summary vector

    //ScountMsgPacketSend++;
    ScountSummaryVectorSend++;
}

// Method used to convert the unordered_map epidemicLocalSummaryVectorData in a string
string BaseWaveApplLayer::getLocalSummaryVectorData() {
    ostringstream ss;
    for (auto& x: epidemicLocalSummaryVector) {
        ss << x.first << "|" << x.second << "|";
    }

    string s = ss.str();
    s = s.substr(0, (s.length() - 1));

    //cout << "EpidemicLocalSummaryVector from " << source << "(" << MACToInteger() << "): " << s << endl;
    return s;
}

string BaseWaveApplLayer::getEpidemicRequestMessageVectorData() {
    ostringstream ss;
    // Adding the requester name in order to identify if the requester is also the target of the messages with hopCount == 1
    // In this case, hopCount == 1, the messages can be sent to the target. Otherwise, the message will not be spread
    ss << source << "|";
    for (auto& x: epidemicRequestMessageVector) {
        ss << x.first << "|" << x.second << "|";
    }

    string s = ss.str();
    s = s.substr(0, (s.length() - 1));

    //cout << "String format of EpidemicRequestMessageVector from " << source << ": " << s << endl;
    return s;
}

void BaseWaveApplLayer::printNodesIRecentlySentSummaryVector() {
    if (!nodesIRecentlySentSummaryVector.empty()) {
        unsigned short int i = 0;
        cout << "NodesIRecentlySentSummaryVector from " << source << " (" << MACToInteger() << ") at: " << simTime() << endl;

        for (auto& x: nodesIRecentlySentSummaryVector) {
            cout << ++i << " Node: " << x.first << " added at: " << x.second << endl;
        }
    } else {
        cout << "NodesIRecentlySentSummaryVector from " << source << " is empty at: " << simTime() << endl;
    }
}

void BaseWaveApplLayer::printEpidemicLocalMessageBuffer() {
    if (!epidemicLocalMessageBuffer.empty()) {
        cout << endl << "Printing the epidemicLocalMessageBuffer from " << source << " (" << MACToInteger() <<") at: " << simTime() << endl;

        for (auto& x: epidemicLocalMessageBuffer) {
            WaveShortMessage wsmBuffered = x.second;

            cout << " GlobalID " << ": " << wsmBuffered.getGlobalMessageIdentificaton() << endl;
            cout << " Message Content: " << wsmBuffered.getWsmData() << endl;
            cout << " source: " << wsmBuffered.getSource() << endl;
            cout << " target: " << wsmBuffered.getTarget() << endl;
            cout << " Timestamp: " << wsmBuffered.getTimestamp() << endl;
            cout << " HopCount: " << wsmBuffered.getHopCount() << endl << endl;
        }
    } else {
        cout << "EpidemicLocalMessageBuffer from " << source << " is empty at: " << simTime() << endl;
    }
}

void BaseWaveApplLayer::printEpidemicRequestMessageVector() {
    if (!epidemicRequestMessageVector.empty()) {
        ostringstream ss;
        for (auto& x: epidemicRequestMessageVector) {
            ss << x.first << "|" << x.second << "|";
        }

        string s = ss.str();
        s = s.substr(0, (s.length() - 1));

        cout << "EpidemicRequestMessageVector from " << source << ": " << s << " at: " << simTime() << endl;
    } else {
        cout << "EpidemicRequestMessageVector from " << source << " is empty at: " << simTime() << endl;
    }
}

void BaseWaveApplLayer::printEpidemicLocalSummaryVectorData() {
    if (!epidemicLocalSummaryVector.empty()) {
        ostringstream ss;
        for (auto& x: epidemicLocalSummaryVector) {
            ss << x.first << "|" << x.second << "|";
        }

        string s = ss.str();
        s = s.substr(0, s.length() - 1);

        cout << "EpidemicLocalSummaryVector from " << source << "(" << MACToInteger() << "): " << s << " at: " << simTime() << endl;
    } else {
        cout << "EpidemicLocalSummaryVector from " << source << " is empty at: " << simTime() << endl;
    }
}

void BaseWaveApplLayer::printEpidemicRemoteSummaryVectorData() {
    if (!epidemicRemoteSummaryVector.empty()) {
        ostringstream ss;
        for (auto& x: epidemicRemoteSummaryVector) {
            ss << x.first << "|" << x.second << "|";
        }

        string s = ss.str();
        s = s.substr(0, (s.length() - 1));

        cout << "EpidemicRemoteSummaryVector from " << source << ": " << s << " at: " << simTime() << endl;
    } else {
        cout << "EpidemicRemoteSummaryVector from " << source << " is empty at: " << simTime() << endl;
    }
}

void BaseWaveApplLayer::sendEpidemicRequestMessageVector(unsigned int rcvId) {
    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, rcvId, 2);

    wsm->setSummaryVector(false);
    wsm->setRequestMessages(true);

    wsm->setWsmData(getEpidemicRequestMessageVectorData().c_str()); // Put the summary vector here

    //cout << "Sending a vector of request messages from " << source << "(" << MACToInteger() << ") to " << rcvId << endl;
    sendWSM(wsm); // Sending the RequestMessageVector

    //ScountMsgPacketSend++;
    ScountRequestMessageVectorSend++;
}

void BaseWaveApplLayer::createEpidemicRequestMessageVector() {
    epidemicRequestMessageVector.clear(); // Clean the unordered_map requestMessage before create a new one

    for (auto& x: epidemicRemoteSummaryVector) {
        unordered_map <string, bool>::iterator got = epidemicLocalSummaryVector.find(x.first);

        //cout << source << " in createEpidemicRequestMessageVector(). x.first: " << x.first << " x.second: " << x.second << endl;
        if (got == epidemicLocalSummaryVector.end()) { // True value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            string s = x.first;
            epidemicRequestMessageVector.insert(make_pair(s.c_str(), true)); // Putting the message in the EpidemicRequestMessageVector
        }/* else { // An entry in the unordered_map was found
            cout << source << " The message " << got->first << " in the epidemicRemoteSummaryVector was found in my epidemicLocalSummaryVector" << endl;
        }*/
    }
}

void BaseWaveApplLayer::createEpidemicRemoteSummaryVector(string s) {
    epidemicRemoteSummaryVector.clear();

    string delimiter = "|";
    size_t pos = 0;
    unsigned short int i = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        if (i%2 == 0) { // Catch just the key of the local summary vector
            epidemicRemoteSummaryVector.insert(make_pair(s.substr(0, pos), true));
        }
        s.erase(0, (pos + delimiter.length()));
        i++;
    }
    //cout << source << " createEpidemicRemoteSummaryVector: " << s << endl;
}

void BaseWaveApplLayer::sendMessagesRequested(string s, unsigned int rcvId) {
    cout << source << "(" << MACToInteger() << ") sending the following messages requested: " << s << " to " << rcvId << " at: " << simTime() << endl;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM_epidemic("data", dataLengthBits, channel, dataPriority, rcvId, 2);

    string delimiter = "|";
    size_t pos = 0;

    pos = s.find(delimiter);
    string tokenRequester = s.substr(0, pos); // Extracting who request message
    s.erase(0, (pos + delimiter.length()));

    unsigned short int i = 0;
    bool startSend = epidemicMessageSend.empty();

    //printEpidemicLocalMessageBuffer();
    while ((pos = s.find(delimiter)) != std::string::npos) {
        bool insert = false;
        if (i%2 == 0) { // Catch just the key of the local summary vector
            unordered_map <string, WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(s.substr(0, pos));

            if (got != epidemicLocalMessageBuffer.end()) {
                insert = true;

                wsm->setSource(got->second.getSource());
                wsm->setTarget(got->second.getTarget());
                wsm->setTimestamp(got->second.getTimestamp());
                wsm->setHopCount(got->second.getHopCount() - 1);
                wsm->setGlobalMessageIdentificaton(got->second.getGlobalMessageIdentificaton()); // To check
                wsm->setWsmData(got->second.getWsmData());
            }/* else {
                // True value means that there is no entry in the epidemicLocalSummaryVector for a epidemicRemoteSummaryVector key
            }*/
        }
        s.erase(0, (pos + delimiter.length()));
        i++;

        if (insert) {
            //cout << source << "One of message that is sending as a result of a requisition vector request, wsm->setWsmData: " << message << endl;

            wsm->setSummaryVector(false);
            wsm->setRequestMessages(false);
            //printWaveShortMessageEpidemic(wsm);

            epidemicMessageSend.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
        }
    }

    if (startSend && !epidemicMessageSend.empty()) {
        sendEpidemicMessageRequestEvt = new cMessage("beacon evt", Send_EpidemicMessageRequestEvt);
        scheduleAt(simTime(), sendEpidemicMessageRequestEvt);
    }
}

void BaseWaveApplLayer::receivedOnBeaconEpidemic(WaveShortMessage* wsm) {
    if (wsm->getSenderAddress() > MACToInteger()) { // Verifying if have the smaller address, with start the anti-entropy session sending out a summary vector
        unordered_map <unsigned int, simtime_t>::iterator got = nodesIRecentlySentSummaryVector.find(wsm->getSenderAddress());
        if (got == nodesIRecentlySentSummaryVector.end()) {
            //cout << source << " contact not found in nodesIRecentlySentSummaryVector. Sending my summary vector to " << wsm->getSenderAddress() << endl;
            nodesIRecentlySentSummaryVector.insert(make_pair(wsm->getSenderAddress(), simTime()));

            sendLocalSummaryVector(wsm->getSenderAddress());

            //printNodesIRecentlySentSummaryVector();
        } else {
           if ((simTime() - got->second) >= SsendSummaryVectorInterval) { // Send a summary vector to this node if passed the "sendSummaryVectorInterval" interval
               //cout << source << " updating the entry in the nodesIRecentlySentSummaryVector" << endl;

               got->second = simTime();
               sendLocalSummaryVector(wsm->getSenderAddress());

               //printNodesIRecentlySentSummaryVector();
           }
        }
    } /*else { // My address is bigger than the Beacon sender -> Do nothing
        cout << source << "(" << MACToInteger() << ")" << " SenderAddress: " << wsm->getSenderAddress() << " My ID is bigger than the Beacon sender" << endl;
    }*/
}

void BaseWaveApplLayer::receivedOnDataEpidemic(WaveShortMessage* wsm) {
    if (wsm->getRecipientAddress() == MACToInteger()) { // Checking if is the recipient of this message
        if (wsm->getSummaryVector()) {
            //cout << source << "(" << MACToInteger() << ") received the summary vector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << " at: " << simTime() << endl;

            createEpidemicRemoteSummaryVector(wsm->getWsmData()); // Creating the remote summary vector

            //printEpidemicRemoteSummaryVectorData();
            //printEpidemicLocalSummaryVectorData();

            createEpidemicRequestMessageVector(); // Creating a key vector in order to request messages that I still do not have in my buffer

            //printEpidemicRequestMessageVector();

            // Verifying if this is the end of second round of the anti-entropy session when the EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector are equals
            if ((epidemicRequestMessageVector.empty() || (strcmp(wsm->getWsmData(), "") == 0)) && (wsm->getSenderAddress() > MACToInteger())) {
                //cout << "EpidemicRequestMessageVector from " << source << " is empty" << endl;
                //cout << "Or strcmp(wsm->getWsmData(),\"\") == 0) " << endl;
                //cout << "And  wsm->getSenderAddress() > MACToInteger() " << endl;
            } else if (epidemicRequestMessageVector.empty()) {
                // Changing the turn of the anti-entropy session.
                // In this case not have any differences between EpidemicRemoteSummaryVector and EpidemicLocalSummaryVector, but I need to change the round of anti-entropy session
                sendLocalSummaryVector(wsm->getSenderAddress());
            } else {
                sendEpidemicRequestMessageVector(wsm->getSenderAddress()); // Sending a request vector in order to get messages that I don't have
            }
        } else {
            if (wsm->getRequestMessages()) { // Searching for elements in the epidemicLocalMessageBuffer and sending them to requester
                cout << source << " received the epidemicRequestMessageVector |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << endl;
                sendMessagesRequested(wsm->getWsmData(), wsm->getSenderAddress());
            } else { // It's data content
                cout << source << " (" << MACToInteger() << ") received a message requested " << wsm->getGlobalMessageIdentificaton();
                cout << " |> " << wsm->getWsmData() << " <| from " << wsm->getSenderAddress() << " at: " << simTime() << endl;

                if (source.compare(wsm->getTarget()) == 0) { // Verifying if is the target of this message
                    cout << source << " received a message for him at: " << simTime() << endl;
                    if (source.substr(0, 3).compare("rsu") == 0) {
                        findHost()->bubble("Received Message");
                        saveMessagesOnFile(wsm, fileMessagesUnicastRsu);

                        messagesReceivedMeasuringRSU(wsm);
                    } else { // This message has target destination one vehicle
                        saveMessagesOnFile(wsm, fileMessagesBroadcastRsu);
                    }
                } else {
                    //cout << "Before message processing - printEpidemicLocalMessageBuffer()" << endl;
                    //printEpidemicLocalMessageBuffer();
                    //cout << "Before message processing - printEpidemicLocalSummaryVectorData()" << endl;
                    //printEpidemicLocalSummaryVectorData();

                    // Verifying if there is no entry for current message received in my epidemicLocalMessageBuffer
                    unordered_map <string, WaveShortMessage>::iterator got = epidemicLocalMessageBuffer.find(wsm->getGlobalMessageIdentificaton());

                    if (got == epidemicLocalMessageBuffer.end()) { // True value means that there is no entry in the epidemicLocalMessageBuffer for the current message identification
                        if (epidemicLocalMessageBuffer.size() > SmessageBufferSize) { // The maximum buffer size was reached, so remove the oldest item
                            unordered_map <string, WaveShortMessage>::iterator itEpLocalMsg = epidemicLocalMessageBuffer.begin();
                            WaveShortMessage wsmBuffered = itEpLocalMsg->second;

                            simtime_t minTime = wsmBuffered.getTimestamp();
                            string idMessage = wsmBuffered.getGlobalMessageIdentificaton();
                            itEpLocalMsg++;

                            for (; itEpLocalMsg != epidemicLocalMessageBuffer.end(); itEpLocalMsg++) {
                                wsmBuffered = itEpLocalMsg->second;

                                if (minTime > wsmBuffered.getTimestamp()) {
                                    minTime = wsmBuffered.getTimestamp();
                                    idMessage = wsmBuffered.getGlobalMessageIdentificaton();
                                }
                            }

                            insertMessageDropVeh(idMessage, 1, minTime);

                            epidemicLocalMessageBuffer.erase(idMessage);
                            epidemicLocalSummaryVector.erase(idMessage);
                            //colorCarryMessageMfcv(epidemicLocalMessageBuffer);
                        }

                        epidemicLocalMessageBuffer.insert(make_pair(wsm->getGlobalMessageIdentificaton(), *wsm));
                        epidemicLocalSummaryVector.insert(make_pair(wsm->getGlobalMessageIdentificaton(), true));
                        msgBufferUse++;
                        //colorCarryMessageMfcv(epidemicLocalMessageBuffer);
                    }

                    //cout << "After message processing" << endl;
                    //printEpidemicLocalMessageBuffer();
                    //cout << "After message processing" << endl;
                    //printEpidemicLocalSummaryVectorData();
                }

                // Changing the turn of the anti-entropy session
                if (wsm->getSenderAddress() < MACToInteger()) { // To check, send a by time
                    bool SendOrNotLocalSummaryVector = false;
                    if (wsm->getSenderAddress() == nodesRecentlySendLocalSummaryVector) {
                        if ((simTime() - SsendSummaryVectorInterval) >= lastTimeSendLocalSummaryVector) {
                            SendOrNotLocalSummaryVector = true;
                        }
                    } else {
                        SendOrNotLocalSummaryVector = true;
                    }

                    if (SendOrNotLocalSummaryVector) {
                        sendLocalSummaryVector(wsm->getSenderAddress());
                        nodesRecentlySendLocalSummaryVector = wsm->getSenderAddress();
                        lastTimeSendLocalSummaryVector = simTime();
                    }
                }
            }
        }
    }
}
//############################################ Epidemic - end ##################################################################

void BaseWaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) {
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
    } else if (std::string(wsm->getName()) == "data") {
        onData(wsm);
    } else {
        cout << "\nUnknown message (name: " << wsm->getName() << ") received\n";
        ASSERT2(0, "JBe - unknown message type received -");
    }
    delete(msg);
}

void BaseWaveApplLayer::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            ScountBeaconSend++;
            break;
        }
        case SEND_BEACON_EVT_epidemic: {
            sendWSM(prepareWSM_epidemic("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            ScountBeaconSend++;
            break;
        }
        case SendEvtGenerateMessage: {
            vehGenerateBeaconMessageAfterBeginVeh();
            scheduleAt(simTime() + 1, sendGenerateMessageEvt);
            break;
        }
        case SendEvtSelectVehGenerateMessage: {
            selectVehGenerateMessage();
            scheduleAt(simTime() + par("timeGenerateNewMessage").doubleValue(), sendSelectVehGenerateMessageEvt);
            break;
        }
        case Send_EpidemicMessageRequestEvt: {
            unordered_map <string, WaveShortMessage>::iterator itEpidemicMessageSend;
            itEpidemicMessageSend = epidemicMessageSend.begin();

            sendWSM(itEpidemicMessageSend->second.dup());
            ScountMsgPacketSend++;

            string idMessage = itEpidemicMessageSend->second.getGlobalMessageIdentificaton();

            if (!SallowMessageCopy) {
                cout << source << " send the message " << idMessage << " to " << epidemicMessageSend[idMessage].getRecipientAddress() << " and removing at: " << simTime() << endl;
                insertMessageDropVeh(idMessage, 2, epidemicMessageSend[idMessage].getTimestamp());

                epidemicLocalMessageBuffer.erase(idMessage);
                epidemicLocalSummaryVector.erase(idMessage);
                //colorCarryMessageMfcv(epidemicLocalMessageBuffer);
            }

            epidemicMessageSend.erase(idMessage);

            if (!epidemicMessageSend.empty()) {
                scheduleAt((simTime() + 0.1), sendEpidemicMessageRequestEvt);
            }
            break;
        }
        case SendGetTrafficMethodCheck: {
            SvehGetTrafficMethodCheck.insert(make_pair(simTime(), Veins::TraCIScenarioManagerLaunchdAccess().get()->getManagedHosts().size()));

            scheduleAt(simTime() + par("trafficGranularitySum"), getTrafficEvtMethodCheck);
            break;
        }
        default: {
            if (msg) {
                cout << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
                ASSERT2(0, "JBe - APP: Error: Got Self Message of unknown kind! -");
            }
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
    } else {
        delete sendBeaconEvt;
    }
    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}

BaseWaveApplLayer::~BaseWaveApplLayer() {
}
