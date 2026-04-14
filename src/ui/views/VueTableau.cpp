#include "VueTableau.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QGroupBox>
#include <QStyle>
#include <QApplication>
#include <QDebug>

VueTableau::VueTableau(QWidget* parent)
    : QWidget(parent)
{
    creerInterface();
    chargerDonnees();
    appliquerStyle();
    
    qDebug() << "[TABLEAU] Vue tableau de bord créée";
}

VueTableau::~VueTableau()
{
}

void VueTableau::creerInterface()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    layoutPrincipal->setSpacing(20);
    
    // ═════════════════════════════════════���═════════════════════
    // SECTION: En-tête avec bienvenue
    // ═══════════════════════════════════════════════════════════
    
    QHBoxLayout* layoutEntete = new QHBoxLayout();
    
    m_labelBienvenue = new QLabel("🏠 Tableau de Bord SEMULIKI ERP");
    m_labelBienvenue->setStyleSheet("font-size: 28px; font-weight: bold; color: #2c3e50;");
    
    m_labelDate = new QLabel(QDateTime::currentDateTime().toString("dddd d MMMM yyyy - hh:mm"));
    m_labelDate->setStyleSheet("font-size: 14px; color: #7f8c8d;");
    m_labelDate->setAlignment(Qt::AlignRight);
    
    layoutEntete->addWidget(m_labelBienvenue);
    layoutEntete->addWidget(m_labelDate, 1, Qt::AlignRight);
    
    layoutPrincipal->addLayout(layoutEntete);
    layoutPrincipal->addSpacing(10);
    
    // ═══════════════════════════════════════════════════════════
    // SECTION: Cartes KPI
    // ═══════════════════════════════════════════════════════════
    
    creerCarteKPI();
    
    // ═══════════════════════════════════════════════════════════
    // SECTION: Raccourcis vers modules
    // ═══════════════════════════════════════════════════════════
    
    creerRaccourcis();
    
    // ═══════════════════════════════════════════════════════════
    // SECTION: Dernières activités (stub)
    // ═══════════════════════════════════════════════════════════
    
    creerDernieresActivites();
    
    // Ajouter un stretch pour remplir l'espace vide
    layoutPrincipal->addStretch();
}

void VueTableau::creerCarteKPI()
{
    // Groupe des KPIs
    QGroupBox* groupKPI = new QGroupBox("Indicateurs Clés de Performance");
    groupKPI->setStyleSheet(
        "QGroupBox { "
        "    border: 2px solid #3498db; "
        "    border-radius: 5px; "
        "    margin-top: 10px; "
        "    padding-top: 10px; "
        "    font-weight: bold; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    left: 10px; "
        "    padding: 0 3px 0 3px; "
        "}"
    );
    
    QGridLayout* layoutKPI = new QGridLayout(groupKPI);
    layoutKPI->setSpacing(15);
    layoutKPI->setContentsMargins(15, 20, 15, 15);
    
    // Carte 1: Stock Total
    QVBoxLayout* cardStock = new QVBoxLayout();
    QLabel* labelStockTitre = new QLabel("📦 Stock Total");
    labelStockTitre->setStyleSheet("font-weight: bold; font-size: 12px; color: #7f8c8d;");
    m_stockTotal = new QLabel("0 unités");
    m_stockTotal->setStyleSheet("font-size: 24px; font-weight: bold; color: #27ae60;");
    cardStock->addWidget(labelStockTitre);
    cardStock->addWidget(m_stockTotal);
    cardStock->addStretch();
    
    // Carte 2: Ventes du jour
    QVBoxLayout* cardVentes = new QVBoxLayout();
    QLabel* labelVentesTitre = new QLabel("💰 Ventes du Jour");
    labelVentesTitre->setStyleSheet("font-weight: bold; font-size: 12px; color: #7f8c8d;");
    m_ventesJour = new QLabel("0 XOF");
    m_ventesJour->setStyleSheet("font-size: 24px; font-weight: bold; color: #2980b9;");
    cardVentes->addWidget(labelVentesTitre);
    cardVentes->addWidget(m_ventesJour);
    cardVentes->addStretch();
    
    // Carte 3: Clients Actifs
    QVBoxLayout* cardClients = new QVBoxLayout();
    QLabel* labelClientsTitre = new QLabel("👥 Clients Actifs");
    labelClientsTitre->setStyleSheet("font-weight: bold; font-size: 12px; color: #7f8c8d;");
    m_clientsActifs = new QLabel("0 clients");
    m_clientsActifs->setStyleSheet("font-size: 24px; font-weight: bold; color: #e74c3c;");
    cardClients->addWidget(labelClientsTitre);
    cardClients->addWidget(m_clientsActifs);
    cardClients->addStretch();
    
    // Carte 4: Crédits en cours
    QVBoxLayout* cardCredits = new QVBoxLayout();
    QLabel* labelCreditsTitre = new QLabel("📊 Crédits en Cours");
    labelCreditsTitre->setStyleSheet("font-weight: bold; font-size: 12px; color: #7f8c8d;");
    m_creditsEnCours = new QLabel("0 XOF");
    m_creditsEnCours->setStyleSheet("font-size: 24px; font-weight: bold; color: #f39c12;");
    cardCredits->addWidget(labelCreditsTitre);
    cardCredits->addWidget(m_creditsEnCours);
    cardCredits->addStretch();
    
    // Ajouter les cartes au layout
    QWidget* widgetStock = new QWidget();
    widgetStock->setLayout(cardStock);
    QWidget* widgetVentes = new QWidget();
    widgetVentes->setLayout(cardVentes);
    QWidget* widgetClients = new QWidget();
    widgetClients->setLayout(cardClients);
    QWidget* widgetCredits = new QWidget();
    widgetCredits->setLayout(cardCredits);
    
    layoutKPI->addWidget(widgetStock, 0, 0);
    layoutKPI->addWidget(widgetVentes, 0, 1);
    layoutKPI->addWidget(widgetClients, 0, 2);
    layoutKPI->addWidget(widgetCredits, 0, 3);
    
    // Ajouter le groupe au layout principal
    QVBoxLayout* layoutMain = qobject_cast<QVBoxLayout*>(layout());
    if (layoutMain) {
        layoutMain->insertWidget(layoutMain->count() - 1, groupKPI);
    }
}

