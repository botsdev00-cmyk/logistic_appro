#include "VueCaisse.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

VueCaisse::VueCaisse(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueCaisse::~VueCaisse()
{
}

void VueCaisse::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(15);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Gestion de la Caisse");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Statistiques
    QHBoxLayout* layoutStats = new QHBoxLayout();
    
    QGroupBox* groupJour = new QGroupBox("Montant du jour");
    QVBoxLayout* layoutJour = new QVBoxLayout(groupJour);
    m_labelMontantJour = std::make_unique<QLabel>("75 500 FCFA");
    m_labelMontantJour->setStyleSheet("font-size: 18px; font-weight: bold; color: #27ae60;");
    layoutJour->addWidget(m_labelMontantJour.get());
    layoutStats->addWidget(groupJour);
    
    QGroupBox* groupAttente = new QGroupBox("En attente");
    QVBoxLayout* layoutAttente = new QVBoxLayout(groupAttente);
    m_labelMontantAttente = std::make_unique<QLabel>("25 000 FCFA");
    m_labelMontantAttente->setStyleSheet("font-size: 18px; font-weight: bold; color: #f39c12;");
    layoutAttente->addWidget(m_labelMontantAttente.get());
    layoutStats->addWidget(groupAttente);
    
    QGroupBox* groupDiscrepances = new QGroupBox("Discordances");
    QVBoxLayout* layoutDiscrepances = new QVBoxLayout(groupDiscrepances);
    m_labelDiscrepances = std::make_unique<QLabel>("2");
    m_labelDiscrepances->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");
    layoutDiscrepances->addWidget(m_labelDiscrepances.get());
    layoutStats->addWidget(groupDiscrepances);
    
    layoutPrincipal->addLayout(layoutStats);
    
    // Informations
    QGroupBox* groupInfo = new QGroupBox("Informations");
    QVBoxLayout* layoutInfo = new QVBoxLayout(groupInfo);
    QLabel* labelInfo = new QLabel(
        "Total réceptions: 15\n"
        "Dernier reçu: RC-2026-00015\n"
        "Dernière mise à jour: 2026-04-10 14:35"
    );
    layoutInfo->addWidget(labelInfo);
    layoutPrincipal->addWidget(groupInfo);
    
    layoutPrincipal->addStretch();
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    QPushButton* btnReception = new QPushButton("Enregistrer réception");
    connect(btnReception, &QPushButton::clicked, this, &VueCaisse::enregistrerReception);
    layoutBoutons->addWidget(btnReception);
    
    QPushButton* btnHistorique = new QPushButton("Historique");
    connect(btnHistorique, &QPushButton::clicked, this, &VueCaisse::afficherHistorique);
    layoutBoutons->addWidget(btnHistorique);
    
    QPushButton* btnDiscrepances = new QPushButton("Discordances");
    connect(btnDiscrepances, &QPushButton::clicked, this, &VueCaisse::afficherDiscrepances);
    layoutBoutons->addWidget(btnDiscrepances);
    
    QPushButton* btnRapport = new QPushButton("Rapport caisse");
    connect(btnRapport, &QPushButton::clicked, this, &VueCaisse::genererRapportCaisse);
    layoutBoutons->addWidget(btnRapport);
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void VueCaisse::initialiserConnexions()
{
    mettreAJourStatistiques();
}

void VueCaisse::mettreAJourStatistiques()
{
    // À implémenter avec les vraies données
}

void VueCaisse::enregistrerReception()
{
    QMessageBox::information(this, "Réception", "Ouvrir la boîte de dialogue de réception");
}

void VueCaisse::afficherHistorique()
{
    QMessageBox::information(this, "Historique", "Afficher l'historique des réceptions");
}

void VueCaisse::afficherDiscrepances()
{
    QMessageBox::information(this, "Discordances", "Afficher les discordances de caisse");
}

void VueCaisse::genererRapportCaisse()
{
    QMessageBox::information(this, "Rapport", "Rapport de caisse généré");
}