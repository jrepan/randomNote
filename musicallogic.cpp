#include "musicallogic.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <QTimer>
#include <QQmlContext>
#include <QQmlProperty>
#include <fftw3.h>

#define PLOT 0

const int millisecondsPerSecond = 1000;
const int microsecondsPerSecond = millisecondsPerSecond * 1000;
const int nanosecondsPerSecond = microsecondsPerSecond * 1000;
const int majorScale[7] = {2, 1, 2, 2, 1, 2, 2};
const int scales[12][8] =
{
	//A, B, C, D, E, F, G
	{-1,-1, 0,-1,-1, 0,-1 }, // Db major
	{-1,-1, 0,-1,-1, 0, 0 }, // Ab major
	{-1,-1, 0, 0,-1, 0, 0 }, // Eb major
	{ 0,-1, 0, 0,-1, 0, 0 }, // Bb major
	{ 0,-1, 0, 0, 0, 0, 0 }, // F  major
	{ 0, 0, 0, 0, 0, 0, 0 }, // C  major
	{ 0, 0, 0, 0, 0, 1, 0 }, // G  major
	{ 0, 0, 1, 0, 0, 1, 0 }, // D  major
	{ 0, 0, 1, 0, 0, 1, 1 }, // A  major
	{ 0, 0, 1, 1, 0, 1, 1 }, // E  major
	{ 1, 0, 1, 1, 0, 1, 1 }, // B  major
	{ 1, 0, 1, 1, 1, 1, 1 }  // F# major
};

MusicalLogic::MusicalLogic(QQmlContext *c, QObject *rootObject):
	context(c),
	fromNote(rootObject->findChild<QObject*>("fromNote")),
	fromOctave(rootObject->findChild<QObject*>("fromOctave")),
	toNote(rootObject->findChild<QObject*>("toNote")),
	toOctave(rootObject->findChild<QObject*>("toOctave")),
	transposition(rootObject->findChild<QObject*>("transposition")),
	semitones(rootObject->findChild<QObject*>("semitones")),
	scale(rootObject->findChild<QObject*>("scale")),
	margin(rootObject->findChild<QObject*>("marginOfError")),
	canvas(rootObject->findChild<QObject*>("canvas"))
{
	assert(A4Index == getIndex(0, 4));
	assert(getName(A4Index) == "A4");
	assert(getFulltoneNumber(A4Index, true) == 0);

	frames = new double[framesCount];

	format.setSampleRate(sampleRate);
	format.setChannelCount(1);
	format.setSampleSize(8 * sizeof(float));
	format.setCodec("audio/pcm");
	format.setSampleType(QAudioFormat::Float);
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	if (!info.isFormatSupported(format)) {
	   qWarning() << "Default format not supported, trying to use the nearest.";
	   format = info.nearestFormat(format);
	}

	input = new QAudioInput(format);
	buffer.open(QIODevice::ReadWrite);

	timer.setInterval(milliseconds);
	connect(&timer, SIGNAL(timeout()), this, SLOT(inputReady()));

	input->start(&buffer);
	timer.start();
}

MusicalLogic::~MusicalLogic()
{
	delete input;
	delete[] frames;
}

void MusicalLogic::inputReady()
{
	input->stop();

	int N = buffer.size() / sizeof(float);
	int Nf = N / 2 + 1;
	float *data = (float*) buffer.buffer().data();

	fftwf_complex *freq = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * Nf);
	fftwf_plan plan = fftwf_plan_dft_r2c_1d(N, data, freq, FFTW_ESTIMATE);
	fftwf_execute(plan);

	int max = -1;
	float best = 0;
	for (int i = 1; i < Nf; i++)
	{
		float cur = sqrt(freq[i][0]*freq[i][0] + freq[i][1]*freq[i][1]);
		if (cur > best)
			best = cur;
	}
	float prev;
	for (int i = 1; i < Nf; i++)
	{
		float cur = sqrt(freq[i][0]*freq[i][0] + freq[i][1]*freq[i][1]);
		if (cur * 10 > best || (max == i - 1 && cur > prev))
		{
//			qDebug() << i << cur;
			max = i;
			break;
		}
		prev = cur;
	}
	frames[currentFrame++] = max / ((double) N / sampleRate);

