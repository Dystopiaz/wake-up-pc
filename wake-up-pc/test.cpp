#include "nNet.h"
#include "filterbank.h"
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <time.h>
int main()
{
	Wave w = filter_bank("F:\\newrecord.wav");
	dnn(&w, "F:\\x\\final\\wake_up_word_model_weight.txt");//need DNN param file
	return 0;
}
