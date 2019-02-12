#include "ihm.h"
#include "ui_ihm.h"
#include <sstream>
#include <time.h>
#include <QtGui>

char hostAdress[] = "192.168.1.4";
unsigned char key_ff[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
unsigned char key1[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
unsigned char key2[6] = { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5 };
unsigned char key3[6] = { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5 };
unsigned char key4[6] = { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5 };

IHM::IHM(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IHM)
{
    ui->setupUi(this);
}

IHM::~IHM()
{
    delete ui;
}

void IHM::on_connectButton_clicked()
{
    uint32_t value = 0;
    status = CloseCOM1(&cardReader);
    cardReader.Type = ReaderCDC;
    cardReader.device = 0;
    status = OpenCOM1(&cardReader);
    status = LEDBuzzer(&cardReader, LED_RED_ON);
    //status = LEDBuzzer(&cardReader, BUZZER_ON);
    RF_Power_Control(&cardReader, TRUE, 2);

    status = ISO14443_3_A_PollCard(&cardReader, atq, sak, uid, &uid_len);
    if(status == MI_OK) {
        std::cout << "Connection successful. Card data sent code " << status << std::endl;
        status = LEDBuzzer(&cardReader, LED_GREEN_ON);
        RF_Power_Control(&cardReader, TRUE, 2);

        status = Version(&cardReader, version, serial, stackReader);
        this->ui->statusDisplay->setText(version);
        this->ui->statusDisplay->update();

        status = Mf_Classic_LoadKey(&cardReader, Auth_KeyA, key1, 2);
        std::cout << status << std::endl;
        status = Mf_Classic_LoadKey(&cardReader, Auth_KeyB, key2, 2);
        std::cout << status << std::endl;
        status = Mf_Classic_LoadKey(&cardReader, Auth_KeyA, key3, 3);
        std::cout << status << std::endl;
        status = Mf_Classic_LoadKey(&cardReader, Auth_KeyB, key4, 3);
        std::cout << status << std::endl;

        status = Mf_Classic_Read_Block(&cardReader, TRUE, 9, dataToRead, Auth_KeyA, 2);
        std::cout << status << std::endl;
        status = Mf_Classic_Read_Block(&cardReader, TRUE, 10, dataToRead, Auth_KeyA, 2);
        std::cout << status << std::endl;

        status = Mf_Classic_Read_Value(&cardReader, TRUE, 13, &value, Auth_KeyA, 3);
        std::cout << status << std::endl;
        status = Mf_Classic_Read_Value(&cardReader, TRUE, 14, &value, Auth_KeyA, 3);
        std::cout << status << std::endl;
    } else {
        this->ui->statusDisplay->setText("Error: no card detected.");
        this->ui->statusDisplay->update();
    }

}

void IHM::on_writeBlockButton_clicked()
{
    sendData(sect, 8, 0);
    sendData(sect, 9, 1);
    sendData(sect, 10, 2);
}

void IHM::on_quitButton_clicked()
{
    status = 0;
    RF_Power_Control(&cardReader, FALSE, 0);
    status = LEDBuzzer(&cardReader, LED_OFF);
    status = CloseCOM1(&cardReader);
    qApp->quit();
}

void IHM::on_readBlockButton_clicked()
{
    getData(sect, 8, 0);
    getData(sect, 9, 1);
    getData(sect, 10,2);
}

void IHM::getData(int sector, int block, int display)
{
    status = Mf_Classic_Read_Block(&cardReader, TRUE, block, dataToRead, Auth_KeyA, 2);
    if(status != MI_OK) {
        this->ui->textSend->setText(this->ui->textSend->toPlainText()+"\nUnable to read block. "+GetErrorMessage(status));
        this->ui->textSend->update();
    }
    else
    {
        QString result = "";
        for(int i = 0; i < 16; i++) result += (QString) dataToRead[i];

        switch(display)
        {
            case 0: this->ui->statusDisplay->setText("Nom de l'application : "+result);
                    this->ui->statusDisplay->update();
            break;
            case 1: this->ui->nameBox->setText("Nom : "+result);
                    this->ui->nameBox->update();
            break;
            case 2: this->ui->surnameBox->setText("Prénom : "+result);
                    this->ui->surnameBox->update();
            break;
        }
    }
}

void IHM::sendData(int sector, int block, int display)
{
    //Clear sector 2 - Block block
    for(int i = 0; i<16; i++) dataToWrite[i] = ' ';
    status = Mf_Classic_Write_Block(&cardReader, TRUE, block, dataToWrite, Auth_KeyB, 2);

    std::string toWrite;

    switch(display)
    {
        case 0: toWrite = this->ui->statusDisplay->toPlainText().toStdString();
        break;
        case 1: toWrite = this->ui->nameBox->toPlainText().toStdString();
        break;
        case 2: toWrite = this->ui->surnameBox->toPlainText().toStdString();
        break;
    }

    strcpy( (char *) dataToWrite, toWrite.c_str() );

    status = Mf_Classic_Write_Block(&cardReader, TRUE, block, dataToWrite, Auth_KeyB, sector);
    if(status != MI_OK) {
        this->ui->textSend->setText(this->ui->textSend->toPlainText()+"\nUnable to write block. "+GetErrorMessage(status));
        this->ui->textSend->update();
    }
}


void IHM::readIncrementDecrementStatus(){
    //Clear sector 3 - Block 12
    char tabToClear[16];
    sprintf(tabToClear, "                ",16);
    status = Mf_Classic_Write_Block(&cardReader, TRUE, 12, dataToWrite, Auth_KeyB, 3);

    //writing the increment-decrement status
    char statusDisplay[16];
    sprintf(statusDisplay, "Porte Monnaie",16);
    status = Mf_Classic_Write_Block(&cardReader, TRUE, 12, (uint8_t *)statusDisplay, Auth_KeyB, 3);
    status = Mf_Classic_Read_Block(&cardReader, TRUE, 12, dataToRead, Auth_KeyA, 3);
    QString result = "";
    for(int i = 0; i < 16; i++) result += (QString) dataToRead[i];
    this->ui->statusDisplay->setText("Nom de l'application : "+result);
    this->ui->statusDisplay->update();
}

void IHM::on_Increment_Button_clicked()
{
    readIncrementDecrementStatus();
    uint32_t value =0;

    status = Mf_Classic_Increment_Value(&cardReader, TRUE, 14, 1, 13, Auth_KeyB, 3);
    status = Mf_Classic_Restore_Value(&cardReader, TRUE, 13, 14, Auth_KeyB, 3);

    status = Mf_Classic_Read_Value(&cardReader, TRUE, 13, &value, Auth_KeyA, 3);
    std::cout << status << std::endl;
    status = Mf_Classic_Read_Value(&cardReader, TRUE, 14, &value, Auth_KeyA, 3);
    std::cout << status << std::endl;

    QString displayValue = QString::number(value);

    this->ui->textSend->setText(this->ui->textSend->toPlainText()+"Increment :  "+ displayValue+ "\n");
    this->ui->textSend->update();
    this->ui->creditLabel->setText(displayValue);
    this->ui->creditLabel->update();
}

void IHM::on_Decrement_Button_clicked()
{
    readIncrementDecrementStatus();
    uint32_t  value = 0;
    status = Mf_Classic_Decrement_Value(&cardReader, TRUE, 14, 1, 13, Auth_KeyA, 3);

    status = Mf_Classic_Restore_Value(&cardReader, TRUE, 13, 14, Auth_KeyA, 3);

    status = Mf_Classic_Read_Value(&cardReader, TRUE, 13, &value, Auth_KeyA, 3);
    std::cout << status << std::endl;
    status = Mf_Classic_Read_Value(&cardReader, TRUE, 14, &value, Auth_KeyA, 3);
    std::cout << status << std::endl;

    QString displayValue = QString::number(value);

    this->ui->textSend->setText(this->ui->textSend->toPlainText()+"Decrement :  "+ displayValue+ "\n");
    this->ui->textSend->update();
    this->ui->creditLabel->setText(displayValue);
    this->ui->creditLabel->update();
}
