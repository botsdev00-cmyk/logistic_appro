#include "FenetreMain.h"
#include "../../business/managers/GestionnaireCatalogue.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../business/managers/GestionnaireSales.h"
#include "../../business/managers/GestionnaireCredit.h"
#include "../../business/managers/GestionnaireCaisse.h"
#include "../../business/managers/GestionnaireClient.h"
#include "../../business/managers/GestionnaireRapport.h"
#include "../../business/services/ServicePermissions.h"
#include "../../data/repositories/RepositoryEntreeStock.h"
#include "../../data/repositories/RepositoryRetourStock.h"
#include "../../data/repositories/RepositoryStockSoldes.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryCategorieProduit.h"
#include "../views/VueTableau.h"
#include "../views/VueStock.h"
#include "../views/VueRepartition.h"
#include "../views/VueVentes.h"
#include "../views/VueCredit.h"
#include "../views/VueCaisse.h"
#include "../views/VueRapport.h"
#include "../views/VueClient.h"
#include "../widgets/TableauCatalogue.h"
#include "../../core/entities/Utilisateur.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QCloseEvent>

// ============================================================================
// CONSTRUCTEUR
// ============================================================================

FenetreMain::FenetreMain(const Utilisateur& utilisateur, QWidget* parent)
    : QMainWindow(parent),
      m_utilisateur(utilisateur),
      m_utilisateurId(utilisateur.getUtilisateurId()),
      m_vueTableau(nullptr),
      m_vueStock(nullptr),
      m_vueRepartition(nullptr),
      m_vueVentes(nullptr),
      m_vueCredit(nullptr),
      m_vueCaisse(nullptr),
      m_vueRapport(nullptr),
      m_vueClient(nullptr),
      m_Catalogue(nullptr)
{
    qDebug() << "[FENETRE MAIN] ═══════════════════════════════════════════════";
    qDebug() << "[FENETRE MAIN] Utilisateur:" << utilisateur.getNomUtilisateur();
    qDebug() << "[FENETRE MAIN] UUID:" << m_utilisateurId.toString();
    qDebug() << "[FENETRE MAIN] Email:" << utilisateur.getEmail();
    qDebug() << "[FENETRE MAIN] ═══════════════════════════════════════════════";

    initializeUI();
    initializeManagers();
    setupTabs();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    connectSignals();
    verifierPermissions();
    afficherAlertes();

    // Timer pour actualisation périodique (toutes les 5 minutes)
    QTimer* timerActualisation = new QTimer(this);
    connect(timerActualisation, &QTimer::timeout, this, &FenetreMain::onActualiserDonnees);
    timerActualisation->start(300000);

    // Timer pour vérifier les alertes stock (toutes les 30 secondes)
    QTimer* timerAlertes = new QTimer(this);
    connect(timerAlertes, &QTimer::timeout, this, &FenetreMain::afficherAlertes);
    timerAlertes->start(30000);

    setWindowTitle("SEMULIKI ERP v1.0.0 - Gestion Logistique et Répartition");
    setMinimumSize(1400, 900);
    setWindowState(Qt::WindowMaximized);

    qDebug() << "[FENETRE MAIN] ✓ Initialisation OK";
    qDebug() << "[FENETRE MAIN] ═══════════════════════════════════════════════";
}

// ============================================================================
// DESTRUCTEUR
// ============================================================================

FenetreMain::~FenetreMain()
{
    qDebug() << "[FENETRE MAIN] Destruction";
}

// ============================================================================
// INITIALISATION UI
// ============================================================================

void FenetreMain::initializeUI()
{
    qDebug() << "[FENETRE MAIN] Initialisation UI";

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_tabWidget = new QTabWidget();
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #E0E0E0; } "
        "QTabBar::tab { padding: 8px 20px; margin-right: 2px; background-color: #F5F5F5; }"
        "QTabBar::tab:selected { background-color: white; }"
    );

    mainLayout->addWidget(m_tabWidget);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