void VueTableau::creerRaccourcis()
{
    // Groupe des raccourcis
    QGroupBox* groupRaccourcis = new QGroupBox("Accès Rapide aux Modules");
    groupRaccourcis->setStyleSheet(
        "QGroupBox { "
        "    border: 2px solid #27ae60; "
        "    border-radius: 5px; "
        "    margin-top: 10px; "
        "    padding-top: 10px; "
        "    font-weight: bold; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    left: 10px; "
        "    padding: 0 3px 0 3px; "
        "}"
    );
    
    QHBoxLayout* layoutRaccourcis = new QHBoxLayout(groupRaccourcis);
    layoutRaccourcis->setSpacing(10);
    layoutRaccourcis->setContentsMargins(15, 20, 15, 15);
    
    // Bouton Stock
    m_btnStock = new QPushButton("📦 Gestion Stock");
    m_btnStock->setMinimumHeight(60);
    m_btnStock->setStyleSheet(
        "QPushButton { "
        "    background-color: #3498db; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 12px; "
        "} "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:pressed { background-color: #21618c; }"
    );
    connect(m_btnStock, &QPushButton::clicked, this, &VueTableau::allerVersStock);
    
    // Bouton Répartition
    m_btnRepartition = new QPushButton("🚚 Répartitions");
    m_btnRepartition->setMinimumHeight(60);
    m_btnRepartition->setStyleSheet(
        "QPushButton { "
        "    background-color: #9b59b6; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 12px; "
        "} "
        "QPushButton:hover { background-color: #8e44ad; } "
        "QPushButton:pressed { background-color: #76448a; }"
    );
    connect(m_btnRepartition, &QPushButton::clicked, this, &VueTableau::allerVersRepartition);
    
    // Bouton Ventes
    m_btnVentes = new QPushButton("💳 Ventes");
    m_btnVentes->setMinimumHeight(60);
    m_btnVentes->setStyleSheet(
        "QPushButton { "
        "    background-color: #27ae60; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 12px; "
        "} "
        "QPushButton:hover { background-color: #229954; } "
        "QPushButton:pressed { background-color: #1e8449; }"
    );
    connect(m_btnVentes, &QPushButton::clicked, this, &VueTableau::allerVersVentes);
    
    // Bouton Crédits
    m_btnCredit = new QPushButton("💰 Crédits");
    m_btnCredit->setMinimumHeight(60);
    m_btnCredit->setStyleSheet(
        "QPushButton { "
        "    background-color: #f39c12; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 12px; "
        "} "
        "QPushButton:hover { background-color: #e67e22; } "
        "QPushButton:pressed { background-color: #d35400; }"
    );
    connect(m_btnCredit, &QPushButton::clicked, this, &VueTableau::allerVersCredit);
    
    // Bouton Caisse
    m_btnCaisse = new QPushButton("🏦 Caisse");
    m_btnCaisse->setMinimumHeight(60);
    m_btnCaisse->setStyleSheet(
        "QPushButton { "
        "    background-color: #e74c3c; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 12px; "
        "} "
        "QPushButton:hover { background-color: #c0392b; } "
        "QPushButton:pressed { background-color: #a93226; }"
    );
    connect(m_btnCaisse, &QPushButton::clicked, this, &VueTableau::allerVersCaisse);
    
    // Bouton Clients
    m_btnClients = new QPushButton("👥 Clients");
    m_btnClients->setMinimumHeight(60);
    m_btnClients->setStyleSheet(
        "QPushButton { "
        "    background-color: #1abc9c; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 12px; "
        "} "
        "QPushButton:hover { background-color: #16a085; } "
        "QPushButton:pressed { background-color: #138d75; }"
    );
    connect(m_btnClients, &QPushButton::clicked, this, &VueTableau::allerVersClients);
    
    layoutRaccourcis->addWidget(m_btnStock);
    layoutRaccourcis->addWidget(m_btnRepartition);
    layoutRaccourcis->addWidget(m_btnVentes);
    layoutRaccourcis->addWidget(m_btnCredit);
    layoutRaccourcis->addWidget(m_btnCaisse);
    layoutRaccourcis->addWidget(m_btnClients);
    
    // Ajouter le groupe au layout principal
    QVBoxLayout* layoutMain = qobject_cast<QVBoxLayout*>(layout());
    if (layoutMain) {
        layoutMain->insertWidget(layoutMain->count() - 1, groupRaccourcis);
    }
}

void VueTableau::creerDernieresActivites()
{
    // Groupe des dernières activités
    QGroupBox* groupActivites = new QGroupBox("Dernières Activités");
    groupActivites->setStyleSheet(
        "QGroupBox { "
        "    border: 2px solid #34495e; "
        "    border-radius: 5px; "
        "    margin-top: 10px; "
        "    padding-top: 10px; "
        "    font-weight: bold; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    left: 10px; "
        "    padding: 0 3px 0 3px; "
        "}"
    );
    
    QVBoxLayout* layoutActivites = new QVBoxLayout(groupActivites);
    
    QLabel* labelActivite1 = new QLabel("✓ Admin s'est connecté le " + QDateTime::currentDateTime().toString("dd/MM/yyyy à hh:mm"));
    labelActivite1->setStyleSheet("color: #27ae60; padding: 5px;");
    
    QLabel* labelActivite2 = new QLabel("ℹ️ Système prêt pour les opérations logistiques");
    labelActivite2->setStyleSheet("color: #3498db; padding: 5px;");
    
    layoutActivites->addWidget(labelActivite1);
    layoutActivites->addWidget(labelActivite2);
    layoutActivites->addStretch();
    
    // Ajouter le groupe au layout principal
    QVBoxLayout* layoutMain = qobject_cast<QVBoxLayout*>(layout());
    if (layoutMain) {
        layoutMain->insertWidget(layoutMain->count() - 1, groupActivites);
    }
}

void VueTableau::chargerDonnees()
{
    // Charger les données depuis la base de données
    // Pour l'instant, afficher des valeurs par défaut
    
    m_stockTotal->setText("0 unités");
    m_ventesJour->setText("0 XOF");
    m_clientsActifs->setText("0 clients");
    m_creditsEnCours->setText("0 XOF");
    
    qDebug() << "[TABLEAU] Données chargées";
}

void VueTableau::appliquerStyle()
{
    setStyleSheet(
        "VueTableau { background-color: #ecf0f1; }"
        "QGroupBox { background-color: white; }"
        "QLabel { color: #2c3e50; }"
    );
}

void VueTableau::allerVersStock()
{
    qDebug() << "[TABLEAU] Navigation vers Gestion Stock";
}

void VueTableau::allerVersRepartition()
{
    qDebug() << "[TABLEAU] Navigation vers Répartitions";
}

void VueTableau::allerVersVentes()
{
    qDebug() << "[TABLEAU] Navigation vers Ventes";
}

void VueTableau::allerVersCredit()
{
    qDebug() << "[TABLEAU] Navigation vers Crédits";
}

void VueTableau::allerVersCaisse()
{
    qDebug() << "[TABLEAU] Navigation vers Caisse";
}

void VueTableau::allerVersClients()
{
    qDebug() << "[TABLEAU] Navigation vers Clients";
}