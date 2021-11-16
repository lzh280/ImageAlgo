#include "QStateProgressBar.h"
#include <QPainter>
#include <QDebug>

QStateProgressBar::QStateProgressBar(QWidget *parent)
    : QWidget(parent)
{
    m_value = 0;

    m_bigRadius = 18;
    m_smallRadius = 14;

    m_lineHeight = 4;
    m_lineWidth = SPACE;
    m_innerLineHeight = m_lineHeight - 2;

    m_tip = "";

    m_barColor = QColor(255,104,124);
    m_barBackgroundColor = QColor(100,100,100);
    m_stateColor = QColor(255,104,124);
    m_tipColor = QColor(255,255,255);
    m_tipBackgroundColor = QColor(24,189,155);
}

void QStateProgressBar::setValue(int value)
{
    if(value < 0 && value > m_states.count()) return;
    m_value = value;
    update();
}

void QStateProgressBar::setTip(QString tip)
{
    m_tip = tip;
    update();
}

void QStateProgressBar::setStates(QStringList states)
{
    m_states = states;
    update();
}

void QStateProgressBar::setBarColor(QColor color)
{
    m_barColor = color;
    update();
}

void QStateProgressBar::setBarBackgroundColor(QColor color)
{
    m_barBackgroundColor = color;
    update();
}

void QStateProgressBar::setStateColor(QColor color)
{
    m_stateColor = color;
    update();
}

void QStateProgressBar::setTipColor(QColor color)
{
    m_tipColor = color;
    update();
}

void QStateProgressBar::setTipBackgroundColor(QColor color)
{
    m_tipBackgroundColor = color;
    update();
}

void QStateProgressBar::drawBackground(QPainter *painter)
{
    painter->save();

    painter->setBrush(QColor(241,241,241));
    painter->setPen(Qt::NoPen);
    painter->drawRoundRect(rect(), 0, 0);
    painter->restore();
}

void QStateProgressBar::drawBarBackground(QPainter *painter)
{
    for (int i = 0; i < m_states.count(); i++)
    {
        painter->save();
        painter->setPen(m_barBackgroundColor);
        painter->setBrush(m_barBackgroundColor);
        QPointF pot(LEFT_MARGIN + i*m_lineWidth, height()/2);
        painter->drawEllipse(pot, m_bigRadius, m_bigRadius);
        painter->restore();

        if (i > 0)
        {
            painter->save();
            painter->setPen(m_barBackgroundColor);
            painter->setBrush(m_barBackgroundColor);
            painter->drawRect(LEFT_MARGIN + (i-1)*(m_lineWidth),
                              height()/2 - m_lineHeight/2, m_lineWidth, m_lineHeight);
            painter->restore();
        }

        painter->save();
        painter->setPen(m_barBackgroundColor);
        qreal textWidth = fontMetrics().width(m_states[i]);
        qreal textHeight = fontMetrics().height();
        painter->drawText(QRectF(LEFT_MARGIN - textWidth*3/4 + i*(m_lineWidth),
                                 height()/2 + m_bigRadius + textHeight/2, textWidth*3/2, textHeight*3/2),
                          Qt::AlignCenter, m_states[i]);
        painter->restore();
    }
}

void QStateProgressBar::drawBar(QPainter *painter)
{
    if (m_value > m_states.count() || m_value < 1){
        return;
    }

    QPointF pt;
    QFont ft;
    for (int i = 0; i < m_value; i++)
    {
        painter->save();
        painter->setPen(m_barColor);
        painter->setBrush(m_barColor);
        pt = QPointF(LEFT_MARGIN + i*(m_lineWidth), height()/2);
        painter->drawEllipse(pt, m_smallRadius, m_smallRadius);
        painter->restore();

        painter->save();
        painter->setPen(m_barColor);
        painter->setBrush(m_barColor);

        if(i == m_value-1){
            if(m_value != m_states.count()){
                painter->drawRect(LEFT_MARGIN + i*m_lineWidth, height()/2 - m_innerLineHeight/2,  m_lineWidth/2, m_innerLineHeight);
            }else{

            }
        }else{
            painter->drawRect(LEFT_MARGIN + i*m_lineWidth, height()/2 - m_innerLineHeight/2,  m_lineWidth, m_innerLineHeight);
        }
        painter->restore();

        painter->save();
        painter->setPen(m_stateColor);
        painter->setFont(ft);
        qreal textWidth = fontMetrics().width(m_states[i]);
        qreal textHeight = fontMetrics().height();

        painter->drawText(QRectF(LEFT_MARGIN - textWidth*3/4 + i*(m_lineWidth),
                                 height()/2 + m_bigRadius + textHeight/2, textWidth*3/2, textHeight*3/2),
                          Qt::AlignCenter, m_states[i]);
        painter->restore();
    }

    painter->save();
    painter->setPen(m_tipBackgroundColor);
    painter->setBrush(m_tipBackgroundColor);
    QRect rect(LEFT_MARGIN - 78/2 + (m_value - 1)*(m_lineWidth), height()/2 - 60,78,25);
    painter->drawRoundedRect(rect, 10,10);
    painter->restore();

    painter->save();
    painter->setPen(m_tipBackgroundColor);
    painter->setBrush(m_tipBackgroundColor);
    QPointF firstPt(LEFT_MARGIN + (m_value - 1)*(m_lineWidth) - 10, height()/2 - 60 + 25);
    QPointF secondPt(LEFT_MARGIN + 20 + (m_value - 1)*(m_lineWidth) - 10, height()/2 - 60 + 25);
    QPointF thiirdPt(LEFT_MARGIN + 10 + (m_value - 1)*(m_lineWidth) - 10, height()/2 - 60 + 25 + 10);
    QPolygonF polygon;
    polygon << firstPt << secondPt << thiirdPt;
    painter->drawPolygon(polygon);
    painter->restore();

    painter->save();
    painter->setPen(m_tipColor);
    ft.setPointSize(9);
    painter->setFont(ft);
    qreal textWidth2 = fontMetrics().width(m_tip);
    qreal textHeight2 = fontMetrics().height();
    QPointF textPt(LEFT_MARGIN - textWidth2/2 - 1 + (m_value - 1)*(m_lineWidth),
                   height()/2 - 60 + 25/2+ textHeight2/2);
    painter->drawText(textPt, m_tip);
    painter->restore();
}

void QStateProgressBar::drawBarNumber(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::white);

    for (int i = 0; i < m_states.count(); i++)
    {
        QString str = QString("%1").arg(i+1);

        qreal textWidth = fontMetrics().width(str);
        qreal textHeight = fontMetrics().height();

        QFont ft;
        ft.setPointSize(12);
        painter->setFont(ft);
        QPointF pt(LEFT_MARGIN - textWidth/2 + i*m_lineWidth, height()/2 + textHeight/2);
        painter->drawText(pt, str);
    }

    painter->restore();
}

void QStateProgressBar::adjustWidth()
{
    if(m_states.count() == 0) return;
    if(width() > SPACE * (m_states.count() - 1) + LEFT_MARGIN*2){
        m_lineWidth = (width() - LEFT_MARGIN*2) / (m_states.count() -1);
    }else{
        m_lineWidth = SPACE;
    }
}

void QStateProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    adjustWidth();

    drawBackground(&painter);
    drawBarBackground(&painter);
    drawBar(&painter);
    drawBarNumber(&painter);
}
