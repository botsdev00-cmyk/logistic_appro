#ifndef TABLEAUCREDIT_H
#define TABLEAUCREDIT_H

#include <QTableWidget>
#include <memory>

class GestionnaireCredit;

class TableauCredit : public QTableWidget
{
    Q_OBJECT

public:
    TableauCredit(QWidget* parent = nullptr);
    ~TableauCredit();

    void chargerDonnees();
    void rafraichir();
    void afficherCreditsEnRetard();
    void afficherTousLesCredits();

private slots:
    void afficherDetailsCredit();
    void marquerCommePayé();
    void envoyerRappel();
    void exporterEnCSV();

private:
    void creerContextMenu();
    void initialiserColonnes();
    void remplirTableau();
    void mettreEnEvidence();

    std::unique_ptr<GestionnaireCredit> m_gestionnaire;
};

#endif // TABLEAUCREDIT_H