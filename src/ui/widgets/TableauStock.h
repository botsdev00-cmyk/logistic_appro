#ifndef TABLEAUSTOCK_H
#define TABLEAUSTOCK_H

#include <QTableWidget>
#include <memory>

class GestionnaireStock;

class TableauStock : public QTableWidget
{
    Q_OBJECT

public:
    TableauStock(QWidget* parent = nullptr);
    ~TableauStock();

    void chargerDonnees();
    void rafraichir();

private slots:
    void afficherDetailsStock();
    void exporterEnCSV();

private:
    void creerContextMenu();
    void initialiserColonnes();
    void remplirTableau();
    void mettreEnEvidence();

    std::unique_ptr<GestionnaireStock> m_gestionnaire;
};

#endif // TABLEAUSTOCK_H