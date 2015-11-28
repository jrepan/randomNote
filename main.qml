import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2

ApplicationWindow {
    title: qsTr("randomNote")
    width: 480
    height: 640
    visible: true

    // Last attempt ---------------------------------
    Label {
        id: lastLabel;
        text: lastText;
    }

    // Note display ---------------------------------
    Canvas {
        id: noteDisplay
        objectName: "canvas";
        property string clefimage: "qrc:///notes/clef.svg";
        property string noteimage: "qrc:///notes/note.svg";
        property string sharpimage: "qrc:///notes/sharp.svg";
        property string flatimage: "qrc:///notes/flat.svg";

        anchors.bottom: fromLabel.top;
        anchors.top: lastLabel.bottom;
        anchors.left: parent.left;
        anchors.right: parent.right;

        Component.onCompleted: {
            loadImage(clefimage);
            loadImage(noteimage);
            loadImage(sharpimage);
            loadImage(flatimage);
        }

        onPaint: {
            var ctx = noteDisplay.getContext('2d');
            ctx.clearRect(0, 0, noteDisplay.width, noteDisplay.height);
//TODO: remove commented out code
//ctx.clearRect(0, 0, 1000, 1000);
            ctx.drawImage(clefimage, 50, 50)
            ctx.drawImage(noteimage, 300, 52 - 11 * (noteY + 3))
            if (sharp) {
                ctx.drawImage(sharpimage, 280, 130 - 11 * (noteY + 3))
            }

            // Scale
            if (scale.currentIndex >= 6) {
                ctx.drawImage(sharpimage, 130, 130 - 11 * (5 + 3) + 3)
            }
            if (scale.currentIndex >= 7) {
                ctx.drawImage(sharpimage, 150, 130 - 11 * (2 + 3) + 3)
            }
            if (scale.currentIndex >= 8) {
                ctx.drawImage(sharpimage, 170, 130 - 11 * (6 + 3) + 3)
            }
            if (scale.currentIndex >= 9) {
                ctx.drawImage(sharpimage, 190, 130 - 11 * (3 + 3) + 3)
            }
            if (scale.currentIndex >= 10) {
                ctx.drawImage(sharpimage, 210, 130 - 11 * (0 + 3) + 3)
            }
            if (scale.currentIndex >= 11) {
                ctx.drawImage(sharpimage, 230, 130 - 11 * (4 + 3) + 3)
            }

            if (scale.currentIndex <= 4) {
                ctx.drawImage(flatimage, 130, 130 - 11 * (1 + 3) + 3)
            }
            if (scale.currentIndex <= 3) {
                ctx.drawImage(flatimage, 150, 130 - 11 * (4 + 3) + 3)
            }
            if (scale.currentIndex <= 2) {
                ctx.drawImage(flatimage, 170, 130 - 11 * (0 + 3) + 3)
            }
            if (scale.currentIndex <= 1) {
                ctx.drawImage(flatimage, 190, 130 - 11 * (3 + 3) + 3)
            }
            if (scale.currentIndex <= 0) {
                ctx.drawImage(flatimage, 210, 130 - 11 * (-1 + 3) + 3)
            }

//ctx.stroke();
//for (var i = 0; i <= 1200; i += 200)
//{
//ctx.beginPath();
//ctx.lineTo(i, 300);
//ctx.lineTo(i + 100, 300);
//ctx.stroke();
//}
            for (var i = 1; i <= extraLinesAbove; i++) {
                ctx.beginPath()
                ctx.lineTo(300, 78 - 21 * i)
                ctx.lineTo(370, 78 - 21 * i)
                ctx.stroke()
            }
            for (var i = 1; i <= extraLinesBelow; i++) {
                ctx.beginPath()
                ctx.lineTo(300, 164 + 21 * i)
                ctx.lineTo(370, 164 + 21 * i)
                ctx.stroke()
            }
//ctx.beginPath();
//ctx.lineTo(0, 400);
        }

        onImageLoaded: {
            requestPaint();
        }

//function plot(freq, value) {
//    var ctx = noteDisplay.getContext('2d');
//    ctx.lineTo(freq, 400 - value * 100);
//}
    }

    // Transposition --------------------------------
    Label {
        id: transpositionLabel;
        text: qsTr("Transpose ");

        anchors.top: transposition.top;
    }

    SpinBox {
        id: transposition;
        objectName: "transposition";
        minimumValue: -maximumValue;
        value: 0;

        anchors.left: transpositionLabel.right;
        anchors.bottom: semitones.top;
    }

    Label {
        id: transpositionUnitsLabel;
        text: qsTr(" full tones");

        anchors.left: transposition.right;
        anchors.top: transposition.top;
    }

    // Semitones ------------------------------------
    Label {
        id: semiLabel;
        text: qsTr("Allow semitones: ");

        anchors.top: semitones.top;
    }

    CheckBox {
        id: semitones;

        anchors.left: semiLabel.right;
        anchors.bottom: scale.top;
    }

    // Scale ----------------------------------------
    Label {
        id: scaleLabel;
        text: qsTr("Scale: ");

        anchors.top: scale.top;
    }

    ComboBox {
        id: scale;
        objectName: "scale";
        model: [ 'Db major', 'Ab major', 'Eb major', 'Bb major', 'F major', 'C major', 'G major', 'D major', 'A major', 'E major', 'B major', 'F# major' ];
        width: 150;
        currentIndex: 5;

        anchors.left: scaleLabel.right;
        anchors.bottom: fromNote.top;

        onActivated: {
            noteDisplay.requestPaint();
        }
    }

    // Range ----------------------------------------
    Label {
        id: fromLabel;
        text: qsTr("Choose notes from ");

        anchors.top: fromNote.top;
    }

    ComboBox {
        id: fromNote;
        objectName: "fromNote";
        model: [ 'A', 'B', 'C', 'D', 'E', 'F', 'G' ];
        width: 50;
        currentIndex: 6;

        anchors.left: fromLabel.right;
        anchors.bottom: cents.top;
    }

    SpinBox {
        id: fromOctave;
        objectName: "fromOctave";
        minimumValue: 1;
        maximumValue: 9;
        value: 3;

        anchors.left: fromNote.right;
        anchors.bottom: cents.top;
    }

    Label {
        id: toLabel;
        text: qsTr(" to ");

        anchors.left: fromOctave.right;
        anchors.top: fromNote.top;
    }

    ComboBox {
        id: toNote;
        objectName: "toNote";
        model: [ 'A', 'B', 'C', 'D', 'E', 'F', 'G' ];
        width: 50;
        currentIndex: 1;

        anchors.left: toLabel.right;
        anchors.top: fromNote.top;
    }

    SpinBox {
        id: toOctave;
        objectName: "toOctave";
        minimumValue: 1;
        maximumValue: 9;
        value: 5;

        anchors.left: toNote.right;
        anchors.top: fromNote.top;
    }

    // Margin of error ------------------------------
    Label {
        id: marginLabel;
        text: "Allowed margin of error: ";

        anchors.top: cents.top;
        anchors.left: parent.left;
    }

    Slider {
        id: cents;
        objectName: "marginOfError";
        minimumValue: 0;
        maximumValue: 50;
        stepSize: 1;
        value: 10;

        anchors.bottom: parent.bottom;
        anchors.left: marginLabel.right;
    }

    Label {
        id: centsLabel;
        text: cents.value + qsTr(" cents");

        anchors.left: cents.right;
        anchors.top: cents.top;
    }
}
