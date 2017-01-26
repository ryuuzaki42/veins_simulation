// Copyright (C) 2015-2016 João Batista <joao.b@usp.br>

#ifndef mfcv_rsu_H
#define mfcv_rsu_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

class mfcv_rsu : public BaseWaveApplLayer {
    public:
        virtual void initialize(int stage);

    protected:
        BaseMobility* mobi;
        string vehToDelivery;

    protected:
        void onBeacon(WaveShortMessage* wsm);
        void onData(WaveShortMessage* wsm);

        void finish();
        void handleSelfMsg(cMessage* msg);
        WaveShortMessage* prepareBeaconStatusWSM(string name, int lengthBits, t_channel channel, int priority, int serial);

        void mfcvInitializeVariablesRsu();
};
#endif
