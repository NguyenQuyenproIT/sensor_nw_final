/*========================================================
    FILE: mlp_model.h
========================================================*/

#ifndef MLP_MODEL_H
#define MLP_MODEL_H

#include <math.h>
#include <stdint.h>

/*========================================================
    NETWORK SIZE
========================================================*/

#define INPUT_SIZE     3
#define HIDDEN1_SIZE   16
#define HIDDEN2_SIZE   8
#define OUTPUT_SIZE    1

/*========================================================
    FUNCTION
========================================================*/

float mlp_predict(float soil,
                  float temp,
                  float hum);

uint8_t mlp_classify(float soil,
                     float temp,
                     float hum);

#endif
