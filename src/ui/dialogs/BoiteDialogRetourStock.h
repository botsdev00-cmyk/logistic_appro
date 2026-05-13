#ifndef BOITEDIALOGRETOURSTOCK_H
#define BOITEDIALOGRETOURSTOCK_H

#include <QDialog>
#include <QUuid>
#include <QLabel>

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
                                    const QUuid& utilisateurId,
                                    QWidget* parent = nullptr,
                                    const QUuid& repartitionPreselectionnee = QUuid());
    ~BoiteDialogRetourStock();

    void setEquipe(const QString& nomEquipe);
    void setProduit(const QUuid& produitId, const QString& nomProduit);
    void setQuantite(int quantite);

    QUuid repartitionId() const { return m_repartitionId; } // Pour consulter le choix (optionnel)

private slots:
    void onValider();
    void onAnnuler();

private:
    void initializeUI();
    void chargerProduits();
    void chargerRaisons();
    void chargerRepartitionPreremplie();

    GestionnaireStock* m_gestionnaire;
    GestionnaireRaisonsRetour* m_gestionnaireRaisons;
    GestionnaireRepartition* m_gestionnaireRepartition;
    QUuid m_utilisateurId;
    QUuid m_repartitionId; // interne, car plus de sélection utilisateur

    QComboBox* m_comboProduit;
    QComboBox* m_comboRaison;
    QLabel* m_labelRepartition; // Remplace le QComboBox
    QLabel* m_labelEquipe;
    QSpinBox* m_spinQuantite;
    QTextEdit* m_editObservations;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;
};

#endif // BOITEDIALOGRETOURSTOCK_H
