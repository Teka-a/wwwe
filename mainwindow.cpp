#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/rc/icons/passIcon.ico"));
    setWindowTitle("Password Manager");


    ui->stackedWidget->setCurrentWidget(ui->page);

    ui->pinEntered->setFocus();

    ui->wrongPin->setVisible(false);
    ui->wrongPin->setStyleSheet("QLabel { color: red; }");


    connect(ui->checkPinBtn, &QPushButton::clicked, this, &MainWindow::checkPin);
    connect(ui->filterByHostname, &QPushButton::clicked, this, &MainWindow::filterHosts);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::checkPin()
{
    QString pin = ui->pinEntered->text();

    bool isDecryptedCorrectly = 0;

    QByteArray key;
    QByteArray iv;
    deriveKeyIvFromPin(pin, saltForFile, key, iv);

    this->jsonData = decryptFileAES256CBC(key, iv);
    key.fill(0);
    iv.fill(0);

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(this->jsonData, &jsonError);


    if (jsonError.error == QJsonParseError::NoError) {

        if (parseJsonFile()) {

            printCredsTableWidget(this->jsonCredsArr);

            ui->stackedWidget->setCurrentWidget(ui->page_2);
        } else {
            ui->wrongPin->setText("Ошибка при парсинге!");
            ui->wrongPin->setVisible(true);

        }


    } else {
        ui->wrongPin->setVisible(true);
        ui->pinEntered->setText("");

    }
}


QString MainWindow::decryptLoginPasswordLine(const QString &encryptedData, QByteArray &key, QByteArray &iv) {
    QByteArray encryptedBytes = QByteArray::fromHex(encryptedData.toUtf8());
    QByteArray decryptedData(encryptedBytes.size(), 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Ошибка создания контекста OpenSSL!";
        return QString();
    }

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char*>(key.data()),
                       reinterpret_cast<const unsigned char*>(iv.data()));

    int outLen = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decryptedData.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(encryptedBytes.constData()), encryptedBytes.size());

    int finalLen = 0;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decryptedData.data()) + outLen, &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    decryptedData.resize(outLen + finalLen);

    return QString::fromUtf8(decryptedData);
}


void MainWindow::checkPinToCopy()
{
    QString pin = ui->pinEntered->text();

    bool isDecryptedCorrectly = 0;

    QByteArray key;
    QByteArray iv;
    deriveKeyIvFromPin(pin, saltForFile, key, iv);

    this->jsonData = decryptFileAES256CBC(key, iv);
    key.fill(0);
    iv.fill(0);

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(this->jsonData, &jsonError);


    if (jsonError.error == QJsonParseError::NoError) {
        QPushButton *button = qobject_cast<QPushButton*>(sender());
        if (button) {
            int id = button->property("id").toInt();

            QJsonObject jsonCredElement = this->jsonCredsArr.at(id).toObject();
            QString encInfoLoginPassword = jsonCredElement["login_password"].toString();
            QString salt = jsonCredElement["salt"].toString();


            QByteArray keyInfo;
            QByteArray ivInfo;
            deriveKeyIvFromPin(pin, salt, keyInfo, ivInfo);

            QString decryptedText = decryptLoginPasswordLine(encInfoLoginPassword, keyInfo, ivInfo);
            QStringList parts = decryptedText.split(' ');

            keyInfo.fill(0);
            ivInfo.fill(0);


            int col = button->property("col").toInt();
            QClipboard *clipboard = QApplication::clipboard();

            if (col == 1) {
                clipboard->setText(parts[0]);

            } else {
                clipboard->setText(parts[1]);
            }


            filterHosts();
            ui->stackedWidget->setCurrentWidget(ui->page_2);
             ui->infoLabel->setText("");
            QTime dieTime = QTime::currentTime().addMSecs( 10 );
            while( QTime::currentTime() < dieTime )
            {
                QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
            }
            ui->infoLabel->setText("Значение успешно скопировано");
        }

    } else {
        ui->wrongPin->setVisible(true);
        ui->pinEntered->setText("");
        this->jsonData.fill(0);


    }
}


