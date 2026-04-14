#ifndef BOITEDIALOGPRODUIT_H
#define BOITEDIALOGPRODUIT_H

#include <QDialog>
#include <QUuid>
#include <QComboBox>
#include <QPushButton>
#include "../../core/entities/Produit.h"
#include "../../core/entities/CategorieProduit.h"

class QLineEdit;
class QTextEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QPushButton;
class GestionnaireCatalogue;

class BoiteDialogProduit : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogProduit(GestionnaireCatalogue* gestionnaire, QWidget* parent = nullptr);
    ~BoiteDialogProduit();

    void chargerProduit(const Produit& produit);

private slots:
    void onSkuChanged(const QString& texte);
    void onValiderClicked();
    void onAjouterNouveauType();
    void chargerTypes();

private:
    void initializeUI();
    void chargerCategories();

    GestionnaireCatalogue* m_gestionnaire;
    Produit m_produit;
    QList<CategorieProduit> m_categories;

    QLineEdit* m_editNom;
    QLineEdit* m_editSku;
    QComboBox* m_comboCategorie;
    QDoubleSpinBox* m_spinPrix;
    QSpinBox* m_spinStockMin;
    QComboBox* m_comboType;
    QPushButton* m_btnNouveauType;
    QTextEdit* m_editDescription;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;

    bool m_modeModification;
    QUuid m_produitId;
};

#endif // BOITEDIALOGPRODUIT_H