#if PLOT
	double seconds = (double) milliseconds / millisecondsPerSecond;
	for (int i = 0; i < Nf && i < 1200 / seconds; i++)
	{
		float cur = freq[i][0]*freq[i][0] + freq[i][1]*freq[i][1];
		QMetaObject::invokeMethod(canvas, "plot", Q_ARG(QVariant, i * seconds), Q_ARG(QVariant, cur / best));
	}
#endif

	if (currentFrame == framesCount)
	{
		double frequency = 0;
		for (int i = 0; i < framesCount; i++)
			frequency += frames[i];
		frequency /= framesCount;

		double cents = 1200 * log2(frequency / getFrequency());
		double marginOfError = QQmlProperty(margin, "value").read().toDouble();
		context->setContextProperty("lastText", "difference: " + QString::number(cents) + " cents; got " + QString::number(frequency) + " Hz, expected " + QString::number(getFrequency()) + " Hz");
		if (fabs(cents) < marginOfError)
		{
			// Correct, choose new note
			int from = getIndex(QQmlProperty(fromNote, "currentIndex").read().toInt(), QQmlProperty(fromOctave, "value").read().toInt());
			int to = getIndex(QQmlProperty(toNote, "currentIndex").read().toInt(), QQmlProperty(toOctave, "value").read().toInt());

			//TODO: prettier solution
			do
			{
				nextNote = rand() % (to - from + 1) + from;
			}
			while (!QQmlProperty(semitones, "isChecked").read().toBool() && !isFulltone(nextNote));

			context->setContextProperty("sharp", !isFulltone(nextNote));
			context->setContextProperty("noteY", getFulltoneNumber(nextNote, true));
			int extraLinesAbove = (getFulltoneNumber(nextNote, true) - 5) / 2;
			int extraLinesBelow = (-getFulltoneNumber(nextNote, true) - 3) / 2;
			context->setContextProperty("extraLinesAbove", extraLinesAbove > 0 ? extraLinesAbove : 0);
			context->setContextProperty("extraLinesBelow", extraLinesBelow > 0 ? extraLinesBelow : 0);
#if !PLOT
			QMetaObject::invokeMethod(canvas, "requestPaint");
#endif
		}
		currentFrame = 0;
	}
#if PLOT
	QMetaObject::invokeMethod(canvas, "requestPaint");
#endif

	fftwf_destroy_plan(plan);
	fftwf_free(freq);

	buffer.close();
	buffer.setData(emptyData);
	buffer.open(QIODevice::ReadWrite);

	input->start(&buffer);
}

int MusicalLogic::getIndex(int note, int octave) const
{
	if (note >= 2) // Octave starts from C
		octave--;
	return noteIndexes[note] + (octave - 1) * 12;
}

bool MusicalLogic::isFulltone(int index) const
{
	int note = index % 12;
	if (note >= 3) // B
		note++;
	if (note >= 9) // E
		note++;
	return note % 2 == 0;
}

// If index is full tone, returns full tone number (e.g. 4 for D5)
// If index is semi tone, rounds down (D#5 is rounded to D5)
int MusicalLogic::getFulltoneNumber(int index, bool withOctave) const
{
	int note = index % 12;
	int octave = index / 12 + 1 - 4;
	if (note >= 3) // B
		note++;
	if (note >= 9) // E
		note++;
	int fulltone = note / 2;
	return fulltone + (withOctave ? octave * 7 : 0);
}

QString MusicalLogic::getName(int index) const
{
	int note = index % 12;
	int octave = index / 12 + 1;
	if (note >= 3) // Octave starts from C
		octave++;
	return notes[note] + QString('0' + octave);
}

double MusicalLogic::getFrequency()
{
	int currentTransposition = QQmlProperty(transposition, "value").read().toInt();
	int currentScale = QQmlProperty(scale, "currentIndex").read().toInt();

	int transpositionTones = 0;
	if (currentTransposition > 0)
		for (int i = 0; i < currentTransposition; i++)
			transpositionTones += majorScale[(i + getFulltoneNumber(nextNote, false)) % 7];
	else
		for (int i = 0; i < -currentTransposition; i++)
			transpositionTones -= majorScale[6 - ((i + 1 + getFulltoneNumber(nextNote, false)) % 7)];

	int index = nextNote - A4Index + transpositionTones + scales[currentScale][getFulltoneNumber(nextNote, false)];
	return A4Frequency * pow(2, index / 12.0);
}
