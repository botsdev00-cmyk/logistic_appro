#ifndef BOITEDIALOGVENTE_H
#define BOITEDIALOGVENTE_H

#include <QDialog>
#include <QUuid>
#include <memory>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTextEdit;
class QPushButton;
class QLabel;
class GestionnaireSales;

class BoiteDialogVente : public QDialog
{
    Q_OBJECT

public:
    BoiteDialogVente(const QUuid& repartitionId, QWidget* parent = nullptr);
    ~BoiteDialogVente();

private slots:
    void enregistrerVente();
    void mettreAJourPrix();

private:
    void creerWidgets();
    void initialiserConnexions();
    void chargerDonnees();
    void remplirCombos();

    std::unique_ptr<QComboBox> m_comboProduit;
    std::unique_ptr<QComboBox> m_comboClient;
    std::unique_ptr<QComboBox> m_comboType;
    std::unique_ptr<QSpinBox> m_spinQuantite;
    std::unique_ptr<QDoubleSpinBox> m_doubleSpinPrix;
    std::unique_ptr<QLabel> m_labelMontant;
    std::unique_ptr<QTextEdit> m_textNotes;
    std::unique_ptr<QPushButton> m_boutonEnregistrer;
    std::unique_ptr<QPushButton> m_boutonAnnuler;
    
    std::unique_ptr<GestionnaireSales> m_gestionnaire;
    QUuid m_repartitionId;
};

#endif // BOITEDIALOGVENTE_H