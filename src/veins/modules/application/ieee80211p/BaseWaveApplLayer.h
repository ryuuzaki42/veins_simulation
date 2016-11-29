// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#ifndef BASEWAVEAPPLLAYER_H_
#define BASEWAVEAPPLLAYER_H_

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"

// To use Veins::TraCIScenarioManagerLaunchdAccess()
#include "veins/modules/mobility/traci/TraCIScenarioManagerLaunchd.h"

#include <map>
#include <iomanip> // std::setprecision
#include <sstream>
#include <fstream>
#include <iostream> // std::cout, std::fixed
#include <string.h>
#include <algorithm> // std::find
#include <unordered_map>

using namespace std;

#ifndef DBG
#define DBG EV
#endif

class BaseWaveApplLayer : public BaseApplLayer {
    public:
        ~BaseWaveApplLayer();
        virtual void initialize_veins_TraCI(int stage);
        virtual void finish();

        virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details);

        enum WaveApplMessageKinds {
            SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
            SEND_BEACON_EVT, SEND_BEACON_EVT_vehDist, SEND_BEACON_EVT_epidemic, SEND_BEACON_EVT_minicurso,
            SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, // Modificado OSDP, add SERVICE_EXPIRED_EVT, SERVICE_QUERY_EVT, SERVICE_EVT, antes era apenas SEND_BEACON_EVT
            Send_EpidemicMessageRequestEvt, SendEvtGenerateMessage, SendGetTrafficMethodCheck, SendEvtSelectVehGenerateMessage
        };

    protected:
        static const simsignalwrap_t mobilityStateChangedSignal;

        virtual void handleLowerMsg(cMessage* msg); /** @brief handle messages from below */

        virtual void handleSelfMsg(cMessage* msg); /** @brief handle self messages */

        //virtual WaveShortMessage* prepareWSM(string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
        virtual WaveShortMessage* prepareWSM(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);

//######################################### vehDist #########################################
        //## Used to another projects
        void generalInitializeVariables_executionByExpNumberVehDist();
        string getFolderResultVehDist(unsigned short int experimentSendbyDSR);

        void toFinishRSU();
        void toFinishVeh();
        string boolToString(bool value);

        void restartFilesResultRSU();
        void restartFilesResultVeh(string projectInfo, Coord initialPos);
        void saveVehStartPositionVeh(string fileNameLocation, Coord initialPos);
        void insertMessageDropVeh(string ID, unsigned short int type, simtime_t timeGenarted);

        void printCountMessagesReceivedRSU();
        void messagesReceivedMeasuringRSU(WaveShortMessage* wsm);

        void vehGenerateMessageBeginVeh(double vehOffSet);
        void rsuSelectVehGenerateMessageBegin();
        void vehGenerateBeaconMessageAfterBeginVeh();
        void selectVehGenerateMessage();

        void generateBeaconMessageVehDist();
        void selectTarget();
        void colorCarryMessageVehDist(unordered_map <string, WaveShortMessage> bufferOfMessages);
        void printCountBeaconMessagesDropVeh();

        void saveMessagesOnFile(WaveShortMessage* wsm, string fileName);
        void printHeaderfileExecution();
        void openFileAndClose(string fileName, bool justForAppend);

        void vehInitializeValuesVehDist(string category, Coord position);
        void rsuInitializeValuesVehDist(Coord position);

        int mt19937GetRandomValue(int upperLimmit);

        string getCFGVAR();

        void insertVehTraffic();
        void printVehTraffic();
        void printVehTrafficMethodCheck();
//######################################### vehDist #########################################

//######################################### Epidemic #########################################
        virtual WaveShortMessage* prepareWSM_epidemic(string name, int dataLengthBits, t_channel channel, int priority, unsigned int rcvId, int serial=0);
        unsigned int MACToInteger();

        void receivedOnBeaconEpidemic(WaveShortMessage* wsm);
        void receivedOnDataEpidemic(WaveShortMessage* wsm);

        void printWaveShortMessageEpidemic(WaveShortMessage* wsm);

        void sendLocalSummaryVector(unsigned int newRecipientAddress);

        string getLocalSummaryVectorData();
        string getEpidemicRequestMessageVectorData();

        void printNodesIRecentlySentSummaryVector();

        void printEpidemicLocalMessageBuffer();
        void printEpidemicRequestMessageVector();
        void printEpidemicLocalSummaryVectorData();
        void printEpidemicRemoteSummaryVectorData();

        void sendEpidemicRequestMessageVector(unsigned int newRecipientAddress);
        void sendMessagesRequested(string s, unsigned int recipientAddress);

        void createEpidemicRequestMessageVector();
        void createEpidemicRemoteSummaryVector(string s);

        void generateMessageEpidemic();
        string insertZeroIntRetunrString(int value);
