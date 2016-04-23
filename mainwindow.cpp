#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollBar>
#include <QSet>
#include <QShortcut>
#include <QTextBlock>
#include <QToolBar>
#include <QVector>

#include "bomaction.h"
#include "ctags.h"
#include "documentsystem.h"
#include "finddialog.h"
#include "newlinecharacteraction.h"
#include "settings.h"
#include "tagjumpdialog.h"
#include "textcodecaction.h"
#include "textdocument.h"
#include "textedit.h"

namespace mote
{
    MainWindow::MainWindow(
        Settings* settings,
        DocumentSystem* documentSystem,
        QWidget* parent )
        : QMainWindow( parent ),
          m_settings( settings ),
          m_documentSystem( documentSystem ),
          m_findDialog( NULL )
    {
        TextDocument* textDocument = documentSystem->createDocument();

        TextEdit* textEdit = new TextEdit( textDocument );

        m_tabWidget = new QTabWidget;
        m_tabWidget->setTabsClosable( true );
        m_tabWidget->setMovable( true );
        m_tabWidget->addTab( textEdit, makeTabTitle( textDocument ) );

        setCentralWidget( m_tabWidget );

        QToolBar* toolBar = addToolBar( tr( "Standard" ) );
        toolBar->setObjectName( "moteStandardToolBar" );

        toolBar->addAction( new NewlineCharacterAction( documentSystem, this ) );
        toolBar->addAction( new TextCodecAction( documentSystem, this ) );
        toolBar->addAction( new BOMAction( documentSystem, this ) );

        statusBar();

        QMenuBar* menuBar = this->menuBar();

        QMenu* fileMenu = menuBar->addMenu( tr( "&File" ) );
        fileMenu->addAction(
            tr( "&New" ),
            this, SLOT( createNewDocument( void ) ),
            QKeySequence( QKeySequence::New ) );
        fileMenu->addAction(
            tr( "&Open..." ),
            this, SLOT( openFile( void ) ),
            QKeySequence( QKeySequence::Open ) );
        fileMenu->addAction(
            tr( "&Save" ),
            this, SLOT( saveFile( void ) ),
            QKeySequence( QKeySequence::Save ) );
        fileMenu->addAction(
            tr( "Save &As..." ),
            this, SLOT( saveFileAs( void ) ),
            QKeySequence( QKeySequence::SaveAs ) );
        fileMenu->addAction(
            tr( "Close" ),
            this, SLOT( closeTab( void ) ),
            QKeySequence( QKeySequence::Close ) );

        QMenu* editMenu = menuBar->addMenu( tr( "&Edit" ) );
        editMenu->addAction(
            tr( "Sort Ascending" ),
            this, SLOT( sortAscending( void ) ),
            QKeySequence( Qt::CTRL | Qt::Key_1 ) );
        editMenu->addAction(
            tr( "Format Source Code" ),
            this, SLOT( formatSourceCode( void ) ),
            QKeySequence( Qt::CTRL | Qt::Key_2 ) );
        editMenu->addAction(
            tr( "Delete Duplicate" ),
            this, SLOT( deleteDuplicate( void ) ),
            QKeySequence( Qt::CTRL | Qt::Key_3 ) );
        editMenu->addAction(
            tr( "Reload" ),
            this, SLOT( reload( void ) ) );

        QMenu* searchMenu = menuBar->addMenu( tr( "&Search" ) );
        searchMenu->addAction(
            tr( "Find..." ),
            this, SLOT( findText( void ) ),
            QKeySequence( QKeySequence::Find ) );
        searchMenu->addAction(
            tr( "Replace..." ),
            this, SLOT( replaceText( void ) ),
            QKeySequence( QKeySequence::Replace ) );
        searchMenu->addAction(
            tr( "Find Next" ),
            this, SLOT( findNext( void ) ),
            QKeySequence( QKeySequence::FindNext ) );
        searchMenu->addAction(
            tr( "Find Previous" ),
            this, SLOT( findPrevious( void ) ),
            QKeySequence( QKeySequence::FindPrevious ) );
        searchMenu->addAction(
            tr( "Tag Jump..." ),
            this, SLOT( tagJump( void ) ),
            QKeySequence( Qt::CTRL | Qt::Key_T ) );
        searchMenu->addAction(
            tr( "Go to Corresponding Parenthesis" ),
            this, SLOT( jumpToCoBrace( void ) ),
            QKeySequence( Qt::CTRL | Qt::Key_K ) );
        searchMenu->addAction(
            tr( "Go to Line..." ),
            this, SLOT( jumpToLine( void ) ),
            QKeySequence( Qt::CTRL | Qt::Key_I ) );

        QMenu* viewMenu = menuBar->addMenu( tr( "&View" ) );
        viewMenu->addAction( tr( "Font..." ), this, SLOT( changeFont( void ) ) );
        m_lineNumberAction = viewMenu->addAction(
                                 tr( "Line Number" ),
                                 settings, SLOT( setLineNumberVisible( bool ) ) );
        m_lineNumberAction->setCheckable( true );

        QMenu* windowMenu = menuBar->addMenu( tr( "&Window" ) );
        windowMenu->addAction( tr( "New Window" ), this, SLOT( createNewWindow( void ) ) );

        textDocument->setFont( settings->font() );

        textEdit->setLineNumberVisible( settings->isLineNumberVisible() );

        m_lineNumberAction->setChecked( textEdit->isLineNumberVisible() );

        setWindowTitle( makeWindowTitle( textDocument ) );

        textEdit->setFocus();

        m_findData.replaceMode = false;
        m_findData.flags = 0;
        if( settings->findCaseSensitivity() )
        {
            m_findData.flags |= QTextDocument::FindCaseSensitively;
        }
        if( settings->findWholeWords() )
        {
            m_findData.flags |= QTextDocument::FindWholeWords;
        }
        m_findData.regularExpressionEnabled = settings->isFindRegularExpressionEnabled();
        m_findData.highlightAllOccurrences = settings->findHighlightAllOccurrences();

        connect(
            documentSystem, SIGNAL( filePathChanged( TextDocument* ) ),
            SLOT( updateWindowTitle( TextDocument* ) ) );
        connect(
            documentSystem, SIGNAL( filePathChanged( TextDocument* ) ),
            SLOT( updateTabTitle( TextDocument* ) ) );
        connect(
            documentSystem, SIGNAL( modificationChanged( TextDocument* ) ),
            SLOT( updateWindowTitle( TextDocument* ) ) );
        connect(
            documentSystem, SIGNAL( modificationChanged( TextDocument* ) ),
            SLOT( updateTabTitle( TextDocument* ) ) );

        connect(
            settings, SIGNAL( lineNumberVisibilityChanged( bool ) ),
            SLOT( onLineNumberVisibilityChanged( bool ) ) );

        connect(
            m_tabWidget, SIGNAL( currentChanged( int ) ),
            SLOT( onCurrentTabChanged( void ) ) );
        connect(
            m_tabWidget, SIGNAL( tabCloseRequested( int ) ),
            SLOT( onTabCloseRequested( int ) ) );

        connect(
            this, SIGNAL( currentDocumentChanged( TextDocument* ) ),
            SLOT( updateWindowTitle( TextDocument* ) ) );
    }

