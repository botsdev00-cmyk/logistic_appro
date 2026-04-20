#ifndef BOITEDIALOGNOUVELLEROUTE_H
#define BOITEDIALOGNOUVELLEROUTE_H

#include <QDialog>
#include <QUuid>

class QLineEdit;
class QPushButton;

class BoiteDialogNouvelleRoute : public QDialog
{
    Q_OBJECT
public:
    explicit BoiteDialogNouvelleRoute(QWidget* parent = nullptr);

    QUuid getRouteCreeeId() const { return m_routeId; }
    QString getRouteCreeeNom() const { return m_nomRoute; }

private slots:
    void onValider();

private:
    QLineEdit* m_editNom;
    QLineEdit* m_editDescription;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;

    QUuid m_routeId;
    QString m_nomRoute;
};

#endif // BOITEDIALOGNOUVELLEROUTE_H