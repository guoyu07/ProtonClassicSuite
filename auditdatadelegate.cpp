#include "auditdatadelegate.h"
#include <QtGui>

auditDataDelegate::auditDataDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void auditDataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /*QString text=index.data().toString();
        painter->save();
        painter->setPen(QColor(255,0,0));
        //Years = column 2 in the table model (0 = id, 1=node_id)
        if(index.column()==2)
        {
            painter->setBrush(QColor(255,0,0));
        }
        else
        {
            painter->setBrush(QColor(0,255,0));
        }
       // painter->drawRect(option.rect);

        painter->drawText(option.rect, Qt::AlignJustify | Qt::AlignVCenter | Qt::TextWordWrap, text);
        painter->restore();
*/
    QStyledItemDelegate::paint(painter,option,index);

}

QWidget *auditDataDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column()==2)
    {
        return 0;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void auditDataDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void auditDataDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);

}
