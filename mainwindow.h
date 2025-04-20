#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTimer>

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QClipboard>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <QByteArray>
#include <QCryptographicHash>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void checkPin();
    void checkPinToCopy();
    void copyCellInfoToClipboard(int row, int col);
    void filterHosts();

private:
    Ui::MainWindow *ui;
    QByteArray jsonData;
    QJsonArray jsonCredsArr;

    QString encCredsFile = "../../creds.enc";

    QString saltForFile = "p[afobjhnsdfsos8936";

    bool parseJsonFile();


    void printCredsTableWidget(QJsonArray &creds);

    QJsonArray filterCredsByHostname(const QString &host);


    QByteArray decryptFileAES256CBC(const QByteArray &key, const QByteArray &iv);
    void deriveKeyIvFromPin(QString &pin, QString &salt, QByteArray &key, QByteArray &iv);


    QString decryptLoginPasswordLine(const QString &encryptedData, QByteArray &key, QByteArray &iv);


};
#endif // MAINWINDOW_H
