#ifndef VUE_STOCK_H
#define VUE_STOCK_H

#include <QWidget>
#include <QUuid>
#include <QList>

class GestionnaireStock;
class QTabWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;
class TableauStock;
class TableauHistoriqueStock;
class PanelAlertes;
class EntreeStock;
class RetourStock;

class VueStock : public QWidget
{
    Q_OBJECT

public:
    explicit VueStock(GestionnaireStock* gestionnaire, const QUuid& utilisateurId, QWidget* parent = nullptr);
    ~VueStock();

    void chargerDonnees();
    void actualiser();

private slots:
    void onAjouterEntree();
    void onAjouterRetour();
    void onEntreesEnAttente();
    void onRetoursEnAttente();
    void onActualiser();
    void onRechercherStock();
    void onFiltrerParStatut(int index);
    void onExporterStock();
    void onSynchroniser();

private:
    void initializeUI();
    void connectSignals();
    void afficherAlertes();

    GestionnaireStock* m_gestionnaire;
    QUuid m_utilisateurId;

    // Widgets
    QTabWidget* m_tabWidget;
    TableauStock* m_tableauStock;
    TableauHistoriqueStock* m_tableauHistorique;
    PanelAlertes* m_panelAlertes;

    // Toolbar
    QPushButton* m_btnAjouterEntree;
    QPushButton* m_btnAjouterRetour;
    QPushButton* m_btnEntreesEnAttente;
    QPushButton* m_btnRetoursEnAttente;
    QPushButton* m_btnActualiser;
    QPushButton* m_btnExporter;
    QPushButton* m_btnSynchroniser;

    QLineEdit* m_searchBox;
    QComboBox* m_filterStatus;
    QLabel* m_labelValeurTotal;
};

#endif // VUE_STOCK_H