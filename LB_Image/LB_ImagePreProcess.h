#ifndef LB_IMAGEPROCESS_H
#define LB_IMAGEPROCESS_H

#include <QImage>

namespace LB_Image
{

//! templet of dealing with image with a kernel
template<typename T>
QImage Convolution(const QImage &img, T *kernel[], const int &kernelSize);

//! auto get the threshold to binary the image
int ThresholdDetect(const QImage &img);

//! auto get the high threshold of Canny method
void CannyThresholdDetec(const QImage &img, int &ThL, int &ThH);

//! scale the image with sepecific factor
QImage Scale(const QImage &img, double factor);

//! changed into gray-scale images, calculate
//! the threshold of binarization
QImage Gray(const QImage &img);

//! Binarization of image
QImage Binary(const QImage &img, int threshold = -1);

//! median filtering
QImage MedianFilter(const QImage &img, const int &winSize);

//! Gauss filter
QImage GaussFilter(const QImage &img);

//! sharpen the image
QImage Sharpen(const QImage &img);

//! find contours
QImage SobelContours(const QImage &img);

QImage CannyContours(const QImage &img);

QImage FindContours(const QImage &img);

//! thinnig
QImage Thinning(const QImage &img);

};

#endif // LB_IMAGEPROCESS_H
