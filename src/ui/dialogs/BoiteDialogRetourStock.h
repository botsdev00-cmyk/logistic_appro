#ifndef BOITEDIALOGRETOURSTOCK_H
#define BOITEDIALOGRETOURSTOCK_H

#include <QDialog>
#include <QUuid>

class GestionnaireStock;
class GestionnaireRaisonsRetour;
class GestionnaireRepartition;
class QComboBox;
class QSpinBox;
class QTextEdit;
class QPushButton;

class BoiteDialogRetourStock : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogRetourStock(GestionnaireStock* gestionnaire,
                                    GestionnaireRaisonsRetour* gestionnaireRaisons,
                                    GestionnaireRepartition* gestionnaireRepartition,
                                    const QUuid& utilisateurId, QWidget* parent = nullptr);
    ~BoiteDialogRetourStock();

    void setRepartitionPreselectionnee(const QUuid& repId);

private slots:
    void onValider();
    void onAnnuler();

private:
    void initializeUI();
    void chargerProduits();
    void chargerRaisons();
    void chargerRepartitions();

    GestionnaireStock* m_gestionnaire;
    GestionnaireRaisonsRetour* m_gestionnaireRaisons;
    GestionnaireRepartition* m_gestionnaireRepartition;
    QUuid m_utilisateurId;

    QComboBox* m_comboProduit;
    QComboBox* m_comboRaison;
    QComboBox* m_comboRepartition;
    QSpinBox* m_spinQuantite;
    QTextEdit* m_editObservations;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;
};

#endif // BOITEDIALOGRETOURSTOCK_H