// ============================================================================
// INITIALISATION DES GESTIONNAIRES
// ============================================================================

void FenetreMain::initializeManagers()
{
    qDebug() << "[FENETRE MAIN] Initialisation des gestionnaires...";

    // ====== REPOSITORIES ======
    m_repoEntrees = std::make_unique<RepositoryEntreeStock>();
    m_repoRetours = std::make_unique<RepositoryRetourStock>();
    m_repoSoldes = std::make_unique<RepositoryStockSoldes>();
    m_repoProduits = std::make_unique<RepositoryProduit>();
    m_repoCategories = std::make_unique<RepositoryCategorieProduit>();
    m_servicePermissions = std::make_unique<ServicePermissions>();

    qDebug() << "[FENETRE MAIN]   ✓ Repositories créés";

    // ====== GESTIONNAIRE CATALOGUE ======
    m_gestionnaireCatalogue = std::make_unique<GestionnaireCatalogue>();
    m_gestionnaireCatalogue->setRepositoryProduit(m_repoProduits.get());
    m_gestionnaireCatalogue->setRepositoryCategorieProduit(m_repoCategories.get());

    qDebug() << "[FENETRE MAIN]   ✓ Gestionnaire Catalogue initialisé";
    qDebug() << "[FENETRE MAIN]   ✓ Nombre de produits:" << m_gestionnaireCatalogue->obtenirNombreProduits();
    qDebug() << "[FENETRE MAIN]   ✓ Nombre de catégories:" << m_gestionnaireCatalogue->obtenirNombreCategories();

    // ====== GESTIONNAIRE STOCK ======
    m_gestionnaireStock = std::make_unique<GestionnaireStock>();
    m_gestionnaireStock->setRepositoryEntreeStock(m_repoEntrees.get());
    m_gestionnaireStock->setRepositoryRetourStock(m_repoRetours.get());
    m_gestionnaireStock->setRepositoryStockSoldes(m_repoSoldes.get());
    m_gestionnaireStock->setServicePermissions(m_servicePermissions.get());

    qDebug() << "[FENETRE MAIN]   ✓ Gestionnaire Stock initialisé";

    // ====== AUTRES GESTIONNAIRES ======
    m_gestionnaireRepartition = std::make_unique<GestionnaireRepartition>();
    m_gestionnaireSales = std::make_unique<GestionnaireSales>();
    m_gestionnaireCredit = std::make_unique<GestionnaireCredit>();
    m_gestionnaireCaisse = std::make_unique<GestionnaireCaisse>();
    m_gestionnaireClient = std::make_unique<GestionnaireClient>();
    m_gestionnaireRapport = std::make_unique<GestionnaireRapport>();

    qDebug() << "[FENETRE MAIN]   ✓ Tous les gestionnaires initialisés";
}

// ============================================================================
// SETUP DES ONGLETS
// ============================================================================

