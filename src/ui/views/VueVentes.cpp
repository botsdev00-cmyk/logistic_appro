#include "VueVentes.h"
#include "../widgets/TableauVentes.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>

VueVentes::VueVentes(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueVentes::~VueVentes()
{
}

void VueVentes::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(10);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Gestion des Ventes");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Filtres
    QGroupBox* groupFiltres = new QGroupBox("Filtres");
    QHBoxLayout* layoutFiltres = new QHBoxLayout(groupFiltres);
    
    layoutFiltres->addWidget(new QLabel("Client:"));
    m_comboClient = std::make_unique<QComboBox>();
    m_comboClient->addItem("Tous");
    layoutFiltres->addWidget(m_comboClient.get());
    
    layoutFiltres->addWidget(new QLabel("Type:"));
    m_comboType = std::make_unique<QComboBox>();
    m_comboType->addItem("Tous");
    m_comboType->addItem("Cash");
    m_comboType->addItem("Crédit");
    layoutFiltres->addWidget(m_comboType.get());
    
    layoutFiltres->addStretch();
    layoutPrincipal->addWidget(groupFiltres);
    
    // Tableau
    m_tableauVentes = std::make_unique<TableauVentes>();
    layoutPrincipal->addWidget(m_tableauVentes.get());
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    QPushButton* btnAjouter = new QPushButton("Ajouter vente");
    connect(btnAjouter, &QPushButton::clicked, this, &VueVentes::ajouterVente);
    layoutBoutons->addWidget(btnAjouter);
    
    QPushButton* btnDetails = new QPushButton("Afficher détails");
    connect(btnDetails, &QPushButton::clicked, this, &VueVentes::afficherDetails);
    layoutBoutons->addWidget(btnDetails);
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void VueVentes::initialiserConnexions()
{
    connect(m_comboClient.get(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VueVentes::filtrerParClient);
    connect(m_comboType.get(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VueVentes::filtrerParType);
    m_tableauVentes->chargerDonnees();
}

void VueVentes::ajouterVente()
{
    // À implémenter
}

void VueVentes::filtrerParClient()
{
    QString client = m_comboClient->currentText();
    m_tableauVentes->filtrerParClient(client);
}

void VueVentes::filtrerParType()
{
    QString type = m_comboType->currentText();
    m_tableauVentes->filtrerParType(type);
}

void VueVentes::afficherDetails()
{
    // À implémenter
}