/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "edge.h"
#include "node.h"

Node::Node():
  minX(-100),
  maxX(+100)
{
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
  nodecolor = QColor(Qt::yellow);
  bPressed  = false;
  centerX   = true;
  centerY   = true;
  fixedX    = false;
  fixedY    = false;
  ballSize = DEFAULT_BALL_SIZE;
}

void Node::addEdge(Edge *edge)
{
  edgeList << edge;
  edge->adjust();
}

void Node::stepToCenter(qreal step)
{
    if(scene() && !bPressed)
    {
        QRectF qr = scene()->sceneRect();
        qreal xStep = qr.width()/step;
        qreal yStep = qr.height()/step;

        QPointF p = pos();
        if(centerX)
        {
            qreal x = qr.right() - qr.width()/2;
            if(fabs(x-p.x())>xStep)
                p.rx() += p.x()>x ? -xStep : xStep;
            else
                p.rx() = x;
        }

        if(centerY)
        {
            qreal y = qr.bottom() - qr.height()/2;
            if(fabs(y-p.y())>yStep)
                p.ry() += p.y()>y ? -yStep : yStep;
            else
                p.ry() = y;
        }
        setPos(p);
    }

}

void Node::setColor(const QColor & color)
{
    nodecolor=color;
}


void Node::setBallSize(int size)
{
    if(size>2) ballSize = size;
}

qreal Node::getX()
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        return 1+((x()-rect.right())*2/rect.width());
    }
    return 0;
}

void Node::setX(qreal newX)
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        QGraphicsObject::setX(rect.left() + (newX+1) * rect.width()/2);
    }
}

qreal Node::getY()
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        return 1+((y()-rect.bottom())*2/rect.height());
    }
    return 0;
}

void Node::setY(qreal newY)
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        QGraphicsObject::setY(rect.top() + (newY+1) * rect.height()/2);
    }
}

QList<Edge *> Node::edges() const
{
    return edgeList;
}

QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    return QRectF(-ballSize/2 - adjust, -ballSize/2 - adjust,
                  ballSize+BALL_HEIGHT + adjust, ballSize+BALL_HEIGHT + adjust);
}


QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-ballSize/2, -ballSize/2, ballSize, ballSize);
    return path;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-ballSize/2 + BALL_HEIGHT, -ballSize/2 + BALL_HEIGHT, ballSize, ballSize);

    QRadialGradient gradient(-BALL_HEIGHT, -BALL_HEIGHT, ballSize/2);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(BALL_HEIGHT, BALL_HEIGHT);
        gradient.setFocalPoint(BALL_HEIGHT, BALL_HEIGHT);
        gradient.setColorAt(1, nodecolor.light(180));
        gradient.setColorAt(0, nodecolor.light(120));
    } else {
        gradient.setColorAt(0, nodecolor);
        gradient.setColorAt(1, nodecolor.dark(120));
    }
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-ballSize/2, -ballSize/2, ballSize, ballSize);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionChange:
        if (scene())
        {
             // value is the new position.
             QPointF newPos = value.toPointF();
             QRectF rect = scene()->sceneRect();
             if(fixedX) newPos.setX(x());//make sure x doesn't change
             if(fixedY) newPos.setY(y());//make sure x doesn't change
             newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));// bound X
             newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));// bound Y

             int newX = -100 + ( (newPos.x()-rect.left()) * 200) / rect.width();
             int newY = 100+(rect.top()-y())*200/rect.height();
             if (newX < minX) newX = minX;
             if (newX > maxX) newX = maxX;

             if (!getFixedX()) {           
               newPos.setX(((newX+100)*rect.width()/200+rect.left()));
             }
             
             emit moved(newX, newY);
             return newPos;
         }
        break;
    case ItemPositionHasChanged:
        foreach (Edge *edge, edgeList)
          edge->adjust();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    bPressed = true;
    QGraphicsItem::mousePressEvent(event);
    emit focus();
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    bPressed = false;
    if (scene()) {
      QGraphicsItem::mouseReleaseEvent(event);
      emit unfocus();
    }
    else {
      QGraphicsItem::mouseReleaseEvent(event);
    }
}
