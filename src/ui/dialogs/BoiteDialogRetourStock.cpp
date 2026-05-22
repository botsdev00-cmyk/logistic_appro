#include "BoiteDialogRetourStock.h"
#include "../../core/entities/RetourStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../business/managers/GestionnaireRaisonsRetour.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryRoute.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

BoiteDialogRetourStock::BoiteDialogRetourStock(
    GestionnaireStock* gestionnaire,
    GestionnaireRaisonsRetour* gestionnaireRaisons,
    GestionnaireRepartition* gestionnaireRepartition,
    const QUuid& utilisateurId,
    QWidget* parent,
    const QUuid& repartitionPreselectionnee
    ) : QDialog(parent),
    m_gestionnaire(gestionnaire),
    m_gestionnaireRaisons(gestionnaireRaisons),
    m_gestionnaireRepartition(gestionnaireRepartition),
    m_utilisateurId(utilisateurId),
    m_repartitionId(repartitionPreselectionnee)
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

    // Répartition d'origine (AFFICHAGE SEULEMENT)
    m_labelRepartition = new QLabel(this);
    m_labelRepartition->setStyleSheet("font-weight:bold; background:#f5f5f5; padding:2px;");
    form->addRow("Répartition d'origine :", m_labelRepartition);

    // Équipe
    m_labelEquipe = new QLabel("(Non renseignée)", this);
    form->addRow("Équipe :", m_labelEquipe);

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
    chargerRepartitionPreremplie();
}

void BoiteDialogRetourStock::chargerProduits()
{
    m_comboProduit->clear();
    m_comboProduit->addItem("Sélectionner…", QVariant());
    if (m_gestionnaire) {
        for (const auto& stock : m_gestionnaire->obtenirTousLesStocks()) {
            m_comboProduit->addItem(stock.produitNom + " (" + stock.codeSKU + ")", QVariant(stock.produitId));
        }
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

void BoiteDialogRetourStock::chargerRepartitionPreremplie()
{
    // 1. Déclare la variable label en haut
    QString label = m_repartitionId.isNull() ? "(Non renseignée)" : m_repartitionId.toString();
    QString nomEquipe = "(Inconnue)";

    // 2. Essaie de récupérer des infos additionnelles, optionnelles
    if (m_gestionnaireRepartition && !m_repartitionId.isNull()) {
        auto rep = m_gestionnaireRepartition->obtenirRepartition(m_repartitionId, false);
        if (!rep.getRepartitionId().isNull()) {
            RepositoryEquipe repoEquipe;
            RepositoryRoute repoRoute;
            auto optEq = repoEquipe.getById(rep.getEquipeId());
            if (optEq && !optEq->getEquipeId().isNull())
                nomEquipe = optEq->getNom();

            QString nomRoute  = repoRoute.getById(rep.getRouteId()).getNom();
            label = QString("%1 | %2 | %3 | %4")
                        .arg(m_repartitionId.toString())
                        .arg(nomEquipe)
                        .arg(nomRoute)
                        .arg(rep.getDateRepartition().toString("dd/MM/yy"));
            setEquipe(nomEquipe);
        } else {
            setEquipe("(Non trouvée)");
        }
    } else {
        setEquipe("(Non trouvée)");
    }
    m_labelRepartition->setText(label);
}

void BoiteDialogRetourStock::setEquipe(const QString& nomEquipe)
{
    if (m_labelEquipe)
        m_labelEquipe->setText(nomEquipe);
}

void BoiteDialogRetourStock::setProduit(const QUuid& produitId, const QString& nomProduit)
{
    for (int i = 0; i < m_comboProduit->count(); ++i) {
        if (m_comboProduit->itemData(i).toUuid() == produitId) {
            m_comboProduit->setCurrentIndex(i);
            m_comboProduit->setEnabled(false);
            break;
        }
    }
    m_comboProduit->setCurrentText(nomProduit);
}

void BoiteDialogRetourStock::setQuantite(int quantite)
{
    m_spinQuantite->setValue(quantite);
    m_spinQuantite->setReadOnly(true);
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
    int quantite = m_spinQuantite->value();
    QString observations = m_editObservations->toPlainText();

    bool ok = false;
    // On ne permet plus de retour sans répartition en contexte clôture répartition
    if (!m_repartitionId.isNull()) {
        ok = m_gestionnaire->creerRetourApresRepartition(
            produitId, quantite, m_repartitionId, raisonId, observations, m_utilisateurId
            );
    } else {
        QMessageBox::warning(this, "Erreur", "Aucune répartition sélectionnée !");
        return;
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
