
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <time.h>
#include "nNet.h"
#include "filterbank.h"

/*��ȡPCM�ļ���wavdata��*/
void LoadFile(char *s, struct Wave *w)
{
	FILE *fp;
	fp = fopen(s, "rb");
	if (!fp)
	{
		printf("can not open this file\n");
		exit(0);
	}
	unsigned char ch1, ch2, ch3, ch4;
	int i;
	float *buf;
	buf = w->wavdata;
	for (i = 0; i < 40; i++)
	{
		ch1 = fgetc(fp);
	}
	ch1 = fgetc(fp); ch2 = fgetc(fp); ch3 = fgetc(fp); ch4 = fgetc(fp);
	w->nSample = (ch2 * 16 * 16 + ch1) + (ch4 * 16 * 16 + ch3) * 16 * 16 * 16 * 16;
	w->nSample /= 2;
	printf("%ld", w->nSample);
	w->nRow = (w->nSample - w->frSize) / w->frRate + 1;
	for (i = 0; i<w->nSample; i++)
	{
		ch1 = fgetc(fp); //ÿ�ζ�ȡ�����ַ�����������ch��
		ch2 = fgetc(fp);
		//if (i % 8 == 0)      //ÿ�����16���ַ���Ӧ��ʮ��������
		//printf("\n");
		float temp = ch2 * 16 * 16 + ch1;
		if (temp < 32768){
			//printf("%d ", temp);
			*buf++ = temp;
			//printf("%lf ", (float)(ch2 * 16 * 16 + ch1) / 32767);
		}
		else{
			//printf("%d ",temp - 65535 - 1);
			*buf++ = temp - 65535 - 1;
			//printf("%lf ", (float)((ch2 * 16 * 16 + ch1)-65535-1) / 32768);
		}
	}
	fclose(fp);
}
/*��wavdata����ȡ��ǰ֡*/
void GetWave(float *buf, struct Wave *w)
{
	int k;
	if (w->frIdx + w->frSize > w->nSample)
	{
		printf("GetWave: attempt to read past end of buffer\n");
		for (k = 0; k < w->frSize; k++)
		{
			buf[k] = 0;
		}
		for (k = 0; w->frIdx + k < w->nSample; k++)
		{
			buf[k] = w->wavdata[w->frIdx + k];
		}

	}

	for (k = 0; k < w->frSize; k++)
	{
		buf[k] = w->wavdata[w->frIdx + k];
	}
	w->frIdx += w->frRate;
}

void ZeroMeanFrame(float *frame)
{
	int size, i;
	float sum = 0.0, off;
	size = frame[0];
	for (i = 1; i <= size; i++) sum += frame[i];
	off = sum / size;
	for (i = 1; i <= size; i++) frame[i] -= off;
}
void PreEmphasise(float *frame, float k)
{
	int i;
	float preE = k;
	int size = frame[0];
	for (i = size; i >= 2; i--)
		frame[i] -= frame[i - 1] * preE;
	frame[1] *= 1.0 - preE;
}
void Ham(float *frame)
{
	int frameSize = frame[0];
	int i;
	float a;
	float *hamWin;
	hamWin = (float*)malloc(sizeof(float)*frameSize);
	a = TPI / (frameSize - 1);
	int b = cos(a * 0);
	for (i = 1; i <= frameSize; i++)
		hamWin[i] = 0.54 - 0.46 * cos(a*(i - 1));
	for (i = 1; i <= frameSize; i++)
	{
		frame[i] *= hamWin[i];
	}
	//free(hamWin);
}

float Mel(int k, float fres)
{
	return 1127 * log(1 + (k - 1)*fres);
}
float WarpFreq(float fcl, float fcu, float freq, float minFreq, float maxFreq, float alpha)
{
	if (alpha == 1.0)
		return freq;
	else {
		float scale = 1.0 / alpha;
		float cu = fcu * 2 / (1 + scale);
		float cl = fcl * 2 / (1 + scale);

		float au = (maxFreq - cu * scale) / (maxFreq - cu);
		float al = (cl * scale - minFreq) / (cl - minFreq);

		if (freq > cu)
			return  au * (freq - cu) + scale * cu;
		else if (freq < cl)
			return al * (freq - minFreq) + minFreq;
		else
			return scale * freq;
	}
}
struct FBankInfo InitFBank(struct IOConfig *cf)
{
	int numChans = 40; int usePower = 0; int takeLogs = 1; int sampPeriod = 625;//sampPeriodҪ����һ��
	float alpha = 1; int warpLowCut = 0; int warpUpCut = 0;
	struct FBankInfo fb;
	float mlo, mhi, ms, melk;
	int k, chan, maxChan, Nby2;
	int doubleFFT = 0;


