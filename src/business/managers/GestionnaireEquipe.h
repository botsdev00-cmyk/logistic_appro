#ifndef GESTIONNAIREEQUIPE_H
#define GESTIONNAIREEQUIPE_H

#include <QString>
#include <QUuid>
#include <QList>
#include "../../core/entities/Equipe.h"

class GestionnaireEquipe
{
public:
    // Retourne l'id de l'équipe créée, ou QUuid() en cas d'échec
    QUuid creerEquipe(const QString& nom, 
                      const QUuid& chefId, 
                      const QList<QUuid>& membres,
                      const QString& telephoneChef = "");
    QString getDernierErreur() const { return m_dernierErreur; }
private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREEQUIPE_H