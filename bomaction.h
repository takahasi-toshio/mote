#pragma once

#include <QPointer>
#include <QWidgetAction>

#include "textdocument.h"

namespace mote
{
    class DocumentSystem;
    class MainWindow;

    class BOMAction : public QWidgetAction
    {
        Q_OBJECT

    public:
        BOMAction(
            DocumentSystem* documentSystem,
            MainWindow* mainWindow );

    protected:
        virtual QWidget* createWidget( QWidget* parent );

    private:
        void updateValue(
            QWidget* w,
            QTextCodec* codec,
            const bool generateByteOrderMark )const;

    private slots:
        void onDocumentValueChanged( TextDocument* textDocument );
        void onBOMChanged( bool onoff );
        void setDocument( TextDocument* textDocument );

    private:
        QPointer<TextDocument> m_document;
    };
}
