#include "GestionnaireRaisonsRetour.h"
#include "../../data/database/ConnexionBaseDonnees.h"
#include <QSqlQuery>

GestionnaireRaisonsRetour::GestionnaireRaisonsRetour()
    : m_cacheInitialise(false)
{
    // Ne charge rien au départ (lazy), pour un vrai offline-first
}

void GestionnaireRaisonsRetour::initialiserCache()
{
    if (m_cacheInitialise)
        return;
    synchroniserDepuisServeur();
    m_cacheInitialise = true;
}

// Offline d’abord : le cache mémoire (`m_cacheRaisons`) est prioritaire
QList<RaisonRetour> GestionnaireRaisonsRetour::obtenirRaisons(bool inclureSupprimees, bool forcerSync)
{
    if (forcerSync || !m_cacheInitialise)
        synchroniserDepuisServeur();

    QList<RaisonRetour> res;
    for (const RaisonRetour& r : std::as_const(m_cacheRaisons))
    {
        if (inclureSupprimees || !r.deletedAt.isValid())
            res.append(r);
    }
    return res;
}

void GestionnaireRaisonsRetour::ajouterOuMajRaison(const RaisonRetour& raison)
{
    m_cacheRaisons[raison.raisonId] = raison;
    // Pour un vrai offline-first, l’ajout à la BDD peut être différé tant qu’on n’est pas connecté…
}

void GestionnaireRaisonsRetour::supprimerRaison(const QUuid& raisonId)
{
    if (m_cacheRaisons.contains(raisonId)) {
        m_cacheRaisons[raisonId].deletedAt = QDateTime::currentDateTime();
        // Ici aussi, la suppression sera propagée à la BDD distante au prochain sync
    }
}

void GestionnaireRaisonsRetour::synchroniserDepuisServeur()
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(db.getDatabase());

    m_cacheRaisons.clear();
    query.prepare(R"(
        SELECT raison_retour_id, code, nom, sync_status, version, deleted_at, created_at, updated_at
          FROM raisons_retour
    )");
    if (query.exec()) {
        while (query.next()) {
            RaisonRetour r;
            r.raisonId   = QUuid(query.value(0).toString());
            r.code       = query.value(1).toString();
            r.nom        = query.value(2).toString();
            r.syncStatus = query.value(3).toString();
            r.version    = query.value(4).toInt();
            r.deletedAt  = query.value(5).toDateTime();
            r.createdAt  = query.value(6).toDateTime();
            r.updatedAt  = query.value(7).toDateTime();
            m_cacheRaisons[r.raisonId] = r;
        }
    }
    m_cacheInitialise = true;
}

RaisonRetour GestionnaireRaisonsRetour::trouverParId(const QUuid& raisonId) const
{
    return m_cacheRaisons.value(raisonId, RaisonRetour());
}
