#include "BoiteDialogRepartition.h"
#include "utils/globals/globals.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryArticleRepartition.h"
#include "../../data/repositories/RepositoryRepartition.h"
#include "../../data/repositories/RepositoryRoute.h"
#include "../../core/entities/Route.h"
#include "../../core/entities/Equipe.h"
#include "../../core/entities/Produit.h"
#include "../../core/entities/ArticleRepartition.h"
#include "../../core/entities/Repartition.h"
#include "BoiteDialogNouvelleEquipe.h"
#include "BoiteDialogNouvelleRoute.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QDateEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

BoiteDialogRepartition::BoiteDialogRepartition(QWidget* parent)
    : QDialog(parent),
    m_comboEquipe(std::make_unique<QComboBox>()),
    m_comboRoute(std::make_unique<QComboBox>()),
    m_dateRepartition(std::make_unique<QDateEdit>()),
    m_comboProduit(std::make_unique<QComboBox>()),
    m_spinVente(std::make_unique<QSpinBox>()),
    m_spinCadeau(std::make_unique<QSpinBox>()),
    m_spinDegustation(std::make_unique<QSpinBox>()),
    m_tableArticles(std::make_unique<QTableWidget>()),
    m_boutonAjouter(std::make_unique<QPushButton>("Ajouter")),
    m_boutonSupprimer(std::make_unique<QPushButton>("Supprimer")),
    m_boutonCreer(std::make_unique<QPushButton>("Créer")),
    m_boutonAnnuler(std::make_unique<QPushButton>("Annuler")),
    m_btnNouvelleEquipe(std::make_unique<QPushButton>("Nouvelle équipe")),
    m_btnNouvelleRoute(std::make_unique<QPushButton>("Nouvelle route"))
{
    setWindowTitle("Nouvelle répartition");
    setMinimumWidth(700);
    creerWidgets();
    initialiserConnexions();
    chargerDonnees();
}

BoiteDialogRepartition::~BoiteDialogRepartition() {}

void BoiteDialogRepartition::creerWidgets()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Équipe / Route / Date
    QHBoxLayout* eqRouteLayout = new QHBoxLayout;
    eqRouteLayout->addWidget(new QLabel("Équipe :"));
    eqRouteLayout->addWidget(m_comboEquipe.get());
    eqRouteLayout->addWidget(m_btnNouvelleEquipe.get());

    eqRouteLayout->addSpacing(10);
    eqRouteLayout->addWidget(new QLabel("Route :"));
    eqRouteLayout->addWidget(m_comboRoute.get());
    eqRouteLayout->addWidget(m_btnNouvelleRoute.get());

    eqRouteLayout->addSpacing(10);
    eqRouteLayout->addWidget(new QLabel("Date :"));
    m_dateRepartition->setDate(QDate::currentDate());
    eqRouteLayout->addWidget(m_dateRepartition.get());
    mainLayout->addLayout(eqRouteLayout);

    // Produit
    QHBoxLayout* ligneProduitLayout = new QHBoxLayout;
    ligneProduitLayout->addWidget(new QLabel("Produit :"));
    ligneProduitLayout->addWidget(m_comboProduit.get());
    ligneProduitLayout->addWidget(new QLabel("Vente :"));
    m_spinVente->setMinimum(0);
    ligneProduitLayout->addWidget(m_spinVente.get());
    ligneProduitLayout->addWidget(new QLabel("Cadeau :"));
    m_spinCadeau->setMinimum(0);
    ligneProduitLayout->addWidget(m_spinCadeau.get());
    ligneProduitLayout->addWidget(new QLabel("Dégustation :"));
    m_spinDegustation->setMinimum(0);
    ligneProduitLayout->addWidget(m_spinDegustation.get());
    ligneProduitLayout->addWidget(m_boutonAjouter.get());
    ligneProduitLayout->addWidget(m_boutonSupprimer.get());
    mainLayout->addLayout(ligneProduitLayout);

    // Table des articles
    m_tableArticles->setColumnCount(5);
    m_tableArticles->setHorizontalHeaderLabels({"Produit", "Vente", "Cadeau", "Dégustation", "ProduitId"});
    m_tableArticles->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableArticles->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableArticles->hideColumn(4); // Id produit caché

    mainLayout->addWidget(m_tableArticles.get());

    // Boutons bas
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_boutonCreer.get());
    btnLayout->addWidget(m_boutonAnnuler.get());
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);
}

