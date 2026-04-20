#ifndef BOITEDIALOGNOUVELLEEQUIPE_H
#define BOITEDIALOGNOUVELLEEQUIPE_H

#include <QDialog>
#include <QUuid>
#include <QList>
#include "../../core/entities/Equipe.h"
#include <QMap>

class QLineEdit;
class QComboBox;
class QListWidget;
class QPushButton;

class BoiteDialogNouvelleEquipe : public QDialog
{
    Q_OBJECT
public:
    explicit BoiteDialogNouvelleEquipe(QWidget* parent = nullptr);

    QUuid getEquipeCreeeId() const { return m_equipeId; }
    QString getEquipeCreeeNom() const { return m_nomEquipe; }

private slots:
    void onValider();

private:
    QLineEdit* m_editNom;
    QComboBox* m_comboChef;
    QListWidget* m_listeMembres;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;

    QUuid m_equipeId;
    QString m_nomEquipe;
    // Pour faire le lien entre affichage et id utilisateurs
    QMap<QString, QUuid> m_utilisateurs;
};

#endif // BOITEDIALOGNOUVELLEEQUIPE_H