	/* Save sizes to cross-check subsequent usage */
	fb.frameSize = cf->frSize;
	fb.numChans = numChans;
	fb.sampPeriod = sampPeriod;
	fb.usePower = usePower;
	fb.takeLogs = takeLogs;
	/* Calculate required FFT size */
	fb.fftN = 2;
	while (fb.frameSize>fb.fftN)
		fb.fftN *= 2;
	if (doubleFFT)//��ִ��
		fb.fftN *= 2;
	Nby2 = fb.fftN / 2;
	fb.fres = 1.0E7 / (sampPeriod * fb.fftN * 700.0);
	maxChan = numChans + 1;
	/* set lo and hi pass cut offs if any */
	fb.klo = 2; fb.khi = Nby2;       /* apply lo/hi pass filtering */
	mlo = 0; mhi = Mel(Nby2 + 1, fb.fres);


	/* Create vector of fbank centre frequencies */
	fb.cf = (float*)malloc(sizeof(float)*maxChan + 1);
	fb.cf[0] = maxChan;
	ms = mhi - mlo;
	for (chan = 1; chan <= maxChan; chan++) {
		if (alpha == 1.0) {
			fb.cf[chan] = ((float)chan / (float)maxChan)*ms + mlo;
		}
		else {
			/* scale assuming scaling starts at lopass */
			float minFreq = 700.0 * (exp(mlo / 1127.0) - 1.0);
			float maxFreq = 700.0 * (exp(mhi / 1127.0) - 1.0);
			float cf = ((float)chan / (float)maxChan) * ms + mlo;

			cf = 700 * (exp(cf / 1127.0) - 1.0);

			fb.cf[chan] = 1127.0 * log(1.0 + WarpFreq(warpLowCut, warpUpCut, cf, minFreq, maxFreq, alpha) / 700.0);
		}
	}

	/* Create loChan map, loChan[fftindex] . lower channel index */
	fb.loChan = (float*)malloc(sizeof(float)*Nby2 + 1);
	fb.loChan[0] = Nby2;
	for (k = 1, chan = 1; k <= Nby2; k++){
		melk = Mel(k, fb.fres);
		if (k<fb.klo || k>fb.khi) fb.loChan[k] = -1;
		else {
			while (fb.cf[chan] < melk  && chan <= maxChan) ++chan;
			fb.loChan[k] = chan - 1;
		}
	}

	/* Create vector of lower channel weights */
	fb.loWt = (float*)malloc(sizeof(float)*Nby2 + 1);
	fb.loWt[0] = Nby2;
	for (k = 1; k <= Nby2; k++) {
		chan = fb.loChan[k];
		if (k<fb.klo || k>fb.khi) fb.loWt[k] = 0.0;
		else {
			if (chan>0)
				fb.loWt[k] = ((fb.cf[chan + 1] - Mel(k, fb.fres)) /
				(fb.cf[chan + 1] - fb.cf[chan]));
			else
				fb.loWt[k] = (fb.cf[1] - Mel(k, fb.fres)) / (fb.cf[1] - mlo);
		}
	}
	/* Create workspace for fft */
	fb.x = (float*)malloc(sizeof(float)*fb.fftN + 1);
	fb.x[0] = fb.fftN;
	return fb;
}

