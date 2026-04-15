#include "BoiteDialogEntreeStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../core/entities/EntreeStock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QDebug>

BoiteDialogEntreeStock::BoiteDialogEntreeStock(GestionnaireStock* gestionnaire, const QUuid& utilisateurId, QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(gestionnaire),
      m_utilisateurId(utilisateurId)
{
    qDebug() << "[DIALOG ENTREE] Initialisation";
    initializeUI();
    setWindowTitle("Nouvelle Entrée de Stock");
    setModal(true);
    setMinimumWidth(500);
}

BoiteDialogEntreeStock::~BoiteDialogEntreeStock()
{
}

void BoiteDialogEntreeStock::initializeUI()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);

    QFormLayout* form = new QFormLayout();

    // Produit
    m_comboProduit = new QComboBox();
    m_comboProduit->addItem("Sélectionner un produit...");
    // TODO: Remplir depuis tous les produits
    form->addRow("Produit:", m_comboProduit);

    // Source
    m_comboSource = new QComboBox();
    m_comboSource->addItem("PRODUCTION");
    m_comboSource->addItem("ACHAT");
    m_comboSource->addItem("RETOUR");
    m_comboSource->addItem("AJUSTEMENT");
    form->addRow("Source:", m_comboSource);

    // Quantité
    m_spinQuantite = new QSpinBox();
    m_spinQuantite->setMinimum(1);
    m_spinQuantite->setMaximum(999999);
    form->addRow("Quantité:", m_spinQuantite);

    // Prix unitaire
    m_spinPrix = new QDoubleSpinBox();
    m_spinPrix->setMinimum(0.01);
    m_spinPrix->setMaximum(999999.99);
    m_spinPrix->setDecimals(2);
    form->addRow("Prix Unitaire (€):", m_spinPrix);

    // Numéro facture
    m_editNumeroFacture = new QLineEdit();
    m_editNumeroFacture->setPlaceholderText("Ex: FAC-2026-001");
    form->addRow("N° Facture:", m_editNumeroFacture);

    // Numéro lot
    m_editNumeroLot = new QLineEdit();
    m_editNumeroLot->setPlaceholderText("Ex: LOT-2026-01");
    form->addRow("N° Lot:", m_editNumeroLot);

    // Date expiration
    m_dateExpiration = new QDateEdit();
    m_dateExpiration->setDate(QDate::currentDate().addMonths(6));
    m_dateExpiration->setCalendarPopup(true);
    form->addRow("Date Expiration:", m_dateExpiration);

    // Observations
    m_editObservations = new QTextEdit();
    m_editObservations->setPlaceholderText("Observations supplémentaires...");
    m_editObservations->setMaximumHeight(80);
    form->addRow("Observations:", m_editObservations);

    layoutPrincipal->addLayout(form);

    // Boutons
    QHBoxLayout* boutonsLayout = new QHBoxLayout();
    m_btnValider = new QPushButton("✓ Enregistrer");
    m_btnAnnuler = new QPushButton("✕ Annuler");
    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onValider);
    connect(m_btnAnnuler, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onAnnuler);

    boutonsLayout->addStretch();
    boutonsLayout->addWidget(m_btnValider);
    boutonsLayout->addWidget(m_btnAnnuler);

    layoutPrincipal->addLayout(boutonsLayout);
    setLayout(layoutPrincipal);
}

void BoiteDialogEntreeStock::onValider()
{
    qDebug() << "[DIALOG ENTREE] Validation";

    if (m_spinQuantite->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "La quantité doit être positive");
        return;
    }

    EntreeStock entree;
    entree.setQuantite(m_spinQuantite->value());
    entree.setPrixUnitaire(m_spinPrix->value());
    entree.setNumeroFacture(m_editNumeroFacture->text());
    entree.setNumeroLot(m_editNumeroLot->text());
    entree.setDateExpiration(m_dateExpiration->date());
    entree.setCreePar(m_utilisateurId);

    if (m_gestionnaire->creerEntreeStock(entree)) {
        QMessageBox::information(this, "Succès", "Entrée de stock créée avec succès");
        accept();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la création: " + m_gestionnaire->obtenirDernierErreur());
    }
}

void BoiteDialogEntreeStock::onAnnuler()
{
    reject();
}