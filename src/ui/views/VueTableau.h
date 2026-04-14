#ifndef VUETABLEAU_H
#define VUETABLEAU_H

#include <QWidget>
#include <memory>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QGridLayout;

class VueTableau : public QWidget
{
    Q_OBJECT

public:
    VueTableau(QWidget* parent = nullptr);
    ~VueTableau();

private slots:
    void allerVersStock();
    void allerVersRepartition();
    void allerVersVentes();
    void allerVersCredit();
    void allerVersCaisse();
    void allerVersClients();

private:
    void creerInterface();
    void creerCarteKPI();
    void creerRaccourcis();
    void creerDernieresActivites();
    void chargerDonnees();
    void appliquerStyle();
    
    // Composants
    QLabel* m_labelBienvenue;
    QLabel* m_labelDate;
    
    // Cartes KPI
    QLabel* m_stockTotal;
    QLabel* m_ventesJour;
    QLabel* m_clientsActifs;
    QLabel* m_creditsEnCours;
    
    // Boutons raccourcis
    QPushButton* m_btnStock;
    QPushButton* m_btnRepartition;
    QPushButton* m_btnVentes;
    QPushButton* m_btnCredit;
    QPushButton* m_btnCaisse;
    QPushButton* m_btnClients;
};

#endif // VUETABLEAU_H