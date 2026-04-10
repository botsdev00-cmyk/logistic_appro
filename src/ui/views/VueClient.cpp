#include "VueClient.h"
#include "../widgets/TableauClient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

VueClient::VueClient(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueClient::~VueClient()
{
}

void VueClient::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(10);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Gestion des Clients");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Recherche et filtres
    QGroupBox* groupRecherche = new QGroupBox("Recherche et filtres");
    QHBoxLayout* layoutRecherche = new QHBoxLayout(groupRecherche);
    
    layoutRecherche->addWidget(new QLabel("Rechercher:"));
    m_champRecherche = std::make_unique<QLineEdit>();
    m_champRecherche->setPlaceholderText("Nom du client...");
    layoutRecherche->addWidget(m_champRecherche.get());
    
    layoutRecherche->addWidget(new QLabel("Route:"));
    m_comboRoute = std::make_unique<QComboBox>();
    m_comboRoute->addItem("Tous");
    m_comboRoute->addItem("Route Nord");
    m_comboRoute->addItem("Route Sud");
    m_comboRoute->addItem("Route Est");
    m_comboRoute->addItem("Route Ouest");
    layoutRecherche->addWidget(m_comboRoute.get());
    
    QPushButton* btnRechercher = new QPushButton("Rechercher");
    connect(btnRechercher, &QPushButton::clicked, this, &VueClient::rechercher);
    layoutRecherche->addWidget(btnRechercher);
    
    layoutPrincipal->addWidget(groupRecherche);
    
    // Tableau
    m_tableauClient = std::make_unique<TableauClient>();
    layoutPrincipal->addWidget(m_tableauClient.get());
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    QPushButton* btnAjouter = new QPushButton("Ajouter client");
    connect(btnAjouter, &QPushButton::clicked, this, &VueClient::ajouterClient);
    layoutBoutons->addWidget(btnAjouter);
    
    QPushButton* btnEditer = new QPushButton("Éditer");
    connect(btnEditer, &QPushButton::clicked, this, &VueClient::editerClient);
    layoutBoutons->addWidget(btnEditer);
    
    QPushButton* btnSupprimer = new QPushButton("Supprimer");
    connect(btnSupprimer, &QPushButton::clicked, this, &VueClient::supprimerClient);
    layoutBoutons->addWidget(btnSupprimer);
    
    QPushButton* btnHistorique = new QPushButton("Historique");
    connect(btnHistorique, &QPushButton::clicked, this, &VueClient::afficherHistorique);
    layoutBoutons->addWidget(btnHistorique);
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void VueClient::initialiserConnexions()
{
    connect(m_comboRoute.get(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VueClient::filtrerParRoute);
    m_tableauClient->chargerDonnees();
}

void VueClient::rechercher()
{
    QString terme = m_champRecherche->text();
    m_tableauClient->rechercher(terme);
}

void VueClient::ajouterClient()
{
    QMessageBox::information(this, "Ajouter", "Ouvrir dialog ajout client");
}

void VueClient::editerClient()
{
    QMessageBox::information(this, "Éditer", "Ouvrir dialog édition client");
}

void VueClient::supprimerClient()
{
    QMessageBox::information(this, "Supprimer", "Client supprimé");
}

void VueClient::afficherHistorique()
{
    QMessageBox::information(this, "Historique", "Afficher historique du client");
}

void VueClient::filtrerParRoute()
{
    QString route = m_comboRoute->currentText();
    m_tableauClient->filtrerParRoute(route);
}