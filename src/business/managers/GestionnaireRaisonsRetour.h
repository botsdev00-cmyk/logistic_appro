#ifndef GESTIONNAIRE_RAISON_RETOUR_H
#define GESTIONNAIRE_RAISON_RETOUR_H

#include <QList>
#include <QUuid>
#include <QString>

struct RaisonRetour
{
    QUuid raisonId;
    QString nom;
};

class GestionnaireRaisonsRetour
{
public:
    QList<RaisonRetour> obtenirRaisons() const;
};

#endif // GESTIONNAIRE_RAISON_RETOUR_H