void FFT(float *s, int invert)
{
	int ii, jj, n, nn, limit, m, j, inc, i;
	double wx, wr, wpr, wpi, wi, theta;
	double xre, xri, x;

	n = s[0];
	nn = n / 2; j = 1;
	for (ii = 1; ii <= nn; ii++) {
		i = 2 * ii - 1;
		if (j>i) {
			xre = s[j]; xri = s[j + 1];
			s[j] = s[i];  s[j + 1] = s[i + 1];
			s[i] = xre; s[i + 1] = xri;
		}
		m = n / 2;
		while (m >= 2 && j > m) {
			j -= m; m /= 2;
		}
		j += m;
	};
	limit = 2;
	while (limit < n) {
		inc = 2 * limit; theta = TPI / limit;
		if (invert) theta = -theta;
		x = sin(0.5 * theta);
		wpr = -2.0 * x * x; wpi = sin(theta);
		wr = 1.0; wi = 0.0;
		for (ii = 1; ii <= limit / 2; ii++) {
			m = 2 * ii - 1;
			for (jj = 0; jj <= (n - m) / inc; jj++) {
				i = m + jj * inc;
				j = i + limit;
				xre = wr * s[j] - wi * s[j + 1];
				xri = wr * s[j + 1] + wi * s[j];
				s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
				s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
			}
			wx = wr;
			wr = wr * wpr - wi * wpi + wr;
			wi = wi * wpr + wx * wpi + wi;
		}
		limit = inc;
	}
	if (invert)
		for (i = 1; i <= n; i++)
			s[i] = s[i] / nn;

}
void Realft(float *s)
{
	int n, n2, i, i1, i2, i3, i4;
	double xr1, xi1, xr2, xi2, wrs, wis;
	double yr, yi, yr2, yi2, yr0, theta, x;

	n = s[0] / 2; n2 = n / 2;
	theta = PI / n;
	FFT(s, 0);
	x = sin(0.5 * theta);
	yr2 = -2.0 * x * x;
	yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
	for (i = 2; i <= n2; i++) {
		i1 = i + i - 1;      i2 = i1 + 1;
		i3 = n + n + 3 - i2; i4 = i3 + 1;
		wrs = yr; wis = yi;
		xr1 = (s[i1] + s[i3]) / 2.0; xi1 = (s[i2] - s[i4]) / 2.0;
		xr2 = (s[i2] + s[i4]) / 2.0; xi2 = (s[i3] - s[i1]) / 2.0;
		s[i1] = xr1 + wrs * xr2 - wis * xi2;
		s[i2] = xi1 + wrs * xi2 + wis * xr2;
		s[i3] = xr1 - wrs * xr2 + wis * xi2;
		s[i4] = -xi1 + wrs * xi2 + wis * xr2;
		yr0 = yr;
		yr = yr * yr2 - yi  * yi2 + yr;
		yi = yi * yr2 + yr0 * yi2 + yi;
	}
	xr1 = s[1];
	s[1] = xr1 + s[2];
	s[2] = 0.0;
}
void Wave2FBank(float *s, float *fbank, struct FBankInfo info)
{
	const float melfloor = 1.0;
	int k, bin;
	float t1, t2;   /* real and imag parts */
	float ek;      /* energy of k'th fft channel */

	float te = 0.0;
	for (k = 1; k <= info.frameSize; k++)
		te += (s[k] * s[k]);
	/* Apply FFT */
	for (k = 1; k <= info.frameSize; k++)
		info.x[k] = s[k];    /* copy to workspace */
	for (k = info.frameSize + 1; k <= info.fftN; k++)
		info.x[k] = 0.0;   /* pad with zeroes */
	Realft(info.x);                            /* take fft */

	/* Fill filterbank channels */
	int i = 0;
	for (i = 1; i <= fbank[0]; i++)
		fbank[i] = 0.0;
	for (k = info.klo; k <= info.khi; k++) {             /* fill bins */
		t1 = info.x[2 * k - 1]; t2 = info.x[2 * k];
		if (info.usePower)
			ek = t1*t1 + t2*t2;
		else
			ek = sqrt(t1*t1 + t2*t2);
		bin = info.loChan[k];
		t1 = info.loWt[k] * ek;
		if (bin > 0) fbank[bin] += t1;
		if (bin < info.numChans) fbank[bin + 1] += ek - t1;
		//printf("k:%d bin:%d info.loWt:%f fbank[bin]:%f\n", k, bin, info.loWt[k], fbank[bin]);

	}

	/* Take logs */
	if (info.takeLogs)
		for (bin = 1; bin <= info.numChans; bin++) {
			t1 = fbank[bin];
			if (t1 < melfloor) t1 = melfloor;
			fbank[bin] = log(t1);
		}
}

