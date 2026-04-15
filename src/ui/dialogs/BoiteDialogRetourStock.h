#ifndef BOITEDIALOGRETOURSTOCK_H
#define BOITEDIALOGRETOURSTOCK_H

#include <QDialog>
#include <QUuid>

class GestionnaireStock;
class QComboBox;
class QSpinBox;
class QTextEdit;
class QPushButton;

class BoiteDialogRetourStock : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogRetourStock(GestionnaireStock* gestionnaire, const QUuid& utilisateurId, QWidget* parent = nullptr);
    ~BoiteDialogRetourStock();

private slots:
    void onValider();
    void onAnnuler();

private:
    void initializeUI();

    GestionnaireStock* m_gestionnaire;
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