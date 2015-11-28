#include <cstdlib>
#include <ctime>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "musicallogic.h"

int main(int argc, char *argv[])
{
	srand(time(NULL));

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("lastText", "");
	engine.rootContext()->setContextProperty("sharp", QVariant(false));
	engine.rootContext()->setContextProperty("noteY", 0);
	engine.rootContext()->setContextProperty("extraLinesAbove", 0);
	engine.rootContext()->setContextProperty("extraLinesBelow", 0);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	MusicalLogic logic(engine.rootContext(), engine.rootObjects().first());

    return app.exec();
}
