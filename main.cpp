#include <QApplication>
#include "mainwindow.h"
#include "appmanager.h"
#include "serialmanager.h"
#include "settingsmanager.h"
#include <QSplashScreen>
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":///pic/icon.png"));

    QCoreApplication::setOrganizationName(QStringLiteral("LiborSoft"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("www.example.com"));
    QCoreApplication::setApplicationName(QStringLiteral("Digitizer2"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QPixmap logo(":/pic/splash.png"); // Upravte cestu k vašemu obrázku
    QSplashScreen splash(logo);
    splash.show();
    QTimer::singleShot(2000, &splash, &QSplashScreen::close);


    QObject root;                 // kořenový vlastník, není to okno
    auto* applicationManager     = new AppManager(&root);  // dítě rootu (ne MainWindow!)
    auto* settingsManager   = new SettingsManager(&root);
    applicationManager->setSettingsManager(settingsManager);
    MainWindow mainwindow(applicationManager);      // top‑level okno, parent = nullptr
    auto* serial = new SerialManager();      // bez parenta
    applicationManager->setSerialManager(serial);
    QTimer::singleShot(2000, &mainwindow, &MainWindow::show);
    return a.exec();
}
