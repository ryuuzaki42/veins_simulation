// Copyright (C) 2015-2017 João Batista <joao.b@usp.br>

package org.car2x.veins.modules.application.mfcv;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple mfcv extends BaseWaveApplLayer {
    @class(mfcv);
    @display("i=block/app2");
    bool sendWhileParking = default(false); //send messages when the car is parked
}