void FenetreMain::setupTabs()
{
    qDebug() << "[FENETRE MAIN] Configuration des onglets...";
    qDebug() << "[FENETRE MAIN] Utilisateur ID:" << m_utilisateurId.toString();

    // ====== GESTION DU CATALOGUE ======
    m_Catalogue = new TableauCatalogue(m_gestionnaireCatalogue.get(), m_utilisateurId);
    m_tabWidget->addTab(m_Catalogue, "📚 Catalogue");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Catalogue créé avec" 
             << m_gestionnaireCatalogue->obtenirNombreProduits() 
             << "produit(s) et" 
             << m_gestionnaireCatalogue->obtenirNombreCategories() 
             << "catégorie(s)";

    // ====== GESTION DU STOCK ======
    m_vueStock = new VueStock(m_gestionnaireStock.get(), m_utilisateurId);
    m_tabWidget->addTab(m_vueStock, "📦 Gestion du Stock");
    qDebug() << "[FENETRE MAIN] ✓ UUID passé à VueStock:" << m_utilisateurId.toString();
   
    // ====== RÉPARTITION ======
    m_vueRepartition = new VueRepartition();
    m_tabWidget->addTab(m_vueRepartition, "🚚 Répartitions");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Répartition créé";

    // ====== VENTES ======
    m_vueVentes = new VueVentes();
    m_tabWidget->addTab(m_vueVentes, "💰 Ventes");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Ventes créé";

    // ====== CRÉDITS ======
    m_vueCredit = new VueCredit();
    m_tabWidget->addTab(m_vueCredit, "💳 Crédits");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Crédits créé";

    // ====== CAISSE ======
    m_vueCaisse = new VueCaisse();
    m_tabWidget->addTab(m_vueCaisse, "💵 Caisse");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Caisse créé";

    // ====== CLIENTS ======
    m_vueClient = new VueClient();
    m_tabWidget->addTab(m_vueClient, "👥 Clients");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Clients créé";

    // ====== RAPPORTS ======
    m_vueRapport = new VueRapport();
    m_tabWidget->addTab(m_vueRapport, "📈 Rapports");
    qDebug() << "[FENETRE MAIN]   ✓ Onglet Rapports créé";

    qDebug() << "[FENETRE MAIN] ✓ Configuration des onglets OK";
}

// ============================================================================
// SETUP DU MENU BAR
// ============================================================================

void FenetreMain::setupMenuBar()
{
    qDebug() << "[FENETRE MAIN] Configuration du menu bar...";

    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // ====== MENU FICHIER ======
    QMenu* menuFichier = menuBar->addMenu("&Fichier");
    menuFichier->setIcon(QIcon(":/icons/file.png"));

    QAction* actionActualiser = menuFichier->addAction("🔄 Actualiser");
    actionActualiser->setShortcut(Qt::CTRL | Qt::Key_R);
    connect(actionActualiser, &QAction::triggered, this, &FenetreMain::onActualiserDonnees);

    QAction* actionSynchroniser = menuFichier->addAction("🔗 Synchroniser Stock");
    actionSynchroniser->setShortcut(Qt::CTRL | Qt::Key_S);
    connect(actionSynchroniser, &QAction::triggered, this, &FenetreMain::onSynchroniserStock);

    QAction* actionExporter = menuFichier->addAction("📊 Exporter");
    actionExporter->setShortcut(Qt::CTRL | Qt::Key_E);
    connect(actionExporter, &QAction::triggered, this, &FenetreMain::onExporterDonnees);

    menuFichier->addSeparator();

    QAction* actionDeconnexion = menuFichier->addAction("🚪 Déconnexion");
    actionDeconnexion->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(actionDeconnexion, &QAction::triggered, this, &FenetreMain::onDeconnexion);

    // ====== MENU OUTILS ======
    QMenu* menuOutils = menuBar->addMenu("&Outils");
    menuOutils->setIcon(QIcon(":/icons/tools.png"));

    QAction* actionParametres = menuOutils->addAction("⚙️ Paramètres");
    actionParametres->setShortcut(Qt::CTRL | Qt::Key_P);
    connect(actionParametres, &QAction::triggered, this, &FenetreMain::onParametres);

    // ====== MENU AIDE ======
    QMenu* menuAide = menuBar->addMenu("&Aide");
    menuAide->setIcon(QIcon(":/icons/help.png"));

    QAction* actionAPropos = menuAide->addAction("ℹ️ À Propos");
    connect(actionAPropos, &QAction::triggered, this, &FenetreMain::onAPropos);

    qDebug() << "[FENETRE MAIN] ✓ Menu bar configuré";
}

// ============================================================================
// SETUP DE LA TOOLBAR
// ============================================================================

