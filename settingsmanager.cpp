#include "settingsmanager.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , store_(QSettings::NativeFormat, QSettings::UserScope,
             QCoreApplication::organizationName(),
             QCoreApplication::applicationName())

{
    qDebug() << "settings  manager konstruktor";
    current_ = loadFromStore();
    emit settingsChanged(current_);
    emit shortcutsChanged(current_.shortcuts);
    qDebug() << "settings  manager konstruktor konec";

}



void SettingsManager::setUnits(Units u) {
    current_.units = u;
    saveToStore(current_);
    emit settingsChanged(current_);
}

void SettingsManager::setSerial(const QString& port, int baud) {
    current_.portName = port;
    current_.baudRate = baud;
    saveToStore(current_);
    emit settingsChanged(current_);
}

void SettingsManager::setDatasource (DataSource source)
{
    current_.datasource = source;
    saveToStore(current_);
    emit settingsChanged(current_);
}
void SettingsManager::updateSettings(const Settings& s)
{
       current_ = s;
       saveToStore(current_);

       emit settingsChanged(current_);
       emit shortcutsChanged(current_.shortcuts);
}

Settings SettingsManager::loadFromStore()  {
    Settings settings;
    qDebug()<<"loadfromStore....";
    // ====== Komunikace – společná volba zdroje ======
    store_.beginGroup(QStringLiteral("Comm"));
    // výchozí: serial
    settings.datasource = stringToDataSource(store_.value(QStringLiteral("DataSource"), dataSourceToString(settings.datasource)).toString());
    qDebug() << "    ---    "  <<dataSourceToString (settings.datasource);
    store_.endGroup();
    store_.beginGroup(QStringLiteral("Serial"));
    // nové: SerialParams
        settings.serial.portName = store_.value(QStringLiteral("PortName"), settings.portName).toString();
        settings.serial.baudRate = store_.value(QStringLiteral("Baud"),     settings.baudRate).toInt();
        settings.serial.dataBits = store_.value(QStringLiteral("DataBits"), settings.serial.dataBits).toInt();
        settings.serial.stopBits = store_.value(QStringLiteral("StopBits"), settings.serial.stopBits).toInt();
        settings.serial.parity   = store_.value(QStringLiteral("Parity"),   settings.serial.parity).toInt();
        settings.serial.flow     = store_.value(QStringLiteral("Flow"),     settings.serial.flow).toInt();
    store_.endGroup();
    // ====== MODBUS ======
        store_.beginGroup(QStringLiteral("Modbus"));
        settings.modbus.isTcp      = store_.value(QStringLiteral("IsTcp"),      settings.modbus.isTcp).toBool();
        settings.modbus.serialPort = store_.value(QStringLiteral("SerialPort"), settings.modbus.serialPort).toString();
        settings.modbus.baudRate   = store_.value(QStringLiteral("Baud"),       settings.modbus.baudRate).toInt();
        settings.modbus.host       = store_.value(QStringLiteral("Host"),       settings.modbus.host).toString();
        settings.modbus.port       = store_.value(QStringLiteral("Port"),       settings.modbus.port).toInt();
        settings.modbus.serverId   = store_.value(QStringLiteral("ServerId"),   settings.modbus.serverId).toInt();
        store_.endGroup();

        // ====== SIMULATION ======
        store_.beginGroup(QStringLiteral("Simulation"));
        settings.simulation.logFile     = store_.value(QStringLiteral("LogFile"),     settings.simulation.logFile).toString();
        settings.simulation.speedFactor = store_.value(QStringLiteral("SpeedFactor"), settings.simulation.speedFactor).toDouble();
        store_.endGroup();

        // ====== UI / Sho

    //qDebug()<<" Baud" << settings.baudRate ;
    store_.beginGroup(QStringLiteral("UI"));
    settings.units       = stringToUnits(store_.value(QStringLiteral("Units"), unitsToString(settings.units)).toString());
    settings.arms_color = store_.value("arms_color", settings.arms_color).value<QColor>();
    settings.save_main_window_position_on_exit = store_.value(QStringLiteral("save_main_window_position_on_exit"), true).toBool();
    qDebug()<<"save_main_window_position_on_exit " << settings.save_main_window_position_on_exit ;
    settings.main_window_position = store_.value(QStringLiteral("main_window_position"), QRect()).toRect();
    settings.save_measure_window_position_on_exit = store_.value(QStringLiteral("save_measure_window_position_on_exit"), true).toBool();
       settings.measure_window_position = store_.value(QStringLiteral("measure_window_position"), QRect()).toRect();
    qDebug() << " load 2";
    settings.language = store_.value(QStringLiteral("language"),settings.language).toString();
    qDebug() << " load 3";
    settings.directory_save_dxf = store_.value(QStringLiteral("directory_save_dxf"),"c:/").toString();
    settings.directory_save_data = store_.value(QStringLiteral("directory_save_data"),"c:/").toString();
    qDebug() << " load 3";
    store_.endGroup();
    qDebug() << " load !";
    store_.beginGroup(QStringLiteral("Shortcuts"));
    settings.shortcuts = Shortcuts::defaults();
    for (auto it = settings.shortcuts.map.begin(); it != settings.shortcuts.map.end(); ++it) {
        const QString key = it.key();
        const QString seq = store_.value(key, it.value().toString()).toString();
        //qDebug() << " load " << it.value() << "   "  << seq;
        it.value() = QKeySequence(seq);
    }
    store_.endGroup();
    qDebug() << " load ";
    store_.beginGroup(QStringLiteral("Program"));
    settings.auto_step= store_.value(QStringLiteral("auto_step"),settings.auto_step).toDouble();
   // alfa_offset = double(setting.value("alfa_offset",0).toDouble ());
   // beta_offset = double(setting.value("beta_offset",0).toDouble ());
    settings.alfa_offset= store_.value(QStringLiteral("alfa_offset"),settings.alfa_offset).toDouble();
    settings.beta_offset= store_.value(QStringLiteral("beta_offset"),settings.beta_offset).toDouble();
    settings.arm1_length= store_.value(QStringLiteral("arm1_length"),settings.arm1_length).toDouble();
    settings.arm2_length= store_.value(QStringLiteral("arm2_length"),settings.arm2_length).toDouble();
    store_.endGroup();
    qDebug() << " load from store end ";
    return settings;
}