void BoiteDialogRepartition::initialiserConnexions()
{
    connect(m_boutonAnnuler.get(), &QPushButton::clicked, this, &QDialog::reject);
    connect(m_boutonCreer.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::creerRepartition);
    connect(m_boutonAjouter.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::ajouterArticle);
    connect(m_boutonSupprimer.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::supprimerArticle);
    connect(m_btnNouvelleEquipe.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::onNouvelleEquipe);
    connect(m_btnNouvelleRoute.get(), &QPushButton::clicked, this, &BoiteDialogRepartition::onNouvelleRoute);
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
    RepositoryEquipe repoEquipe;
    QList<Equipe> equipes = repoEquipe.getAll();
    for (const Equipe& eq : equipes)
        m_comboEquipe->addItem(eq.getNom(), eq.getEquipeId());
    if (m_comboEquipe->count() > 0)
        m_comboEquipe->setCurrentIndex(0);
}

void BoiteDialogRepartition::remplirComboRoutes()
{
    m_comboRoute->clear();
    RepositoryRoute repoRoute;
    m_comboRoute->addItem("(Aucune route)", QUuid());

    QList<Route> routes = repoRoute.getAll();
    for (const Route& route : routes) {
        m_comboRoute->addItem(route.getNom(), route.getRouteId());
    }
    // Pour UX, sélectionner par défaut la première réelle si elle existe
    if (routes.size() > 0)
        m_comboRoute->setCurrentIndex(1); // index 0 = "Aucune route"
    else
        m_comboRoute->setCurrentIndex(0);
}


void BoiteDialogRepartition::remplirTableProduits()
{
    m_comboProduit->clear();
    RepositoryProduit repoProduit;
    auto produits = repoProduit.getAll();
    for (const Produit& prod : produits)
        m_comboProduit->addItem(prod.getNom(), prod.getProduitId());
    if (m_comboProduit->count() > 0)
        m_comboProduit->setCurrentIndex(0);
}


void BoiteDialogRepartition::creerRepartition()
{
    int idxEquipe = m_comboEquipe->currentIndex();
    if (idxEquipe < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une équipe.");
        return;
    }
    QUuid equipeId = m_comboEquipe->currentData().toUuid();

    int idxRoute = m_comboRoute->currentIndex();
    QUuid routeId = (idxRoute >= 0) ? m_comboRoute->currentData().toUuid() : QUuid();

    QDate dateRep = m_dateRepartition->date();

    if (m_tableArticles->rowCount() == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez ajouter au moins un article.");
        return;
    }

    // 1. Générer un UUID et insérer la répartition EN BASE
    m_repartitionId = QUuid::createUuid();

    Repartition rep;
    rep.setRepartitionId(m_repartitionId);
    rep.setEquipeId(equipeId);
    rep.setRouteId(routeId);
    rep.setDateRepartition(dateRep);
    rep.setStatut(Repartition::Statut::EnCours); // ou autre statut selon le besoin métier
    rep.setMontantCashAttendu(0.0);
    rep.setChefId(QUuid()); // ou un vrai chef, si tu en as un
    rep.setCreatedAt(QDateTime::currentDateTime());
    rep.setUpdatedAt(QDateTime::currentDateTime());
    rep.setDeletedAt(QDateTime());
    rep.setVersion(1);
    rep.setSyncStatus("PENDING");

    RepositoryRepartition repoRepartition;
    if (!repoRepartition.create(rep)) {
        QMessageBox::warning(this, "Erreur", QString("Impossible de créer la répartition : %1").arg(repoRepartition.getLastError()));
        return;
    }

    // 2. Ensuite, insérer les articles dans la base
    // 2. Ensuite, insérer les articles dans la base
    RepositoryArticleRepartition repoArtRep;
    bool toutOK = true;
    for (int row = 0; row < m_tableArticles->rowCount(); ++row) {
        QUuid produitId(m_tableArticles->item(row, 4)->text());
        int vente = m_tableArticles->item(row, 1)->text().toInt();
        int cadeau = m_tableArticles->item(row, 2)->text().toInt();
        int degust = m_tableArticles->item(row, 3)->text().toInt();

        // 💡 UTILISATION DU CONSTRUCTEUR SURCHARGÉ AVEC L'ID GLOBAL
        ArticleRepartition artRep(g_utilisateurId);

        artRep.setArticleRepartitionId(QUuid::createUuid());
        artRep.setRepartitionId(m_repartitionId);
        artRep.setProduitId(produitId);
        artRep.setQuantiteVente(vente);
        artRep.setQuantiteCadeau(cadeau);
        artRep.setQuantiteDegustation(degust);
        artRep.setSyncStatus(ArticleRepartition::SyncStatus::PENDING);
        artRep.setVersion(1);
        artRep.setCreatedAt(QDateTime::currentDateTime());
        artRep.setUpdatedAt(QDateTime::currentDateTime());
        artRep.setDeletedAt(QDateTime());

        // Optionnel : Si tu n'utilises pas le constructeur surchargé,
        // tu peux aussi forcer l'assignation via les setters :
        // artRep.setCreatedBy(g_utilisateurId);
        // artRep.setUpdatedBy(g_utilisateurId);

        if (!repoArtRep.create(artRep)) {
            toutOK = false;
            QMessageBox::warning(this, "Erreur", QString("Impossible de sauvegarder un article : %1").arg(repoArtRep.getLastError()));
        }
    }
    if (toutOK) {
        QMessageBox::information(this, "Succès", "Répartition enregistrée avec succès !");
        accept();
    }
}

