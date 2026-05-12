#include "BoiteDialogNouvelleEquipe.h"
#include "../../business/managers/GestionnaireEquipe.h"
#include "../../core/entities/Utilisateur.h"
#include "../../data/repositories/RepositoryUtilisateur.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>

BoiteDialogNouvelleEquipe::BoiteDialogNouvelleEquipe(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Nouvelle équipe");
    setMinimumWidth(350);

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Nom équipe
    QHBoxLayout* nomLayout = new QHBoxLayout;
    nomLayout->addWidget(new QLabel("Nom :"));
    m_editNom = new QLineEdit;
    nomLayout->addWidget(m_editNom);
    layout->addLayout(nomLayout);

    // Chef d’équipe
    QHBoxLayout* chefLayout = new QHBoxLayout;
    chefLayout->addWidget(new QLabel("Chef :"));
    m_comboChef = new QComboBox;
    chefLayout->addWidget(m_comboChef);
    layout->addLayout(chefLayout);

    // // Description
    // QHBoxLayout* descLayout = new QHBoxLayout;
    // descLayout->addWidget(new QLabel("Description :"));
    // m_editDescription = new QTextEdit;
    // m_editDescription->setFixedHeight(40);
    // descLayout->addWidget(m_editDescription);
    // layout->addLayout(descLayout);

    // Membres
    layout->addWidget(new QLabel("Membres :"));
    m_listeMembres = new QListWidget;
    m_listeMembres->setSelectionMode(QAbstractItemView::MultiSelection);
    layout->addWidget(m_listeMembres);

    // Récupération des utilisateurs
    RepositoryUtilisateur repoUtil;
    QList<Utilisateur> userList = repoUtil.getAll();
    for (const Utilisateur& u : userList) {
        QString displayName = u.getNomComplet();
        QUuid uid = u.getUtilisateurId();

        m_comboChef->addItem(displayName, uid);
        m_utilisateurs[displayName] = uid;

        QListWidgetItem* item = new QListWidgetItem(displayName, m_listeMembres);
        item->setData(Qt::UserRole, uid);
    }

    // Boutons
    QHBoxLayout* btnLayout = new QHBoxLayout;
    m_btnValider = new QPushButton("Créer");
    m_btnAnnuler = new QPushButton("Annuler");
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnValider);
    btnLayout->addWidget(m_btnAnnuler);
    layout->addLayout(btnLayout);

    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogNouvelleEquipe::onValider);
    connect(m_btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
}

void BoiteDialogNouvelleEquipe::onValider()
{
    QString nom = m_editNom->text().trimmed();
    int iChef = m_comboChef->currentIndex();
    QString nomChef = m_comboChef->currentText();
    QUuid chefId = m_comboChef->currentData().toUuid();

    if (nom.isEmpty() || iChef < 0 || nomChef.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir le nom, sélectionner un chef et remplir ses informations.");
        return;
    }

    // Membres sélectionnés (hors chef)
    QList<QUuid> membres;
    for (QListWidgetItem* item : m_listeMembres->selectedItems()) {
        QUuid uid = item->data(Qt::UserRole).toUuid();
        if (uid != chefId)
            membres << uid;
    }

    // Création via manager (utilisation du chef comme createdBy)
    GestionnaireEquipe manager;
    m_equipeId = manager.creerEquipe(nom, nomChef, /*description,*/ chefId);
    if (m_equipeId.isNull()) {
        QMessageBox::critical(this, "Erreur", "Création de l'équipe impossible.");
        return;
    }
    m_nomEquipe = nom;
    accept();
}