    TextEdit* MainWindow::currentEdit( void )const
    {
        return qobject_cast<TextEdit*>( m_tabWidget->currentWidget() );
    }

    TextDocument* MainWindow::currentDocument( void )const
    {
        TextEdit* textEdit = currentEdit();
        return textEdit ? qobject_cast<TextDocument*>( textEdit->document() ) : NULL;
    }

    void MainWindow::saveGeometryAndState( void )
    {
        m_settings->setValue( "mainWindowGeometry", saveGeometry() );
        m_settings->setValue( "mainWindowState", saveState() );
    }

    void MainWindow::restoreGeometryAndState( void )
    {
        const QVariant mainWindowGeometry = m_settings->value( "mainWindowGeometry" );
        if( mainWindowGeometry.isValid() )
        {
            restoreGeometry( mainWindowGeometry.toByteArray() );
        }

        const QVariant mainWindowState = m_settings->value( "mainWindowState" );
        if( mainWindowState.isValid() )
        {
            restoreState( mainWindowState.toByteArray() );
        }
    }

    QList<TextEdit*> MainWindow::findEdits( TextDocument* textDocument )const
    {
        QList<TextEdit*> edits;
        for( int i = 0; i < m_tabWidget->count(); ++i )
        {
            TextEdit* textEdit = qobject_cast<TextEdit*>( m_tabWidget->widget( i ) );
            if( textEdit &&
                ( !textDocument || ( textEdit->document() == textDocument ) ) )
            {
                edits.append( textEdit );
            }
        }
        return edits;
    }

