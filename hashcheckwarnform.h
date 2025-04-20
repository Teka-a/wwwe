#ifndef HASHCHECKWARNFORM_H
#define HASHCHECKWARNFORM_H

#include <QWidget>

namespace Ui {
class HashCheckWarnForm;
}

class HashCheckWarnForm : public QWidget
{
    Q_OBJECT

public:
    explicit HashCheckWarnForm(QWidget *parent = nullptr);
    ~HashCheckWarnForm();

private:
    Ui::HashCheckWarnForm *ui;
};

#endif // HASHCHECKWARNFORM_H
