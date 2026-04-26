#include "BoiteDialogRetourStock.h"
#include "../../core/entities/RetourStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../business/managers/GestionnaireRaisonsRetour.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryRoute.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryRoute.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

BoiteDialogRetourStock::BoiteDialogRetourStock(GestionnaireStock* gestionnaire,
                                               GestionnaireRaisonsRetour* gestionnaireRaisons,
                                               GestionnaireRepartition* gestionnaireRepartition,
                                               const QUuid& utilisateurId,
                                               QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(gestionnaire),
      m_gestionnaireRaisons(gestionnaireRaisons),
      m_gestionnaireRepartition(gestionnaireRepartition),
      m_utilisateurId(utilisateurId)
{
    setWindowTitle("Nouveau Retour de Stock");
    setModal(true);
    setMinimumWidth(500);
    initializeUI();
}

BoiteDialogRetourStock::~BoiteDialogRetourStock()
{
}

void BoiteDialogRetourStock::initializeUI()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    // Produit
    m_comboProduit = new QComboBox();
    form->addRow("Produit à retourner:", m_comboProduit);

    // Raison
    m_comboRaison = new QComboBox();
    form->addRow("Raison du retour:", m_comboRaison);

    // Répartition d'origine
    m_comboRepartition = new QComboBox();
    form->addRow("Répartition d'origine :", m_comboRepartition);

    // Quantité
    m_spinQuantite = new QSpinBox();
    m_spinQuantite->setMinimum(1);
    m_spinQuantite->setMaximum(999999);
    form->addRow("Quantité:", m_spinQuantite);

    // Observations
    m_editObservations = new QTextEdit();
    m_editObservations->setPlaceholderText("Observations supplémentaires...");
    m_editObservations->setMaximumHeight(100);
    form->addRow("Observations:", m_editObservations);

    layoutPrincipal->addLayout(form);

    // Boutons
    QHBoxLayout* boutonsLayout = new QHBoxLayout();
    m_btnValider = new QPushButton("✓ Enregistrer");
    m_btnAnnuler = new QPushButton("✕ Annuler");
    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogRetourStock::onValider);
    connect(m_btnAnnuler, &QPushButton::clicked, this, &BoiteDialogRetourStock::onAnnuler);
    boutonsLayout->addStretch();
    boutonsLayout->addWidget(m_btnValider);
    boutonsLayout->addWidget(m_btnAnnuler);

    layoutPrincipal->addLayout(boutonsLayout);
    setLayout(layoutPrincipal);

    // Peuplement
    chargerProduits();
    chargerRaisons();
    chargerRepartitions();
}

void BoiteDialogRetourStock::chargerProduits()
{
    m_comboProduit->clear();
    m_comboProduit->addItem("Sélectionner…", QVariant());
    for (const auto& stock : m_gestionnaire->obtenirTousLesStocks()) {
        m_comboProduit->addItem(stock.produitNom + " (" + stock.codeSKU + ")", QVariant(stock.produitId));
    }
}

void BoiteDialogRetourStock::chargerRaisons()
{
    m_comboRaison->clear();
    if (m_gestionnaireRaisons) {
        for (const auto& raison : m_gestionnaireRaisons->obtenirRaisons()) {
            m_comboRaison->addItem(raison.nom, QVariant(raison.raisonId));
        }
    }
}

void BoiteDialogRetourStock::chargerRepartitions()
{
    m_comboRepartition->clear();
    m_comboRepartition->addItem("--Aucune répartition--", QVariant());
    if (m_gestionnaireRepartition) {
        for (const auto& rep : m_gestionnaireRepartition->obtenirRepartitionsEnCours()) {
            QString nomEquipe;
            QString nomRoute;

            RepositoryEquipe repoEquipe;
            RepositoryRoute repoRoute;

            // Récupère le nom depuis l’ID
            nomEquipe = repoEquipe.getById(rep.getEquipeId()).getNom();
            nomRoute  = repoRoute.getById(rep.getRouteId()).getNom();

            QString label = QString("Équipe: %1 | Route: %2 | %3")
                                .arg(nomEquipe)
                                .arg(nomRoute)
                                .arg(rep.getDateRepartition().toString("dd/MM/yy"));
            m_comboRepartition->addItem(label, QVariant(rep.getRepartitionId()));
        }
    }
}

void BoiteDialogRetourStock::setRepartitionPreselectionnee(const QUuid& repId)
{
    for (int i=0; i < m_comboRepartition->count(); ++i) {
        if (m_comboRepartition->itemData(i).toUuid() == repId) {
            m_comboRepartition->setCurrentIndex(i);
            m_comboRepartition->setEnabled(false);
            break;
        }
    }
}

void BoiteDialogRetourStock::onValider()
{
    if (m_comboProduit->currentIndex() <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un produit.");
        return;
    }
    if (m_comboRaison->currentIndex() < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une raison.");
        return;
    }
    if (m_spinQuantite->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "La quantité doit être positive.");
        return;
    }

    QUuid produitId = m_comboProduit->currentData().toUuid();
    QUuid raisonId = m_comboRaison->currentData().toUuid();
    QUuid repartitionId = m_comboRepartition->currentData().toUuid();
    int quantite = m_spinQuantite->value();
    QString observations = m_editObservations->toPlainText();

    bool ok = false;
    // Si répartition renseignée, utilise la méthode dédiée
    if (!repartitionId.isNull()) {
        ok = m_gestionnaire->creerRetourApresRepartition(
            produitId, quantite, repartitionId, raisonId, observations, m_utilisateurId
        );
    } else {
        RetourStock retour;
        retour.setProduitId(produitId);
        retour.setQuantite(quantite);
        retour.setRaisonRetourId(raisonId);
        retour.setRepartitionId(QUuid());
        retour.setObservations(observations);
        retour.setCreePar(m_utilisateurId);
        retour.setStatutValidation("EN_ATTENTE");
        ok = m_gestionnaire->creerRetourStock(retour);
    }
    if (ok) {
        QMessageBox::information(this, "Succès", "Retour de stock créé avec succès");
        accept();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la création: " + m_gestionnaire->obtenirDernierErreur());
    }
}

void BoiteDialogRetourStock::onAnnuler()
{
    reject();
}