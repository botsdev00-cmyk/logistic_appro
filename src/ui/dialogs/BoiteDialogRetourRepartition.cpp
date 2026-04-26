#include "BoiteDialogRetourRepartition.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QMessageBox>

BoiteDialogRetourRepartition::BoiteDialogRetourRepartition(const QList<LigneRetourRepartition>& donneesProduits, QWidget* parent)
    : QDialog(parent),
      m_lignes(donneesProduits)
{
    setWindowTitle("Clôture de répartition - Saisie des Retours");
    setModal(true);
    setMinimumWidth(850);
    setupUI();
    remplirTableau();
}

void BoiteDialogRetourRepartition::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    QLabel* titre = new QLabel("<b>Merci de compléter la répartition retour :</b>");
    mainLayout->addWidget(titre);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"Produit", "Sortie", "Vendu cash", "Vendu crédit", "Invendus", "Bonus consommés"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(m_table);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_btnValider = new QPushButton("Valider");
    m_btnAnnuler = new QPushButton("Annuler");
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnValider);
    btnLayout->addWidget(m_btnAnnuler);

    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);

    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogRetourRepartition::validerRetour);
    connect(m_btnAnnuler, &QPushButton::clicked, this, &BoiteDialogRetourRepartition::annulerRetour);
}

void BoiteDialogRetourRepartition::remplirTableau()
{
    m_table->setRowCount(m_lignes.size());
    for (int row = 0; row < m_lignes.size(); ++row) {
        const auto& ligne = m_lignes[row];
        auto produitItem = new QTableWidgetItem(ligne.produitNom);
        produitItem->setFlags(produitItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 0, produitItem);

        auto sortieItem = new QTableWidgetItem(QString::number(ligne.quantiteSortie));
        sortieItem->setFlags(sortieItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 1, sortieItem);

        // Création des cellules éditables (QSpinBox)
        for (int col = 2; col <= 5; ++col) {
            QSpinBox* spin = new QSpinBox(m_table);
            spin->setRange(0, ligne.quantiteSortie);
            m_table->setCellWidget(row, col, spin);
        }
    }
}

void BoiteDialogRetourRepartition::validerRetour()
{
    // Vérifie la cohérence AVANT extraction
    QString msgErreur;
    if (!controleCohérence(&msgErreur)) {
        QMessageBox::warning(this, "Cohérence manquante", msgErreur);
        return;
    }

    // Extraction des résultats
    QList<LigneRetourRepartition> retourValide;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        LigneRetourRepartition lig = m_lignes[row];
        lig.quantiteVenduCash   = static_cast<QSpinBox*>(m_table->cellWidget(row, 2))->value();
        lig.quantiteVenduCredit = static_cast<QSpinBox*>(m_table->cellWidget(row, 3))->value();
        lig.quantiteInvendu     = static_cast<QSpinBox*>(m_table->cellWidget(row, 4))->value();
        lig.quantiteBonus       = static_cast<QSpinBox*>(m_table->cellWidget(row, 5))->value();
        retourValide.append(lig);
    }
    m_lignes = retourValide;
    accept();
}

void BoiteDialogRetourRepartition::annulerRetour()
{
    reject();
}

bool BoiteDialogRetourRepartition::controleCohérence(QString* msgErreur)
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        int sortie = m_table->item(row, 1)->text().toInt();
        int cash   = static_cast<QSpinBox*>(m_table->cellWidget(row, 2))->value();
        int credit = static_cast<QSpinBox*>(m_table->cellWidget(row, 3))->value();
        int invendu= static_cast<QSpinBox*>(m_table->cellWidget(row, 4))->value();
        int bonus  = static_cast<QSpinBox*>(m_table->cellWidget(row, 5))->value();
        int somme  = cash + credit + invendu + bonus;
        if (sortie != somme) {
            if (msgErreur) {
                *msgErreur = QString("Produit : %1\nQuantité sortie = %2 ≠ %3 (cash + crédit + invendus + bonus)")
                             .arg(m_table->item(row,0)->text()).arg(sortie).arg(somme);
            }
            return false;
        }
    }
    return true;
}

QList<LigneRetourRepartition> BoiteDialogRetourRepartition::resultats() const
{
    return m_lignes;
}