void FenetreMain::setupToolBar()
{
    qDebug() << "[FENETRE MAIN] Configuration de la toolbar...";

    QToolBar* toolBar = addToolBar("Barre Principale");
    toolBar->setObjectName("MainToolBar");
    toolBar->setIconSize(QSize(24, 24));
    toolBar->setMovable(false);

    QAction* actionActualiser = toolBar->addAction("🔄 Actualiser");
    connect(actionActualiser, &QAction::triggered, this, &FenetreMain::onActualiserDonnees);

    QAction* actionSynchroniser = toolBar->addAction("🔗 Synchroniser");
    connect(actionSynchroniser, &QAction::triggered, this, &FenetreMain::onSynchroniserStock);

    toolBar->addSeparator();

    QLabel* infoLabel = new QLabel("SEMULIKI ERP v1.0.0 | Système de Gestion Logistique");
    infoLabel->setStyleSheet("color: #1976D2; font-weight: bold; margin: 0 10px;");
    toolBar->addWidget(infoLabel);

    qDebug() << "[FENETRE MAIN] ✓ Toolbar configurée";
}

// ============================================================================
// SETUP DE LA STATUSBAR
// ============================================================================

void FenetreMain::setupStatusBar()
{
    qDebug() << "[FENETRE MAIN] Configuration de la statusbar...";

    // Utilisateur connecté
    m_userLabel = new QLabel();
    m_userLabel->setText(QString("👤 %1 | %2").arg(m_utilisateur.getNomUtilisateur(), m_utilisateur.getNomComplet()));
    m_userLabel->setStyleSheet("padding: 5px 10px; color: #2196F3; font-weight: bold;");
    statusBar()->addWidget(m_userLabel);

    statusBar()->addPermanentWidget(new QLabel(" | "));

    // Statut général
    m_statusLabel = new QLabel("✓ Connecté");
    m_statusLabel->setStyleSheet("padding: 5px 10px; color: #4CAF50; font-weight: bold;");
    statusBar()->addPermanentWidget(m_statusLabel);

    statusBar()->addPermanentWidget(new QLabel(" | "));

    // Alertes stock
    m_stockAlertLabel = new QLabel("📦 Stock OK");
    m_stockAlertLabel->setStyleSheet("padding: 5px 10px; color: #4CAF50; font-weight: bold;");
    statusBar()->addPermanentWidget(m_stockAlertLabel);

    qDebug() << "[FENETRE MAIN] ✓ Statusbar configurée";
}

// ============================================================================
// CONNEXION DES SIGNAUX
// ============================================================================

void FenetreMain::connectSignals()
{
    qDebug() << "[FENETRE MAIN] Connexion des signaux...";
    // Les signaux sont connectés dans setupMenuBar et setupToolBar
    qDebug() << "[FENETRE MAIN] ✓ Signaux connectés";
}

// ============================================================================
// VÉRIFICATION DES PERMISSIONS
// ============================================================================

void FenetreMain::verifierPermissions()
{
    qDebug() << "[FENETRE MAIN] Vérification des permissions...";

    // TODO: Récupérer les permissions de l'utilisateur depuis la base
    // et activer/désactiver les onglets en conséquence

    QStringList permissions = {
        "STOCK_VIEW", "STOCK_EDIT", "STOCK_APPROVE",
        "VENTE_CREATE", "VENTE_EDIT",
        "CAISSE_VALIDER",
        "CREDIT_MANAGE"
    };

    qDebug() << "[FENETRE MAIN] Permissions:" << permissions;

    // Masquer les onglets selon les permissions
    // if (!permissions.contains("STOCK_VIEW")) {
    //     m_tabWidget->setTabEnabled(1, false);
    // }

    qDebug() << "[FENETRE MAIN] ✓ Permissions vérifiées";
}

// ============================================================================
// AFFICHAGE DES ALERTES
// ============================================================================

