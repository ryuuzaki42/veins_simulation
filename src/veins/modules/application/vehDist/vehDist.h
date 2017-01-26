// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#ifndef vehDist_H
#define vehDist_H

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;

class vehDist : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);

        enum WaveApplMessageKinds {
            SendEvtBeaconMessage, SendEvtUpdateRateTimeToSendVeh, SendEvtSaveBusPosition, SendEvtTaxiChangeRoute
        };

    protected:
        TraCIMobility* mobility;
        TraCICommandInterface* traci;
        TraCICommandInterface::Vehicle* traciVehicle;

        cMessage* sendBeaconMessageEvt;
        cMessage* sendUpdateRateTimeToSendVeh;
        cMessage* sendSaveBusPosition;
        cMessage* sendTaxiChangeRoute;

        vector <string> messagesDelivered;
        string busMsgToDelivery;
        string taxiRoadIDNow;

        unordered_map <string, string> messagesSendLog;

        unordered_map <string, WaveShortMessage> beaconStatusNeighbors;

        unsigned short int messageToSend, rateTimeToSend, msgOnlyDeliveryFunctionResult, msgUseOnlyDeliveryBuffer;
        unsigned short int rateTimeToSendLimitTime, rateTimeToSendUpdateTime, rateTimeToSendDistanceControl;

        simtime_t timeToFinishLastStartSend;

        struct shortestDistance {
            Coord senderPos;
            string categoryVeh;
            double distanceToTargetNow, distanceToTargetBefore, distanceToTargetCategory, decisionValueDistanceSpeed;
            double decisionValueDistanceRateTimeToSend, decisionValueDistanceSpeedRateTimeToSend, speedVeh;
            unsigned short int rateTimeToSendVeh;
        };

    protected:
        void onBeacon(WaveShortMessage* wsm);
        void onData(WaveShortMessage* wsm);

        void finish();
        void handleSelfMsg(cMessage* msg);

        void printBeaconStatusNeighbors();
        WaveShortMessage* prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial);

        void busPositionFunctionsRun();
        void busPosInsertByTimeMap();
        void saveBusPositionFile();
        void busPosLoadFromFile();

        void loadLaneNames();
        void printPlannedRoadIds(list<string> plannedRoadIds);
        void tryChangeRouteUntilSuccess();
        int selectLaneNamePositionID();
        void createEvtTaxiChangeRoute();

        unsigned short int busRouteDiffTarget(string busID, Coord targetPos, double localVehDistanceNow);
        void sendMessageDeliveryBuffer(string beaconSource);
        void createBusPositionSaveEvent();

        void vehCreateEventTrySendBeaconMessage();
        void sendBeaconMessage();
        void printMessagesBuffer();
        void printVehShortestDistanceToTarget(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, string idMessage);
        WaveShortMessage* updateBeaconMessageWSM(WaveShortMessage* wsm, string rcvId);

        void trySendBeaconMessage();
        string neighborWithShortestDistanceToTarge(string idMessage);
        string neighborWithShortestDistanceToTargeOnlyDelivery(string idMessage);

        void vehUpdateRateTimeToSend();
        void vehCreateUpdateRateTimeToSendEvent();

        void vehDistInitializeVariablesVeh();

        void sendMessageToOneNeighborTarget(string beaconSource);
        bool sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm);

        void removeOldestInputBeaconMessage();
        void removeOldestInputBeaconStatus();

        string chooseCategory_RandomNumber1to100(unsigned short int percentP, string vehIdP, string vehIdT);
        string chooseByDistance(unordered_map <string, shortestDistance> vehShortestDistanceToTarget); // 0001
        string chooseByDistance_Speed(unordered_map <string, shortestDistance> vehShortestDistanceToTarget); // 0012
        string chooseByDistance_CategoryA(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 0013
        string chooseByDistance_RateTimeToSend(unordered_map <string, shortestDistance> vehShortestDistanceToTarget); // 0014
        string chooseByDistance_Speed_Category(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 0123
        string chooseByDistance_Speed_Category_RateTimeToSend(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 1234

        string chooseByDistance_CategoryB(unordered_map <string, shortestDistance> vehShortestDistanceToTarget, int percentP); // 0013 with uncomment

        unsigned short int getVehHeading8();
        unsigned short int getVehHeading4();
};

vector <string> BaseWaveApplLayer::SnumVehicles, BaseWaveApplLayer::SvehGenerateMessage;

map <SimTime, int> BaseWaveApplLayer::SvehGetTrafficMethodCheck;
unordered_map <string, int> BaseWaveApplLayer::SvehCategoryCount;
unordered_map <string, WaveShortMessage> BaseWaveApplLayer::SvehScenario;
map <string, struct BaseWaveApplLayer::traffic> BaseWaveApplLayer::SvehTraffic;

unordered_map <int, Coord> BaseWaveApplLayer::SrsuPositions;

int BaseWaveApplLayer::SmsgDroppedbyTTL, BaseWaveApplLayer::SmsgDroppedbyCopy, BaseWaveApplLayer::SmsgDroppedbyBuffer;
int BaseWaveApplLayer::ScountMsgPacketSend, BaseWaveApplLayer::SmsgBufferUseGeneral, BaseWaveApplLayer::SmessageHopLimit;
int BaseWaveApplLayer::ScountMesssageDrop, BaseWaveApplLayer::ScountMeetN, BaseWaveApplLayer::ScountTwoCategoryN, BaseWaveApplLayer::ScountVehicleBus;
int BaseWaveApplLayer::ScountMeetPshortestT, BaseWaveApplLayer::ScountVehicleAll, BaseWaveApplLayer::SmessageId, BaseWaveApplLayer::ScountVehicleTaxi;
int BaseWaveApplLayer::ScountBeaconSend, BaseWaveApplLayer::ScountSummaryVectorSend, BaseWaveApplLayer::ScountRequestMessageVectorSend;

unsigned short int BaseWaveApplLayer::SbeaconTypeInitialize;
unsigned short int BaseWaveApplLayer::SrepeatNumber, BaseWaveApplLayer::SexpNumber, BaseWaveApplLayer::SexpSendbyDSCR;
unsigned short int BaseWaveApplLayer::ScountGenerateMessage, BaseWaveApplLayer::SttlMessage, BaseWaveApplLayer::SvehTimeLimitToAcceptGenerateMgs;

unsigned short int BaseWaveApplLayer::SbeaconStatusBufferSize, BaseWaveApplLayer::SttlBeaconStatus, BaseWaveApplLayer::SpercentP;
unsigned short int BaseWaveApplLayer::StimeLimitGenerateMessage, BaseWaveApplLayer::StimeToUpdatePosition, BaseWaveApplLayer::SmessageBufferSize;
bool BaseWaveApplLayer::SuseBeaconStatusBufferSize;

int BaseWaveApplLayer::SsimulationTimeLimit;
vector <string> BaseWaveApplLayer::SlaneNameLoaded;
unordered_map <string, struct BaseWaveApplLayer::busPosByTime> BaseWaveApplLayer::SposTimeBus;
unordered_map <string, struct BaseWaveApplLayer::busPosByTime> BaseWaveApplLayer::SposTimeBusLoaded;
unordered_map <string, string> BaseWaveApplLayer::SrouteIDVehID;
int BaseWaveApplLayer::ScountToDeliveryMsg;
unsigned int BaseWaveApplLayer::SbufferMessageOnlyDeliveryLimit, BaseWaveApplLayer::SmsgUseOnlyDeliveryBufferGeneral;

string BaseWaveApplLayer::SfileMessagesUnicastVeh, BaseWaveApplLayer::SfileMessagesDropVeh;
string BaseWaveApplLayer::SfileMessagesCountRsu, BaseWaveApplLayer::SfileMessagesGeneratedVehRsu;
string BaseWaveApplLayer::SresultFolder;

string BaseWaveApplLayer::SfirstCategory, BaseWaveApplLayer::SsecondCategory;
struct BaseWaveApplLayer::targetResultMsg BaseWaveApplLayer::SresultMsgReceived;

string BaseWaveApplLayer::SprojectInfo;
bool BaseWaveApplLayer::SusePathHistory, BaseWaveApplLayer::SallowMessageCopy;
bool BaseWaveApplLayer::SselectFromAllVehicles, BaseWaveApplLayer::SuseMessagesSendLog;
bool BaseWaveApplLayer::SvehDistCreateEventGenerateMessage, BaseWaveApplLayer::SuseRateTimeToSend;

mt19937 BaseWaveApplLayer::mtSelectVehicleGenarateMessage, BaseWaveApplLayer::mtTargetMessageSelect, BaseWaveApplLayer::mtSelectLaneName;

#endif
