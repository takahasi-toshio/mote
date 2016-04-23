#include "tagjumpdialog.h"

#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "ctags.h"
#include "settings.h"

namespace mote
{
    TagJumpDialog::TagJumpDialog(
        const CTags& ctags,
        const int lineNumber,
        QWidget* parent )
        : QDialog( parent ),
          m_selectedEntry( -1 )
    {
        setWindowTitle( tr( "Tag Jump" ) );

        m_tree = new QTreeWidget;
        m_tree->setRootIsDecorated( false );
        m_tree->setColumnCount( 4 );
        m_tree->setHeaderLabels(
            QStringList() << tr( "Tag Name" ) << tr( "Scope" ) << tr( "Kind" ) << tr( "Line Numer" ) );

        QTreeWidgetItem* currentItem = NULL;
        for( int i = 0; i < ctags.count(); ++i )
        {
            const CTags::Entry& entry = ctags.entry( i );
            QTreeWidgetItem* item = new QTreeWidgetItem( m_tree );
            item->setText( 0, entry.tagName );
            item->setData( 0, Qt::UserRole, i );
            item->setText( 1, entry.scope() );
            item->setText( 2, entry.kind() );
            item->setText( 3, entry.exCmd );
            if( !currentItem || ( entry.exCmd.toInt() <= lineNumber ) )
            {
                currentItem = item;
            }
        }
        m_tree->resizeColumnToContents( 0 );
        m_tree->resizeColumnToContents( 1 );
        m_tree->resizeColumnToContents( 2 );
        if( currentItem )
        {
            m_tree->setCurrentItem( currentItem );
        }

        QDialogButtonBox* buttonBox =
            new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

        connect(
            m_tree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
            SLOT( accept() ) );

        connect( buttonBox, SIGNAL( accepted() ), SLOT( accept() ) );
        connect( buttonBox, SIGNAL( rejected() ), SLOT( reject() ) );

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget( m_tree );
        layout->addWidget( buttonBox );

        setLayout( layout );
    }

    QSize TagJumpDialog::sizeHint()const
    {
        return m_preferredSize.isValid() ? m_preferredSize : QDialog::sizeHint();
    }

    void TagJumpDialog::accept()
    {
        const QTreeWidgetItem* item = m_tree->currentItem();
        if( !item )
        {
            return;
        }

        m_selectedEntry = item->data( 0, Qt::UserRole ).toInt();

        QDialog::accept();
    }

    void TagJumpDialog::saveSize( Settings* settings )const
    {
        settings->setValue( "tagJumpDialogWidth", width() );
        settings->setValue( "tagJumpDialogHeight", height() );
    }

    void TagJumpDialog::restoreSize( Settings* settings )
    {
        const QVariant width = settings->value( "tagJumpDialogWidth" );
        const QVariant height = settings->value( "tagJumpDialogHeight" );
        if( width.isValid() && height.isValid() )
        {
            m_preferredSize.setWidth( width.toInt() );
            m_preferredSize.setHeight( height.toInt() );
        }
    }

    int TagJumpDialog::selectedEntry( void )const
    {
        return m_selectedEntry;
    }
}
