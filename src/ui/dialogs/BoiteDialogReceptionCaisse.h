#ifndef BOITEDIALOGERECEPTIONCAISSE_H
#define BOITEDIALOGERECEPTIONCAISSE_H

#include <QDialog>
#include <QUuid>
#include <memory>

class QLineEdit;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class GestionnaireCaisse;

class BoiteDialogReceptionCaisse : public QDialog
{
    Q_OBJECT

public:
    BoiteDialogReceptionCaisse(const QUuid& repartitionId, double montantAttendu, QWidget* parent = nullptr);
    ~BoiteDialogReceptionCaisse();

private slots:
    void validerReception();
    void mettreAJourEcart();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<QLineEdit> m_champNumeroRecu;
    std::unique_ptr<QLabel> m_labelMontantAttendu;
    std::unique_ptr<QDoubleSpinBox> m_doubleMontantRecu;
    std::unique_ptr<QLabel> m_labelEcart;
    std::unique_ptr<QLabel> m_labelStatut;
    std::unique_ptr<QPushButton> m_boutonValider;
    std::unique_ptr<QPushButton> m_boutonAnnuler;
    
    std::unique_ptr<GestionnaireCaisse> m_gestionnaire;
    QUuid m_repartitionId;
    double m_montantAttendu;
};

#endif // BOITEDIALOGERECEPTIONCAISSE_H