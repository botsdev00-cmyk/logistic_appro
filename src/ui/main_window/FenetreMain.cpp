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

FenetreMain::FenetreMain(QWidget* parent)
    : QMainWindow(parent),
      m_authService(std::make_unique<ServiceAuthentification>())
{
    setWindowTitle("SEMULIKI ERP - Système de Gestion Logistique");
    setWindowIcon(QIcon(":/images/icon.png"));
    
    // Configuration initiale
    configurerInterface();
    creerMenus();
    creerBarreOutils();
    creerVues();
    initialiserConnexions();
    verifierPermissions();
    
    // Taille par défaut
    resize(1400, 900);
    
    // Afficher le tableau de bord par défaut
    afficherVueTableau();
    
    // Afficher dans la barre de statut
    statusBar()->showMessage("Utilisateur: Connecté | SEMULIKI ERP v1.0.0");
}

FenetreMain::~FenetreMain()
{
}

void FenetreMain::configurerInterface()
{
    // Créer le widget central (stacked widget)
    m_stackedWidget = std::make_unique<QStackedWidget>();
    setCentralWidget(m_stackedWidget.get());
    
    // Appliquer le style
    setStyleSheet(
        "QMainWindow { background-color: #f5f5f5; }"
        "QMenuBar { background-color: #2c3e50; color: white; }"
        "QMenuBar::item:selected { background-color: #34495e; }"
        "QMenu { background-color: #ecf0f1; color: #2c3e50; }"
        "QMenu::item:selected { background-color: #3498db; color: white; }"
        "QToolBar { background-color: #34495e; border: none; }"
        "QToolBar::separator { background-color: #7f8c8d; }"
        "QStatusBar { background-color: #2c3e50; color: white; }"
    );
}

void FenetreMain::creerMenus()
{
    // Menu Fichier
    QMenu* menuFichier = menuBar()->addMenu("&Fichier");
    m_actionDeconnecter = menuFichier->addAction("&Déconnecter");
    m_actionDeconnecter->setShortcut(Qt::CTRL | Qt::Key_L);
    m_actionQuitter = menuFichier->addAction("&Quitter");
    m_actionQuitter->setShortcut(Qt::CTRL + Qt::Key_Q);
    
    menuFichier->addSeparator();
    connect(m_actionDeconnecter, &QAction::triggered, this, &FenetreMain::deconnecter);
    connect(m_actionQuitter, &QAction::triggered, this, &QApplication::quit);
    
    // Menu Logistique
    QMenu* menuLogistique = menuBar()->addMenu("&Logistique");
    m_actionStock = menuLogistique->addAction("Gestion &Stock");
    m_actionStock->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_actionRepartition = menuLogistique->addAction("&Répartitions");
    m_actionRepartition->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    
    connect(m_actionStock, &QAction::triggered, this, &FenetreMain::afficherVueStock);
    connect(m_actionRepartition, &QAction::triggered, this, &FenetreMain::afficherVueRepartition);
    
    // Menu Ventes & Clients
    QMenu* menuVentes = menuBar()->addMenu("&Ventes");
    m_actionVentes = menuVentes->addAction("&Ventes");
    m_actionVentes->setIcon(style()->standardIcon(QStyle::SP_DialogYesButton));
    m_actionClient = menuVentes->addAction("&Clients");
    m_actionClient->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    
    connect(m_actionVentes, &QAction::triggered, this, &FenetreMain::afficherVueVentes);
    connect(m_actionClient, &QAction::triggered, this, &FenetreMain::afficherVueClient);
    
    // Menu Finance
    QMenu* menuFinance = menuBar()->addMenu("&Finance");
    m_actionCredit = menuFinance->addAction("Gestion &Crédits");
    m_actionCaisse = menuFinance->addAction("Gestion &Caisse");
    
    connect(m_actionCredit, &QAction::triggered, this, &FenetreMain::afficherVueCredit);
    connect(m_actionCaisse, &QAction::triggered, this, &FenetreMain::afficherVueCaisse);
    
    // Menu Rapports
    QMenu* menuRapports = menuBar()->addMenu("&Rapports");
    m_actionRapport = menuRapports->addAction("Tableau de &Bord");
    m_actionRapport->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    
    connect(m_actionRapport, &QAction::triggered, this, &FenetreMain::afficherVueRapport);
    
    // Menu Aide
    QMenu* menuAide = menuBar()->addMenu("&Aide");
    m_actionAPropos = menuAide->addAction("&À Propos");
    
    connect(m_actionAPropos, &QAction::triggered, this, &FenetreMain::afficherAPropos);
}

void FenetreMain::creerBarreOutils()
{
    QToolBar* barreOutils = addToolBar("Barre d'outils principale");
    barreOutils->setMovable(false);
    barreOutils->setIconSize(QSize(24, 24));
    
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
}

void FenetreMain::creerVues()
{
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
}

void FenetreMain::initialiserConnexions()
{
    // Les connexions sont déjà faites dans creerMenus()
}

void FenetreMain::verifierPermissions()
{
    // Vérifier les rôles et activer/désactiver les actions
    // À implémenter selon les permissions
}

void FenetreMain::afficherVueTableau()
{
    m_stackedWidget->setCurrentWidget(m_vueTableau.get());
    statusBar()->showMessage("Affichage: Tableau de bord");
}

void FenetreMain::afficherVueStock()
{
    m_stackedWidget->setCurrentWidget(m_vueStock.get());
    statusBar()->showMessage("Affichage: Gestion du stock");
}

void FenetreMain::afficherVueRepartition()
{
    m_stackedWidget->setCurrentWidget(m_vueRepartition.get());
    statusBar()->showMessage("Affichage: Répartitions");
}

void FenetreMain::afficherVueVentes()
{
    m_stackedWidget->setCurrentWidget(m_vueVentes.get());
    statusBar()->showMessage("Affichage: Ventes");
}

void FenetreMain::afficherVueCredit()
{
    m_stackedWidget->setCurrentWidget(m_vueCredit.get());
    statusBar()->showMessage("Affichage: Gestion des crédits");
}

void FenetreMain::afficherVueCaisse()
{
    m_stackedWidget->setCurrentWidget(m_vueCaisse.get());
    statusBar()->showMessage("Affichage: Gestion de la caisse");
}

void FenetreMain::afficherVueRapport()
{
    m_stackedWidget->setCurrentWidget(m_vueRapport.get());
    statusBar()->showMessage("Affichage: Rapports et Analytics");
}

void FenetreMain::afficherVueClient()
{
    m_stackedWidget->setCurrentWidget(m_vueClient.get());
    statusBar()->showMessage("Affichage: Gestion des clients");
}

void FenetreMain::deconnecter()
{
    m_authService->logout();
    
    QMessageBox::information(this, "Déconnexion", 
        "Vous avez été déconnecté avec succès.\n"
        "L'application va se fermer.");
    
    QApplication::quit();
}

void FenetreMain::afficherAPropos()
{
    QMessageBox::about(this, "À Propos de SEMULIKI ERP",
        "SEMULIKI ERP v1.0.0\n\n"
        "Système de Gestion Logistique et Approvisionnement\n\n"
        "© 2026 SEMULIKI\n"
        "Tous droits réservés\n\n"
        "Développé avec Qt6 et PostgreSQL");
}

void FenetreMain::closeEvent(QCloseEvent* event)
{
    int resultat = QMessageBox::question(this, "Quitter",
        "Êtes-vous sûr de vouloir quitter l'application ?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (resultat == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}