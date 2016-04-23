#pragma once

#include <QPointer>
#include <QWidgetAction>

#include "textdocument.h"

namespace mote
{
    class DocumentSystem;
    class MainWindow;

    class TextCodecAction : public QWidgetAction
    {
        Q_OBJECT

    public:
        TextCodecAction(
            DocumentSystem* documentSystem,
            MainWindow* mainWindow );

    protected:
        virtual QWidget* createWidget( QWidget* parent );

    private:
        void updateValue( QWidget* w, QTextCodec* codec )const;

    private slots:
        void onDocumentTextCodecChanged( TextDocument* textDocument );
        void onTextCodecChanged( const QString& text );
        void setDocument( TextDocument* textDocument );

    private:
        QPointer<TextDocument> m_document;
    };
}
