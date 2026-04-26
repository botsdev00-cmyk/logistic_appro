#include "GestionnaireRaisonsRetour.h"
#include "../../data/database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QVariant>

QList<RaisonRetour> GestionnaireRaisonsRetour::obtenirRaisons() const
{
    QList<RaisonRetour> raisons;
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(db.getDatabase());
    query.prepare("SELECT raison_retour_id, nom FROM raisons_retour ORDER BY nom");
    if (query.exec()) {
        while (query.next()) {
            RaisonRetour r;
            r.raisonId = QUuid(query.value(0).toString());
            r.nom = query.value(1).toString();
            raisons.append(r);
        }
    }
    return raisons;
}