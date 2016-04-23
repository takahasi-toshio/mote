#pragma once

#include <QDialog>
#include <QTreeWidget>

namespace mote
{
    class CTags;
    class Settings;

    class TagJumpDialog : public QDialog
    {
        Q_OBJECT

    public:
        TagJumpDialog(
            const CTags& ctags,
            const int lineNumber,
            QWidget* parent = 0 );

    public:
        virtual QSize sizeHint()const;
        virtual void accept();

    public:
        void saveSize( Settings* settings )const;
        void restoreSize( Settings* settings );
        int selectedEntry( void )const;

    private:
        QSize m_preferredSize;
        QTreeWidget* m_tree;
        int m_selectedEntry;
    };
}
