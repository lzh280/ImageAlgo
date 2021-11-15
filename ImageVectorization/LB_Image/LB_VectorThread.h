#ifndef LB_VECTORTHREAD_H
#define LB_VECTORTHREAD_H

#include <QThread>
#include <QImage>

class LB_VectorThread : public QThread
{
    Q_OBJECT
public:
    LB_VectorThread(QObject* parent = nullptr);

    void SetVectorImage(const QImage& img, bool useDP = false) {
        myImage = img;
        myDPSimplify = useDP;
    }

protected:
    void run() override;

private:
    QImage myImage;
    bool myDPSimplify;

signals:
    void ComputeFinish(const QVector<QPolygonF>& result);
};

#endif // LB_VECTORTHREAD_H
