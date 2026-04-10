#include "VueStock.h"
#include "../widgets/TableauStock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>

VueStock::VueStock(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueStock::~VueStock()
{
}

void VueStock::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(10);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Gestion du Stock");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Barre de recherche et filtres
    QGroupBox* groupRecherche = new QGroupBox("Recherche et filtres");
    QHBoxLayout* layoutRecherche = new QHBoxLayout(groupRecherche);
    
    layoutRecherche->addWidget(new QLabel("Rechercher:"));
    m_champRecherche = std::make_unique<QLineEdit>();
    m_champRecherche->setPlaceholderText("Nom du produit...");
    layoutRecherche->addWidget(m_champRecherche.get());
    
    layoutRecherche->addWidget(new QLabel("Catégorie:"));
    m_comboCategorie = std::make_unique<QComboBox>();
    m_comboCategorie->addItem("Tous");
    m_comboCategorie->addItem("Vin Rouge");
    m_comboCategorie->addItem("Vin Blanc");
    m_comboCategorie->addItem("Bière");
    layoutRecherche->addWidget(m_comboCategorie.get());
    
    QPushButton* btnRechercher = new QPushButton("Rechercher");
    connect(btnRechercher, &QPushButton::clicked, this, &VueStock::rechercher);
    layoutRecherche->addWidget(btnRechercher);
    
    layoutPrincipal->addWidget(groupRecherche);
    
    // Tableau stock
    m_tableauStock = std::make_unique<TableauStock>();
    layoutPrincipal->addWidget(m_tableauStock.get());
    
    // Boutons d'actions
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    QPushButton* btnEntree = new QPushButton("Nouvelle entrée");
    connect(btnEntree, &QPushButton::clicked, this, &VueStock::creerEntree);
    layoutBoutons->addWidget(btnEntree);
    
    QPushButton* btnRetour = new QPushButton("Enregistrer retour");
    connect(btnRetour, &QPushButton::clicked, this, &VueStock::enregistrerRetour);
    layoutBoutons->addWidget(btnRetour);
    
    QPushButton* btnAjustement = new QPushButton("Ajustement");
    connect(btnAjustement, &QPushButton::clicked, this, &VueStock::ajustement);
    layoutBoutons->addWidget(btnAjustement);
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void VueStock::initialiserConnexions()
{
    connect(m_comboCategorie.get(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VueStock::filtrer);
    m_tableauStock->chargerDonnees();
}

void VueStock::rechercher()
{
    QString terme = m_champRecherche->text();
    // À implémenter
}

void VueStock::filtrer()
{
    QString categorie = m_comboCategorie->currentText();
    // À implémenter
}

void VueStock::creerEntree()
{
    // À implémenter - ouvrir dialog
}

void VueStock::enregistrerRetour()
{
    // À implémenter - ouvrir dialog
}

void VueStock::ajustement()
{
    // À implémenter - ouvrir dialog
}