void ConvertFrame(struct IOConfig *cf, struct Wave *w)
{
	float re, rawte = 0.0, te, *p, cepScale = 1.0;
	int i, bsize = 0;
	char buf[50];
	int rawE;//boolen
	cf->frIdx = w->frIdx; cf->frSize = w->frSize; cf->frRate = w->frRate;
	cf->preEmph = 0.97;
	ZeroMeanFrame(cf->s);//���ֵ����
	PreEmphasise(cf->s, cf->preEmph);//Ԥ���ش���
	Ham(cf->s);//�Ӵ� �˴������Ż�������ham�����ʮ�ֺķ�ʱ��,������ȫ������ham���ڴ�
	cf->fbInfo = InitFBank(cf);
	cf->fbank = (float*)malloc(sizeof(float)* NUMCHANS);
	cf->fbank[0] = NUMCHANS;
	Wave2FBank(cf->s, cf->fbank, cf->fbInfo);//��ȡNUMCHANSΪ40��fbank

}
void linkdata(struct IOConfig *cf, struct Wave *w, int k)
{
	for (int i = 0; i < NUMCHANS; i++)
	{
		*(w->Rdata + i + (k*NUMCHANS)) = *(cf->fbank + 1 + i);

	}
}
void zeromean(struct Wave *w)
{
	int i, j;
	float sum[NUMCHANS];
	int n = w->nRow;
	for (i = 0; i < NUMCHANS; i++)
	{
		sum[i] = 0.0;
		for (j = 0; j < n; j++)
		{
			sum[i] += *(w->Rdata + j*NUMCHANS + i);
		}
		sum[i] = sum[i] / n;
	}
	for (i = 0; i < NUMCHANS*n; i++)
	{
		*(w->Rdata + i) -= sum[i%NUMCHANS];
	}
}

struct Wave filter_bank(char *s)
{
	/*��ʼ��*/
	/*��������Ϊ16028*/
	/*��HTK��������Ϊ25ms����֡��Ϊ400��Ĭ��֡��Ϊ160*/
	struct Wave *w;
	w = (struct Wave*)malloc(sizeof(struct Wave));
	w->frIdx = 0; w->frRate = 160; w->frSize = 400;
	w->nSample = 160000;
	w->nRow = (w->nSample - w->frSize) / w->frRate + 1;
	w->wavdata = (float*)malloc(sizeof(float) * w->nSample);
	LoadFile(s, w);
	w->Rdata = (float*)malloc(sizeof(float)*NUMCHANS*w->nRow);

	struct IOConfig *cf;
	cf = (struct IOConfig*)malloc(sizeof(struct IOConfig));
	cf->s = (float*)malloc(sizeof(float) * w->frSize + 1);
	/*��PCM�ļ���wavdata�У�����ֱ�Ӷ�ȡ�������ݣ��˴���Ϊ�˷���PC����*/


	for (int k = 0; k < w->nRow + 1; k++)
	{
		/*��֡,����֡����*/

		cf->s[0] = w->frSize;
		GetWave(cf->s + 1, w);
		int j, m, e, x;
		for (j = 1, m = e = 0.0; j <= w->frSize; j++) {
			x = (int)cf->s[j];
			m += x; e += x*x;
		}
		m = m / w->frSize; e = e / w->frSize - m*m;
		if (e>0.0) e = 10.0*log10(e / 0.32768);
		else e = 0.0;
		cf->curVol = e;
		/*����*/
		ConvertFrame(cf, w);
		linkdata(cf, w, k);
	}
	zeromean(w);
	for (int i = 0; i < NUMCHANS*w->nRow; i++)
	{
		if (i%NUMCHANS == 0)
			printf("\n%d��\n", i / NUMCHANS);
		printf("%f ", *(w->Rdata + i));

	}
	/*free(cf->fbInfo.cf);
	free(cf->fbInfo.loChan);
	free(cf->fbInfo.loWt);
	free(cf->fbInfo.x);
	free(cf->fbank);
	free(cf->s);
	free(cf);*/
	return *w;
}