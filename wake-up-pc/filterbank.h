#ifndef __FILTERFBANK_H__
#define __FILTERFBANK_H__
#define INPUT_DIMEN 1640
#define LAYER_DIMEN 128
#define OUTPUT_DIMEN 3
#define LAYER 4

#define PI   3.14159265358979
#define TPI  6.28318530717959     /* PI*2 */
#define NUMCHANS 40

typedef struct Wave
{
	int nSample;//wav中样本个数
	int frSize;//一帧中的样本数
	int frIdx;//当前帧位置
	int frRate;//帧移
	float *wavdata;
	int nRow;
	float *Rdata;
};
typedef struct FBankInfo
{
	int frameSize;       /* speech frameSize */
	int numChans;        /* number of channels */
	long sampPeriod;     /* sample period */
	int fftN;            /* fft size */
	int klo, khi;         /* lopass to hipass cut-off fft indices */
	int usePower;    /* use power rather than magnitude *///boolen
	int takeLogs;    /* log filterbank channels *///boolen
	float fres;          /* scaled fft resolution */
	float *cf;           /* array[1..pOrder+1] of centre freqs */
	float *loChan;     /* array[1..fftN/2] of loChan index */
	float *loWt;         /* array[1..fftN/2] of loChan weighting */
	float *x;            /* array[1..fftN] of fftchans */
};
typedef struct IOConfig
{
	float curVol;/* current volume dB (0.0-100.0) */
	float preEmph;
	int frSize;//一帧中的样本数
	int frIdx;//当前帧位置
	int frRate;//帧移
	float *fbank;
	struct FBankInfo fbInfo;
	float *s;//帧数据

};
/*读取PCM文件到wavdata中*/
void LoadFile(char *s, struct Wave *w);
void GetWave(float *buf, struct Wave *w);
void ZeroMeanFrame(float *frame);
void PreEmphasise(float *frame, float k);
void Ham(float *frame);
float Mel(int k, float fres);
float WarpFreq(float fcl, float fcu, float freq, float minFreq, float maxFreq, float alpha);
struct FBankInfo InitFBank(struct IOConfig *cf);
void FFT(float *s, int invert);
void Realft(float *s);
void Wave2FBank(float *s, float *fbank, struct FBankInfo info);
void ConvertFrame(struct IOConfig *cf, struct Wave *w);
void linkdata(struct IOConfig *cf, struct Wave *w, int k);
void zeromean(struct Wave *w);
struct Wave filter_bank(char *s);

#endif 