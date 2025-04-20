#include "mainwindow.h"
#include "hashcheckwarnform.h"

#include <QApplication>

#include <QDebug>
#include <Windows.h>
#include <QMessageBox>
#include <QByteArray>
#include <QCryptographicHash>

typedef unsigned __int64 QWORD;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1) определить, где в памяти начало сегмента .text
    QWORD imageBase = (QWORD)GetModuleHandle(NULL);
    QWORD baseOfCode = 0x1000;
    QWORD textBase = imageBase + baseOfCode;

    // 2) определить, какой он длины
    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBase);
    PIMAGE_NT_HEADERS peHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
        imageBase + dosHeader->e_lfanew);
    QWORD textSize = peHeader->OptionalHeader.SizeOfCode;

    // 3) от бинарного блока в диапазоне textBase...(textBase+textSize) посчитать хеш
    QByteArray textSegmentContents = QByteArray((char*)textBase, textSize);
    QByteArray calculatedTextHash = QCryptographicHash::hash(
        textSegmentContents, QCryptographicHash::Sha256);
    QByteArray calculatedTextHashBase64 = calculatedTextHash.toBase64();

    // 4) сравнить полученный хеш с заранее рассчитанным
    const QByteArray referenceTextHashBase64 =
        QByteArray("bk44JLQ4XPCSp5cRJTmDGaMyOAsentSmaVF5e+AVcfs=");

    qDebug() << "textBase = " << Qt::hex << textBase;
    qDebug() << "textSize = " << textSize;
    qDebug() << "textSegmentContents = " << Qt::hex << textSegmentContents.first(100);
    qDebug() << "calculatedTextHashBase64 = " << calculatedTextHashBase64;

    bool checkresult = (calculatedTextHashBase64==referenceTextHashBase64);
    qDebug() << "checkresult = " << (checkresult != true);
    // Проверка наличия отладчика
    if (IsDebuggerPresent()) {
        qDebug() << "Обнаружен отладчик";
    }


    MainWindow w;
    HashCheckWarnForm warningForm;
    if (checkresult != true) {
        warningForm.show();
    } else {
        w.show();
    }
    return a.exec();
}
