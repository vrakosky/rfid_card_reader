#ifndef IHM_H
#define IHM_H

#include <QMainWindow>
#pragma comment(lib, "ODALID.lib")
#include "ODALID.h"
#include <iostream>
#include <string>
#include <stdio.h>

#define MI_OK 0
#define Auth_KeyA   TRUE
#define Auth_KeyB   FALSE

namespace Ui {
class IHM;
}

class IHM : public QMainWindow
{
    Q_OBJECT

public:
    explicit IHM(QWidget *parent = 0);
    ~IHM();

private slots:
    void on_connectButton_clicked();

    void on_writeBlockButton_clicked();

    void on_quitButton_clicked();

    void on_readBlockButton_clicked();

    void on_Increment_Button_clicked();

    void on_Decrement_Button_clicked();

private:

    void getData(int sector, int block, int display);
    void sendData(int sector, int block, int display);
    void readIncrementDecrementStatus();

    Ui::IHM *ui;
    ReaderName cardReader;
    char version[30];
    uint8_t serial[4];
    char stackReader[20];
    unsigned char dataToRead[240];
    unsigned char dataToWrite[240];

    int16_t status = 0;
    QString text;

    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint8_t write_key_index = 1;
    uint8_t read_key_index = 0;
    uint8_t bloc_count, bloc;
    bool bench = true;
    clock_t t0, t1;

    int sect = 2;

};

#endif // IHM_H
