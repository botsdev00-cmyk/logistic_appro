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
class TableauStockLocation;
class TableauReconciliation;
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
    // Entrées/Retours
    void onAjouterEntree();
    void onAjouterRetour();
    void onEntreesEnAttente();
    void onRetoursEnAttente();
    
    // Recherche/Filtres
    void onRechercherStock();
    void onFiltrerParStatut(int index);
    
    // Actions principales
    void onActualiser();
    void onExporterStock();
    void onSynchroniser();
    
    // ✅ NOUVEAU: Outils avancés
    void onVerifierIntegrite();
    void onReparerStock();
    void onAfficherStockParLocation();

private:
    void initializeUI();
    void initializeToolbar();
    void initializeSearchBar();
    void initializeTabs();
    void initializeAdvancedTools();
    
    void connectSignals();
    void afficherAlertes();

    GestionnaireStock* m_gestionnaire;
    QUuid m_utilisateurId;

    // Widgets principaux
    QTabWidget* m_tabWidget;
    TableauStock* m_tableauStock;
    TableauHistoriqueStock* m_tableauHistorique;
    PanelAlertes* m_panelAlertes;
    TableauStockLocation* m_tableauStockLocation;
    TableauReconciliation* m_tableauReconciliation;

    // Toolbar
    QPushButton* m_btnAjouterEntree;
    QPushButton* m_btnAjouterRetour;
    QPushButton* m_btnEntreesEnAttente;
    QPushButton* m_btnRetoursEnAttente;
    QPushButton* m_btnActualiser;
    QPushButton* m_btnExporter;
    QPushButton* m_btnSynchroniser;
    
    // ✅ NOUVEAU: Boutons avancés
    QPushButton* m_btnVerifierIntegrite;
    QPushButton* m_btnReparerStock;
    QPushButton* m_btnStockLocation;

    QLineEdit* m_searchBox;
    QComboBox* m_filterStatus;
    QLabel* m_labelValeurTotal;
    QLabel* m_labelStatistiques;  // ✅ NOUVEAU
};

#endif // VUE_STOCK_H