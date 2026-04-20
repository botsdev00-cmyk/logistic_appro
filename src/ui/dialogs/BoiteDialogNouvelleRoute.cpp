#include "BoiteDialogNouvelleRoute.h"
#include "../../business/managers/GestionnaireRoute.h" // À faire si inexistant
#include "../../core/entities/Route.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

BoiteDialogNouvelleRoute::BoiteDialogNouvelleRoute(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Nouvelle route");
    setMinimumWidth(300);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout* nomLayout = new QHBoxLayout;
    nomLayout->addWidget(new QLabel("Nom :"));
    m_editNom = new QLineEdit;
    nomLayout->addWidget(m_editNom);
    layout->addLayout(nomLayout);

    QHBoxLayout* descLayout = new QHBoxLayout;
    descLayout->addWidget(new QLabel("Description :"));
    m_editDescription = new QLineEdit;
    descLayout->addWidget(m_editDescription);
    layout->addLayout(descLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    m_btnValider = new QPushButton("Créer");
    m_btnAnnuler = new QPushButton("Annuler");
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnValider);
    btnLayout->addWidget(m_btnAnnuler);
    layout->addLayout(btnLayout);

    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogNouvelleRoute::onValider);
    connect(m_btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
}

void BoiteDialogNouvelleRoute::onValider()
{
    QString nom = m_editNom->text().trimmed();
    QString desc = m_editDescription->text().trimmed();
    if (nom.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir le nom de la route.");
        return;
    }

    GestionnaireRoute manager;
    m_routeId = manager.creerRoute(nom, desc);
    if (m_routeId.isNull()) {
        QMessageBox::critical(this, "Erreur", "Création de la route impossible.");
        return;
    }
    m_nomRoute = nom;
    accept();
}