void FenetreMain::afficherAlertes()
{
    if (!m_gestionnaireStock) return;

    auto alertes = m_gestionnaireStock->obtenirAlertesCritiques();

    if (alertes.isEmpty()) {
        m_stockAlertLabel->setText("📦 Stock OK");
        m_stockAlertLabel->setStyleSheet("padding: 5px 10px; color: #4CAF50; font-weight: bold;");
    } else {
        m_stockAlertLabel->setText(QString("⚠️ %1 alerte(s) CRITIQUE(S)").arg(alertes.count()));
        m_stockAlertLabel->setStyleSheet("padding: 5px 10px; color: #FF5722; font-weight: bold;");

        qDebug() << "[FENETRE MAIN] ⚠️ ALERTES STOCK:" << alertes.count();
    }
}

// ============================================================================
// SLOTS - MENU FICHIER
// ============================================================================

void FenetreMain::onDeconnexion()
{
    qDebug() << "[FENETRE MAIN] Déconnexion demandée";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirmation de Déconnexion",
        "Êtes-vous sûr de vouloir vous déconnecter?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        qDebug() << "[FENETRE MAIN] Déconnexion confirmée";
        close();
    }
}

void FenetreMain::onAPropos()
{
    qDebug() << "[FENETRE MAIN] Affichage à propos";

    QString message = QString(
        "SEMULIKI ERP v1.0.0\n\n"
        "Système de Gestion de Logistique et Répartition\n\n"
        "© 2026 - Tous droits réservés\n\n"
        "Utilisateur: %1\n"
        "Rôle: %2\n\n"
        "Technologie: Qt 6.9.3 + PostgreSQL 14\n"
        "Langage: C++ 17"
    ).arg(m_utilisateur.getNomUtilisateur(), m_utilisateur.getNomComplet());

    QMessageBox::information(this, "À Propos de SEMULIKI ERP", message);
}

void FenetreMain::onParametres()
{
    qDebug() << "[FENETRE MAIN] Ouverture des paramètres";
    QMessageBox::information(this, "Paramètres", 
                            "Paramètres utilisateur (Fonctionnalité à implémenter)");
}

// ============================================================================
// SLOTS - MENU OUTILS
// ============================================================================

void FenetreMain::onActualiserDonnees()
{
    qDebug() << "[FENETRE MAIN] Actualisation des données";

    m_statusLabel->setText("🔄 Actualisation...");

    if (m_vueStock) {
        m_vueStock->actualiser();
    }

    QTimer::singleShot(1000, this, [this]() {
        m_statusLabel->setText("✓ Connecté");
    });
}

void FenetreMain::onSynchroniserStock()
{
    qDebug() << "[FENETRE MAIN] Synchronisation du stock";

    if (!m_gestionnaireStock) {
        QMessageBox::warning(this, "Erreur", "Gestionnaire stock non initialisé");
        return;
    }

    m_statusLabel->setText("🔗 Synchronisation...");

    if (m_gestionnaireStock->synchroniserStockSoldes()) {
        if (m_vueStock) {
            m_vueStock->actualiser();
        }
        QMessageBox::information(this, "✓ Succès", 
                                "Stock synchronisé avec succès");
        afficherAlertes();
    } else {
        QMessageBox::critical(this, "Erreur", 
                             "Erreur lors de la synchronisation:\n" + 
                             m_gestionnaireStock->obtenirDernierErreur());
    }

    m_statusLabel->setText("✓ Connecté");
}

void FenetreMain::onExporterDonnees()
{
    qDebug() << "[FENETRE MAIN] Export des données";
    QMessageBox::information(this, "Export", 
                            "Export des données (Fonctionnalité à implémenter)");
}

// ============================================================================
// EVENT - FERMETURE
// ============================================================================

void FenetreMain::closeEvent(QCloseEvent* event)
{
    qDebug() << "[FENETRE MAIN] Fermeture de l'application demandée";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirmation de Fermeture",
        "Êtes-vous sûr de vouloir fermer l'application?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        qDebug() << "[FENETRE MAIN] ✓ Fermeture confirmée";
        event->accept();
    } else {
        event->ignore();
    }
}