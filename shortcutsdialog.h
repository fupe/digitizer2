#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>
#include "settings.h"                 // kvůli Shortcuts

namespace Ui { class ShortCutsDialog; }

class ShortCutsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ShortCutsDialog(QWidget* parent = nullptr);
    ~ShortCutsDialog();

    void setShortcuts(const Shortcuts& s);
    Shortcuts shortcuts() const;

protected:
    void accept() override;           // sebere data z UI a zavře

private:
    void populate();
    void pullFromUi();
    void resetToDefaults();

    Ui::ShortCutsDialog* ui = nullptr;
    Shortcuts tmp_;
};

#endif // SHORTCUTSDIALOG_H
