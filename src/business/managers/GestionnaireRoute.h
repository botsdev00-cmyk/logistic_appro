#ifndef GESTIONNAIREROUTE_H
#define GESTIONNAIREROUTE_H

#include <QString>
#include <QUuid>
#include "../../core/entities/Route.h"

class GestionnaireRoute
{
public:
    // Retourne l'id de la route créée, ou QUuid() si erreur
    QUuid creerRoute(const QString& nom, const QString& description);
    QString getDernierErreur() const { return m_dernierErreur; }
private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREROUTE_H