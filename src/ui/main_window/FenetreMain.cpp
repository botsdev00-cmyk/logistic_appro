#include "FenetreMain.h"
#include "../views/VueTableau.h"
#include "../views/VueStock.h"
#include "../views/VueRepartition.h"
#include "../views/VueVentes.h"
#include "../views/VueCredit.h"
#include "../views/VueCaisse.h"
#include "../views/VueRapport.h"
#include "../views/VueClient.h"
#include "../../business/services/ServiceAuthentification.h"
#include "../../core/entities/Utilisateur.h"
#include "../../data/repositories/RepositoryUtilisateur.h"
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyle>
#include <QDebug>
#include <QCloseEvent>
#include <QLabel>
#include <QSize>
#include <QDateTime>
#include <QTimer>
#include <QSplitter>
#include <QVBoxLayout>

FenetreMain::FenetreMain(QWidget* parent)
    : QMainWindow(parent),
      m_authService(std::make_unique<ServiceAuthentification>()),
      m_labelUtilisateur(nullptr),
      m_labelHeure(nullptr),
      m_labelStatut(nullptr)
{
    qDebug() << "[MAIN] Initialisation de FenetreMain...";
    
    setWindowTitle("SEMULIKI ERP - Système de Gestion Logistique");
    setWindowIcon(QIcon(":/images/icon.png"));
    
    // Configuration initiale dans l'ordre correct
    configurerInterface();
    creerVues();
    creerMenus();
    creerBarreOutils();
    creerBarreStatut();
    initialiserConnexions();
    verifierPermissions();
    afficherInfosUtilisateur();
    
    // Taille par défaut
    resize(1600, 1000);
    
    // Centrer la fenêtre
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Afficher le tableau de bord par défaut
    afficherVueTableau();
    
    qDebug() << "[MAIN] FenetreMain initialisée avec succès";
}

FenetreMain::~FenetreMain()
{
    qDebug() << "[MAIN] Destruction de FenetreMain";
}

void FenetreMain::configurerInterface()
{
    qDebug() << "[MAIN] Configuration de l'interface...";
    
    // Créer le widget central (stacked widget)
    m_stackedWidget = std::make_unique<QStackedWidget>();
    setCentralWidget(m_stackedWidget.get());
    
    // Appliquer le style global
    setStyleSheet(
        // Main Window
        "QMainWindow { background-color: #f5f5f5; }"
        
        // Menu Bar
        "QMenuBar { "
        "    background-color: #2c3e50; "
        "    color: white; "
        "    border-bottom: 1px solid #34495e; "
        "} "
        "QMenuBar::item:selected { "
        "    background-color: #34495e; "
        "} "
        
        // Menus
        "QMenu { "
        "    background-color: #ecf0f1; "
        "    color: #2c3e50; "
        "    border: 1px solid #bdc3c7; "
        "} "
        "QMenu::item:selected { "
        "    background-color: #3498db; "
        "    color: white; "
        "} "
        
        // Tool Bar
        "QToolBar { "
        "    background-color: #34495e; "
        "    border: none; "
        "    border-bottom: 1px solid #2c3e50; "
        "    spacing: 0px; "
        "} "
        "QToolBar::separator { "
        "    background-color: #7f8c8d; "
        "    width: 1px; "
        "    margin: 5px; "
        "} "
        
        // Status Bar
        "QStatusBar { "
        "    background-color: #2c3e50; "
        "    color: white; "
        "    border-top: 1px solid #34495e; "
        "} "
        "QStatusBar::item { "
        "    border: none; "
        "} "
    );
}

