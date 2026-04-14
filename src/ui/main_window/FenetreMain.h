#ifndef FENETREMAIN_H
#define FENETREMAIN_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <memory>

class ServiceAuthentification;
class Utilisateur;
class VueTableau;
class VueStock;
class VueRepartition;
class VueVentes;
class VueCredit;
class VueCaisse;
class VueRapport;
class VueClient;
class TableauCatalogue;
class GestionnaireCatalogue;

class FenetreMain : public QMainWindow
{
    Q_OBJECT

public:
    FenetreMain(QWidget* parent = nullptr);
    ~FenetreMain();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void afficherVueTableau();
    void afficherVueStock();
    void afficherVueRepartition();
    void afficherVueVentes();
    void afficherVueCredit();
    void afficherVueCaisse();
    void afficherVueRapport();
    void afficherVueClient();
    void afficherVueCatalogue();
    void deconnecter();
    void afficherAPropos();

private:
    void initialiserGestionnaires();
    void creerMenus();
    void creerBarreOutils();
    void creerBarreStatut();
    void creerVues();
    void initialiserConnexions();
    void configurerInterface();
    void verifierPermissions();
    void afficherInfosUtilisateur();

    std::unique_ptr<ServiceAuthentification> m_authService;
    std::unique_ptr<QStackedWidget> m_stackedWidget;
    std::unique_ptr<GestionnaireCatalogue> m_gestionnaireCatalogue;

    // Vues
    std::unique_ptr<VueTableau> m_vueTableau;
    std::unique_ptr<VueStock> m_vueStock;
    std::unique_ptr<VueRepartition> m_vueRepartition;
    std::unique_ptr<VueVentes> m_vueVentes;
    std::unique_ptr<VueCredit> m_vueCredit;
    std::unique_ptr<VueCaisse> m_vueCaisse;
    std::unique_ptr<VueRapport> m_vueRapport;
    std::unique_ptr<VueClient> m_vueClient;
    std::unique_ptr<TableauCatalogue> m_vueCatalogue;

    // Actions menu
    QAction* m_actionQuitter;
    QAction* m_actionTableau;
    QAction* m_actionStock;
    QAction* m_actionRepartition;
    QAction* m_actionVentes;
    QAction* m_actionCredit;
    QAction* m_actionCaisse;
    QAction* m_actionRapport;
    QAction* m_actionClient;
    QAction* m_actionCatalogue;
    QAction* m_actionDeconnecter;
    QAction* m_actionAPropos;

    // Labels barre de statut
    QLabel* m_labelUtilisateur;
    QLabel* m_labelHeure;
    QLabel* m_labelStatut;
};

#endif // FENETREMAIN_H