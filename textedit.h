#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTextEdit>


QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class TextEdit:public QMainWindow
{
    //使用qt的moc预处理器在编译前提前处理该类
    Q_OBJECT
public:
    TextEdit(QWidget *parent=0);

    bool load(const QString &f);

public slots:
    void fileNew();

protected:
    //close all events
    virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void clipboardDataChanged();

private:
    //private functions
    void setupFileActions();
    void setupEditActions();
    bool maybeSave();
    void setCurrentFileName(const QString &filename);

    //Actions
    QAction *actionSave;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    //appearance
    QToolBar *bar;
    QString fileName;
    QTextEdit *textEdit;

};

#endif // TEXTEDIT_H
