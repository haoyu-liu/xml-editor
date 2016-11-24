#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QFontDatabase>
#include <QCloseEvent>
#include <QMessageBox>
#include <QtCore>
#include <QClipboard>


#include "textedit.h"

const QString rsrcPath=":images/";
TextEdit::TextEdit(QWidget *parent):QMainWindow(parent)
{
    setWindowTitle(QCoreApplication::applicationName());
    textEdit=new QTextEdit(this);
    setCentralWidget(textEdit);
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();

    //set textedit font
    QFont textFont("Helvetica");
    textFont.setStyleHint(QFont::SansSerif);
    textEdit->setFont(textFont);

    connect(textEdit->document(),&QTextDocument::modificationChanged,
            actionSave,&QAction::setEnabled);
    connect(textEdit->document(),&QTextDocument::modificationChanged,
            this,&QWidget::setWindowModified);
    connect(textEdit->document(),&QTextDocument::undoAvailable,
            actionUndo,&QAction::setEnabled);
    connect(textEdit->document(),&QTextDocument::redoAvailable,
            actionRedo,&QAction::setEnabled);

    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isUndoAvailable());
    actionSave->setEnabled(textEdit->document()->isRedoAvailable());
    textEdit->setFocus();
    setCurrentFileName(QString());
}

bool TextEdit::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    str = QString::fromLocal8Bit(data);
    textEdit->setPlainText(str);

    setCurrentFileName(f);
    return true;
}

void TextEdit::fileNew()
{
    if(maybeSave())
    {
        textEdit->clear();
        setCurrentFileName(QString());
    }
}

void TextEdit::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

void TextEdit::fileOpen()
{
    QFileDialog fileDialog(this, tr("Open File..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters(QStringList() << "text/xml" << "text/plain");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    const QString fn = fileDialog.selectedFiles().first();
    if (load(fn))
        statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));
}

bool TextEdit::fileSave()
{
    if(fileName.isEmpty())
    {
        return fileSaveAs();
    }
    if(fileName.startsWith(QStringLiteral(":/")))
    {
        return fileSaveAs();
    }
    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
    return success;


}

bool TextEdit::fileSaveAs()
{
    QFileDialog fileDialog(this,tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "text/xml" << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.setDefaultSuffix("odt");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void TextEdit::clipboardDataChanged()
{
    if(const QMimeData *md=QApplication::clipboard()->mimeData())
    {
        actionPaste->setEnabled(md->hasText());
    }
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &TextEdit::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png"));
    a=menu->addAction(openIcon,tr("&Open..."),this,&TextEdit::fileOpen);

    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &TextEdit::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);

    a = menu->addAction(tr("Save &As..."), this, &TextEdit::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();
#ifndef QT_NO_PRINTER

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void TextEdit::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo",QIcon(rsrcPath+"/editundo.png"));
    actionUndo = menu->addAction(undoIcon,tr("&Undo"),textEdit,&QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo",
                                            QIcon(rsrcPath+"/editredo.png"));
    actionRedo=menu->addAction(redoIcon,tr("&Redo"),textEdit,&QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

    const QIcon cutIcon = QIcon::fromTheme("edit-cut",QIcon(rsrcPath+"/editcut.png"));
    actionCut = menu->addAction(cutIcon,tr("Cu&t"),textEdit,&QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy",QIcon(rsrcPath+"/editcopy.png"));
    actionCopy = menu->addAction(copyIcon,tr("&Copy"),textEdit,&QTextEdit::cut);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste",QIcon(rsrcPath+"/editpaste.png"));
    actionPaste = menu->addAction(pasteIcon,tr("&Paste"),textEdit,&QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb->addAction(actionPaste);
    if(const QMimeData *md=QApplication::clipboard()->mimeData())
    {
        actionPaste->setEnabled(md->hasText());
    }
}


bool TextEdit::maybeSave()
{
    if(!textEdit->document()->isModified())
    {
        return true;
    }

    const QMessageBox::StandardButton ret =
                    QMessageBox::warning(this,QCoreApplication::applicationName(),
                                         tr("The document has been modified.\n"
                                            "Do you want to save your changes?"),
                                         QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if(ret == QMessageBox::Save)
    {
        return fileSave();
    }else if(ret==QMessageBox::Cancel)
    {
        return false;
    }
    return true;
}

void TextEdit::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
    {
        shownName = "untitled.txt";
    }
    else
    {
        shownName = QFileInfo(fileName).fileName();
    }
    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}