void FenetreMain::creerMenus()
{
    qDebug() << "[MAIN] Création des menus...";
    
    // ═══════════════════════════════════════════════════════════
    // MENU FICHIER
    // ═══════════════════════════════════════════════════════════
    
    QMenu* menuFichier = menuBar()->addMenu("&Fichier");
    menuFichier->setToolTipsVisible(true);
    
    m_actionTableau = menuFichier->addAction("🏠 &Tableau de Bord");
    m_actionTableau->setShortcut(Qt::CTRL + Qt::Key_Home);
    m_actionTableau->setStatusTip("Afficher le tableau de bord principal");
    connect(m_actionTableau, &QAction::triggered, this, &FenetreMain::afficherVueTableau);
    
    menuFichier->addSeparator();
    
    m_actionDeconnecter = menuFichier->addAction("🚪 &Déconnecter");
    m_actionDeconnecter->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionDeconnecter->setStatusTip("Se déconnecter de l'application");
    connect(m_actionDeconnecter, &QAction::triggered, this, &FenetreMain::deconnecter);
    
    m_actionQuitter = menuFichier->addAction("❌ &Quitter");
    m_actionQuitter->setShortcut(Qt::CTRL + Qt::Key_Q);
    m_actionQuitter->setStatusTip("Fermer l'application");
    connect(m_actionQuitter, &QAction::triggered, this, &QApplication::quit);
    
    // ═════════════════════════════════════��═════════════════════
    // MENU LOGISTIQUE
    // ═══════════════════════════════════════════════════════════
    
    QMenu* menuLogistique = menuBar()->addMenu("&Logistique");
    
    m_actionStock = menuLogistique->addAction("📦 Gestion &Stock");
    m_actionStock->setShortcut(Qt::CTRL + Qt::Key_S);
    m_actionStock->setStatusTip("Gérer les stocks de produits");
    m_actionStock->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    connect(m_actionStock, &QAction::triggered, this, &FenetreMain::afficherVueStock);
    
    m_actionRepartition = menuLogistique->addAction("🚚 &Répartitions");
    m_actionRepartition->setShortcut(Qt::CTRL + Qt::Key_R);
    m_actionRepartition->setStatusTip("Gérer les répartitions");
    m_actionRepartition->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    connect(m_actionRepartition, &QAction::triggered, this, &FenetreMain::afficherVueRepartition);
    
    // ═══════════════════════════════════════════════════════════
    // MENU VENTES
    // ═══════════════════════════════════════════════════════════
    
    QMenu* menuVentes = menuBar()->addMenu("&Ventes");
    
    m_actionVentes = menuVentes->addAction("💳 &Ventes");
    m_actionVentes->setShortcut(Qt::CTRL + Qt::Key_V);
    m_actionVentes->setStatusTip("Enregistrer les ventes");
    m_actionVentes->setIcon(style()->standardIcon(QStyle::SP_DialogYesButton));
    connect(m_actionVentes, &QAction::triggered, this, &FenetreMain::afficherVueVentes);
    
    m_actionClient = menuVentes->addAction("👥 &Clients");
    m_actionClient->setShortcut(Qt::CTRL + Qt::Key_C);
    m_actionClient->setStatusTip("Gérer les clients");
    m_actionClient->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    connect(m_actionClient, &QAction::triggered, this, &FenetreMain::afficherVueClient);
    
    // ═══════════════════════════════════════════════════════════
    // MENU FINANCE
    // ═══════════════════════════════════════════════════════════
    
    QMenu* menuFinance = menuBar()->addMenu("&Finance");
    
    m_actionCredit = menuFinance->addAction("💰 Gestion &Crédits");
    m_actionCredit->setShortcut(Qt::CTRL + Qt::Key_K);
    m_actionCredit->setStatusTip("Gérer les crédits clients");
    connect(m_actionCredit, &QAction::triggered, this, &FenetreMain::afficherVueCredit);
    
    m_actionCaisse = menuFinance->addAction("🏦 Gestion &Caisse");
    m_actionCaisse->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actionCaisse->setStatusTip("Gérer la caisse et les encaissements");
    connect(m_actionCaisse, &QAction::triggered, this, &FenetreMain::afficherVueCaisse);
    
    // ═══════════════════════════════════════════════════════════
    // MENU RAPPORTS
    // ══════════════════════════════════════════════════════���════
    
    QMenu* menuRapports = menuBar()->addMenu("&Rapports");
    
    m_actionRapport = menuRapports->addAction("📊 Tableau de &Bord Analytique");
    m_actionRapport->setShortcut(Qt::CTRL + Qt::Key_D);
    m_actionRapport->setStatusTip("Voir les rapports et analytics");
    m_actionRapport->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    connect(m_actionRapport, &QAction::triggered, this, &FenetreMain::afficherVueRapport);
    
    // ═══════════════════════════════════════════════════════════
    // MENU AIDE
    // ═══════════════════════════════════════════════════════════
    
    QMenu* menuAide = menuBar()->addMenu("&Aide");
    
    m_actionAPropos = menuAide->addAction("ℹ️ &À Propos");
    m_actionAPropos->setStatusTip("Informations sur SEMULIKI ERP");
    connect(m_actionAPropos, &QAction::triggered, this, &FenetreMain::afficherAPropos);
    
    qDebug() << "[MAIN] Menus créés avec succès";
}