void SettingsManager::saveToStore(const Settings& settings)  {

    // ====== Comm: vybraný zdroj ======
    store_.beginGroup(QStringLiteral("Comm"));
        store_.setValue(QStringLiteral("DataSource"), dataSourceToString(settings.datasource));
    store_.endGroup();

    store_.beginGroup(QStringLiteral("Serial"));
        store_.setValue(QStringLiteral("PortName"), settings.serial.portName);
        store_.setValue(QStringLiteral("Baud"),     settings.serial.baudRate);
        store_.setValue(QStringLiteral("DataBits"), settings.serial.dataBits);
        store_.setValue(QStringLiteral("StopBits"), settings.serial.stopBits);
        store_.setValue(QStringLiteral("Parity"),   settings.serial.parity);
        store_.setValue(QStringLiteral("Flow"),     settings.serial.flow);
    store_.endGroup();

    // ====== Modbus ======
    store_.beginGroup(QStringLiteral("Modbus"));
        store_.setValue(QStringLiteral("IsTcp"),      settings.modbus.isTcp);
        store_.setValue(QStringLiteral("SerialPort"), settings.modbus.serialPort);
        store_.setValue(QStringLiteral("Baud"),       settings.modbus.baudRate);
        store_.setValue(QStringLiteral("Host"),       settings.modbus.host);
        store_.setValue(QStringLiteral("Port"),       settings.modbus.port);
        store_.setValue(QStringLiteral("ServerId"),   settings.modbus.serverId);
    store_.endGroup();

        // ====== Simulation ======
    store_.beginGroup(QStringLiteral("Simulation"));
        store_.setValue(QStringLiteral("LogFile"),     settings.simulation.logFile);
        store_.setValue(QStringLiteral("SpeedFactor"), settings.simulation.speedFactor);
    store_.endGroup();

        // ====== UI / Shortcuts / Program ======
    store_.beginGroup(QStringLiteral("UI"));
        store_.setValue(QStringLiteral("Units"),unitsToString(settings.units));
        store_.setValue(QStringLiteral("arms_color"),settings.arms_color);
        store_.setValue(QStringLiteral("save_main_window_position_on_exit"),settings.save_main_window_position_on_exit);
        qDebug()<<" ukladam save_measure_window_position_on_exit " << settings.save_measure_window_position_on_exit;
        store_.setValue(QStringLiteral("main_window_position"),settings.main_window_position);
        store_.setValue(QStringLiteral("save_measure_window_position_on_exit"),settings.save_measure_window_position_on_exit);
        store_.setValue(QStringLiteral("measure_window_position"),settings.measure_window_position);
        store_.setValue(QStringLiteral("language"),settings.language);
        store_.setValue(QStringLiteral("directory_save_dxf"),settings.directory_save_dxf);
        store_.setValue(QStringLiteral("directory_save_data"),settings.directory_save_data);
    store_.endGroup();

    store_.beginGroup(QStringLiteral("Shortcuts"));
    for (auto it = settings.shortcuts.map.constBegin(); it != settings.shortcuts.map.constEnd(); ++it) {
        //qDebug()<<"shortcuts " << it.key() << "  " << it.value().toString();
        store_.setValue(it.key(), it.value().toString());
    }
    store_.endGroup();

    store_.beginGroup(QStringLiteral("Program"));
        store_.setValue(QStringLiteral("auto_step"),(settings.auto_step));
        store_.setValue(QStringLiteral("alfa_offset"),(settings.alfa_offset));
        store_.setValue(QStringLiteral("beta_offset"),(settings.beta_offset));
        store_.setValue(QStringLiteral("arm1_length"),(settings.arm1_length));
        store_.setValue(QStringLiteral("arm2_length"),(settings.arm2_length));
    store_.endGroup();


    store_.sync();
}

