#pragma once
#include <QObject>
#include <QSettings>
#include <QJsonObject>
#include "settings.h"

class SettingsManager : public QObject {
    Q_OBJECT
public:
    explicit SettingsManager(QObject* parent = nullptr);

    // READ: bezpečně vrací const-ref na aktuální konfiguraci
    const Settings& currentSettings() const { return current_; }

    // WRITE: provede porovnání, uložení do QSettings a emisi signálů pouze při změně
    void updateSettings(const Settings& s, bool admin = false);


    // convenient setters (optional)
    void setUnits(Units u, bool admin = false);
    void setSerial(const QString& port, int baud, bool admin = false);
    void setDatasource(const DataSource data, bool admin = false);

    // export/import JSON (optional)
    bool exportJson(const QString& filePath, QString* err = nullptr) const;
    bool importJson(const QString& filePath, QString* err = nullptr);

signals:
    void settingsChanged(const Settings& s);
    void shortcutsChanged(const Shortcuts& sc);

private:
    Settings  current_;   // hodnotový typ, žádné pointry
    QSettings userStore_; // NativeFormat/UserScope  (HKCU\Software\<Org>\<App>)
    QSettings adminStore_; // NativeFormat/SystemScope (HKLM\Software\<Org>\<App>)

    Settings loadFromStore();
    void     saveToStore(const Settings& st, bool admin = false);
    void     loadFrom(QSettings& store, Settings& st);

    static QJsonObject shortcutsToJson(const Shortcuts& sc);
    static Shortcuts   jsonToShortcuts(const QJsonObject& obj);

    static QString replaceEnvVars(const QString& path);
    static QString expandEnvVars(const QString& path);
};