void FenetreMain::creerBarreOutils()
{
    qDebug() << "[MAIN] Création de la barre d'outils...";
    
    QToolBar* barreOutils = addToolBar("Barre d'outils principale");
    barreOutils->setObjectName("ToolBarPrincipale");
    barreOutils->setMovable(false);
    barreOutils->setIconSize(QSize(24, 24));
    barreOutils->setStyleSheet(
        "QToolBar { spacing: 2px; }"
        "QToolButton { padding: 4px; }"
        "QToolButton:hover { background-color: rgba(255,255,255,0.2); border-radius: 3px; }"
    );
    
    // Ajouter les actions
    barreOutils->addAction(m_actionTableau);
    barreOutils->addSeparator();
    barreOutils->addAction(m_actionStock);
    barreOutils->addAction(m_actionRepartition);
    barreOutils->addSeparator();
    barreOutils->addAction(m_actionVentes);
    barreOutils->addAction(m_actionClient);
    barreOutils->addSeparator();
    barreOutils->addAction(m_actionCredit);
    barreOutils->addAction(m_actionCaisse);
    barreOutils->addSeparator();
    barreOutils->addAction(m_actionRapport);
    barreOutils->addSeparator();
    barreOutils->addAction(m_actionDeconnecter);
    
    qDebug() << "[MAIN] Barre d'outils créée";
}

void FenetreMain::creerBarreStatut()
{
    qDebug() << "[MAIN] Création de la barre de statut...";
    
    // Label utilisateur
    m_labelUtilisateur = new QLabel("Utilisateur: ");
    m_labelUtilisateur->setStyleSheet("QLabel { margin-left: 10px; }");
    
    // Label statut
    m_labelStatut = new QLabel("Prêt");
    m_labelStatut->setStyleSheet("QLabel { margin: 0px 10px; }");
    
    // Label heure
    m_labelHeure = new QLabel(QDateTime::currentDateTime().toString("hh:mm:ss"));
    m_labelHeure->setStyleSheet("QLabel { margin-right: 10px; }");
    
    // Ajouter les labels à la barre de statut
    statusBar()->addWidget(m_labelUtilisateur);
    statusBar()->addWidget(m_labelStatut, 1);
    statusBar()->addPermanentWidget(m_labelHeure);
    
    // Mettre à jour l'heure toutes les secondes
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        m_labelHeure->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    });
    timer->start(1000);
    
    qDebug() << "[MAIN] Barre de statut créée";
}

void FenetreMain::creerVues()
{
    qDebug() << "[MAIN] Création des vues...";
    
    m_vueTableau = std::make_unique<VueTableau>();
    m_vueStock = std::make_unique<VueStock>();
    m_vueRepartition = std::make_unique<VueRepartition>();
    m_vueVentes = std::make_unique<VueVentes>();
    m_vueCredit = std::make_unique<VueCredit>();
    m_vueCaisse = std::make_unique<VueCaisse>();
    m_vueRapport = std::make_unique<VueRapport>();
    m_vueClient = std::make_unique<VueClient>();
    
    // Ajouter les vues au stacked widget
    m_stackedWidget->addWidget(m_vueTableau.get());
    m_stackedWidget->addWidget(m_vueStock.get());
    m_stackedWidget->addWidget(m_vueRepartition.get());
    m_stackedWidget->addWidget(m_vueVentes.get());
    m_stackedWidget->addWidget(m_vueCredit.get());
    m_stackedWidget->addWidget(m_vueCaisse.get());
    m_stackedWidget->addWidget(m_vueRapport.get());
    m_stackedWidget->addWidget(m_vueClient.get());
    
    qDebug() << "[MAIN] Toutes les vues créées";
}

void FenetreMain::initialiserConnexions()
{
    qDebug() << "[MAIN] Initialisation des connexions...";
    // Les connexions sont déjà faites dans creerMenus()
}

void FenetreMain::verifierPermissions()
{
    qDebug() << "[MAIN] Vérification des permissions...";
    
    // Pour l'instant, tous les modules sont accessibles pour l'admin
    // À personnaliser selon les rôles et permissions
    
    if (m_authService && m_authService->isAuthenticated()) {
        Utilisateur* user = m_authService->getCurrentUser();
        if (user) {
            qDebug() << "[MAIN] Utilisateur connecté:" << user->getNomUtilisateur();
        }
    }
}

