#include <QApplication>
#include "mainwindow.h"
#include "appmanager.h"
#include "serialmanager.h"
#include "settingsmanager.h"
#include "SettingsDialog.h"
#include "idatasourceworker.h"
#include <QMetaType>
#include <QSplashScreen>
#include <QApplication>
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Frame>("Frame");
    a.setWindowIcon(QIcon(":///pic/icon.png"));
    QCoreApplication::setOrganizationName(QStringLiteral("LiborSoft"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("www.example.com"));
    QCoreApplication::setApplicationName(QStringLiteral("Digitizer2"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QPixmap logo(":/pic/splash.png"); // Upravte cestu k vašemu obrázku
    QSplashScreen splash(logo);
    splash.show();
    QTimer::singleShot(2000, &splash, &QSplashScreen::close);

    QObject root;                 // kořenový vlastník, není to okno, kvuli vlastnictvi a zavirani oken
    auto* applicationManager     = new AppManager(&root);  // dítě rootu (ne MainWindow!)
    auto* settingsManager   = new SettingsManager(&root);
    applicationManager->setSettingsManager(settingsManager);
    MainWindow mainwindow(applicationManager);      // top‑level okno, parent = nullptr
    auto* serial_mng = new SerialManager();      // bez parenta
    applicationManager->setSerialManager(serial_mng);
    const Settings loadedSettings = settingsManager->currentSettings();

    const QString port = loadedSettings.serial.portName.isEmpty()
                         ? loadedSettings.portName
                         : loadedSettings.serial.portName;
    qDebug() << "serial port je" << port ;
    if (loadedSettings.datasource == DataSource::Serial && port.trimmed().isEmpty()) {
        SettingsDialog dlg(&mainwindow, settingsManager);
        dlg.setSettings(loadedSettings);
        if (dlg.exec() == QDialog::Accepted) {
            settingsManager->updateSettings(dlg.result());
        }
    }
    QTimer::singleShot(2000, &mainwindow, &MainWindow::show);
    return a.exec();
}
