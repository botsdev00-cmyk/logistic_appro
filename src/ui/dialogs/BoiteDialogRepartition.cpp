#include "BoiteDialogRepartition.h"
#include "../../core/entities/Equipe.h"
#include "../../core/entities/Route.h"
#include "../../business/managers/GestionnaireRepartition.h"

// >> Il FAUT créer ces deux includes !
#include "BoiteDialogNouvelleEquipe.h"
#include "BoiteDialogNouvelleRoute.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QSpinBox>
#include <QHeaderView>
#include <QMessageBox>

BoiteDialogRepartition::BoiteDialogRepartition(QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(std::make_unique<GestionnaireRepartition>())
{
    creerWidgets();
    initialiserConnexions();
    chargerDonnees();
    setWindowTitle("Création d'une répartition");
    setMinimumWidth(600);
}

BoiteDialogRepartition::~BoiteDialogRepartition() {}

void BoiteDialogRepartition::creerWidgets()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // ------ Ligne Equipe + bouton ------
    QHBoxLayout* equipeLayout = new QHBoxLayout;
    m_comboEquipe = std::make_unique<QComboBox>();
    m_btnNouvelleEquipe = std::make_unique<QPushButton>("Nouvelle équipe...");
    equipeLayout->addWidget(new QLabel("Équipe :"));
    equipeLayout->addWidget(m_comboEquipe.get());
    equipeLayout->addWidget(m_btnNouvelleEquipe.get());
    layout->addLayout(equipeLayout);

    // ------ Ligne Route + bouton ------
    QHBoxLayout* routeLayout = new QHBoxLayout;
    m_comboRoute = std::make_unique<QComboBox>();
    m_btnNouvelleRoute = std::make_unique<QPushButton>("Nouvelle route...");
    routeLayout->addWidget(new QLabel("Route :"));
    routeLayout->addWidget(m_comboRoute.get());
    routeLayout->addWidget(m_btnNouvelleRoute.get());
    layout->addLayout(routeLayout);

    // ------ Date ------
    QHBoxLayout* dateLayout = new QHBoxLayout;
    m_dateRepartition = std::make_unique<QDateEdit>(QDate::currentDate());
    m_dateRepartition->setCalendarPopup(true);
    dateLayout->addWidget(new QLabel("Date :"));
    dateLayout->addWidget(m_dateRepartition.get());
    layout->addLayout(dateLayout);

    // ------ Table produits/articles ------
    m_tableArticles = std::make_unique<QTableWidget>(0, 6);
    QStringList headers = {"Produit", "Qté vente", "Qté cadeau", "Qté dégustation", "Total", "Obs."};
    m_tableArticles->setHorizontalHeaderLabels(headers);
    m_tableArticles->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(m_tableArticles.get());

    // ------ Ajout article ------
    QHBoxLayout* ajoutLayout = new QHBoxLayout;
    m_comboProduit = std::make_unique<QComboBox>();
    m_spinVente = std::make_unique<QSpinBox>();
    m_spinVente->setRange(0, 9999);
    m_spinCadeau = std::make_unique<QSpinBox>();
    m_spinCadeau->setRange(0, 9999);
    m_spinDegustation = std::make_unique<QSpinBox>();
    m_spinDegustation->setRange(0, 9999);
    m_boutonAjouter = std::make_unique<QPushButton>("Ajouter");
    m_boutonSupprimer = std::make_unique<QPushButton>("Supprimer");
    ajoutLayout->addWidget(new QLabel("Produit :"));
    ajoutLayout->addWidget(m_comboProduit.get());
    ajoutLayout->addWidget(new QLabel("Vente")); ajoutLayout->addWidget(m_spinVente.get());
    ajoutLayout->addWidget(new QLabel("Cadeau")); ajoutLayout->addWidget(m_spinCadeau.get());
    ajoutLayout->addWidget(new QLabel("Dégust.")); ajoutLayout->addWidget(m_spinDegustation.get());
    ajoutLayout->addWidget(m_boutonAjouter.get());
    ajoutLayout->addWidget(m_boutonSupprimer.get());
    layout->addLayout(ajoutLayout);

    // ------ Boutons bas ------
    QHBoxLayout* btnLayout = new QHBoxLayout;
    m_boutonCreer = std::make_unique<QPushButton>("Créer");
    m_boutonAnnuler = std::make_unique<QPushButton>("Annuler");
    btnLayout->addStretch();
    btnLayout->addWidget(m_boutonCreer.get());
    btnLayout->addWidget(m_boutonAnnuler.get());
    layout->addLayout(btnLayout);
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
    // À remplacer par : lecture BDD des équipes
    m_comboEquipe->clear();
    // Exemple fictif :
    m_comboEquipe->addItem("Sélectionnez une équipe", QVariant()); // valeur nulle
    // TODO: charger équipes réelles
    // Pour chaque équipe :
    // m_comboEquipe->addItem(equipe.getNom(), equipe.getEquipeId());
}

