#pragma once

#include <QAction>
#include <QList>
#include <QMainWindow>
#include <QRegExp>
#include <QTabWidget>
#include <QTextDocument>

namespace mote
{
    class DocumentSystem;
    class FindDialog;
    class Settings;
    class TextDocument;
    class TextEdit;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(
            Settings* settings,
            DocumentSystem* documentSystem,
            QWidget* parent = 0 );

    public:
        TextEdit* currentEdit( void )const;
        TextDocument* currentDocument( void )const;
        void saveGeometryAndState( void );
        void restoreGeometryAndState( void );
        QList<TextEdit*> findEdits( TextDocument* textDocument = NULL )const;
        QList<TextEdit*> findEdits( const QString& path )const;
        bool saveFile( TextDocument* textDocument );
        bool saveFileAs( TextDocument* textDocument );
        void activate( TextEdit* textEdit );
        void openFile( const QString& path );

    public slots:
        void openFile( void );
        void saveFile( void );
        void saveFileAs( void );
        void changeFont( void );
        void formatSourceCode( void );
        void closeTab( void );
        void createNewWindow( void );
        void createNewDocument( void );
        void jumpToCoBrace( void );
        void sortAscending( void );
        void deleteDuplicate( void );
        void findText( void );
        void findNext( void );
        void findPrevious( void );
        void replaceText( void );
        void reload( void );

    signals:
        void currentDocumentChanged( TextDocument* textDocument );

    protected:
        virtual void closeEvent( QCloseEvent* event );

    private:
        QString makeTabTitle( const TextDocument* textDocument )const;
        QString makeWindowTitle( const TextDocument* textDocument )const;
        bool _closeTab( const int index = -1 );
        void commitFindDialog( void );
        void createFindDialog( void );

    private slots:
        void updateWindowTitle( TextDocument* textDocument );
        void updateTabTitle( TextDocument* textDocument );
        void onLineNumberVisibilityChanged( bool onoff );
        void onCurrentTabChanged( void );
        void onTabCloseRequested( int index );
        void tagJump( void );
        void onFindTextAccepted( void );
        void jumpToLine( void );

    private:
        Settings* m_settings;
        DocumentSystem* m_documentSystem;
        QTabWidget* m_tabWidget;
        QAction* m_lineNumberAction;
        FindDialog* m_findDialog;
        struct
        {
            bool replaceMode;
            QString text;
            QString after;
            QRegExp regularExpression;
            QTextDocument::FindFlags flags;
            bool regularExpressionEnabled;
            bool highlightAllOccurrences;
        } m_findData;
    };
}
