#ifndef STATEPROGRESSBAR_H
#define STATEPROGRESSBAR_H

#include <QWidget>

#define LEFT_MARGIN     40
#define SPACE           80

class QStateProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit QStateProgressBar(QWidget *parent = nullptr);

    void setValue(int value);
    int getValue() const {
        return m_value;
    }
    void setTip(QString tip);
    QString getTip() const {
        return m_tip;
    }

    void setStates(QStringList states);
    QStringList getStates() const {
        return m_states;
    }

    void setBarColor(QColor color);
    void setBarBackgroundColor(QColor color);
    void setStateColor(QColor color);
    void setTipColor(QColor color);
    void setTipBackgroundColor(QColor color);

private:
    void drawBackground(QPainter* painter);
    void drawBarBackground(QPainter* painter);
    void drawBar(QPainter* painter);
    void drawBarNumber(QPainter* painter);
    void adjustWidth();

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_value;
    int m_bigRadius;
    int m_smallRadius;

    int m_lineHeight;
    int m_lineWidth;
    int m_innerLineHeight;

    QStringList m_states;

    QString m_tip;

    QColor m_barColor;
    QColor m_barBackgroundColor;
    QColor m_stateColor;
    QColor m_tipColor;
    QColor m_tipBackgroundColor;

};

#endif // STATEPROGRESSBAR_H
