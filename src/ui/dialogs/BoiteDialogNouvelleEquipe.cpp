#include "BoiteDialogNouvelleEquipe.h"
#include "../../business/managers/GestionnaireEquipe.h"
#include "../../core/entities/Employee.h"
#include "../../data/repositories/RepositoryEmployee.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QSqlQuery>
#include <QUuid>

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

    // Membres
    layout->addWidget(new QLabel("Membres :"));
    m_listeMembres = new QListWidget;
    m_listeMembres->setSelectionMode(QAbstractItemView::MultiSelection);
    layout->addWidget(m_listeMembres);

    // Récupération des employés et du grade Chef d'équipe
    RepositoryEmployee repoEmp;
    QList<Employee> allEmployees = repoEmp.getAll();

    // Trouver le grade_id du Chef d'équipe
    QUuid chefEquipeGradeId;
    QSqlQuery q("SELECT grade_id FROM grade WHERE nom = 'Chef équipe' LIMIT 1");
    if (q.next()) {
        chefEquipeGradeId = q.value(0).toUuid();
    }

    // Remplir la combo chef
    for (const Employee& e : allEmployees) {
        QString displayName = e.nom() + " " + e.prenom();
        QUuid eid = e.employeId();
        if (e.gradeId() == chefEquipeGradeId) {
            m_comboChef->addItem(displayName, eid);
            m_utilisateurs[displayName] = eid;
        }
    }

    // Remplir la liste membres (tous les employés)
    for (const Employee& e : allEmployees) {
        QString displayName = e.nom() + " " + e.prenom();
        QUuid eid = e.employeId();
        QListWidgetItem* item = new QListWidgetItem(displayName, m_listeMembres);
        item->setData(Qt::UserRole, eid);
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