//######################################### Epidemic #########################################

        virtual void sendWSM(WaveShortMessage* wsm);
        virtual void onBeacon(WaveShortMessage* wsm) = 0;
        virtual void onData(WaveShortMessage* wsm) = 0;
        virtual void handlePositionUpdate(cObject* obj);

    protected:
        int beaconLengthBits, beaconPriority, mySCH, myId, dataLengthBits, dataPriority;
        bool sendData, sendBeacons, dataOnSch;
        simtime_t individualOffset;
        Coord curPosition;

        string source, target;
        bool toDeliveryMsg;
        static int ScountToDeliveryMsg;

        cMessage* sendBeaconEvt;

        WaveAppToMac1609_4Interface* myMac;

//######################################### vehDist #########################################

        vector <string> messagesOrderReceivedVehDist;

        unordered_map <string, WaveShortMessage> messagesBufferVehDist;
        unordered_map <string, WaveShortMessage> messagesBufferToDelivery;

        //## Used to another projects
        cMessage* sendGenerateMessageEvt;
        cMessage* sendSelectVehGenerateMessageEvt;
        cMessage* getTrafficEvtMethodCheck;

        static mt19937 mtSelectVehicleGenarateMessage, mtTargetMessageSelect;

        double vehOffSet;

        string vehCategory, routeId;
        static string SfirstCategory, SsecondCategory;
        static int SsimulationTimeLimit;

        ofstream myfile; // record in file

        unsigned short int target_x, target_y;
        static unordered_map <int, Coord> SrsuPositions;
        unsigned int msgBufferUse;

        static string SfileMessagesUnicastVeh, SfileMessagesDropVeh;
        static string SfileMessagesUnicastRsu, SfileMessagesBroadcastRsu, SfileMessagesCountRsu;
        static string SfileMessagesGeneratedVehRsu, SresultFolder;

        static unsigned short int SrepeatNumber, SexpNumber, SexpSendbyDSCR, ScountGenerateMessage, SttlMessage;

        static unsigned short int SmsgDroppedbyTTL, SmsgDroppedbyCopy, SmsgDroppedbyBuffer, SvehTimeLimitToAcceptGenerateMgs;
        static unsigned short int ScountMsgPacketSend, SmsgBufferUseGeneral, ScountMesssageDrop, SmessageHopLimit;
        static unsigned short int ScountMeetN, ScountTwoCategoryN, ScountMeetPshortestT, ScountVehicleAll, SmessageId;

        static unsigned short int SbeaconStatusBufferSize, SttlBeaconStatus, SpercentP, SbeaconTypeInitialize;
        static unsigned short int StimeLimitGenerateMessage, StimeToUpdatePosition, SmessageBufferSize;

        static string SprojectInfo;

        static vector <string> SnumVehicles, SvehGenerateMessage;

        struct traffic {
            string source;
            simtime_t entryTime, exitTime;
        };
        static map <string, struct traffic> SvehTraffic;

        static map <simtime_t, int> SvehGetTrafficMethodCheck;
        static unordered_map <string, int> SvehCategoryCount;
        static unordered_map <string, WaveShortMessage> SvehScenario;
        static bool SusePathHistory, SallowMessageCopy, SvehSendWhileParking;
        static bool SselectFromAllVehicles, SuseMessagesSendLog, SvehDistCreateEventGenerateMessage, SuseRateTimeToSend;

        struct messages {
          string firstSource, hops, wsmData, times;
          unsigned short int minHop, maxHop, sumHops, countFirstCategory, countSecondCategory, countCopyMessage;
          simtime_t sumTimeRecived, timeGenerate;
        };
        map <string, struct messages> messagesReceivedRSU;

        struct messagesDropStruct {
            unsigned short int byBuffer, byTime, byCopy;
            simtime_t timeGenerate, timeDroped, timeDifference;
        };
        map <string, struct messagesDropStruct> messagesDropVeh;

        struct busPosByTime {
            //->frist source
            map <simtime_t, Coord> timePos;
        };
        static unordered_map <string, busPosByTime> SposTimeBus; // To save bus position in a .csv
        static unordered_map <string, busPosByTime> SposTimeBusLoaded; // To load bus position to memory form a .csv
        static unordered_map <string, string> SrouteIDVehID; // routeID - vehID
//######################################### vehDist #########################################

//######################################### Epidemic #########################################

        cMessage* sendEpidemicMessageRequestEvt;

        unordered_map <string, WaveShortMessage> epidemicLocalMessageBuffer, epidemicMessageSend;

        unordered_map <string, bool> epidemicLocalSummaryVector, epidemicRemoteSummaryVector, epidemicRequestMessageVector;

        unordered_map <unsigned int, simtime_t> nodesRecentlyFoundList, nodesIRecentlySentSummaryVector;

        static unsigned short int  SsendSummaryVectorInterval;
        unsigned int nodesRecentlySendLocalSummaryVector;
        simtime_t lastTimeSendLocalSummaryVector;
//######################################### Epidemic #########################################
};

#endif
