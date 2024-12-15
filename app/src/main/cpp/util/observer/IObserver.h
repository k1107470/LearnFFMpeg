//
// Created by jinshan on 2024/10/13.
//

#ifndef LEARNFFMPEG_IOBSERVER_H
#define LEARNFFMPEG_IOBSERVER_H


#include "FFData.h"
#include "FFThread.h"
#include <vector>
#include <mutex>

class IObserver : public FFThread {
public:
    virtual void Update(FFData data) {}

    void Notify(FFData data);

    void AddObs(IObserver *obs);

private:
    std::vector<IObserver *> obss;
    std::mutex mux;
};


#endif //LEARNFFMPEG_IOBSERVER_H