    QList<TextEdit*> MainWindow::findEdits( const QString& path )const
    {
        QList<TextEdit*> edits;
        for( int i = 0; i < m_tabWidget->count(); ++i )
        {
            TextEdit* textEdit = qobject_cast<TextEdit*>( m_tabWidget->widget( i ) );
            if( textEdit )
            {
                const TextDocument* textDocument =
                    qobject_cast<TextDocument*>( textEdit->document() );
                if( textDocument )
                {
                    if( QFileInfo( textDocument->filePath() ) == QFileInfo( path ) )
                    {
                        edits.append( textEdit );
                    }
                }
            }
        }
        return edits;
    }

    bool MainWindow::saveFile( TextDocument* textDocument )
    {
        if( !textDocument )
        {
            return false;
        }

        const QString path = textDocument->filePath();
        if( path.isEmpty() )
        {
            return saveFileAs( textDocument );
        }
        else
        {
            return textDocument->saveFile( path );
        }
    }

    bool MainWindow::saveFileAs( TextDocument* textDocument )
    {
        if( !textDocument )
        {
            return false;
        }

        const QString path = QFileDialog::getSaveFileName( this );
        if( path.isEmpty() )
        {
            return false;
        }

        return textDocument->saveFile( path );
    }

    void MainWindow::activate( TextEdit* textEdit )
    {
        const int index = m_tabWidget->indexOf( textEdit );
        if( index != -1 )
        {
            m_tabWidget->setCurrentIndex( index );
        }
    }

    void MainWindow::openFile( const QString& path )
    {
        if( path.isEmpty() )
        {
            return;
        }

        const QList<TextEdit*> edits = TextEdit::findEdits( path );
        if( edits.isEmpty() )
        {
            TextDocument* currentDocument = this->currentDocument();
            if( !currentDocument ||
                currentDocument->isModified() ||
                !currentDocument->filePath().isEmpty() )
            {
                TextDocument* textDocument = m_documentSystem->createDocument();
                if( textDocument->openFile( path ) )
                {
                    textDocument->setFont( m_settings->font() );

                    TextEdit* textEdit = new TextEdit( textDocument );
                    textEdit->setLineNumberVisible( m_settings->isLineNumberVisible() );

                    m_tabWidget->addTab( textEdit, makeTabTitle( textDocument ) );
                    m_tabWidget->setCurrentWidget( textEdit );
                }
            }
            else
            {
                if( currentDocument->openFile( path ) )
                {
                    TextEdit* textEdit = currentEdit();
                    if( textEdit )
                    {
                        textEdit->moveCursor( QTextCursor::Start );
                    }
                }
            }
        }
        else
        {
            TextEdit* textEdit = edits.front();
            MainWindow* win = qobject_cast<MainWindow*>( textEdit->window() );
            if( win )
            {
                win->activate( textEdit );
                win->raise();
                win->activateWindow();
            }
        }
    }

