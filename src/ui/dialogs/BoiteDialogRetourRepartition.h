#ifndef BOITEDIALOGRETOURREPARTITION_H
#define BOITEDIALOGRETOURREPARTITION_H

#include <QDialog>
#include <QUuid>
#include <QList>
#include <QVector>

class QTableWidget;
class QPushButton;

// --- Structure représentant une ligne de retour saisie (par produit) ---
struct LigneRetourRepartition {
    QString produitNom;
    QUuid produitId;
    int quantiteSortie;
    int quantiteVenduCash;
    int quantiteVenduCredit;
    int quantiteInvendu;
    int quantiteBonus;
};

class BoiteDialogRetourRepartition : public QDialog
{
    Q_OBJECT
public:
    BoiteDialogRetourRepartition(const QList<LigneRetourRepartition>& donneesProduits, QWidget* parent = nullptr);
    QList<LigneRetourRepartition> resultats() const;

private slots:
    void validerRetour();
    void annulerRetour();

private:
    void setupUI();
    void remplirTableau();
    bool controleCohérence(QString* msgErreur);

    QList<LigneRetourRepartition> m_lignes;
    QTableWidget* m_table;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;
};

#endif // BOITEDIALOGRETOURREPARTITION_H