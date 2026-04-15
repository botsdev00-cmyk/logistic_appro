#include "BoiteDialogRetourStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../core/entities/RetourStock.h"
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
    qDebug() << "[DIALOG RETOUR] Initialisation";
    initializeUI();
    setWindowTitle("Nouveau Retour de Stock");
    setModal(true);
    setMinimumWidth(500);
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
    m_comboProduit->addItem("Sélectionner un produit...");
    // TODO: Remplir depuis tous les produits
    form->addRow("Produit:", m_comboProduit);

    // Raison
    m_comboRaison = new QComboBox();
    m_comboRaison->addItem("INVENDU");
    m_comboRaison->addItem("ENDOMMAGE");
    m_comboRaison->addItem("EXPIRE");
    m_comboRaison->addItem("ERREUR");
    form->addRow("Raison du retour:", m_comboRaison);

    // Repartition (optionnel)
    m_comboRepartition = new QComboBox();
    m_comboRepartition->addItem("-- Aucune repartition --");
    // TODO: Remplir depuis les repartitions
    form->addRow("Repartition:", m_comboRepartition);

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
}

void BoiteDialogRetourStock::onValider()
{
    qDebug() << "[DIALOG RETOUR] Validation";

    if (m_spinQuantite->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "La quantité doit être positive");
        return;
    }

    RetourStock retour;
    retour.setQuantite(m_spinQuantite->value());
    retour.setObservations(m_editObservations->toPlainText());
    retour.setCreePar(m_utilisateurId);

    if (m_gestionnaire->creerRetourStock(retour)) {
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