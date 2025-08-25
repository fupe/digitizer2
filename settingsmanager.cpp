#include "settingsmanager.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QDebug>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , userStore_(QSettings::NativeFormat, QSettings::UserScope,
                 QCoreApplication::organizationName(),
                 QCoreApplication::applicationName())
    , adminStore_(QSettings::NativeFormat, QSettings::SystemScope,
                  QCoreApplication::organizationName(),
                  QCoreApplication::applicationName())
{
    current_ = loadFromStore();
    emit settingsChanged(current_);
    emit shortcutsChanged(current_.shortcuts);
}



void SettingsManager::setUnits(Units u, bool admin) {
    current_.units = u;
    saveToStore(current_, admin);
    emit settingsChanged(current_);
}

void SettingsManager::setSerial(const QString& port, int baud, bool admin) {
    current_.portName = port;
    current_.baudRate = baud;
    saveToStore(current_, admin);
    emit settingsChanged(current_);
}

void SettingsManager::setDatasource(DataSource source, bool admin)
{
    current_.datasource = source;
    saveToStore(current_, admin);
    emit settingsChanged(current_);
}

void SettingsManager::updateSettings(const Settings& s, bool admin)
{
       current_ = s;
       saveToStore(current_, admin);

       emit settingsChanged(current_);
       emit shortcutsChanged(current_.shortcuts);
}

Settings SettingsManager::loadFromStore()
{
    Settings settings;
    loadFrom(adminStore_, settings);
    loadFrom(userStore_, settings);
    return settings;
}

void SettingsManager::loadFrom(QSettings& store, Settings& settings)
{
    store.beginGroup(QStringLiteral("Comm"));
    settings.datasource = stringToDataSource(store.value(QStringLiteral("DataSource"), dataSourceToString(settings.datasource)).toString());
    store.endGroup();

    store.beginGroup(QStringLiteral("Serial"));
        settings.serial.portName = store.value(QStringLiteral("PortName"), settings.portName).toString();
        settings.serial.baudRate = store.value(QStringLiteral("Baud"), settings.baudRate).toInt();
        settings.serial.dataBits = store.value(QStringLiteral("DataBits"), settings.serial.dataBits).toInt();
        settings.serial.stopBits = store.value(QStringLiteral("StopBits"), settings.serial.stopBits).toInt();
        settings.serial.parity   = store.value(QStringLiteral("Parity"), settings.serial.parity).toInt();
        settings.serial.flow     = store.value(QStringLiteral("Flow"), settings.serial.flow).toInt();
    store.endGroup();

    store.beginGroup(QStringLiteral("Modbus"));
        settings.modbus.isTcp      = store.value(QStringLiteral("IsTcp"), settings.modbus.isTcp).toBool();
        settings.modbus.serialPort = store.value(QStringLiteral("SerialPort"), settings.modbus.serialPort).toString();
        settings.modbus.baudRate   = store.value(QStringLiteral("Baud"), settings.modbus.baudRate).toInt();
        settings.modbus.host       = store.value(QStringLiteral("Host"), settings.modbus.host).toString();
        settings.modbus.port       = store.value(QStringLiteral("Port"), settings.modbus.port).toInt();
        settings.modbus.serverId   = store.value(QStringLiteral("ServerId"), settings.modbus.serverId).toInt();
    store.endGroup();

    store.beginGroup(QStringLiteral("Simulation"));
        settings.simulation.logFile     = store.value(QStringLiteral("LogFile"), settings.simulation.logFile).toString();
        settings.simulation.speedFactor = store.value(QStringLiteral("SpeedFactor"), settings.simulation.speedFactor).toDouble();
    store.endGroup();

    store.beginGroup(QStringLiteral("UI"));
        settings.units       = stringToUnits(store.value(QStringLiteral("Units"), unitsToString(settings.units)).toString());
        settings.arms_color = store.value("arms_color", settings.arms_color).value<QColor>();
        settings.save_main_window_position_on_exit = store.value(QStringLiteral("save_main_window_position_on_exit"), true).toBool();
        settings.main_window_position = store.value(QStringLiteral("main_window_position"), QRect()).toRect();
        settings.save_measure_window_position_on_exit = store.value(QStringLiteral("save_measure_window_position_on_exit"), true).toBool();
        settings.measure_window_position = store.value(QStringLiteral("measure_window_position"), QRect()).toRect();
        settings.language = store.value(QStringLiteral("language"), settings.language).toString();
        settings.directory_save_dxf = store.value(QStringLiteral("directory_save_dxf"), "c:/").toString();
        settings.directory_save_data = store.value(QStringLiteral("directory_save_data"), "c:/").toString();
    store.endGroup();

    store.beginGroup(QStringLiteral("Shortcuts"));
        if (settings.shortcuts.map.isEmpty())
            settings.shortcuts = Shortcuts::defaults();
        for (auto it = settings.shortcuts.map.begin(); it != settings.shortcuts.map.end(); ++it) {
            const QString key = it.key();
            const QString seq = store.value(key, it.value().toString()).toString();
            it.value() = QKeySequence(seq);
        }
    store.endGroup();

    store.beginGroup(QStringLiteral("Program"));
        settings.auto_step   = store.value(QStringLiteral("auto_step"), settings.auto_step).toDouble();
        settings.alfa_offset = store.value(QStringLiteral("alfa_offset"), settings.alfa_offset).toDouble();
        settings.beta_offset = store.value(QStringLiteral("beta_offset"), settings.beta_offset).toDouble();
        settings.arm1_length = store.value(QStringLiteral("arm1_length"), settings.arm1_length).toDouble();
        settings.arm2_length = store.value(QStringLiteral("arm2_length"), settings.arm2_length).toDouble();
    store.endGroup();
}

