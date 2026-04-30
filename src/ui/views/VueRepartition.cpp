#include "VueRepartition.h"
#include "../widgets/TableauRepartition.h"
#include "../dialogs/BoiteDialogRepartition.h"
#include "../dialogs/BoiteDialogRetourRepartition.h"
#include "../../business/managers/GestionnaireSales.h"
#include "../../business/managers/GestionnaireCredit.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../core/entities/ArticleRepartition.h"
#include "../../data/repositories/RepositoryProduit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

extern GestionnaireRepartition* g_repartitionMgr;
extern GestionnaireSales* g_venteMgr;
extern GestionnaireCredit* g_creditMgr;
extern GestionnaireStock* g_stockMgr;
extern QUuid g_utilisateurId;
extern ArticleRepartition* art;


VueRepartition::VueRepartition(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueRepartition::~VueRepartition() {}

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
    BoiteDialogRepartition boite(this);
    if (boite.exec() == QDialog::Accepted) {
        m_tableauRepartition->rafraichir();
    }
}

void VueRepartition::verifierStatut()
{
    int row = m_tableauRepartition->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Sélection requise", "Sélectionnez une répartition.");
        return;
    }
    QUuid repId = m_tableauRepartition->repartitionIdFromRow(row);
    if (repId.isNull()) return;
    auto repartition = g_repartitionMgr->obtenirRepartition(repId, true);

    QString txt;
    txt += "Équipe: " + repartition.getEquipeId().toString() + "\n";
    txt += "Route : " + repartition.getRouteId().toString() + "\n";
    txt += "Statut: " + repartition.getStatutLabel() + "\n";
    txt += "Date  : " + repartition.getDateRepartition().toString("dd/MM/yyyy") + "\n";
    txt += "Articles sortis :\n";
    for(const auto& art : repartition.getArticles())
    txt += QString("— %1 : %2\n").arg(ArticleRepartition::getNomProduitDepuisId(art.getProduitId())).arg(art.getQuantiteTotale());
    QMessageBox::information(this, "Info Statut Répartition", txt);
}

void VueRepartition::chargerRetours()
{
    int row = m_tableauRepartition->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection requise", "Sélectionnez une répartition à clôturer.");
        return;
    }
    QUuid repId = m_tableauRepartition->repartitionIdFromRow(row);
    if (repId.isNull()) return;

    auto repartition = g_repartitionMgr->obtenirRepartition(repId, true);
    QList<LigneRetourRepartition> lignes;
    for(const auto& art : repartition.getArticles()) {
        LigneRetourRepartition ligne;
        ligne.produitNom = ArticleRepartition::getNomProduitDepuisId(art.getProduitId());
        ligne.produitId  = art.getProduitId();
        ligne.quantiteSortie = art.getQuantiteTotale();
        ligne.prixUnitaire = 0.0;
        ligne.quantiteVenduCash = 0;
        ligne.quantiteVenduCredit = 0;
        ligne.quantiteInvendu = 0;
        ligne.quantiteBonus = 0;
        lignes.append(ligne);
    }
    BoiteDialogRetourRepartition boite(lignes, this);
    if (boite.exec() == QDialog::Accepted) {
        const auto retours = boite.resultats();

        for(const auto& ligne : retours) {
            qDebug() << "TRAITEMENT Ligne produitId=" << ligne.produitId;

            // Vérifie les managers globaux
            Q_ASSERT(g_venteMgr); Q_ASSERT(g_creditMgr); Q_ASSERT(g_stockMgr);

            // Vérifie la validité du produit
            auto prodRepo = RepositoryProduit();
            auto produit = prodRepo.getById(ligne.produitId);
            if (produit.getNom().isEmpty()) {
                qDebug() << "Produit introuvable pour l'id" << ligne.produitId;
                QMessageBox::warning(this, "Erreur", "Le produit est introuvable.");
                continue;
            }
            double vraiPrixUnitaire = produit.getPrixUnitaire();
            qDebug() << "Prix récupéré:" << vraiPrixUnitaire;

            if (ligne.quantiteVenduCash > 0) {
                qDebug() << "Enregistrer vente CASH";
                g_venteMgr->enregistrerVente(repId, ligne.produitId, g_utilisateurId, ligne.quantiteVenduCash, "CASH", vraiPrixUnitaire);
            }
            if (ligne.quantiteVenduCredit > 0) {
                qDebug() << "Enregistrer vente CREDIT";
                QUuid venteId = g_venteMgr->enregistrerVente(repId, ligne.produitId, g_utilisateurId, ligne.quantiteVenduCredit, "CREDIT", vraiPrixUnitaire);
                if (!venteId.isNull()) {
                    g_creditMgr->creerCredit(venteId, g_utilisateurId, ligne.quantiteVenduCredit * vraiPrixUnitaire, QDate::currentDate().addDays(30));
                }
            }
            if (ligne.quantiteInvendu > 0) {
                qDebug() << "Créer retour stock";
                g_stockMgr->creerRetourApresRepartition(ligne.produitId, ligne.quantiteInvendu, repId, QUuid(), "Retour invendus répartition", g_utilisateurId);
            }
        }
        if (!g_repartitionMgr->marquerCompletee(repId)) {
            QMessageBox::warning(this, "Erreur", "Impossible de clôturer la répartition :\n" + g_repartitionMgr->getDernierErreur());
        } else {
            QMessageBox::information(this, "Clôture répartition", "Ventes, crédits et retours générés !\nRépartition clôturée.");
            m_tableauRepartition->rafraichir();
        }
    }
}


void VueRepartition::filtrerParStatut()
{
    QString statut = m_comboStatut->currentText();
    m_tableauRepartition->filtrerParStatut(statut);
}