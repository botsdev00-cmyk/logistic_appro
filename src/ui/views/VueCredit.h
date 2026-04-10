#ifndef VUECREDIT_H
#define VUECREDIT_H

#include <QWidget>
#include <memory>

class TableauCredit;
class QPushButton;

class VueCredit : public QWidget
{
    Q_OBJECT

public:
    VueCredit(QWidget* parent = nullptr);
    ~VueCredit();

private slots:
    void afficherCreditsEnRetard();
    void afficherTousLesCredits();
    void enregistrerPaiement();
    void envoyerRappels();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<TableauCredit> m_tableauCredit;
    std::unique_ptr<QPushButton> m_btnEnRetard;
    std::unique_ptr<QPushButton> m_btnTous;
};

#endif // VUECREDIT_H