void BoiteDialogRepartition::ajouterArticle()
{
    int idx = m_comboProduit->currentIndex();
    if (idx < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un produit.");
        return;
    }
    QString nomProduit = m_comboProduit->currentText();
    QUuid produitId = m_comboProduit->currentData().toUuid();
    int vente = m_spinVente->value();
    int cadeau = m_spinCadeau->value();
    int degust = m_spinDegustation->value();

    // Évite le doublon produit dans la table
    for (int row = 0; row < m_tableArticles->rowCount(); ++row) {
        if (m_tableArticles->item(row, 4)->text() == produitId.toString(QUuid::WithoutBraces)) {
            QMessageBox::warning(this, "Erreur", "Cet article existe déjà dans la liste.");
            return;
        }
    }

    int currentRow = m_tableArticles->rowCount();
    m_tableArticles->insertRow(currentRow);
    m_tableArticles->setItem(currentRow, 0, new QTableWidgetItem(nomProduit));
    m_tableArticles->setItem(currentRow, 1, new QTableWidgetItem(QString::number(vente)));
    m_tableArticles->setItem(currentRow, 2, new QTableWidgetItem(QString::number(cadeau)));
    m_tableArticles->setItem(currentRow, 3, new QTableWidgetItem(QString::number(degust)));
    // Stock l’id produit en UserRole et colonne cachée
    auto idItem = new QTableWidgetItem(produitId.toString(QUuid::WithoutBraces));
    m_tableArticles->setItem(currentRow, 4, idItem);
}

void BoiteDialogRepartition::supprimerArticle()
{
    int row = m_tableArticles->currentRow();
    if (row >= 0)
        m_tableArticles->removeRow(row);
}

void BoiteDialogRepartition::mettreAJourArticles()
{
    int row = m_tableArticles->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un article dans le tableau à modifier.");
        return;
    }

    int idxProduit = m_comboProduit->currentIndex();
    if (idxProduit < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un produit.");
        return;
    }

    QString nomProduit = m_comboProduit->currentText();
    QUuid produitId = m_comboProduit->currentData().toUuid();
    int vente = m_spinVente->value();
    int cadeau = m_spinCadeau->value();
    int degust = m_spinDegustation->value();

    // Vérifie que le produit sélectionné n’est pas déjà dans la table (autre ligne)
    for (int r = 0; r < m_tableArticles->rowCount(); ++r) {
        if (r == row) continue;
        if (m_tableArticles->item(r, 4)->text() == produitId.toString(QUuid::WithoutBraces)) {
            QMessageBox::warning(this, "Erreur", "Cet article existe déjà dans la liste.");
            return;
        }
    }

    // Met à jour la ligne
    m_tableArticles->setItem(row, 0, new QTableWidgetItem(nomProduit));
    m_tableArticles->setItem(row, 1, new QTableWidgetItem(QString::number(vente)));
    m_tableArticles->setItem(row, 2, new QTableWidgetItem(QString::number(cadeau)));
    m_tableArticles->setItem(row, 3, new QTableWidgetItem(QString::number(degust)));
    m_tableArticles->setItem(row, 4, new QTableWidgetItem(produitId.toString(QUuid::WithoutBraces)));
}

void BoiteDialogRepartition::onNouvelleEquipe()
{
    BoiteDialogNouvelleEquipe dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        // Récupère l’ID et le nom de la nouvelle équipe
        QUuid id = dialog.getEquipeCreeeId();
        QString nom = dialog.getEquipeCreeeNom();
        if (!id.isNull()) {
            // Rafraîchir la combo : pour garder les équipes de la BD même si le repo a évolué
            remplirComboEquipes();
            // Sélectionne la nouvelle équipe dans la combo (si elle y est)
            int idx = m_comboEquipe->findData(id);
            if (idx >= 0)
                m_comboEquipe->setCurrentIndex(idx);
        }
    }
}

void BoiteDialogRepartition::onNouvelleRoute()
{
    BoiteDialogNouvelleRoute dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QUuid id = dialog.getRouteCreeeId();
        QString nom = dialog.getRouteCreeeNom();
        if (!id.isNull()) {
            remplirComboRoutes();
            int idx = m_comboRoute->findData(id);
            if (idx >= 0)
                m_comboRoute->setCurrentIndex(idx);
        }
    }
}
