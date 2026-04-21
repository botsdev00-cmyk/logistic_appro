#include "BoiteDialogRepartition.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryRoute.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "BoiteDialogNouvelleEquipe.h"
#include "BoiteDialogNouvelleRoute.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>

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

BoiteDialogRepartition::~BoiteDialogRepartition() {}

void BoiteDialogRepartition::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);

    // Groupe Informations
    QGroupBox* groupInfo = new QGroupBox("Informations de répartition");
    QGridLayout* layoutInfo = new QGridLayout(groupInfo);

    layoutInfo->addWidget(new QLabel("Équipe:"), 0, 0);
    m_comboEquipe = std::make_unique<QComboBox>();
    layoutInfo->addWidget(m_comboEquipe.get(), 0, 1);

    m_btnNouvelleEquipe = std::make_unique<QPushButton>("Nouvelle équipe...");
    layoutInfo->addWidget(m_btnNouvelleEquipe.get(), 0, 2);

    layoutInfo->addWidget(new QLabel("Route:"), 1, 0);
    m_comboRoute = std::make_unique<QComboBox>();
    layoutInfo->addWidget(m_comboRoute.get(), 1, 1);

    m_btnNouvelleRoute = std::make_unique<QPushButton>("Nouvelle route...");
    layoutInfo->addWidget(m_btnNouvelleRoute.get(), 1, 2);

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
    connect(m_btnNouvelleEquipe.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::onNouvelleEquipe);
    connect(m_btnNouvelleRoute.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::onNouvelleRoute);
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
    m_comboEquipe->clear();
    RepositoryEquipe repo;
    QList<Equipe> equipes = repo.getAll();
    for(const Equipe& eq : equipes) {
        m_comboEquipe->addItem(eq.getNom(), eq.getEquipeId());
    }
}

void BoiteDialogRepartition::remplirComboRoutes()
{
    m_comboRoute->clear();
    RepositoryRoute repo;
    QList<Route> routes = repo.getAll();
    for(const Route& r : routes) {
        m_comboRoute->addItem(r.getNom(), r.getRouteId());
    }
}

void BoiteDialogRepartition::remplirTableProduits()
{
    m_comboProduit->clear();
    RepositoryProduit repo;
    auto produits = repo.getAll();
    for(const auto& p : produits) {
        m_comboProduit->addItem(p.getNom(), p.getProduitId());
    }
}

void BoiteDialogRepartition::creerRepartition()
{
    // 1. Vérification de base : au moins un article dans le tableau
    if (m_tableArticles->rowCount() == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez ajouter au moins un article avant de valider.");
        return;
    }

    // 2. Récupération des IDs sélectionnés
    QUuid equipeId = m_comboEquipe->currentData().toUuid();
    QUuid routeId = m_comboRoute->currentData().toUuid();
    QDate dateRepartition = m_dateRepartition->date();

    if (equipeId.isNull() || routeId.isNull()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une équipe et une route.");
        return;
    }

    // 3. Récupération du Chef d'équipe (utilisé comme créateur/chef_id)
    RepositoryEquipe repoEquipe;
    Equipe equipeSelectionnee = repoEquipe.getById(equipeId);
    QUuid chefId = QUuid(equipeSelectionnee.getNomChef());

    if (chefId.isNull()) {
        QMessageBox::critical(this, "Erreur", "L'équipe sélectionnée n'a pas de chef assigné valide.");
        return;
    }

    // 4. Création de l'entête de la répartition
    // Note : Le manager doit être mis à jour pour forcer le statut "En cours"
    m_repartitionId = m_gestionnaire->creerRepartition(equipeId, routeId, dateRepartition, chefId);
    
    if (m_repartitionId.isNull()) {
        QMessageBox::critical(this, "Erreur de création", 
                             "Impossible de créer la répartition : " + m_gestionnaire->getDernierErreur());
        return;
    }

    // 5. Boucle d'ajout des articles avec vérification de stock
    bool erreurDetectee = false;
    for (int row = 0; row < m_tableArticles->rowCount(); ++row) {
        QString produitNom = m_tableArticles->item(row, 0)->text();
        
        // Récupération de l'ID du produit via le nom (recherche dans la combo)
        QUuid produitId;
        for(int i = 0; i < m_comboProduit->count(); ++i) {
            if(m_comboProduit->itemText(i).trimmed() == produitNom.trimmed()) {
                produitId = m_comboProduit->itemData(i).toUuid();
                break;
            }
        }

        ArticleRepartition article;
        article.setRepartitionId(m_repartitionId);
        article.setProduitId(produitId);
        article.setQuantiteVente(m_tableArticles->item(row, 1)->text().toInt());
        article.setQuantiteCadeau(m_tableArticles->item(row, 2)->text().toInt());
        article.setQuantiteDegustation(m_tableArticles->item(row, 3)->text().toInt());

        // Appel au gestionnaire qui va vérifier le stock avant d'insérer
        if (!m_gestionnaire->ajouterArticle(article)) {
            QMessageBox::critical(this, "Erreur Stock / Article", 
                                 QString("Erreur sur le produit '%1' : %2")
                                 .arg(produitNom)
                                 .arg(m_gestionnaire->getDernierErreur()));
            erreurDetectee = true;
            break; // On arrête tout si un article échoue (intégrité de la répartition)
        }
    }

    // 6. Finalisation
    if (!erreurDetectee) {
        QMessageBox::information(this, "Succès", "La répartition a été créée avec succès.");
        accept();
    } else {
        // Optionnel : Logique pour supprimer la répartition vide si erreur d'articles
        // m_gestionnaire->supprimerRepartition(m_repartitionId); 
    }
}

