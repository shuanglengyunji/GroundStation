﻿#ifndef MAIN_H
#define MAIN_H

#define Img_Width       80
#define Img_Height      48
#define Img_Size        Img_Width*Img_Height
#define Img_Buf_Size    Img_Size*3

extern unsigned char imageTmpArray[Img_Size];      //灰度数据临时BUFF
extern unsigned char imageByteArray[Img_Buf_Size]; //生成的RGB888图像
extern QImage DisImage;                            //生成的图像
extern QImage imgScaled;

#endif // MAIN_H