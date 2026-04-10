#include "VueCredit.h"
#include "../widgets/TableauCredit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

VueCredit::VueCredit(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueCredit::~VueCredit()
{
}

void VueCredit::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(10);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Gestion des Crédits");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Alertes
    QGroupBox* groupAlertes = new QGroupBox("Alertes");
    QVBoxLayout* layoutAlertes = new QVBoxLayout(groupAlertes);
    QLabel* labelAlertes = new QLabel("⚠ 5 crédits en retard total: 250 000 FCFA");
    labelAlertes->setStyleSheet("color: #e74c3c; font-weight: bold;");
    layoutAlertes->addWidget(labelAlertes);
    layoutPrincipal->addWidget(groupAlertes);
    
    // Tableau
    m_tableauCredit = std::make_unique<TableauCredit>();
    layoutPrincipal->addWidget(m_tableauCredit.get());
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    m_btnEnRetard = std::make_unique<QPushButton>("Crédits en retard");
    connect(m_btnEnRetard.get(), &QPushButton::clicked, this, &VueCredit::afficherCreditsEnRetard);
    layoutBoutons->addWidget(m_btnEnRetard.get());
    
    m_btnTous = std::make_unique<QPushButton>("Tous les crédits");
    connect(m_btnTous.get(), &QPushButton::clicked, this, &VueCredit::afficherTousLesCredits);
    layoutBoutons->addWidget(m_btnTous.get());
    
    QPushButton* btnPaiement = new QPushButton("Enregistrer paiement");
    connect(btnPaiement, &QPushButton::clicked, this, &VueCredit::enregistrerPaiement);
    layoutBoutons->addWidget(btnPaiement);
    
    QPushButton* btnRappels = new QPushButton("Envoyer rappels");
    connect(btnRappels, &QPushButton::clicked, this, &VueCredit::envoyerRappels);
    layoutBoutons->addWidget(btnRappels);
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void VueCredit::initialiserConnexions()
{
    m_tableauCredit->chargerDonnees();
}

void VueCredit::afficherCreditsEnRetard()
{
    m_tableauCredit->afficherCreditsEnRetard();
    m_btnEnRetard->setStyleSheet("background-color: #3498db; color: white;");
    m_btnTous->setStyleSheet("");
}

void VueCredit::afficherTousLesCredits()
{
    m_tableauCredit->afficherTousLesCredits();
    m_btnTous->setStyleSheet("background-color: #3498db; color: white;");
    m_btnEnRetard->setStyleSheet("");
}

void VueCredit::enregistrerPaiement()
{
    QMessageBox::information(this, "Paiement", "Fonctionnalité en cours de développement");
}

void VueCredit::envoyerRappels()
{
    QMessageBox::information(this, "Rappels", "Rappels envoyés avec succès");
}