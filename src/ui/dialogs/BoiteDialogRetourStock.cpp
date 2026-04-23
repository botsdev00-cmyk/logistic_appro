// BoiteDialogRetourStock.cpp

#include "BoiteDialogRetourStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

BoiteDialogRetourStock::BoiteDialogRetourStock(GestionnaireStock* gestionnaire, const QUuid& utilisateurId, QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(gestionnaire),
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

    // Répartition
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

    // Peuplement des combos
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
    // TODO: À adapter si tu as une table des raisons de retour dynamique.
    m_comboRaison->addItem("Invendu", QVariant(/*raisonId*/ QUuid::createUuid())); // ou un UUID constant par type
    m_comboRaison->addItem("Endommagé", QVariant(QUuid::createUuid()));
    m_comboRaison->addItem("Expiré", QVariant(QUuid::createUuid()));
    m_comboRaison->addItem("Erreur", QVariant(QUuid::createUuid()));
}

void BoiteDialogRetourStock::chargerRepartitions()
{
    m_comboRepartition->clear();
    m_comboRepartition->addItem("--Aucune répartition--", QVariant());
    // À remplacer par ton gestionnaire de répartition si plusieurs disponibles
    // ex : auto gestionnaireRepartition = ...;
    // for (auto& rep : gestionnaireRepartition->obtenirRepartitionsEnCours()) {
    //     m_comboRepartition->addItem(rep.getTitreOuResume(), QVariant(rep.getRepartitionId()));
    // }
}

void BoiteDialogRetourStock::setRepartitionPreselectionnee(const QUuid& repId)
{
    // Recherche dans le combo et sélectionne la répartition fournie
    for (int i=0; i < m_comboRepartition->count(); ++i) {
        if (m_comboRepartition->itemData(i).toUuid() == repId) {
            m_comboRepartition->setCurrentIndex(i);
            m_comboRepartition->setEnabled(false); // Évite de changer si retour imposé
            break;
        }
    }
}

void BoiteDialogRetourStock::onValider()
{
    qDebug() << "[DIALOG RETOUR] Validation";

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
    // Priorité à méthode dédiée “retour après répartition”
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
        QMessageBox::information(this, QString::fromUtf8("Succès"), QString::fromUtf8("Retour de stock créé avec succès"));
        accept();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("Erreur"), QString::fromUtf8("Erreur lors de la création: ") + m_gestionnaire->obtenirDernierErreur());
    }
}

void BoiteDialogRetourStock::onAnnuler()
{
    reject();
}