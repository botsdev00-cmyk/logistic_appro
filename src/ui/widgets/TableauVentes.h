#ifndef TABLEAUVENTES_H
#define TABLEAUVENTES_H

#include <QTableWidget>
#include <memory>

class GestionnaireSales;

class TableauVentes : public QTableWidget
{
    Q_OBJECT

public:
    TableauVentes(QWidget* parent = nullptr);
    ~TableauVentes();

    void chargerDonnees();
    void rafraichir();
    void filtrerParClient(const QString& client);
    void filtrerParType(const QString& type);

private slots:
    void afficherDetailsVente();
    void imprimer();
    void exporterEnCSV();

private:
    void creerContextMenu();
    void initialiserColonnes();
    void remplirTableau();
    void calculerTotaux();

    std::unique_ptr<GestionnaireSales> m_gestionnaire;
    double m_totalVentes;
    double m_totalCash;
    double m_totalCredit;
};

#endif // TABLEAUVENTES_H