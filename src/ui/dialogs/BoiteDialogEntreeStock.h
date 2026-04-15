#ifndef BOITEDIALOGENTREESTOCK_H
#define BOITEDIALOGENTREESTOCK_H

#include <QDialog>
#include <QUuid>

class GestionnaireStock;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QDateEdit;
class QComboBox;
class QPushButton;
class QTextEdit;

class BoiteDialogEntreeStock : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogEntreeStock(GestionnaireStock* gestionnaire, const QUuid& utilisateurId, QWidget* parent = nullptr);
    ~BoiteDialogEntreeStock();

private slots:
    void onValider();
    void onAnnuler();

private:
    void initializeUI();

    GestionnaireStock* m_gestionnaire;
    QUuid m_utilisateurId;

    QComboBox* m_comboProduit;
    QComboBox* m_comboSource;
    QSpinBox* m_spinQuantite;
    QDoubleSpinBox* m_spinPrix;
    QLineEdit* m_editNumeroFacture;
    QLineEdit* m_editNumeroLot;
    QDateEdit* m_dateExpiration;
    QTextEdit* m_editObservations;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;
};

#endif // BOITEDIALOGENTREESTOCK_H