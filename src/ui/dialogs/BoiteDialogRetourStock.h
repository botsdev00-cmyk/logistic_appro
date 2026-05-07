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
                                QUuid repartitionPreselectionnee = QUuid());
    ~BoiteDialogRetourStock();

    void setRepartitionPreselectionnee(const QUuid& repId);
    void setEquipe(const QString& nomEquipe);
    void setProduit(const QUuid& produitId, const QString& nomProduit);
    void setQuantite(int quantite);

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
    QLabel* m_labelEquipe;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;
};

#endif // BOITEDIALOGRETOURSTOCK_H