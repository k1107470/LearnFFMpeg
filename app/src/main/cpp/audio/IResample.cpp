//
// Created by jinshan on 2024/10/15.
//

#include "IResample.h"

void IResample::Update(FFData data) {
    FFData d = this->Resample(data);
    if (d.size > 0) {
        this->Notify(d);
    }
}