void BoiteDialogRepartition::ajouterArticle()
{
    if(m_comboProduit->currentIndex() < 0) return;
    QString produit = m_comboProduit->currentText();
    int qVente = m_spinVente->value();
    int qCadeau = m_spinCadeau->value();
    int qDegustation = m_spinDegustation->value();
    int total = qVente + qCadeau + qDegustation;

    if(total == 0) {
        QMessageBox::warning(this, "Erreur", "Saisir au moins une quantité.");
        return;
    }

    // Vérifier doublon produit
    for(int i=0; i<m_tableArticles->rowCount();++i)
        if(m_tableArticles->item(i,0)->text() == produit) {
            QMessageBox::warning(this, "Erreur", "Produit déjà ajouté.");
            return;
        }

    int row = m_tableArticles->rowCount();
    m_tableArticles->insertRow(row);

    m_tableArticles->setItem(row, 0, new QTableWidgetItem(produit));
    m_tableArticles->setItem(row, 1, new QTableWidgetItem(QString::number(qVente)));
    m_tableArticles->setItem(row, 2, new QTableWidgetItem(QString::number(qCadeau)));
    m_tableArticles->setItem(row, 3, new QTableWidgetItem(QString::number(qDegustation)));
    m_tableArticles->setItem(row, 4, new QTableWidgetItem(QString::number(total)));

    m_spinVente->setValue(0);
    m_spinCadeau->setValue(0);
    m_spinDegustation->setValue(0);
}

void BoiteDialogRepartition::supprimerArticle()
{
    if (m_tableArticles->currentRow() >= 0)
        m_tableArticles->removeRow(m_tableArticles->currentRow());
}

void BoiteDialogRepartition::mettreAJourArticles()
{
    // Prévu pour une éventuelle logique future <-> widget synchronisation
}

void BoiteDialogRepartition::onNouvelleEquipe()
{
    BoiteDialogNouvelleEquipe dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        remplirComboEquipes();
        QUuid id = dialog.getEquipeCreeeId();
        int idx = m_comboEquipe->findData(id);
        if (idx >= 0) m_comboEquipe->setCurrentIndex(idx);
    }
}

void BoiteDialogRepartition::onNouvelleRoute()
{
    BoiteDialogNouvelleRoute dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        remplirComboRoutes();
        QUuid id = dialog.getRouteCreeeId();
        int idx = m_comboRoute->findData(id);
        if (idx >= 0) m_comboRoute->setCurrentIndex(idx);
    }
}