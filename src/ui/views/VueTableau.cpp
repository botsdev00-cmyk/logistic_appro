#include "VueTableau.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLCDNumber>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

VueTableau::VueTableau(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
    mettreAJourStatistiques();
}

VueTableau::~VueTableau()
{
}

void VueTableau::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(15);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Tableau de bord");
    labelTitre->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Statistiques principales
    QHBoxLayout* layoutStats = new QHBoxLayout();
    
    // Stock total
    QGroupBox* groupStock = new QGroupBox("Stock total");
    QVBoxLayout* layoutStock = new QVBoxLayout(groupStock);
    m_lcdStockTotal = std::make_unique<QLCDNumber>();
    m_lcdStockTotal->setDigitCount(5);
    m_lcdStockTotal->setStyleSheet("QLCDNumber { color: #27ae60; background-color: #f0f0f0; }");
    layoutStock->addWidget(m_lcdStockTotal.get());
    layoutStats->addWidget(groupStock);
    
    // Ventes du jour
    QGroupBox* groupVentes = new QGroupBox("Ventes aujourd'hui (FCFA)");
    QVBoxLayout* layoutVentes = new QVBoxLayout(groupVentes);
    m_lcdVentesJour = std::make_unique<QLCDNumber>();
    m_lcdVentesJour->setDigitCount(8);
    m_lcdVentesJour->setStyleSheet("QLCDNumber { color: #3498db; background-color: #f0f0f0; }");
    layoutVentes->addWidget(m_lcdVentesJour.get());
    layoutStats->addWidget(groupVentes);
    
    // Crédits en cours
    QGroupBox* groupCredits = new QGroupBox("Crédits en cours (FCFA)");
    QVBoxLayout* layoutCredits = new QVBoxLayout(groupCredits);
    m_lcdCreditsEnCours = std::make_unique<QLCDNumber>();
    m_lcdCreditsEnCours->setDigitCount(8);
    m_lcdCreditsEnCours->setStyleSheet("QLCDNumber { color: #f39c12; background-color: #f0f0f0; }");
    layoutCredits->addWidget(m_lcdCreditsEnCours.get());
    layoutStats->addWidget(groupCredits);
    
    // Crédits en retard
    QGroupBox* groupRetard = new QGroupBox("Crédits en retard (FCFA)");
    QVBoxLayout* layoutRetard = new QVBoxLayout(groupRetard);
    m_lcdCreditEnRetard = std::make_unique<QLCDNumber>();
    m_lcdCreditEnRetard->setDigitCount(8);
    m_lcdCreditEnRetard->setStyleSheet("QLCDNumber { color: #e74c3c; background-color: #f0f0f0; }");
    layoutRetard->addWidget(m_lcdCreditEnRetard.get());
    layoutStats->addWidget(groupRetard);
    
    layoutPrincipal->addLayout(layoutStats);
    
    // Alertes
    QGroupBox* groupAlertes = new QGroupBox("Alertes importantes");
    QVBoxLayout* layoutAlertes = new QVBoxLayout(groupAlertes);
    
    m_labelStockBas = std::make_unique<QLabel>();
    m_labelStockBas->setStyleSheet("color: #f39c12; font-weight: bold;");
    m_labelStockBas->setText("⚠ 3 produits avec stock bas");
    layoutAlertes->addWidget(m_labelStockBas.get());
    
    m_labelAlertes = std::make_unique<QLabel>();
    m_labelAlertes->setStyleSheet("color: #e74c3c; font-weight: bold;");
    m_labelAlertes->setText("⚠ 2 crédits en retard de plus de 5 jours");
    layoutAlertes->addWidget(m_labelAlertes.get());
    
    layoutPrincipal->addWidget(groupAlertes);
    
    // Barre de progression
    m_progressbar = std::make_unique<QProgressBar>();
    m_progressbar->setValue(75);
    m_progressbar->setFormat("Objectif journalier: %v%");
    layoutPrincipal->addWidget(m_progressbar.get());
    
    // Boutons d'action
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    QPushButton* btnRefresh = new QPushButton("Rafraîchir");
    connect(btnRefresh, &QPushButton::clicked, this, &VueTableau::rafraichirDonnees);
    layoutBoutons->addWidget(btnRefresh);
    
    QPushButton* btnAlertes = new QPushButton("Voir toutes les alertes");
    connect(btnAlertes, &QPushButton::clicked, this, &VueTableau::afficherAlertes);
    layoutBoutons->addWidget(btnAlertes);
    
    layoutPrincipal->addLayout(layoutBoutons);
    layoutPrincipal->addStretch();
}

void VueTableau::initialiserConnexions()
{
    // Timer pour rafraîchir automatiquement chaque 5 minutes
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VueTableau::rafraichirDonnees);
    timer->start(300000); // 5 minutes
}

void VueTableau::mettreAJourStatistiques()
{
    // À implémenter avec les données réelles
    m_lcdStockTotal->display(4250);
    m_lcdVentesJour->display(25000);
    m_lcdCreditsEnCours->display(500000);
    m_lcdCreditEnRetard->display(150000);
}

void VueTableau::rafraichirDonnees()
{
    mettreAJourStatistiques();
}

void VueTableau::afficherAlertes()
{
    // À implémenter
}