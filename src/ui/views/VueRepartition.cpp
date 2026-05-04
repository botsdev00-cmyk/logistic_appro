#include "VueRepartition.h"
#include "../widgets/TableauRepartition.h"
#include "../dialogs/BoiteDialogRepartition.h"
#include "../dialogs/BoiteDialogRetourRepartition.h"
#include "../../business/managers/GestionnaireSales.h"
#include "../../business/managers/GestionnaireCredit.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../business/managers/GestionnaireRaisonsRetour.h"
#include "../../core/entities/ArticleRepartition.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryStockMouvements.h"
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
    // 1. Récupération de la répartition actuellement sélectionnée dans le tableau
    int rowSelectionne = m_tableauRepartition->currentRow();
    if (rowSelectionne < 0) return;
    QUuid repId = m_tableauRepartition->repartitionIdFromRow(rowSelectionne);

    // 2. Récupérer la liste des articles concernés (à adapter selon ton flux)
    Repartition repartition = g_repartitionMgr->obtenirRepartition(repId, true);
    QList<LigneRetourRepartition> lignesProduits;
    for (const auto& art : repartition.getArticles()) {
        LigneRetourRepartition ligne;
        ligne.produitNom = ArticleRepartition::getNomProduitDepuisId(art.getProduitId());
        ligne.produitId  = art.getProduitId();
        ligne.quantiteSortie = art.getQuantiteTotale();
        ligne.prixUnitaire = 0.0;
        ligne.quantiteVenduCash = 0;
        ligne.quantiteVenduCredit = 0;
        ligne.quantiteInvendu = 0;
        ligne.quantiteBonus = 0;
        lignesProduits.append(ligne);
    }

    // récupère la liste des produits/articles de la répartition !
    // Si tu ouvres une boîte de dialogue, fais plutôt :
    BoiteDialogRetourRepartition boiteDialogue(lignesProduits, this);
    if (boiteDialogue.exec() != QDialog::Accepted)
        return;
    QList<LigneRetourRepartition> retours = boiteDialogue.resultats();

    // 3. Chercher la raison "invendu"
    GestionnaireRaisonsRetour raisonsMgr;
    QUuid raisonInvenduId;
    for (const auto& raison : raisonsMgr.obtenirRaisons()) {
        if (raison.nom.contains("inven", Qt::CaseInsensitive)) {
            raisonInvenduId = raison.raisonId;
            break;
        }
    }
    if (raisonInvenduId.isNull()) {
        QMessageBox::warning(this, "Erreur", "Aucune raison « invendu » trouvée. Impossible de continuer !");
        return;
    }

    // 4. Traitement de chaque ligne de retour
    bool erreurRetour = false;
    QStringList erreursRetour;

    for (const auto& ligne : retours) {
        // Création du retour invendu
        if (ligne.quantiteInvendu > 0) {
            bool okInvendu = g_stockMgr->creerRetourApresRepartition(
                ligne.produitId,
                ligne.quantiteInvendu,
                repId,
                raisonInvenduId,
                "Retour invendus répartition",
                g_utilisateurId
                );
            if (!okInvendu) {
                erreurRetour = true;
                erreursRetour << QString("Produit %1 : %2")
                                     .arg(ligne.produitNom)
                                     .arg(g_stockMgr->obtenirDernierErreur());
            }
        }

        // Mouvement pour vider le IN_TRANSIT
        if (ligne.quantiteSortie > 0) {
            RepositoryStockMouvements repoMvt;
            ResultatMouvement resMvt = repoMvt.creerMouvementSecurise(
                ligne.produitId,
                "SORTIE",
                -ligne.quantiteSortie,
                repId,
                "REPARTITION",
                g_utilisateurId,
                "IN_TRANSIT",
                "Clôture répartition : purge transit",
                ""
                );
            if (!resMvt.success) {
                qWarning() << "Erreur purge IN_TRANSIT pour" << ligne.produitNom << ":" << resMvt.message;
            }
        }
    }

    // 5. Statut de la répartition
    if (!g_repartitionMgr->marquerCompletee(repId)) {
        QMessageBox::warning(this, "Erreur", "Impossible de clôturer la répartition :\n" + g_repartitionMgr->getDernierErreur());
        return;
    }

    // 6. Notifications utilisateur
    if (erreurRetour) {
        QMessageBox::warning(this, "Retour(s) non créés", "Certains retours invendus n'ont pas pu être créés :\n" + erreursRetour.join("\n"));
    } else {
        QMessageBox::information(this, "Clôture répartition", "Ventes, crédits et retours générés !\nRépartition clôturée et synchronisée.");
    }

    m_tableauRepartition->rafraichir();
}


void VueRepartition::filtrerParStatut()
{
    QString statut = m_comboStatut->currentText();
    m_tableauRepartition->filtrerParStatut(statut);
}