    void MainWindow::openFile( void )
    {
        QString dir;
        const QVariant lastOpenedDir = m_settings->value( "lastOpenedDir" );
        if( lastOpenedDir.isValid() )
        {
            dir = lastOpenedDir.toString();
        }
        const QStringList paths =
#ifdef Q_OS_MAC
            // WORKAROUND: QTBUG-20771
            QFileDialog::getOpenFileNames(
                this,
                QString(),
                dir,
                QString(),
                0,
                QFileDialog::DontUseNativeDialog );
#else
            QFileDialog::getOpenFileNames( this, QString(), dir );
#endif
        if( !paths.isEmpty() )
        {
            m_settings->setValue( "lastOpenedDir", QFileInfo( paths.front() ).dir().path() );

            for( int i = 0; i < paths.size(); ++i )
            {
                openFile( paths.at( i ) );
            }
        }
    }

    void MainWindow::saveFile( void )
    {
        saveFile( currentDocument() );
    }

    void MainWindow::saveFileAs( void )
    {
        saveFileAs( currentDocument() );
    }

    void MainWindow::changeFont( void )
    {
        bool ok = false;
        QFont oldFont = m_settings->font();
        QFont font = QFontDialog::getFont( &ok, oldFont, this );
        if( ok && ( font != oldFont ) )
        {
            m_settings->setFont( font );
        }
    }

    void MainWindow::formatSourceCode( void )
    {
        TextEdit* textEdit = currentEdit();
        if( textEdit )
        {
            textEdit->formatSourceCode();
        }
    }

    void MainWindow::closeTab( void )
    {
        if( m_tabWidget->count() == 1 )
        {
            close();
        }
        else
        {
            _closeTab();
        }
    }

    void MainWindow::createNewWindow( void )
    {
        saveGeometryAndState();

        MainWindow* win = new mote::MainWindow( m_settings, m_documentSystem );
        win->setAttribute( Qt::WA_DeleteOnClose );
        win->restoreGeometryAndState();
        if( !win->isMaximized() )
        {
            const QRect screenRect = QApplication::desktop()->availableGeometry( win );
            const int delta = qMax( screenRect.width() / 40, screenRect.height() / 40 );
            QRect winRect = win->frameGeometry();

            if( winRect.left() - screenRect.left() > screenRect.right() - winRect.right() )
            {
                winRect.translate( -delta, 0 );
            }
            else
            {
                winRect.translate( delta, 0 );
            }

            if( winRect.top() - screenRect.top() > screenRect.bottom() - winRect.bottom() )
            {
                winRect.translate( 0, -delta );
            }
            else
            {
                winRect.translate( 0, delta );
            }

            if( winRect.bottom() > screenRect.bottom() )
            {
                winRect.moveBottom( screenRect.bottom() );
            }

            if( winRect.top() < screenRect.top() )
            {
                winRect.moveTop( screenRect.top() );
            }

            if( winRect.right() > screenRect.right() )
            {
                winRect.moveRight( screenRect.right() );
            }

            if( winRect.left() < screenRect.left() )
            {
                winRect.moveLeft( screenRect.left() );
            }

            win->move( winRect.topLeft() );
        }
        win->show();
    }

    void MainWindow::createNewDocument( void )
    {
        TextDocument* textDocument = m_documentSystem->createDocument();
        textDocument->setFont( m_settings->font() );

        TextEdit* textEdit = new TextEdit( textDocument );
        textEdit->setLineNumberVisible( m_settings->isLineNumberVisible() );

        m_tabWidget->addTab( textEdit, makeTabTitle( textDocument ) );
        m_tabWidget->setCurrentWidget( textEdit );
    }

    void MainWindow::jumpToCoBrace( void )
    {
        TextEdit* textEdit = currentEdit();
        if( textEdit )
        {
            textEdit->jumpToCoBrace();
        }
    }

