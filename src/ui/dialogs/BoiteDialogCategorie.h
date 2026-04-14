#ifndef BOITEDIALOGCATEGORIE_H
#define BOITEDIALOGCATEGORIE_H

#include <QDialog>
#include <QUuid>

class QLineEdit;
class QTextEdit;
class QSpinBox;
class QPushButton;
class GestionnaireCatalogue;
class CategorieProduit;

class BoiteDialogCategorie : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogCategorie(GestionnaireCatalogue* gestionnaire, QWidget* parent = nullptr);
    ~BoiteDialogCategorie();

    void chargerCategorie(const CategorieProduit& categorie);

private slots:
    void onCodeChanged(const QString& texte);
    void onValider();

private:
    void initializeUI();

    GestionnaireCatalogue* m_gestionnaireCatalogue;
    QLineEdit*   m_champNom;
    QLineEdit*   m_champCode;
    QTextEdit*   m_champDescription;
    QSpinBox*    m_spinOrdre;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;

    bool  m_modeModification;
    QUuid m_categorieId;
};

#endif // BOITEDIALOGCATEGORIE_H