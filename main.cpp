#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>
#include <QLocale>
#include <time.h>
#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message=QString("(%5) %1 (%2:%3 [%4])\n").arg(msg).arg(context.file).arg(context.line)
            .arg(context.function).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));

    switch (type) {
    case QtDebugMsg:
        message.prepend("[Debug]");
        std::cerr<<qPrintable(message);
        break;
    case QtWarningMsg:
        message.prepend("[Warning]");
        QMessageBox::warning(0,"Attention",message);
        std::cerr<<qPrintable(message);
        break;
    case QtCriticalMsg:
        message.prepend("[Critical]");
        QMessageBox::critical(0,"Erreur critique",message);
        std::cerr<<qPrintable(message);
        break;
    case QtFatalMsg:
        message.prepend("[Fatal]");
        QMessageBox::critical(0,"Erreur fatale",message);
        std::cerr<<qPrintable(message);
        die();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load(
                "qt_" + QLocale::system().name(),
                QLibraryInfo::location(QLibraryInfo::TranslationsPath)
                );
    a.installTranslator(&qtTranslator);

    //qSetMessagePattern("[%{type}] %{appname} (%{file}:%{line}) - %{message}");

    qInstallMessageHandler(myMessageOutput);

    qDebug()<<"***ProtonClassicSuite starting***";

    int retval;

    QSqlDatabase::addDatabase("QSQLITE");

    MainWindow w;
    w.show();

    retval=a.exec();
    QSqlDatabase::database().close();
    return retval;
}
