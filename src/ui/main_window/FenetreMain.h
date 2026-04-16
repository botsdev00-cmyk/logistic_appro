#ifndef FENETREMAIN_H
#define FENETREMAIN_H

#include <QMainWindow>
#include <QUuid>
#include <memory>
#include "../../core/entities/Utilisateur.h"

class QTabWidget;
class QLabel;
class Utilisateur;
class GestionnaireCatalogue;
class GestionnaireStock;
class GestionnaireRepartition;
class GestionnaireSales;
class GestionnaireCredit;
class GestionnaireCaisse;
class GestionnaireClient;
class GestionnaireRapport;
class RepositoryEntreeStock;
class RepositoryRetourStock;
class RepositoryStockSoldes;
class ServicePermissions;
class TableauCatalogue;

class VueTableau;
class VueStock;
class VueRepartition;
class VueVentes;
class VueCredit;
class VueCaisse;
class VueRapport;
class VueClient;

class FenetreMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit FenetreMain(const Utilisateur& utilisateur, QWidget* parent = nullptr);
    ~FenetreMain();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void initializeUI();
    void initializeManagers();
    void setupTabs();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void connectSignals();
    void verifierPermissions();
    void afficherAlertes();

    // Slots
    void onDeconnexion();
    void onAPropos();
    void onParametres();
    void onActualiserDonnees();
    void onSynchroniserStock();
    void onExporterDonnees();

    // Données utilisateur
    Utilisateur m_utilisateur;
    QUuid m_utilisateurId;

    // Gestionnaires métier
    std::unique_ptr<GestionnaireCatalogue> m_gestionnaireCatalogue;
    std::unique_ptr<RepositoryEntreeStock> m_repoEntrees;
    std::unique_ptr<RepositoryRetourStock> m_repoRetours;
    std::unique_ptr<RepositoryStockSoldes> m_repoSoldes;
    std::unique_ptr<ServicePermissions> m_servicePermissions;
    std::unique_ptr<GestionnaireStock> m_gestionnaireStock;
    std::unique_ptr<GestionnaireRepartition> m_gestionnaireRepartition;
    std::unique_ptr<GestionnaireSales> m_gestionnaireSales;
    std::unique_ptr<GestionnaireCredit> m_gestionnaireCredit;
    std::unique_ptr<GestionnaireCaisse> m_gestionnaireCaisse;
    std::unique_ptr<GestionnaireClient> m_gestionnaireClient;
    std::unique_ptr<GestionnaireRapport> m_gestionnaireRapport;
    TableauCatalogue* m_Catalogue;

    // UI
    QTabWidget* m_tabWidget;
    QLabel* m_statusLabel;
    QLabel* m_userLabel;
    QLabel* m_stockAlertLabel;

    // Vues
    VueTableau* m_vueTableau;
    VueStock* m_vueStock;
    VueRepartition* m_vueRepartition;
    VueVentes* m_vueVentes;
    VueCredit* m_vueCredit;
    VueCaisse* m_vueCaisse;
    VueRapport* m_vueRapport;
    VueClient* m_vueClient;
};

#endif // FENETREMAIN_H