#include "BoiteDialogVente.h"
#include "../../business/managers/GestionnaireSales.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>

BoiteDialogVente::BoiteDialogVente(const QUuid& repartitionId, QWidget* parent)
    : QDialog(parent),
      m_repartitionId(repartitionId),
      m_gestionnaire(std::make_unique<GestionnaireSales>())
{
    setWindowTitle("Enregistrer une vente");
    setModal(true);
    setFixedSize(500, 500);
    
    creerWidgets();
    initialiserConnexions();
    chargerDonnees();
}

BoiteDialogVente::~BoiteDialogVente()
{
}

void BoiteDialogVente::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    
    QGroupBox* groupVente = new QGroupBox("Détails de la vente");
    QGridLayout* layoutVente = new QGridLayout(groupVente);
    
    // Produit
    layoutVente->addWidget(new QLabel("Produit:"), 0, 0);
    m_comboProduit = std::make_unique<QComboBox>();
    layoutVente->addWidget(m_comboProduit.get(), 0, 1);
    
    // Client
    layoutVente->addWidget(new QLabel("Client:"), 1, 0);
    m_comboClient = std::make_unique<QComboBox>();
    layoutVente->addWidget(m_comboClient.get(), 1, 1);
    
    // Quantité
    layoutVente->addWidget(new QLabel("Quantité:"), 2, 0);
    m_spinQuantite = std::make_unique<QSpinBox>();
    m_spinQuantite->setMinimum(1);
    layoutVente->addWidget(m_spinQuantite.get(), 2, 1);
    
    // Type
    layoutVente->addWidget(new QLabel("Type:"), 3, 0);
    m_comboType = std::make_unique<QComboBox>();
    m_comboType->addItem("Cash", "cash");
    m_comboType->addItem("Crédit", "credit");
    layoutVente->addWidget(m_comboType.get(), 3, 1);
    
    // Prix
    layoutVente->addWidget(new QLabel("Prix unitaire:"), 4, 0);
    m_doubleSpinPrix = std::make_unique<QDoubleSpinBox>();
    m_doubleSpinPrix->setDecimals(2);
    m_doubleSpinPrix->setMinimum(0.01);
    layoutVente->addWidget(m_doubleSpinPrix.get(), 4, 1);
    
    // Montant total
    layoutVente->addWidget(new QLabel("Montant total:"), 5, 0);
    m_labelMontant = std::make_unique<QLabel>("0.00 FCFA");
    m_labelMontant->setStyleSheet("font-weight: bold; color: #27ae60;");
    layoutVente->addWidget(m_labelMontant.get(), 5, 1);
    
    layoutPrincipal->addWidget(groupVente);
    
    // Notes
    layoutPrincipal->addWidget(new QLabel("Notes:"));
    m_textNotes = std::make_unique<QTextEdit>();
    m_textNotes->setMaximumHeight(100);
    layoutPrincipal->addWidget(m_textNotes.get());
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    m_boutonEnregistrer = std::make_unique<QPushButton>("Enregistrer");
    m_boutonEnregistrer->setMinimumWidth(120);
    layoutBoutons->addWidget(m_boutonEnregistrer.get());
    
    m_boutonAnnuler = std::make_unique<QPushButton>("Annuler");
    m_boutonAnnuler->setMinimumWidth(120);
    layoutBoutons->addWidget(m_boutonAnnuler.get());
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void BoiteDialogVente::initialiserConnexions()
{
    connect(m_boutonEnregistrer.get(), &QPushButton::clicked, this, &BoiteDialogVente::enregistrerVente);
    connect(m_boutonAnnuler.get(), &QPushButton::clicked, this, &QDialog::reject);
    connect(m_spinQuantite.get(), QOverload<int>::of(&QSpinBox::valueChanged), this, &BoiteDialogVente::mettreAJourPrix);
    connect(m_doubleSpinPrix.get(), QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoiteDialogVente::mettreAJourPrix);
}

void BoiteDialogVente::chargerDonnees()
{
    remplirCombos();
}

void BoiteDialogVente::remplirCombos()
{
    // À implémenter avec les données
    m_comboProduit->addItem("Vin Rouge Shiraz");
    m_comboProduit->addItem("Vin Blanc Chardonnay");
    m_comboProduit->addItem("Bière Blonde");
    
    m_comboClient->addItem("Boutique du Centre");
    m_comboClient->addItem("Supermarché Nord");
    m_comboClient->addItem("Distributeur Principal");
}

void BoiteDialogVente::enregistrerVente()
{
    if (m_comboProduit->currentText().isEmpty() || m_comboClient->currentText().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs");
        return;
    }
    
    // À implémenter
    QMessageBox::information(this, "Succès", "Vente enregistrée avec succès");
    accept();
}

void BoiteDialogVente::mettreAJourPrix()
{
    double montant = m_spinQuantite->value() * m_doubleSpinPrix->value();
    m_labelMontant->setText(QString::number(montant, 'f', 2) + " FCFA");
}