    void MainWindow::sortAscending( void )
    {
        TextEdit* textEdit = currentEdit();
        if( !textEdit )
        {
            return;
        }

        QTextCursor textCursor = textEdit->textCursor();
        if( !textCursor.hasSelection() )
        {
            return;
        }

        QStringList strList = textCursor.selectedText().split( QChar( 0x2029 ) );
        if( strList.isEmpty() )
        {
            return;
        }

        bool lastEmpty = false;
        if( strList.back().isEmpty() )
        {
            strList.pop_back();
            lastEmpty = true;
        }

        qSort( strList.begin(), strList.end() );

        textCursor.beginEditBlock();
        textCursor.deleteChar();
        textCursor.insertText( strList.join( QChar( 0x2029 ) ) );
        if( lastEmpty )
        {
            textCursor.insertText( QChar( 0x2029 ) );
        }
        textCursor.endEditBlock();
        textEdit->setTextCursor( textCursor );
    }

    void MainWindow::deleteDuplicate( void )
    {
        TextEdit* textEdit = currentEdit();
        if( !textEdit )
        {
            return;
        }

        QTextCursor textCursor = textEdit->textCursor();
        if( !textCursor.hasSelection() )
        {
            return;
        }

        QStringList strList = textCursor.selectedText().split( QChar( 0x2029 ) );
        if( strList.isEmpty() )
        {
            return;
        }

        bool lastEmpty = false;
        if( strList.back().isEmpty() )
        {
            strList.pop_back();
            lastEmpty = true;
        }

        QSet<QString> strSet;
        for( int i = 0; i < strList.size(); ++i )
        {
            if( strSet.contains( strList.at( i ) ) )
            {
                strList.removeAt( i );
                --i;
            }
            else
            {
                strSet.insert( strList.at( i ) );
            }
        }

        textCursor.beginEditBlock();
        textCursor.deleteChar();
        textCursor.insertText( strList.join( QChar( 0x2029 ) ) );
        if( lastEmpty )
        {
            textCursor.insertText( QChar( 0x2029 ) );
        }
        textCursor.endEditBlock();
        textEdit->setTextCursor( textCursor );
    }

    void MainWindow::findText( void )
    {
        createFindDialog();
        m_findDialog->setReplaceMode( false );
        if( !m_findDialog->isVisible() )
        {
            TextEdit* textEdit = currentEdit();
            if( textEdit )
            {
                m_findDialog->setText( textEdit->textCursor().selectedText() );
            }
            m_findDialog->setCaseSensitivity( m_findData.flags & QTextDocument::FindCaseSensitively );
            m_findDialog->setWholeWords( m_findData.flags & QTextDocument::FindWholeWords );
            m_findDialog->setRegularExpressionEnabled( m_findData.regularExpressionEnabled );
            m_findDialog->setHighlightAllOccurrences( m_findData.highlightAllOccurrences );
            m_findDialog->show();
        }
        else
        {
            m_findDialog->raise();
            m_findDialog->activateWindow();
        }
    }

    void MainWindow::findNext( void )
    {
        if( m_findDialog && m_findDialog->isVisible() )
        {
            commitFindDialog();
            m_findDialog->hide();
        }

        TextEdit* textEdit = currentEdit();
        if( textEdit )
        {
            if( m_findData.regularExpressionEnabled )
            {
                if( m_findData.replaceMode )
                {
                    textEdit->replaceNext(
                        m_findData.regularExpression,
                        m_findData.after,
                        m_findData.flags );
                }
                else
                {
                    textEdit->findNext( m_findData.regularExpression, m_findData.flags );
                }
            }
            else
            {
                if( m_findData.replaceMode )
                {
                    textEdit->replaceNext( m_findData.text, m_findData.after, m_findData.flags );
                }
                else
                {
                    textEdit->findNext( m_findData.text, m_findData.flags );
                }
            }
        }
    }

