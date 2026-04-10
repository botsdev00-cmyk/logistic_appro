#include "BoiteDialogRepartition.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>

BoiteDialogRepartition::BoiteDialogRepartition(QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(std::make_unique<GestionnaireRepartition>())
{
    setWindowTitle("Créer une nouvelle répartition");
    setModal(true);
    setFixedSize(800, 600);
    
    creerWidgets();
    initialiserConnexions();
    chargerDonnees();
}

BoiteDialogRepartition::~BoiteDialogRepartition()
{
}

void BoiteDialogRepartition::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    
    // Groupe Informations
    QGroupBox* groupInfo = new QGroupBox("Informations de répartition");
    QGridLayout* layoutInfo = new QGridLayout(groupInfo);
    
    layoutInfo->addWidget(new QLabel("Équipe:"), 0, 0);
    m_comboEquipe = std::make_unique<QComboBox>();
    layoutInfo->addWidget(m_comboEquipe.get(), 0, 1);
    
    layoutInfo->addWidget(new QLabel("Route:"), 1, 0);
    m_comboRoute = std::make_unique<QComboBox>();
    layoutInfo->addWidget(m_comboRoute.get(), 1, 1);
    
    layoutInfo->addWidget(new QLabel("Date:"), 2, 0);
    m_dateRepartition = std::make_unique<QDateEdit>();
    m_dateRepartition->setDate(QDate::currentDate());
    layoutInfo->addWidget(m_dateRepartition.get(), 2, 1);
    
    layoutPrincipal->addWidget(groupInfo);
    
    // Groupe Articles
    QGroupBox* groupArticles = new QGroupBox("Articles à répartir");
    QVBoxLayout* layoutArticles = new QVBoxLayout(groupArticles);
    
    QHBoxLayout* layoutAjout = new QHBoxLayout();
    layoutAjout->addWidget(new QLabel("Produit:"));
    m_comboProduit = std::make_unique<QComboBox>();
    layoutAjout->addWidget(m_comboProduit.get());
    
    layoutAjout->addWidget(new QLabel("Vente:"));
    m_spinVente = std::make_unique<QSpinBox>();
    m_spinVente->setMinimum(0);
    layoutAjout->addWidget(m_spinVente.get());
    
    layoutAjout->addWidget(new QLabel("Cadeau:"));
    m_spinCadeau = std::make_unique<QSpinBox>();
    m_spinCadeau->setMinimum(0);
    layoutAjout->addWidget(m_spinCadeau.get());
    
    layoutAjout->addWidget(new QLabel("Dégustation:"));
    m_spinDegustation = std::make_unique<QSpinBox>();
    m_spinDegustation->setMinimum(0);
    layoutAjout->addWidget(m_spinDegustation.get());
    
    m_boutonAjouter = std::make_unique<QPushButton>("Ajouter");
    layoutAjout->addWidget(m_boutonAjouter.get());
    
    layoutArticles->addLayout(layoutAjout);
    
    // Table des articles
    m_tableArticles = std::make_unique<QTableWidget>();
    m_tableArticles->setColumnCount(5);
    m_tableArticles->setHorizontalHeaderLabels({"Produit", "Vente", "Cadeau", "Dégustation", "Total"});
    m_tableArticles->horizontalHeader()->setStretchLastSection(false);
    m_tableArticles->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableArticles->setSelectionMode(QAbstractItemView::SingleSelection);
    layoutArticles->addWidget(m_tableArticles.get());
    
    m_boutonSupprimer = std::make_unique<QPushButton>("Supprimer article");
    layoutArticles->addWidget(m_boutonSupprimer.get());
    
    layoutPrincipal->addWidget(groupArticles);
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    m_boutonCreer = std::make_unique<QPushButton>("Créer répartition");
    m_boutonCreer->setMinimumWidth(150);
    layoutBoutons->addWidget(m_boutonCreer.get());
    
    m_boutonAnnuler = std::make_unique<QPushButton>("Annuler");
    m_boutonAnnuler->setMinimumWidth(150);
    layoutBoutons->addWidget(m_boutonAnnuler.get());
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void BoiteDialogRepartition::initialiserConnexions()
{
    connect(m_boutonCreer.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::creerRepartition);
    connect(m_boutonAnnuler.get(), &QPushButton::clicked, this, &QDialog::reject);
    connect(m_boutonAjouter.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::ajouterArticle);
    connect(m_boutonSupprimer.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::supprimerArticle);
}

void BoiteDialogRepartition::chargerDonnees()
{
    remplirComboEquipes();
    remplirComboRoutes();
    remplirTableProduits();
}

void BoiteDialogRepartition::remplirComboEquipes()
{
    // À implémenter avec les données de la base
    m_comboEquipe->addItem("Équipe Alpha");
    m_comboEquipe->addItem("Équipe Bravo");
    m_comboEquipe->addItem("Équipe Charlie");
}

void BoiteDialogRepartition::remplirComboRoutes()
{
    // À implémenter avec les données de la base
    m_comboRoute->addItem("Route Nord");
    m_comboRoute->addItem("Route Sud");
    m_comboRoute->addItem("Route Est");
    m_comboRoute->addItem("Route Ouest");
}

void BoiteDialogRepartition::remplirTableProduits()
{
    // À implémenter avec les données de la base
    m_comboProduit->addItem("Vin Rouge Shiraz");
    m_comboProduit->addItem("Vin Blanc Chardonnay");
    m_comboProduit->addItem("Bière Blonde");
    m_comboProduit->addItem("Bière Brune");
}

void BoiteDialogRepartition::creerRepartition()
{
    if (m_tableArticles->rowCount() == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez ajouter au moins un article");
        return;
    }
    
    // À implémenter
    QMessageBox::information(this, "Succès", "Répartition créée avec succès");
    accept();
}

void BoiteDialogRepartition::ajouterArticle()
{
    int row = m_tableArticles->rowCount();
    m_tableArticles->insertRow(row);
    
    m_tableArticles->setItem(row, 0, new QTableWidgetItem(m_comboProduit->currentText()));
    m_tableArticles->setItem(row, 1, new QTableWidgetItem(QString::number(m_spinVente->value())));
    m_tableArticles->setItem(row, 2, new QTableWidgetItem(QString::number(m_spinCadeau->value())));
    m_tableArticles->setItem(row, 3, new QTableWidgetItem(QString::number(m_spinDegustation->value())));
    
    int total = m_spinVente->value() + m_spinCadeau->value() + m_spinDegustation->value();
    m_tableArticles->setItem(row, 4, new QTableWidgetItem(QString::number(total)));
    
    // Réinitialiser
    m_spinVente->setValue(0);
    m_spinCadeau->setValue(0);
    m_spinDegustation->setValue(0);
}

void BoiteDialogRepartition::supprimerArticle()
{
    if (m_tableArticles->currentRow() >= 0) {
        m_tableArticles->removeRow(m_tableArticles->currentRow());
    }
}

void BoiteDialogRepartition::mettreAJourArticles()
{
    // À implémenter
}