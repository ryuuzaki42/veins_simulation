// Copyright (C) 2015-2017 João Batista <joao.b@usp.br>

#ifndef mfcv_H
#define mfcv_H

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;

class mfcv : public BaseWaveApplLayer {
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
            double distanceToTargetNow, distanceToTargetBefore, decisionValueDistanceCategory, decisionValueDistanceSpeed;
            double decisionValueDistanceRateTimeToSend, decisionValueDistanceCategoryRateTimeToSend, speedVeh;
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
        bool has3inSexpSendbyDSCR();

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

        void mfcvInitializeVariablesVeh();

        void sendMessageToOneNeighborTarget(string beaconSource);
        bool sendOneNewMessageToOneNeighborTarget(WaveShortMessage wsm);

        void removeOldestInputBeaconMessage();
        void removeOldestInputBeaconStatus();

        string selectVehIdWithSmallValueBySexpSendbyDSCR(unordered_map <string, shortestDistance> vehShortestDistanceToTarget);

        unsigned short int getVehHeading8();
        unsigned short int getVehHeading4();
};

vector <string> BaseWaveApplLayer::SnumVehicles, BaseWaveApplLayer::SvehGenerateMessage;

map <SimTime, int> BaseWaveApplLayer::SvehGetTrafficMethodCheck;
unordered_map <string, int> BaseWaveApplLayer::SvehCategoryCount;
unordered_map <string, WaveShortMessage> BaseWaveApplLayer::SvehScenario;
map <string, struct BaseWaveApplLayer::traffic> BaseWaveApplLayer::SvehTraffic;

unordered_map <int, Coord> BaseWaveApplLayer::SrsuPositions;

int BaseWaveApplLayer::SmsgDroppedbyTTL, BaseWaveApplLayer::SmsgDroppedbyCopy, BaseWaveApplLayer::SmsgDroppedbyBuffer, BaseWaveApplLayer::ScountMessageOnlyDeliveryBus;
int BaseWaveApplLayer::ScountMsgPacketSend, BaseWaveApplLayer::SmsgBufferUseGeneral, BaseWaveApplLayer::SmessageHopLimit;
int BaseWaveApplLayer::ScountMesssageDrop, BaseWaveApplLayer::ScountMeetJustOneCategory, BaseWaveApplLayer::ScountMeetTwoCategory, BaseWaveApplLayer::ScountVehicleBus;
int BaseWaveApplLayer::ScountMeetTotal, BaseWaveApplLayer::ScountVehicleAll, BaseWaveApplLayer::SmessageId, BaseWaveApplLayer::ScountVehicleTaxi;
int BaseWaveApplLayer::ScountBeaconSend, BaseWaveApplLayer::ScountSummaryVectorSend, BaseWaveApplLayer::ScountRequestMessageVectorSend;

unsigned short int BaseWaveApplLayer::SbeaconTypeInitialize;
unsigned short int BaseWaveApplLayer::SrepeatNumber, BaseWaveApplLayer::SexpNumber, BaseWaveApplLayer::SexpSendbyDSCR;
unsigned short int BaseWaveApplLayer::ScountGenerateMessage, BaseWaveApplLayer::SttlMessage, BaseWaveApplLayer::SvehTimeLimitToAcceptGenerateMgs;

unsigned short int BaseWaveApplLayer::SbeaconStatusBufferSize, BaseWaveApplLayer::SttlBeaconStatus;
unsigned short int BaseWaveApplLayer::StimeLimitGenerateMessage, BaseWaveApplLayer::StimeToUpdatePosition, BaseWaveApplLayer::SmessageBufferSize;
bool BaseWaveApplLayer::SuseBeaconStatusBufferSize;

int BaseWaveApplLayer::SsimulationTimeLimit;
vector <string> BaseWaveApplLayer::SlaneNameLoaded;
unordered_map <string, struct BaseWaveApplLayer::busPosByTime> BaseWaveApplLayer::SposTimeBus;
unordered_map <string, struct BaseWaveApplLayer::busPosByTime> BaseWaveApplLayer::SposTimeBusLoaded;
unordered_map <string, string> BaseWaveApplLayer::SrouteIDVehID;
int BaseWaveApplLayer::ScountToDeliveryMsg, BaseWaveApplLayer::ScountToDeliveryMsgUnicID;
unsigned int BaseWaveApplLayer::SbufferMessageOnlyDeliveryLimit, BaseWaveApplLayer::SmsgUseOnlyDeliveryBufferGeneral;

string BaseWaveApplLayer::SfileMessagesUnicastVeh, BaseWaveApplLayer::SfileMessagesDropVeh;
string BaseWaveApplLayer::SfileMessagesCountRsu, BaseWaveApplLayer::SfileMessagesGeneratedVehRsu;
string BaseWaveApplLayer::SresultFolder;

string BaseWaveApplLayer::SfirstCategoryPrivateCar, BaseWaveApplLayer::SsecondCategoryBus, BaseWaveApplLayer::SthirdCategoryTaxi;
struct BaseWaveApplLayer::targetResultMsg BaseWaveApplLayer::SresultMsgReceived;

string BaseWaveApplLayer::SprojectInfo;
bool BaseWaveApplLayer::SusePathHistory, BaseWaveApplLayer::SallowMessageCopy;
bool BaseWaveApplLayer::SselectFromAllVehicles, BaseWaveApplLayer::SuseMessagesSendLog;
bool BaseWaveApplLayer::SmfcvCreateEventGenerateMessage, BaseWaveApplLayer::SuseRateTimeToSend;

mt19937 BaseWaveApplLayer::mtSelectVehicleGenarateMessage, BaseWaveApplLayer::mtTargetMessageSelect, BaseWaveApplLayer::mtSelectLaneName;

#endif
