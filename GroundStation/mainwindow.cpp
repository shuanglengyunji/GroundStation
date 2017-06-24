﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//***************** 把串口托管给线程 ************************************

    MyCom.moveToThread(&MyComThread);
    MyComThread.start();

    //更新串口列表
    ui->comboBox_PortName->addItems(MyCom.SerialPort_Get_Port_List());

    //串口对外信号连接
    connect(&MyCom,SerialPort::SerialPort_Out_Of_Port_Data_Signals,this,Display_on_DataDisplay_ReceiveBox);

//***************** 把Tcp端口托管给线程 *********************************
    MyTcp.moveToThread(&MyTcpThread);
    MyTcpThread.start();

    connect(&MyTcp,tcp::Tcp_Connect_ok_Signals,this,MainWindow::Tcp_Connect_Ok_Slots);
    connect(&MyTcp,tcp::Tcp_Disconnect_Signals,this,MainWindow::Tcp_Disconnect_Slots);

//***************** 把图像处理函数托管给线程 ******************************

    MyImg.moveToThread(&MyImgThread);
    MyImgThread.start();

    //对外信号连接
    connect(&MyCom,SerialPort::SerialPort_Get_Image_Signals,&MyImg,imagedatamanage::Image_Generate);

    //显示默认文件路径
    ui->lineEdit_filepath->setText(file_path);

    //初始化图像数组并显示图像
    for(int i=0;i<Img_Size;i++)
    {
        imageTmpArray[i] = 0xFF;
    }
    MyImg.Image_Generate();
    DisplayImage();

    //默认不开启图像显示
    connect(&MyImg,imagedatamanage::Image_Ok_Signals,this,MainWindow::DisplayImage);

//***************** 把图像处理函数托管给线程 ******************************

    MyImgSave.moveToThread(&MyImgSaveThread);
    MyImgSaveThread.start();

    connect(&MyImg,imagedatamanage::Image_Ok_Signals,&MyImgSave,ImageSave::Image_Save);

    //计算本地fps
    fps_receive = 0;
    //MyTimer.start(5000);    //5s一次的Timer
    //connect(&MyTimer,QTimer::timeout,this,MainWindow::Timer_Handler);

    connect(&MyCom,SerialPort::SerialPort_Get_Fps_Signals,this,MainWindow::Plane_fps_Dis);

}

MainWindow::~MainWindow()
{
    MyComThread.quit();   //结束串口线程
    MyComThread.wait();   //等待线程完全结束

    MyTcpThread.quit();
    MyTcpThread.wait();

    MyImgThread.quit();
    MyImgThread.wait();

    MyImgSaveThread.quit();
    MyImgSaveThread.wait();

    delete ui;
}

//用timer计算本地fps
void MainWindow::Timer_Handler()
{
    fps_receive = fps_receive / 5.0;

    QString str;
    str.setNum(fps_receive,10,2);
    ui->lineEdit_receivefps->setText(str);
}

//显示飞机端回传的fps
void MainWindow::Plane_fps_Dis(double fps)
{
    QString str;
    str.setNum(fps,10,2);
    ui->lineEdit_planefps->setText(str);
}

//字符串转16进制（处理字符串部分）
void MainWindow::StringToHex(QString str, QByteArray &senddata) //字符串转换为十六进制数据0-F
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;

    for(int i=0; i<len; )
    {
        hstr=str[i].toLatin1();

        //忽略空格
        if(hstr == ' ')
        {
            i++;
            continue;
        }

        //长度计数
        i++;
        if(i >= len)
            break;

        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);

        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;

        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}

//字符串转16进制（ASCII码转换部分）
char MainWindow::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return ch-ch;//不在0-f范围内的会发送成0
}

void MainWindow::on_pushButton_GetPort_clicked()
{
    ui->comboBox_PortName->clear();
    ui->comboBox_PortName->addItems(MyCom.SerialPort_Get_Port_List());
}

