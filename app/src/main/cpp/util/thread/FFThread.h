//
// Created by jinshan on 2024/10/13.
//

#ifndef LEARNFFMPEG_FFTHREAD_H
#define LEARNFFMPEG_FFTHREAD_H

void FFSleep(int ms);

class FFThread {
public:
    virtual void Start();
    virtual void Stop();

    virtual void Main() {};
protected:
    bool isExit = false;
    bool isRunning = false;
private:
    void ThreadMain();
};


#endif //LEARNFFMPEG_FFTHREAD_H
