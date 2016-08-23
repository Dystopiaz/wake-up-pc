#include <time.h>
#include<stdio.h>
#include<math.h>
#include <stdlib.h>

#include "nNet.h"
#include "filterbank.h"

float ReLU(float x)
{
	return x >= 0 ? x : 0;
}
void softmax(float x[OUTPUT_DIMEN])
{
	double max = -999999.0;
	double sum = 0.0;

	for (int i = 0; i < OUTPUT_DIMEN; i++)
	{
		if (max < x[i]) max = x[i];
	}
	//printf("max:%f", max);
	for (int i = 0; i< OUTPUT_DIMEN; i++)
	{
		x[i] = exp(x[i] - max);
		sum += x[i];
	}
	//printf("sum:%f\n", sum);
	for (int i = 0; i < OUTPUT_DIMEN; i++)
	{
		x[i] /= sum;
	}
}
void Input2Layer(float x[INPUT_DIMEN], float w[INPUT_DIMEN][LAYER_DIMEN], float b[LAYER_DIMEN], float y[LAYER_DIMEN])
{
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		y[i] = 0;
		for (int j = 0; j < INPUT_DIMEN; j++)
		{
			y[i] += x[j] * w[j][i];

		}

		y[i] += b[i];
		y[i] = ReLU(y[i]);
	}
}

void Layer2Layer(float x[LAYER_DIMEN], float w[LAYER_DIMEN][LAYER_DIMEN], float b[LAYER_DIMEN], float y[LAYER_DIMEN])
{
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		y[i] = 0;
		for (int j = 0; j < LAYER_DIMEN; j++)
		{
			y[i] += x[j] * w[j][i];
		}
		y[i] += b[i];
		y[i] = ReLU(y[i]);

	}
}
void Layer2Output(float x[LAYER_DIMEN], float w[LAYER_DIMEN][OUTPUT_DIMEN], float b[OUTPUT_DIMEN], float y[OUTPUT_DIMEN])
{
	for (int i = 0; i < OUTPUT_DIMEN; i++)
	{
		y[i] = 0;
		for (int j = 0; j < LAYER_DIMEN; j++)
		{
			y[i] += x[j] * w[j][i];
		}
		y[i] += b[i];
		//y[i] = ReLU(y[i]);
	}
	softmax(y);
	//printf("%f %f %f\n",y[0], y[1], y[2]);
}


void init(
	float w0[INPUT_DIMEN][LAYER_DIMEN], float b0[LAYER_DIMEN],
	float w1[LAYER_DIMEN][LAYER_DIMEN], float b1[LAYER_DIMEN],
	float w2[LAYER_DIMEN][LAYER_DIMEN], float b2[LAYER_DIMEN],
	float w3[LAYER_DIMEN][OUTPUT_DIMEN], float b3[OUTPUT_DIMEN],char *s)
{
	FILE *fp;
	fp = fopen(s, "rb");
	if (!fp)
	{
		printf("can not open the file\n");
		exit(0);
	}
	////w0b0
	for (int i = 0; i < INPUT_DIMEN; i++)
	{
		for (int j = 0; j < LAYER_DIMEN; j++)
		{
			fscanf(fp, "%f", &w0[i][j]);
		}
	}
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		fscanf(fp, "%f", &b0[i]);
	}
	////w1b1
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		for (int j = 0; j < LAYER_DIMEN; j++)
		{
			fscanf(fp, "%f", &w1[i][j]);
		}
	}
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		fscanf(fp, "%f", &b1[i]);
	}
	////w2b2
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		for (int j = 0; j < LAYER_DIMEN; j++)
		{
			fscanf(fp, "%f", &w2[i][j]);
		}
	}
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		fscanf(fp, "%f", &b2[i]);
	}
	////w3b3
	for (int i = 0; i < LAYER_DIMEN; i++)
	{
		for (int j = 0; j < OUTPUT_DIMEN; j++)
		{
			fscanf(fp, "%f", &w3[i][j]);
		}
	}
	for (int i = 0; i < OUTPUT_DIMEN; i++)
	{
		fscanf(fp, "%f", &b3[i]);
	}
}
void dnn(struct Wave *w,char *s)
{
	float InMat[INPUT_DIMEN] = { 1 };
	float OutMat[OUTPUT_DIMEN] = { 1 };
	float w0[INPUT_DIMEN][LAYER_DIMEN], b0[LAYER_DIMEN];
	float w1[LAYER_DIMEN][LAYER_DIMEN], b1[LAYER_DIMEN];
	float w2[LAYER_DIMEN][LAYER_DIMEN], b2[LAYER_DIMEN];
	float w3[LAYER_DIMEN][OUTPUT_DIMEN], b3[OUTPUT_DIMEN];
	init(w0, b0, w1, b1, w2, b2, w3, b3,s);
	float layerout0[LAYER_DIMEN] = { 0 };
	float layerout1[LAYER_DIMEN] = { 0 };
	float layerout2[LAYER_DIMEN] = { 0 };
	int count = 0;
	int row = 0;
	while ((count + NUMCHANS * 11) < NUMCHANS*w->nRow)
	{
		for (int i = count - NUMCHANS * 30, j = 0; i < count + NUMCHANS * 11; i++, j++)
		{
			if (i < 0)
				InMat[j] = *(w->Rdata + ((-i) % NUMCHANS));
			else
				InMat[j] = *(w->Rdata + i);
		}
		count = count + NUMCHANS;
		Input2Layer(InMat, w0, b0, layerout0);
		Layer2Layer(layerout0, w1, b1, layerout1);
		Layer2Layer(layerout1, w2, b2, layerout2);
		Layer2Output(layerout2, w3, b3, OutMat);

		printf("%f %f %f\n", OutMat[0], OutMat[1], OutMat[2]);
	}

}