void MainWindow::on_pushButton_OpenPort_clicked()
{
    if(ui->pushButton_OpenPort->text() == "打开串口")
    {
        QString PortName = ui->comboBox_PortName->currentText();
        int Baud = ui->comboBox_BaudRate->currentText().toInt();

        if(MyCom.SerialPort_Open(PortName,Baud))
        {
            ui->pushButton_OpenPort->setText("关闭串口");
            ui->comboBox_BaudRate->setEnabled(false);
            ui->comboBox_PortName->setEnabled(false);
            qDebug() << "串口打开成功，串口号：" << PortName << "，波特率：" << Baud;
        }
    }
    else
    {
        if(MyCom.SerialPort_Close())
        {
            ui->pushButton_OpenPort->setText("打开串口");
            ui->comboBox_BaudRate->setEnabled(true);
            ui->comboBox_PortName->setEnabled(true);
            qDebug() << "关闭串口成功";
        }
    }
}

void MainWindow::on_Button_Tcpconnnect_clicked()
{
    if(ui->Button_Tcpconnnect->text() == "连接")
    {
        QString IP = ui->LineEdit_IP->text();
        QString Port = ui->LineEdit_port->text();
        MyTcp.Tcp_Open(IP,Port);
    }
    else if(ui->Button_Tcpconnnect->text() == "断开连接")
    {
        MyTcp.Tcp_Close();
    }
}

void MainWindow::Tcp_Connect_Ok_Slots()
{
    ui->Button_Tcpconnnect->setText("断开连接");
}

void MainWindow::Tcp_Disconnect_Slots()
{
    ui->Button_Tcpconnnect->setText("连接");
}

void MainWindow::on_DataDisplay_Clear_clicked()
{
    ui->DataDisplay_ReceiveBox->clear();
}

void MainWindow::on_DataDisplay_Send_clicked()
{
    QString str = ui->DataDisplay_SendBox->text();//从LineEdit得到字符串
    QByteArray tmp = str.toLatin1();

    MyCom.SerialPort_In_To_Port(tmp);
}

void MainWindow::Display_on_DataDisplay_ReceiveBox(QByteArray data)
{
    if(flag_datatrans)
    {
        int size = data.size();

        QString tmpstr = ui->DataDisplay_ReceiveBox->toPlainText();

        if(tmpstr.length() > 100000)
            tmpstr.clear();

        for(int i=0;i<size;i++)
        {
            unsigned char tmp = data[i];
            //QString str = QString::number(tmp,16).toUpper();
            QString str = QString("%1").arg(tmp&0xFF,2,16,QLatin1Char('0'));    //带自动补0
            tmpstr += str;
            tmpstr += " ";
        }

        ui->DataDisplay_ReceiveBox->clear();
        ui->DataDisplay_ReceiveBox->append(tmpstr);
    }
}

//将数组中的图像显示在屏幕上
void MainWindow::DisplayImage()
{
    fps_receive++;  //fps累加

    if(flag_imagedisplay)
        ui->label_image->setPixmap(QPixmap::fromImage(imgScaled)); //显示变换大小后的QImage对象
}

void MainWindow::on_Button_pathchange_clicked()
{
    QString file_path_tmp = QFileDialog::getExistingDirectory(this,"请选择数据保存文件夹","./");
    if(file_path_tmp.isEmpty())
    {
        return;
    }
    else
    {
        file_path = file_path_tmp;
        ui->lineEdit_filepath->setText(file_path);
    }
}

void MainWindow::on_checkBox_imagesave_stateChanged(int arg1)
{
    if(arg1)
    {
        flag_imagesave = 1;
        //connect(&MyImg,imagedatamanage::Image_Ok_Signals,&MyImgSave,ImageSave::Image_Save);
    }
    else
    {
        flag_imagesave = 0;
        //disconnect(&MyImg,imagedatamanage::Image_Ok_Signals,&MyImgSave,ImageSave::Image_Save);
    }
}

void MainWindow::on_Button_numberclear_clicked()
{
    MyImgSave.image_counter = 0;
}

void MainWindow::on_Button_Closetrans_clicked()
{
    flag_datatrans = 0;
}

void MainWindow::on_Button_Opentrans_clicked()
{
    flag_datatrans = 1;
}

void MainWindow::on_Button_OpenImage_clicked()
{
    flag_imagedisplay = 1;
}

void MainWindow::on_Button_CloseImage_clicked()
{
    flag_imagedisplay = 0;
}

