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
    void updateSettings(const Settings& s);


    // convenient setters (optional)
    void setUnits(Units u);
    void setSerial(const QString& port, int baud);
    void setDatasource(const DataSource data);

    // export/import JSON (optional)
    bool exportJson(const QString& filePath, QString* err = nullptr) const;
    bool importJson(const QString& filePath, QString* err = nullptr);

signals:
    void settingsChanged(const Settings& s);
    void shortcutsChanged(const Shortcuts& sc);

private:
    Settings  current_;   // hodnotový typ, žádné pointry
    QSettings store_;     // NativeFormat/UserScope (Windows: HKCU\Software\<Org>\<App>)

    Settings loadFromStore();
    void     saveToStore(const Settings& st) ;

    static QJsonObject shortcutsToJson(const Shortcuts& sc);
    static Shortcuts   jsonToShortcuts(const QJsonObject& obj);
};
