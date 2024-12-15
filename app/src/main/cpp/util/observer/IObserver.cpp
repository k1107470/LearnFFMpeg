//
// Created by jinshan on 2024/10/13.
//

#include "IObserver.h"

void IObserver::AddObs(IObserver *obs) {
    if (!obs) return;
    mux.lock();
    obss.push_back(obs);
    mux.unlock();
}

void IObserver::Notify(FFData data) {
    mux.lock();
    for(int i = 0; i < obss.size(); ++i) {
        obss[i]->Update(data);
    }
    mux.unlock();
}