    void MainWindow::findPrevious( void )
    {
        if( m_findDialog && m_findDialog->isVisible() )
        {
            commitFindDialog();
            m_findDialog->hide();
        }

        TextEdit* textEdit = currentEdit();
        if( textEdit )
        {
            if( m_findData.regularExpressionEnabled )
            {
                if( m_findData.replaceMode )
                {
                    textEdit->replacePrevious(
                        m_findData.regularExpression,
                        m_findData.after,
                        m_findData.flags );
                }
                else
                {
                    textEdit->findPrevious( m_findData.regularExpression, m_findData.flags );
                }
            }
            else
            {
                if( m_findData.replaceMode )
                {
                    textEdit->replacePrevious(
                        m_findData.text,
                        m_findData.after,
                        m_findData.flags );
                }
                else
                {
                    textEdit->findPrevious( m_findData.text, m_findData.flags );
                }
            }
        }
    }

    void MainWindow::replaceText( void )
    {
        createFindDialog();
        m_findDialog->setReplaceMode( true );
        if( !m_findDialog->isVisible() )
        {
            TextEdit* textEdit = currentEdit();
            if( textEdit )
            {
                m_findDialog->setText( textEdit->textCursor().selectedText() );
            }
            m_findDialog->setCaseSensitivity( m_findData.flags & QTextDocument::FindCaseSensitively );
            m_findDialog->setWholeWords( m_findData.flags & QTextDocument::FindWholeWords );
            m_findDialog->setRegularExpressionEnabled( m_findData.regularExpressionEnabled );
            m_findDialog->setHighlightAllOccurrences( m_findData.highlightAllOccurrences );
            m_findDialog->show();
        }
        else
        {
            m_findDialog->raise();
            m_findDialog->activateWindow();
        }
    }

    void MainWindow::reload( void )
    {
        TextDocument* textDocument = currentDocument();
        if( !textDocument )
        {
            return;
        }

        const QString fileName = textDocument->fileName();
        if( fileName.isEmpty() )
        {
            return;
        }

        if( textDocument->isModified() )
        {
            const QMessageBox::StandardButton ret =
                QMessageBox::question(
                    this,
                    qApp->applicationName(),
                    tr( "%1 has been modified. Do you want to continue?" ).arg( fileName ),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No );
            if( ret == QMessageBox::No )
            {
                return;
            }
        }

        const QList<TextEdit*> edits = TextEdit::findEdits( textDocument );
        QVector<int> cursorPositions( edits.size() );
        QVector<int> scrollValues( edits.size() );
        for( int i = 0; i < edits.size(); ++i )
        {
            TextEdit* textEdit = edits.at( i );
            cursorPositions[i] = textEdit->textCursor().position();
            scrollValues[i] = textEdit->verticalScrollBar()->sliderPosition();
        }

        if( textDocument->reload() )
        {
            for( int i = 0; i < edits.size(); ++i )
            {
                TextEdit* textEdit = edits.at( i );

                QTextCursor textCursor = edits.at( i )->textCursor();
                textCursor.movePosition( QTextCursor::Start );
                textCursor.movePosition(
                    QTextCursor::NextCharacter,
                    QTextCursor::MoveAnchor,
                    cursorPositions.at( i ) );
                edits.at( i )->setTextCursor( textCursor );

                textEdit->verticalScrollBar()->setSliderPosition( scrollValues.at( i ) );
                textEdit->ensureCursorVisible();
            }
        }
        else
        {
            // TODO
        }
    }

    void MainWindow::closeEvent( QCloseEvent* event )
    {
        while( m_tabWidget->count() > 1 )
        {
            if( !_closeTab() )
            {
                event->ignore();
                return;
            }
        }

        if( !_closeTab() )
        {
            event->ignore();
            return;
        }

        saveGeometryAndState();

        m_settings->setFindCaseSensitivity(
            m_findData.flags & QTextDocument::FindCaseSensitively );
        m_settings->setFindWholeWords(
            m_findData.flags & QTextDocument::FindWholeWords );
        m_settings->setFindRegularExpressionEnabled(
            m_findData.regularExpressionEnabled );
        m_settings->setFindHighlightAllOccurrences(
            m_findData.highlightAllOccurrences );

        if( m_findDialog )
        {
            m_findDialog->saveSize( m_settings );
        }

        event->accept();
    }

