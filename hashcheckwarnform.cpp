#include "hashcheckwarnform.h"
#include "ui_hashcheckwarnform.h"

HashCheckWarnForm::HashCheckWarnForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HashCheckWarnForm)
{
    setWindowIcon(QIcon(":/rc/icons/passIcon.ico"));
    ui->setupUi(this);
}

HashCheckWarnForm::~HashCheckWarnForm()
{
    delete ui;
}
