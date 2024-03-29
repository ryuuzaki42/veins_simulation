// Copyright (C) 2015-2017 João Batista <joao.b@usp.br>

#include "veins/modules/application/mfcv/mfcv_rsu.h"

Define_Module(mfcv_rsu);

void mfcv_rsu::initialize(int stage) {
    BaseWaveApplLayer::initialize_veins_TraCI(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);

        mfcvInitializeVariablesRsu();
    }
}

void mfcv_rsu::mfcvInitializeVariablesRsu() {
    rsuInitializeValuesMfcv(mobi->getInitialPositionFromIniFileRSU());

    if (source.compare("rsu[0]") == 0) {
        vehToDelivery = "SenderAddress MsgID timeSim rsuID\n";
    }
}

void mfcv_rsu::onBeacon(WaveShortMessage* wsm) {
}

void mfcv_rsu::onData(WaveShortMessage* wsm) {
    if (source.compare(wsm->getRecipientAddressTemporary()) == 0) {
        if (source.compare(wsm->getTarget()) == 0) {
            //findHost()->bubble("Received Message");
            saveMessagesOnFile(wsm, fileMessagesUnicastRsu);

            if (wsm->getOnlyDelivery()) {
                ScountToDeliveryMsg++;
                countToDeliveryMsgLocal++;
                vehToDelivery += wsm->getSenderAddressTemporary() + string(" msg: ") + wsm->getGlobalMessageIdentificaton();
                vehToDelivery += string(" at: ") + to_string(simTime().dbl()) + string(" to ") + source + string("\n");
            }
            messagesReceivedMeasuringRSU(wsm);
        }/* else {
            real scenario save this message in the buffer
        }*/
    } else {
        saveMessagesOnFile(wsm, fileMessagesBroadcastRsu);
    }
}

void mfcv_rsu::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case SEND_BEACON_EVT_mfcv: { // Call prepareBeaconStatusWSM local to the rsu
            sendWSM(prepareBeaconStatusWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, -1));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            ScountBeaconSend++;
            break;
        }
        default: {
            BaseWaveApplLayer::handleSelfMsg(msg);
            break;
        }
    }
}

WaveShortMessage* mfcv_rsu::prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial) {
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

    //wsm->setSenderPos(curPosition);
    wsm->setSenderPos(Coord(30000, 30000, 3)); // Value set to be not select by small distance
    wsm->setSenderPosPrevious(Coord(30000, 30000, 3)); // RSU can't move
    wsm->setRateTimeToSend(10000); // Value set to be not select by small rateTimeToSend
    wsm->setCategory("rsu");

    //beacon don't need
    //wsm->setRecipientAddressString(); => "BROADCAST"
    //wsm->setSenderAddressTemporary();
    //wsm->setTarget(); => "BROADCAST"

    DBG << "Creating BeaconStatus with Priority " << priority << " at Applayer at: " << wsm->getTimestamp() << std::endl;
    return wsm;
}

void mfcv_rsu::finish() {
    toFinishRSU();

    myfile.open(SfileMessagesCountRsu, std::ios_base::app);
    cout << "\nvehToDelivery from " << source << "\n" << vehToDelivery << "\n";
    myfile << "\nvehToDelivery from " << source << "\n" << vehToDelivery << "\n";
    myfile.close();
}
