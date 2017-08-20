#ifndef MUSICALLOGIC_H
#define MUSICALLOGIC_H

#include <cstdint>
#include <QDebug>
#include <QAudioInput>
#include <QBuffer>
#include <QTimer>
#include <aubio/aubio.h>

class QQmlContext;

class MusicalLogic: public QObject
{
	Q_OBJECT
public:
	MusicalLogic(QQmlContext *context, QObject *rootObject);
    ~MusicalLogic();
public slots:
	void inputReady();
private:
	const char* notes[12] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
	const int noteIndexes[7] = {0, 2, 3, 5, 7, 8, 10};

	int getIndex(int note, int octave) const;
	bool isFulltone(int index) const;
	int getFulltoneNumber(int index, bool withOctave) const;
	QString getName(int index) const; // Currently not used, but probably will be in the future
	double getFrequency();

	const int sampleRate = 8000;
	const int milliseconds = 1000;

	const double A4Frequency = 440;
	const int A4Index = 0 + (4 - 1) * 12;

	QQmlContext *context;
	QAudioFormat format;
	QBuffer buffer;
	QAudioInput *input;
	QByteArray emptyData;
	QTimer timer;
	QObject *fromNote, *fromOctave, *toNote, *toOctave;
	QObject *transposition;
	QObject *semitones;
	QObject *scale;
	QObject *margin;
	QObject *canvas;
	fvec_t *output;

	int nextNote = A4Index;
};

#endif // MUSICALLOGIC_H