void FenetreMain::afficherInfosUtilisateur()
{
    qDebug() << "[MAIN] Affichage des infos utilisateur...";
    
    if (m_authService && m_authService->isAuthenticated()) {
        Utilisateur* user = m_authService->getCurrentUser();
        if (user && m_labelUtilisateur) {
            QString nomComplet = user->getNomComplet();
            m_labelUtilisateur->setText(QString("👤 Utilisateur: %1").arg(nomComplet));
            qDebug() << "[MAIN] Infos utilisateur affichées:" << nomComplet;
        }
    }
}

// ═══════════════════════════════════════════════════════════
// SLOTS - Affichage des vues
// ═══════════════════════════════════════════════════════════

void FenetreMain::afficherVueTableau()
{
    m_stackedWidget->setCurrentWidget(m_vueTableau.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Tableau de Bord");
    qDebug() << "[MAIN] Affichage VueTableau";
}

void FenetreMain::afficherVueStock()
{
    m_stackedWidget->setCurrentWidget(m_vueStock.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Gestion du Stock");
    qDebug() << "[MAIN] Affichage VueStock";
}

void FenetreMain::afficherVueRepartition()
{
    m_stackedWidget->setCurrentWidget(m_vueRepartition.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Répartitions");
    qDebug() << "[MAIN] Affichage VueRepartition";
}

void FenetreMain::afficherVueVentes()
{
    m_stackedWidget->setCurrentWidget(m_vueVentes.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Ventes");
    qDebug() << "[MAIN] Affichage VueVentes";
}

void FenetreMain::afficherVueCredit()
{
    m_stackedWidget->setCurrentWidget(m_vueCredit.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Crédits");
    qDebug() << "[MAIN] Affichage VueCredit";
}

void FenetreMain::afficherVueCaisse()
{
    m_stackedWidget->setCurrentWidget(m_vueCaisse.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Caisse");
    qDebug() << "[MAIN] Affichage VueCaisse";
}

void FenetreMain::afficherVueRapport()
{
    m_stackedWidget->setCurrentWidget(m_vueRapport.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Rapports & Analytics");
    qDebug() << "[MAIN] Affichage VueRapport";
}

void FenetreMain::afficherVueClient()
{
    m_stackedWidget->setCurrentWidget(m_vueClient.get());
    if (m_labelStatut) m_labelStatut->setText("Affichage: Clients");
    qDebug() << "[MAIN] Affichage VueClient";
}

void FenetreMain::deconnecter()
{
    qDebug() << "[MAIN] Déconnexion de l'utilisateur...";
    
    m_authService->logout();
    
    QMessageBox::information(this, "Déconnexion", 
        "Vous avez été déconnecté avec succès.\n\n"
        "L'application va se fermer.\n\n"
        "Merci d'avoir utilisé SEMULIKI ERP !");
    
    QApplication::quit();
}

void FenetreMain::afficherAPropos()
{
    qDebug() << "[MAIN] Affichage de la fenêtre À Propos";
    
    QMessageBox::about(this, "À Propos de SEMULIKI ERP",
        "╔════════════════════════════════════════════════════════════╗\n"
        "║                    SEMULIKI ERP v1.0.0                     ║\n"
        "║                                                            ║\n"
        "║  Système de Gestion Logistique et Approvisionnement      ║\n"
        "║                                                            ║\n"
        "║  • Gestion complète des stocks                            ║\n"
        "║  • Suivi des répartitions                                 ║\n"
        "║  • Enregistrement des ventes                              ║\n"
        "║  • Gestion des crédits clients                            ║\n"
        "║  • Contrôle de caisse                                     ║\n"
        "║  • Rapports et analytics                                  ║\n"
        "║                                                            ║\n"
        "║  © 2026 SEMULIKI                                          ║\n"
        "║  Tous droits réservés                                     ║\n"
        "║                                                            ║\n"
        "║  Développé avec Qt6 et PostgreSQL                         ║\n"
        "║  Architecture : Clean Architecture + MVVM                 ║\n"
        "╚════════════════════════════════════════════════════════════╝"
    );
}

void FenetreMain::closeEvent(QCloseEvent* event)
{
    qDebug() << "[MAIN] Fermeture de l'application...";
    
    int resultat = QMessageBox::question(this, "Quitter l'application",
        "Êtes-vous sûr de vouloir quitter SEMULIKI ERP ?\n\n"
        "Tous les travaux non sauvegardés seront perdus.",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (resultat == QMessageBox::Yes) {
        qDebug() << "[MAIN] Application fermée par l'utilisateur";
        event->accept();
    } else {
        event->ignore();
    }
}