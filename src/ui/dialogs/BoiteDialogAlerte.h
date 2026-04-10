#ifndef BOITEDIALOGALERTE_H
#define BOITEDIALOGALERTE_H

#include <QDialog>
#include <memory>

class QTextEdit;
class QLabel;
class QPushButton;

class BoiteDialogAlerte : public QDialog
{
    Q_OBJECT

public:
    enum Type { Warning, Error, Info };
    
    BoiteDialogAlerte(Type type, const QString& titre, const QString& message, QWidget* parent = nullptr);
    ~BoiteDialogAlerte();

private:
    void creerWidgets(const QString& titre, const QString& message);
    void configurerStyle(Type type);

    std::unique_ptr<QLabel> m_labelTitre;
    std::unique_ptr<QTextEdit> m_textMessage;
    std::unique_ptr<QPushButton> m_boutonOK;
};

#endif // BOITEDIALOGALERTE_H