void SettingsManager::saveToStore(const Settings& settings, bool admin)
{
    QSettings& store = admin ? adminStore_ : userStore_;

    store.beginGroup(QStringLiteral("Comm"));
        store.setValue(QStringLiteral("DataSource"), dataSourceToString(settings.datasource));
    store.endGroup();

    store.beginGroup(QStringLiteral("Serial"));
        store.setValue(QStringLiteral("PortName"), settings.serial.portName);
        store.setValue(QStringLiteral("Baud"), settings.serial.baudRate);
        store.setValue(QStringLiteral("DataBits"), settings.serial.dataBits);
        store.setValue(QStringLiteral("StopBits"), settings.serial.stopBits);
        store.setValue(QStringLiteral("Parity"), settings.serial.parity);
        store.setValue(QStringLiteral("Flow"), settings.serial.flow);
    store.endGroup();

    store.beginGroup(QStringLiteral("Modbus"));
        store.setValue(QStringLiteral("IsTcp"), settings.modbus.isTcp);
        store.setValue(QStringLiteral("SerialPort"), settings.modbus.serialPort);
        store.setValue(QStringLiteral("Baud"), settings.modbus.baudRate);
        store.setValue(QStringLiteral("Host"), settings.modbus.host);
        store.setValue(QStringLiteral("Port"), settings.modbus.port);
        store.setValue(QStringLiteral("ServerId"), settings.modbus.serverId);
    store.endGroup();

    store.beginGroup(QStringLiteral("Simulation"));
        store.setValue(QStringLiteral("LogFile"), settings.simulation.logFile);
        store.setValue(QStringLiteral("SpeedFactor"), settings.simulation.speedFactor);
    store.endGroup();

    store.beginGroup(QStringLiteral("UI"));
        store.setValue(QStringLiteral("Units"), unitsToString(settings.units));
        store.setValue(QStringLiteral("arms_color"), settings.arms_color);
        store.setValue(QStringLiteral("save_main_window_position_on_exit"), settings.save_main_window_position_on_exit);
        store.setValue(QStringLiteral("main_window_position"), settings.main_window_position);
        store.setValue(QStringLiteral("save_measure_window_position_on_exit"), settings.save_measure_window_position_on_exit);
        store.setValue(QStringLiteral("measure_window_position"), settings.measure_window_position);
        store.setValue(QStringLiteral("language"), settings.language);
        store.setValue(QStringLiteral("directory_save_dxf"), settings.directory_save_dxf);
        store.setValue(QStringLiteral("directory_save_data"), settings.directory_save_data);
    store.endGroup();

    store.beginGroup(QStringLiteral("Shortcuts"));
        for (auto it = settings.shortcuts.map.constBegin(); it != settings.shortcuts.map.constEnd(); ++it)
            store.setValue(it.key(), it.value().toString());
    store.endGroup();

    store.beginGroup(QStringLiteral("Program"));
        store.setValue(QStringLiteral("auto_step"), settings.auto_step);
        store.setValue(QStringLiteral("alfa_offset"), settings.alfa_offset);
        store.setValue(QStringLiteral("beta_offset"), settings.beta_offset);
        store.setValue(QStringLiteral("arm1_length"), settings.arm1_length);
        store.setValue(QStringLiteral("arm2_length"), settings.arm2_length);
    store.endGroup();

    store.sync();
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
    QJsonObject root, comm, serial, modbus, simulation, ui, program;

    // Communication
    comm[QStringLiteral("DataSource")] = dataSourceToString(st.datasource);

    serial[QStringLiteral("PortName")] = st.serial.portName;
    serial[QStringLiteral("Baud")]     = st.serial.baudRate;
    serial[QStringLiteral("DataBits")] = st.serial.dataBits;
    serial[QStringLiteral("StopBits")] = st.serial.stopBits;
    serial[QStringLiteral("Parity")]   = st.serial.parity;
    serial[QStringLiteral("Flow")]     = st.serial.flow;

    modbus[QStringLiteral("IsTcp")]      = st.modbus.isTcp;
    modbus[QStringLiteral("SerialPort")] = st.modbus.serialPort;
    modbus[QStringLiteral("Baud")]       = st.modbus.baudRate;
    modbus[QStringLiteral("Host")]       = st.modbus.host;
    modbus[QStringLiteral("Port")]       = st.modbus.port;
    modbus[QStringLiteral("ServerId")]   = st.modbus.serverId;

    simulation[QStringLiteral("LogFile")]     = st.simulation.logFile;
    simulation[QStringLiteral("SpeedFactor")] = st.simulation.speedFactor;

    ui[QStringLiteral("Units")] = unitsToString(st.units);
    ui[QStringLiteral("ArmsColor")] = st.arms_color.name(QColor::HexArgb);
    ui[QStringLiteral("SaveMainWindowPos")] = st.save_main_window_position_on_exit;
    ui[QStringLiteral("SaveMeasureWindowPos")] = st.save_measure_window_position_on_exit;
    QJsonObject mainRect, measureRect;
    mainRect[QStringLiteral("x")] = st.main_window_position.x();
    mainRect[QStringLiteral("y")] = st.main_window_position.y();
    mainRect[QStringLiteral("w")] = st.main_window_position.width();
    mainRect[QStringLiteral("h")] = st.main_window_position.height();
    measureRect[QStringLiteral("x")] = st.measure_window_position.x();
    measureRect[QStringLiteral("y")] = st.measure_window_position.y();
    measureRect[QStringLiteral("w")] = st.measure_window_position.width();
    measureRect[QStringLiteral("h")] = st.measure_window_position.height();
    ui[QStringLiteral("MainWindow")] = mainRect;
    ui[QStringLiteral("MeasureWindow")] = measureRect;
    ui[QStringLiteral("Language")] = st.language;
    ui[QStringLiteral("DirectorySaveDxf")] = st.directory_save_dxf;
    ui[QStringLiteral("DirectorySaveData")] = st.directory_save_data;

    program[QStringLiteral("Arm1Length")]   = st.arm1_length;
    program[QStringLiteral("Arm2Length")]   = st.arm2_length;
    program[QStringLiteral("AutoStep")]     = st.auto_step;
    program[QStringLiteral("AlfaOffset")]   = st.alfa_offset;
    program[QStringLiteral("BetaOffset")]   = st.beta_offset;
    program[QStringLiteral("DocumentModified")] = st.document_modified;
    program[QStringLiteral("DocumentSaved")]    = st.document_saved;
    program[QStringLiteral("UnitsScale")]  = st.units_scale;

    root[QStringLiteral("Comm")]       = comm;
    root[QStringLiteral("Serial")]     = serial;
    root[QStringLiteral("Modbus")]     = modbus;
    root[QStringLiteral("Simulation")] = simulation;
    root[QStringLiteral("UI")]         = ui;
    root[QStringLiteral("Program")]    = program;
    root[QStringLiteral("Shortcuts")]  = shortcutsToJson(st.shortcuts);

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly)) { if (err) *err = QStringLiteral("Nelze otevřít k zápisu"); return false; }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool SettingsManager::importJson(const QString& filePath, QString* err) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) { if (err) *err = QStringLiteral("Nelze otevřít k čtení"); return false; }

    QJsonParseError jsonErr;
    const auto doc = QJsonDocument::fromJson(f.readAll(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError) {
        if (err) *err = QStringLiteral("Chyba v syntaxi JSON: %1").arg(jsonErr.errorString());
        return false;
    }
    if (!doc.isObject()) { if (err) *err = QStringLiteral("Neplatný JSON"); return false; }

    const QJsonObject root = doc.object();

    if (root.contains(QStringLiteral("Comm"))) {
        const auto o = root.value(QStringLiteral("Comm")).toObject();
        current_.datasource = stringToDataSource(o.value(QStringLiteral("DataSource")).toString(dataSourceToString(current_.datasource)));
    }
    if (root.contains(QStringLiteral("Serial"))) {
        const auto o = root.value(QStringLiteral("Serial")).toObject();
        current_.serial.portName = o.value(QStringLiteral("PortName")).toString(current_.serial.portName);
        current_.serial.baudRate = o.value(QStringLiteral("Baud")).toInt(current_.serial.baudRate);
        current_.serial.dataBits = o.value(QStringLiteral("DataBits")).toInt(current_.serial.dataBits);
        current_.serial.stopBits = o.value(QStringLiteral("StopBits")).toInt(current_.serial.stopBits);
        current_.serial.parity   = o.value(QStringLiteral("Parity")).toInt(current_.serial.parity);
        current_.serial.flow     = o.value(QStringLiteral("Flow")).toInt(current_.serial.flow);
        current_.portName = current_.serial.portName;
        current_.baudRate = current_.serial.baudRate;
    }
    if (root.contains(QStringLiteral("Modbus"))) {
        const auto o = root.value(QStringLiteral("Modbus")).toObject();
        current_.modbus.isTcp      = o.value(QStringLiteral("IsTcp")).toBool(current_.modbus.isTcp);
        current_.modbus.serialPort = o.value(QStringLiteral("SerialPort")).toString(current_.modbus.serialPort);
        current_.modbus.baudRate   = o.value(QStringLiteral("Baud")).toInt(current_.modbus.baudRate);
        current_.modbus.host       = o.value(QStringLiteral("Host")).toString(current_.modbus.host);
        current_.modbus.port       = o.value(QStringLiteral("Port")).toInt(current_.modbus.port);
        current_.modbus.serverId   = o.value(QStringLiteral("ServerId")).toInt(current_.modbus.serverId);
    }
    if (root.contains(QStringLiteral("Simulation"))) {
        const auto o = root.value(QStringLiteral("Simulation")).toObject();
        current_.simulation.logFile     = o.value(QStringLiteral("LogFile")).toString(current_.simulation.logFile);
        current_.simulation.speedFactor = o.value(QStringLiteral("SpeedFactor")).toDouble(current_.simulation.speedFactor);
    }
    if (root.contains(QStringLiteral("UI"))) {
        const auto o = root.value(QStringLiteral("UI")).toObject();
        current_.units = stringToUnits(o.value(QStringLiteral("Units")).toString(unitsToString(current_.units)));
        current_.arms_color = QColor(o.value(QStringLiteral("ArmsColor")).toString(current_.arms_color.name()));
        current_.save_main_window_position_on_exit = o.value(QStringLiteral("SaveMainWindowPos")).toBool(current_.save_main_window_position_on_exit);
        current_.save_measure_window_position_on_exit = o.value(QStringLiteral("SaveMeasureWindowPos")).toBool(current_.save_measure_window_position_on_exit);
        if (o.contains(QStringLiteral("MainWindow"))) {
            const auto r = o.value(QStringLiteral("MainWindow")).toObject();
            current_.main_window_position = QRect(r.value(QStringLiteral("x")).toInt(),
                                                 r.value(QStringLiteral("y")).toInt(),
                                                 r.value(QStringLiteral("w")).toInt(),
                                                 r.value(QStringLiteral("h")).toInt());
        }
        if (o.contains(QStringLiteral("MeasureWindow"))) {
            const auto r = o.value(QStringLiteral("MeasureWindow")).toObject();
            current_.measure_window_position = QRect(r.value(QStringLiteral("x")).toInt(),
                                                    r.value(QStringLiteral("y")).toInt(),
                                                    r.value(QStringLiteral("w")).toInt(),
                                                    r.value(QStringLiteral("h")).toInt());
        }
        current_.language = o.value(QStringLiteral("Language")).toString(current_.language);
        current_.directory_save_dxf = o.value(QStringLiteral("DirectorySaveDxf")).toString(current_.directory_save_dxf);
        current_.directory_save_data = o.value(QStringLiteral("DirectorySaveData")).toString(current_.directory_save_data);
    }
    if (root.contains(QStringLiteral("Program"))) {
        const auto o = root.value(QStringLiteral("Program")).toObject();
        current_.arm1_length   = o.value(QStringLiteral("Arm1Length")).toDouble(current_.arm1_length);
        current_.arm2_length   = o.value(QStringLiteral("Arm2Length")).toDouble(current_.arm2_length);
        current_.auto_step     = o.value(QStringLiteral("AutoStep")).toDouble(current_.auto_step);
        current_.alfa_offset   = o.value(QStringLiteral("AlfaOffset")).toDouble(current_.alfa_offset);
        current_.beta_offset   = o.value(QStringLiteral("BetaOffset")).toDouble(current_.beta_offset);
        current_.document_modified = o.value(QStringLiteral("DocumentModified")).toBool(current_.document_modified);
        current_.document_saved    = o.value(QStringLiteral("DocumentSaved")).toBool(current_.document_saved);
        current_.units_scale   = o.value(QStringLiteral("UnitsScale")).toDouble(current_.units_scale);
    }
    if (root.contains(QStringLiteral("Shortcuts"))) {
        current_.shortcuts = jsonToShortcuts(root.value(QStringLiteral("Shortcuts")).toObject());
    }

    saveToStore(current_);
    emit settingsChanged(current_);
    emit shortcutsChanged(current_.shortcuts);
    return true;
}