void BoiteDialogRepartition::remplirComboRoutes()
{
    m_comboRoute->clear();
    m_comboRoute->addItem("Sélectionnez une route", QVariant());
    // TODO: charger routes réelles
    // Pour chaque route réelle :
    // m_comboRoute->addItem(route.getNom(), route.getRouteId());
}

void BoiteDialogRepartition::remplirTableProduits()
{
    m_comboProduit->clear();
    // TODO: charger liste produits réels sinon démo/placeholder :
    m_comboProduit->addItem("Produit A", QUuid::createUuid());
    m_comboProduit->addItem("Produit B", QUuid::createUuid());
    m_comboProduit->addItem("Produit C", QUuid::createUuid());
}

void BoiteDialogRepartition::ajouterArticle()
{
    // Récupère données :
    int idx = m_comboProduit->currentIndex();
    if (idx <= 0) return;
    QString produitNom = m_comboProduit->currentText();
    QUuid produitId = m_comboProduit->currentData().toUuid();
    int qV = m_spinVente->value();
    int qC = m_spinCadeau->value();
    int qD = m_spinDegustation->value();
    int total = qV + qC + qD;

    if (total <= 0) {
        QMessageBox::warning(this, "Erreur", "Quantité totale invalide");
        return;
    }
    int row = m_tableArticles->rowCount();
    m_tableArticles->insertRow(row);
    m_tableArticles->setItem(row, 0, new QTableWidgetItem(produitNom));
    m_tableArticles->setItem(row, 1, new QTableWidgetItem(QString::number(qV)));
    m_tableArticles->setItem(row, 2, new QTableWidgetItem(QString::number(qC)));
    m_tableArticles->setItem(row, 3, new QTableWidgetItem(QString::number(qD)));
    m_tableArticles->setItem(row, 4, new QTableWidgetItem(QString::number(total)));
    m_tableArticles->setItem(row, 5, new QTableWidgetItem(""));
}

void BoiteDialogRepartition::supprimerArticle()
{
    int row = m_tableArticles->currentRow();
    if (row >= 0)
        m_tableArticles->removeRow(row);
}

void BoiteDialogRepartition::mettreAJourArticles()
{
    // Si tu veux permettre la maj d’une ligne article : cette fonction peut être complétée
}

void BoiteDialogRepartition::creerRepartition()
{
    int idxEquipe = m_comboEquipe->currentIndex();
    int idxRoute = m_comboRoute->currentIndex();

    if (idxEquipe <= 0 || idxRoute <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une équipe et une route");
        return;
    }
    QUuid equipeId = m_comboEquipe->currentData().toUuid();
    QUuid routeId = m_comboRoute->currentData().toUuid();
    QDate date = m_dateRepartition->date();

    // Création de la répartition via le manager
    QUuid repartitionId = m_gestionnaire->creerRepartition(equipeId, routeId, date, QUuid());//TODO: utilisateur connecté

    if (repartitionId.isNull()) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer la répartition: " + m_gestionnaire->getDernierErreur());
        return;
    }
    // Ajoute les articles
    for (int r = 0; r < m_tableArticles->rowCount(); ++r) {
        QUuid prodId = m_comboProduit->itemData(m_comboProduit->findText(m_tableArticles->item(r,0)->text())).toUuid();
        int qV = m_tableArticles->item(r, 1)->text().toInt();
        int qC = m_tableArticles->item(r, 2)->text().toInt();
        int qD = m_tableArticles->item(r, 3)->text().toInt();
        // À adapter selon structure ArticleRepartition :
        ArticleRepartition art;
        art.setArticleRepartitionId(QUuid::createUuid());
        art.setRepartitionId(repartitionId);
        art.setProduitId(prodId);
        art.setQuantiteVente(qV);
        art.setQuantiteCadeau(qC);
        art.setQuantiteDegustation(qD);
        m_gestionnaire->ajouterArticle(art);
    }

    m_repartitionId = repartitionId;
    accept();
}

void BoiteDialogRepartition::onNouvelleEquipe()
{
    BoiteDialogNouvelleEquipe dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Récupère id et nom de la nouvelle équipe
        QUuid nouvelleEquipeId = dialog.getEquipeCreeeId();
        QString nomNouvelleEquipe = dialog.getEquipeCreeeNom();
        remplirComboEquipes();
        // Sélectionne la nouvelle équipe dans le combo
        int idx = m_comboEquipe->findData(nouvelleEquipeId);
        if (idx >= 0)
            m_comboEquipe->setCurrentIndex(idx);
    }
}

void BoiteDialogRepartition::onNouvelleRoute()
{
    BoiteDialogNouvelleRoute dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QUuid nouvelleRouteId = dialog.getRouteCreeeId();
        QString nomNouvelleRoute = dialog.getRouteCreeeNom();
        remplirComboRoutes();
        int idx = m_comboRoute->findData(nouvelleRouteId);
        if (idx >= 0)
            m_comboRoute->setCurrentIndex(idx);
    }
}