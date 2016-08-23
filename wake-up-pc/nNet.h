#ifndef __NNET_H__
#define __NNET_H__ 
#include "filterbank.h"
#define INPUT_DIMEN 1640
#define LAYER_DIMEN 128
#define OUTPUT_DIMEN 3
#define LAYER 4

float ReLU(float x);
void softmax(float x[OUTPUT_DIMEN]);
void Input2Layer(float x[INPUT_DIMEN], float w[INPUT_DIMEN][LAYER_DIMEN], float b[LAYER_DIMEN], float y[LAYER_DIMEN]);
void Layer2Layer(float x[LAYER_DIMEN], float w[LAYER_DIMEN][LAYER_DIMEN], float b[LAYER_DIMEN], float y[LAYER_DIMEN]);
void Layer2Output(float x[LAYER_DIMEN], float w[LAYER_DIMEN][OUTPUT_DIMEN], float b[OUTPUT_DIMEN], float y[OUTPUT_DIMEN]);
void init(
	float w0[INPUT_DIMEN][LAYER_DIMEN], float b0[LAYER_DIMEN],
	float w1[LAYER_DIMEN][LAYER_DIMEN], float b1[LAYER_DIMEN],
	float w2[LAYER_DIMEN][LAYER_DIMEN], float b2[LAYER_DIMEN],
	float w3[LAYER_DIMEN][OUTPUT_DIMEN], float b3[OUTPUT_DIMEN],char *s);
void dnn(Wave *w,char *s);
#endif 