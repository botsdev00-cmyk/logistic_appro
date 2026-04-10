#include "VueRepartition.h"
#include "../widgets/TableauRepartition.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>

VueRepartition::VueRepartition(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueRepartition::~VueRepartition()
{
}

void VueRepartition::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(10);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Gestion des Répartitions");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Filtres
    QGroupBox* groupFiltres = new QGroupBox("Filtres");
    QHBoxLayout* layoutFiltres = new QHBoxLayout(groupFiltres);
    
    layoutFiltres->addWidget(new QLabel("Statut:"));
    m_comboStatut = std::make_unique<QComboBox>();
    m_comboStatut->addItem("Tous");
    m_comboStatut->addItem("En attente");
    m_comboStatut->addItem("En cours");
    m_comboStatut->addItem("Complétée");
    layoutFiltres->addWidget(m_comboStatut.get());
    
    layoutFiltres->addStretch();
    layoutPrincipal->addWidget(groupFiltres);
    
    // Tableau
    m_tableauRepartition = std::make_unique<TableauRepartition>();
    layoutPrincipal->addWidget(m_tableauRepartition.get());
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    QPushButton* btnCreer = new QPushButton("Créer répartition");
    connect(btnCreer, &QPushButton::clicked, this, &VueRepartition::creerRepartition);
    layoutBoutons->addWidget(btnCreer);
    
    QPushButton* btnVerifier = new QPushButton("Vérifier statut");
    connect(btnVerifier, &QPushButton::clicked, this, &VueRepartition::verifierStatut);
    layoutBoutons->addWidget(btnVerifier);
    
    QPushButton* btnRetours = new QPushButton("Charger retours");
    connect(btnRetours, &QPushButton::clicked, this, &VueRepartition::chargerRetours);
    layoutBoutons->addWidget(btnRetours);
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void VueRepartition::initialiserConnexions()
{
    connect(m_comboStatut.get(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VueRepartition::filtrerParStatut);
    m_tableauRepartition->chargerDonnees();
}

void VueRepartition::creerRepartition()
{
    // À implémenter
}

void VueRepartition::verifierStatut()
{
    // À implémenter
}

void VueRepartition::chargerRetours()
{
    // À implémenter
}

void VueRepartition::filtrerParStatut()
{
    QString statut = m_comboStatut->currentText();
    m_tableauRepartition->filtrerParStatut(statut);
}