// --- export/import JSON (optional) ---
QJsonObject SettingsManager::shortcutsToJson(const Shortcuts& sc) {
    QJsonObject obj;
    for (auto it = sc.map.constBegin(); it != sc.map.constEnd(); ++it)
        obj.insert(it.key(), it.value().toString());
    return obj;
}
Shortcuts SettingsManager::jsonToShortcuts(const QJsonObject& obj) {
    Shortcuts sc = Shortcuts::defaults();
    for (auto it = obj.begin(); it != obj.end(); ++it)
        sc.map[it.key()] = QKeySequence(it.value().toString());
    return sc;
}

bool SettingsManager::exportJson(const QString& filePath, QString* err) const {
    const Settings& st = current_;
    QJsonObject root, serial, ui;

    serial[QStringLiteral("PortName")] = st.portName;
    serial[QStringLiteral("Baud")]     = st.baudRate;

    ui[QStringLiteral("Units")]        = unitsToString(st.units);

    root[QStringLiteral("Serial")]    = serial;
    root[QStringLiteral("UI")]        = ui;
    root[QStringLiteral("Shortcuts")] = shortcutsToJson(st.shortcuts);

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly)) { if (err) *err = QStringLiteral("Nelze otevřít k zápisu"); return false; }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool SettingsManager::importJson(const QString& filePath, QString* err) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) { if (err) *err = QStringLiteral("Nelze otevřít k čtení"); return false; }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) { if (err) *err = QStringLiteral("Neplatný JSON"); return false; }

    const QJsonObject root = doc.object();

    if (root.contains(QStringLiteral("Serial"))) {
        const auto o = root.value(QStringLiteral("Serial")).toObject();
        current_.portName = o.value(QStringLiteral("PortName")).toString(current_.portName);
        current_.baudRate = o.value(QStringLiteral("Baud")).toInt(current_.baudRate);
    }
    if (root.contains(QStringLiteral("UI"))) {
        const auto o = root.value(QStringLiteral("UI")).toObject();
        current_.units = stringToUnits(o.value(QStringLiteral("Units"))
                                       .toString(unitsToString(current_.units)));
    }
    if (root.contains(QStringLiteral("Shortcuts"))) {
        current_.shortcuts = jsonToShortcuts(root.value(QStringLiteral("Shortcuts")).toObject());
    }

    saveToStore(current_);
    emit settingsChanged(current_);
    emit shortcutsChanged(current_.shortcuts);
    return true;
}