    QString MainWindow::makeTabTitle( const TextDocument* textDocument )const
    {
        QString tabTitle = textDocument->fileName();
        if( tabTitle.isEmpty() )
        {
            tabTitle = tr( "(New File)" );
        }

        if( textDocument->isModified() )
        {
            tabTitle += " *";
        }

        return tabTitle;
    }

    QString MainWindow::makeWindowTitle( const TextDocument* textDocument )const
    {
        if( !textDocument )
        {
            return "mote";
        }

        QString windowTitle =
            QDir::toNativeSeparators( textDocument->filePath() );
        if( windowTitle.isEmpty() )
        {
            windowTitle = tr( "(New File)" );
        }

        if( textDocument->isModified() )
        {
            windowTitle += " ";
            windowTitle += tr( "(Modified)" );
        }

        windowTitle += " - mote";

        return windowTitle;
    }

    bool MainWindow::_closeTab( const int index )
    {
        const int idx = ( index == -1 ) ? m_tabWidget->currentIndex() : index;

        TextEdit* textEdit = qobject_cast<TextEdit*>( m_tabWidget->widget( idx ) );
        if( !textEdit )
        {
            Q_ASSERT( 0 );
            return false;
        }

        TextDocument* textDocument = qobject_cast<TextDocument*>( textEdit->document() );
        if( !textDocument )
        {
            Q_ASSERT( 0 );
            return false;
        }

        const QList<TextEdit*> edits = TextEdit::findEdits( textDocument );
        if( textDocument->isModified() && ( edits.size() == 1 ) )
        {
            QString fileName = textDocument->fileName();
            if( fileName.isEmpty() )
            {
                fileName = tr( "(New File)" );
            }

            const QMessageBox::StandardButton ret =
                QMessageBox::question(
                    this,
                    qApp->applicationName(),
                    tr( "%1 has been modified. Do you want to save?" ).arg( fileName ),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                    QMessageBox::Yes );
            switch( ret )
            {
            case QMessageBox::Yes:
                if( !saveFile( textDocument ) )
                {
                    return false;
                }
                break;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
            }
        }

        // not close last tab
        if( m_tabWidget->count() > 1 )
        {
            m_tabWidget->removeTab( idx );

            textEdit->deleteLater();

            if( edits.size() == 1 )
            {
                textDocument->deleteLater();
            }
        }

        return true;
    }

    void MainWindow::commitFindDialog( void )
    {
        if( m_findDialog )
        {
            m_findData.replaceMode = m_findDialog->isReplaceMode();
            m_findData.text = m_findDialog->text();
            m_findData.after = m_findDialog->after();
            m_findData.flags = 0;
            if( m_findDialog->caseSensitivity() )
            {
                m_findData.flags |= QTextDocument::FindCaseSensitively;
            }
            if( m_findDialog->wholeWords() )
            {
                m_findData.flags |= QTextDocument::FindWholeWords;
            }
            m_findData.regularExpressionEnabled = m_findDialog->isRegularExpressionEnabled();
            m_findData.highlightAllOccurrences = m_findDialog->highlightAllOccurrences();
            if( m_findData.regularExpressionEnabled )
            {
                m_findData.regularExpression.setPattern( m_findData.text );
                m_findData.regularExpression.setCaseSensitivity(
                    ( m_findData.flags & QTextDocument::FindCaseSensitively ) ? Qt::CaseSensitive : Qt::CaseInsensitive );
            }
        }
    }

