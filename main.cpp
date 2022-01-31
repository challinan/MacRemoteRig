#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QStringList>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    bool need_reset = false;

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
    if ( f.exists() ) {
        qDebug() << "main(): Another instance of this program is already running - exiting";
        need_reset = true;
    }

    if ( !f.open(QIODevice::WriteOnly) ) {
        qDebug() << "main(): f.open failed";
        return -1;
    }

    QApplication a(argc, argv);
    QApplication::setApplicationDisplayName("Mac Remote Rig");

    if ( need_reset ) {
        // Put up a  message if possible
        QString reset_msg = "Another Instance of this program has been detected!  Another instance may be running, \
                             or the prior instance crashed. Reset?";

        QMessageBox mbox = QMessageBox(QMessageBox::Question, "Application Already Running?", reset_msg, QMessageBox::Abort|QMessageBox::Reset);
        mbox.setDefaultButton(QMessageBox::Reset);
        int std_button = mbox.exec();
        if ( std_button == QMessageBox::Abort) {
            return -4;  // Bail out
        }
    }

    MainWindow w;
    if ( w.failed() ) {        //Check results
        QString fmsg = w.failedReason();
        fmsg.prepend("MacRemoteRig failed initialization:\n\n");
        QMessageBox failBox = QMessageBox(QMessageBox::Critical, "Startup Failed", fmsg, QMessageBox::Abort);
        qDebug() << "main(): MainWindow constructor failed" << w.failed();
        failBox.exec();
        return 14;
    }

    w.show();
    rc = a.exec();
    qDebug() << "main(): exiting via a.exec()";

    f.close();
    if ( !f.remove() )
        qDebug() << "main(): failed to remove file";

    return rc;
}
