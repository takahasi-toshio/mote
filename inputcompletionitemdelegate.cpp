#include "inputcompletionitemdelegate.h"

#include <QStyleOptionViewItemV4>

namespace mote
{
    InputCompletionItemDelegate::InputCompletionItemDelegate( QObject* parent )
        : QStyledItemDelegate( parent )
    {
    }

    void InputCompletionItemDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index )const
    {
        QStyleOptionViewItemV4 optionCopy = option;
        optionCopy.state |= QStyle::State_Active;
        //optionCopy.palette.setCurrentColorGroup( QPalette::Active );
        return QStyledItemDelegate::paint( painter, optionCopy, index );
    }
}