    void MainWindow::createFindDialog( void )
    {
        if( !m_findDialog )
        {
            m_findDialog = new FindDialog( this );
            m_findDialog->restoreSize( m_settings );
            m_findDialog->adjustSize();

            connect(
                m_findDialog, SIGNAL( accepted() ),
                SLOT( onFindTextAccepted( void ) ) );

            QShortcut* shortcut = new QShortcut( QKeySequence( QKeySequence::FindNext ), m_findDialog );
            connect(
                shortcut, SIGNAL( activated() ),
                SLOT( findNext( void ) ) );

            shortcut = new QShortcut( QKeySequence( QKeySequence::FindPrevious ), m_findDialog );
            connect(
                shortcut, SIGNAL( activated() ),
                SLOT( findPrevious( void ) ) );
        }
    }

    void MainWindow::updateWindowTitle( TextDocument* textDocument )
    {
        if( textDocument == currentDocument() )
        {
            setWindowTitle( makeWindowTitle( textDocument ) );
        }
    }

    void MainWindow::updateTabTitle( TextDocument* textDocument )
    {
        for( int i = 0; i < m_tabWidget->count(); ++i )
        {
            TextEdit* textEdit =
                qobject_cast<TextEdit*>( m_tabWidget->widget( i ) );
            if( !textEdit || ( textEdit->document() != textDocument ) )
            {
                continue;
            }

            m_tabWidget->setTabText( i, makeTabTitle( textDocument ) );
        }
    }

    void MainWindow::onLineNumberVisibilityChanged( bool onoff )
    {
        m_lineNumberAction->setChecked( onoff );

        for( int i = 0; i < m_tabWidget->count(); ++i )
        {
            TextEdit* textEdit =
                qobject_cast<TextEdit*>( m_tabWidget->widget( i ) );
            if( textEdit )
            {
                textEdit->setLineNumberVisible( onoff );
            }
        }
    }

    void MainWindow::onCurrentTabChanged( void )
    {
        emit currentDocumentChanged( currentDocument() );
    }

    void MainWindow::onTabCloseRequested( int index )
    {
        if( m_tabWidget->count() == 1 )
        {
            close();
        }
        else
        {
            _closeTab( index );
        }
    }

    void MainWindow::tagJump( void )
    {
        TextEdit* textEdit = currentEdit();
        if( !textEdit )
        {
            return;
        }

        CTags ctags;
        if( !ctags.exec( textEdit->document() ) )
        {
            return;
        }

        TagJumpDialog* dialog = new TagJumpDialog(
            ctags,
            textEdit->textCursor().blockNumber() + 1,
            this );
        dialog->restoreSize( m_settings );
        if( dialog->exec() == QDialog::Accepted )
        {
            const CTags::Entry& entry = ctags.entry( dialog->selectedEntry() );
            QTextBlock block =
                textEdit->document()->findBlockByNumber( entry.exCmd.toInt() - 1 );
            if( block.isValid() )
            {
                QTextCursor textCursor = textEdit->textCursor();
                textCursor.setPosition( block.position() );
                textEdit->setTextCursor( textCursor );
                textEdit->centerCursor();
            }
        }
        dialog->saveSize( m_settings );
        delete dialog;
    }

    void MainWindow::onFindTextAccepted( void )
    {
        commitFindDialog();
        findNext();
    }

    void MainWindow::jumpToLine( void )
    {
        TextEdit* textEdit = currentEdit();
        if( !textEdit )
        {
            return;
        }

        QTextCursor textCursor = textEdit->textCursor();
        bool ok = false;
        const int lineNumber = QInputDialog::getInt(
                                   this,
                                   tr( "Go to Line" ),
                                   tr( "Line number" ),
                                   textCursor.block().blockNumber() + 1,
                                   1,
                                   textEdit->document()->blockCount(),
                                   1,
                                   &ok );
        if( ok )
        {
            textCursor.setPosition(
                textEdit->document()->findBlockByNumber( lineNumber - 1 ).position() );
            textEdit->setTextCursor( textCursor );
            textEdit->centerCursor();
        }
    }
}
