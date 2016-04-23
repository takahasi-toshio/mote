#pragma once

#include <QStyledItemDelegate>

namespace mote
{
    class InputCompletionItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        InputCompletionItemDelegate( QObject* parent = 0 );

    public:
        virtual void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index )const;
    };
}