QByteArray MainWindow::decryptFileAES256CBC(const QByteArray &key, const QByteArray &iv)
{
    QFile inputFile(encCredsFile);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Ошибка открытия файла!";
        return {};
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Ошибка создания контекста OpenSSL!";
        return {};
    }

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char*>(key.data()),
                       reinterpret_cast<const unsigned char*>(iv.data()));

    QByteArray decryptedData;
    QByteArray buffer(4096, 0);
    QByteArray decryptedBuffer(4096 + EVP_CIPHER_block_size(EVP_aes_256_cbc()), 0);

    int outLen = 0;
    while (!inputFile.atEnd()) {
        int bytesRead = inputFile.read(buffer.data(), buffer.size());
        EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decryptedBuffer.data()), &outLen,
                          reinterpret_cast<const unsigned char*>(buffer.constData()), bytesRead);
        decryptedData.append(decryptedBuffer.constData(), outLen);
    }

    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decryptedBuffer.data()), &outLen);
    decryptedData.append(decryptedBuffer.constData(), outLen);

    EVP_CIPHER_CTX_free(ctx);

    return decryptedData;
}


void MainWindow::deriveKeyIvFromPin(QString &pin, QString &salt, QByteArray &key, QByteArray &iv)
{
    QString combinedPinSalt = pin + salt;
    QByteArray infoForHash = combinedPinSalt.toUtf8();
    QByteArray hash = QCryptographicHash::hash(infoForHash, QCryptographicHash::Sha512);

    key = hash.left(32);
    iv = hash.right(16);



    /*combinedPinSalt = pin + "vdfeg9867";
    infoForHash = combinedPinSalt.toUtf8();
    hash = QCryptographicHash::hash(infoForHash, QCryptographicHash::Sha512);

    key = hash.left(32);
    iv = hash.right(16);
    qDebug() << "Key info for site10: ";
    qDebug() << key.toHex();
    qDebug() << iv.toHex();*/

}


bool MainWindow::parseJsonFile()
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(this->jsonData, &jsonError);
    if (!jsonError.error == QJsonParseError::NoError) {
        return false;
    }

    QJsonObject jsonRootObject = jsonDoc.object();
    this->jsonCredsArr = jsonRootObject["credentials"].toArray();

    return true;
}


void MainWindow::copyCellInfoToClipboard(int row, int col)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, col);

    QClipboard *clipboard = QApplication::clipboard();
    QString text = "";
    int id = item->data(Qt::UserRole).toInt();


    if (col == 0) {
        text = item->text();
        clipboard->setText(text);
        ui->infoLabel->setText("");
        QTime dieTime = QTime::currentTime().addMSecs( 10 );
        while( QTime::currentTime() < dieTime )
        {
            QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
        }
        ui->infoLabel->setText("Значение успешно скопировано");

    } else {
        ui->pinEntered->setText("");
        ui->checkPinBtn->setProperty("id", id);
        ui->checkPinBtn->setProperty("col", col);
        ui->stackedWidget->setCurrentWidget(ui->page);

        ui->pinEntered->setFocus();

        ui->wrongPin->setVisible(false);
        connect(ui->checkPinBtn, &QPushButton::clicked, this, &MainWindow::checkPinToCopy);
    }

}



void MainWindow::printCredsTableWidget(QJsonArray &creds)
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);

    QStringList headers = {"Сайт", "Логин", "Пароль"};
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setHorizontalHeaderLabels(headers);


    if (creds.size() == 0) {
        ui->infoLabel->setText("Нет данных, удобвлетворяющих требованиям.");
    } else {
        ui->infoLabel->setText("");
    }

    for (int cred_i = 0; cred_i < creds.size(); cred_i++) {
        QJsonObject jsonCredElement = creds.at(cred_i).toObject();
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        QTableWidgetItem *hostnameItem = new QTableWidgetItem(jsonCredElement["hostname"].toString());
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, hostnameItem);

        QTableWidgetItem *usernameItem = new QTableWidgetItem("****");
        usernameItem->setData(Qt::UserRole, jsonCredElement["id"].toInt() - 1);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, usernameItem);

        QTableWidgetItem *passwordItem = new QTableWidgetItem("****");
        passwordItem->setData(Qt::UserRole,  jsonCredElement["id"].toInt() - 1);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, passwordItem);
    }



    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::copyCellInfoToClipboard);

}


QJsonArray MainWindow::filterCredsByHostname(const QString &host) {
    QJsonArray resultArray;

    for (const QJsonValue &value : this->jsonCredsArr) {
        QJsonObject obj = value.toObject();
        QString hostname = obj["hostname"].toString();

        if (hostname.contains(host, Qt::CaseInsensitive)) {
            resultArray.append(obj);
        }
    }

    return resultArray;
}


void MainWindow::filterHosts()
{

    QString hostnameToFilter = ui->hostnameToFilterBy->text();
    QJsonArray sorted = filterCredsByHostname(hostnameToFilter);

    printCredsTableWidget(sorted);

}
