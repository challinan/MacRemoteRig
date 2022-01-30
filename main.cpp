#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QStringList>

int main(int argc, char *argv[])
{
    QStringList strList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QFile f;
    int rc;

    // The following logic tests for the presence of a file in ~/.macrr and if it exists
    // assume that another instance is running and refuse to start up.

    long size = strList.size();
    if ( size > 1 ) {
        // Something strange happened - this function should return $HOME and nothing else
        qDebug() << "main(): encountered multiple home locations - exiting";
        QApplication::exit(3);
    }


    f.setFileName(strList.at(0) + QString("/.macrr/.running"));
    qDebug() << "main(): file name" << f.fileName();
    if ( f.exists() ) {
        qDebug() << "main(): Another instance of this program is already running - exiting";
        return 19;
    }

    if ( !f.open(QIODevice::WriteOnly) ) {
        qDebug() << "main(): f.open failed";
        return -1;
    }

    QApplication a(argc, argv);
    QApplication::setApplicationDisplayName("Mac Remote Rig");
    MainWindow w;
    w.show();
    rc = a.exec();
    qDebug() << "main(): exiting via a.exec()";

    f.close();
    if ( !f.remove() )
        qDebug() << "main(): failed to remove file";

    return rc;
}
