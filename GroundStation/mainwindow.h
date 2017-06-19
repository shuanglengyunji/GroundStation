﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "serialport.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void main_test_slot();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    //类、线程
    QThread n;
    SerialPort m;

    //测试用
    QTimer timer;
};

#endif // MAINWINDOW_H
