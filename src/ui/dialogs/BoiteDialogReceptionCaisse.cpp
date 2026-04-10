#include "BoiteDialogReceptionCaisse.h"
#include "../../business/managers/GestionnaireCaisse.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>

BoiteDialogReceptionCaisse::BoiteDialogReceptionCaisse(const QUuid& repartitionId, double montantAttendu, QWidget* parent)
    : QDialog(parent),
      m_repartitionId(repartitionId),
      m_montantAttendu(montantAttendu),
      m_gestionnaire(std::make_unique<GestionnaireCaisse>())
{
    setWindowTitle("Réception de caisse");
    setModal(true);
    setFixedSize(500, 400);
    
    creerWidgets();
    initialiserConnexions();
    
    // Initialiser les valeurs
    m_labelMontantAttendu->setText(QString::number(montantAttendu, 'f', 2) + " FCFA");
    m_doubleMontantRecu->setValue(0.0);
}

BoiteDialogReceptionCaisse::~BoiteDialogReceptionCaisse()
{
}

void BoiteDialogReceptionCaisse::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    
    QGroupBox* groupReception = new QGroupBox("Détails de réception");
    QGridLayout* layoutReception = new QGridLayout(groupReception);
    
    // Numéro reçu
    layoutReception->addWidget(new QLabel("Numéro reçu:"), 0, 0);
    m_champNumeroRecu = std::make_unique<QLineEdit>();
    m_champNumeroRecu->setReadOnly(true);
    m_champNumeroRecu->setText("RC-auto");
    layoutReception->addWidget(m_champNumeroRecu.get(), 0, 1);
    
    // Montant attendu
    layoutReception->addWidget(new QLabel("Montant attendu:"), 1, 0);
    m_labelMontantAttendu = std::make_unique<QLabel>();
    m_labelMontantAttendu->setStyleSheet("font-weight: bold; color: #2c3e50;");
    layoutReception->addWidget(m_labelMontantAttendu.get(), 1, 1);
    
    // Montant reçu
    layoutReception->addWidget(new QLabel("Montant reçu:"), 2, 0);
    m_doubleMontantRecu = std::make_unique<QDoubleSpinBox>();
    m_doubleMontantRecu->setDecimals(2);
    m_doubleMontantRecu->setMinimum(0.0);
    m_doubleMontantRecu->setMaximum(999999.99);
    layoutReception->addWidget(m_doubleMontantRecu.get(), 2, 1);
    
    // Écart
    layoutReception->addWidget(new QLabel("Écart:"), 3, 0);
    m_labelEcart = std::make_unique<QLabel>("0.00 FCFA");
    m_labelEcart->setStyleSheet("font-weight: bold;");
    layoutReception->addWidget(m_labelEcart.get(), 3, 1);
    
    // Statut
    layoutReception->addWidget(new QLabel("Statut:"), 4, 0);
    m_labelStatut = std::make_unique<QLabel>("OK");
    m_labelStatut->setStyleSheet("color: #27ae60; font-weight: bold;");
    layoutReception->addWidget(m_labelStatut.get(), 4, 1);
    
    layoutPrincipal->addWidget(groupReception);
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    m_boutonValider = std::make_unique<QPushButton>("Valider");
    m_boutonValider->setMinimumWidth(120);
    layoutBoutons->addWidget(m_boutonValider.get());
    
    m_boutonAnnuler = std::make_unique<QPushButton>("Annuler");
    m_boutonAnnuler->setMinimumWidth(120);
    layoutBoutons->addWidget(m_boutonAnnuler.get());
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void BoiteDialogReceptionCaisse::initialiserConnexions()
{
    connect(m_boutonValider.get(), &QPushButton::clicked, this, &BoiteDialogReceptionCaisse::validerReception);
    connect(m_boutonAnnuler.get(), &QPushButton::clicked, this, &QDialog::reject);
    connect(m_doubleMontantRecu.get(), QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoiteDialogReceptionCaisse::mettreAJourEcart);
}

void BoiteDialogReceptionCaisse::validerReception()
{
    if (m_doubleMontantRecu->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un montant");
        return;
    }
    
    // À implémenter
    QMessageBox::information(this, "Succès", "Caisse validée avec succès");
    accept();
}

void BoiteDialogReceptionCaisse::mettreAJourEcart()
{
    double ecart = m_montantAttendu - m_doubleMontantRecu->value();
    
    m_labelEcart->setText(QString::number(ecart, 'f', 2) + " FCFA");
    
    if (qAbs(ecart) < 0.01) {
        m_labelStatut->setText("✓ OK");
        m_labelStatut->setStyleSheet("color: #27ae60; font-weight: bold;");
    } else {
        m_labelStatut->setText("⚠ Discordance");
        m_labelStatut->setStyleSheet("color: #e74c3c; font-weight: bold;");
    }
}