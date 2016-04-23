#pragma once

#include <QPointer>
#include <QWidgetAction>

#include "textdocument.h"

namespace mote
{
    class DocumentSystem;
    class MainWindow;

    class NewlineCharacterAction : public QWidgetAction
    {
        Q_OBJECT

    public:
        NewlineCharacterAction(
            DocumentSystem* documentSystem,
            MainWindow* mainWindow );

    protected:
        virtual QWidget* createWidget( QWidget* parent );

    private:
        void updateValue( QWidget* w, const QString& newlineCharacter )const;

    private slots:
        void onDocumentNewlineCharacterChanged( TextDocument* textDocument );
        void onNewlineCharacterChanged( int index );
        void setDocument( TextDocument* textDocument );

    private:
        QPointer<TextDocument> m_document;
    };
}
