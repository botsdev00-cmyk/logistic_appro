#ifndef GESTIONNAIRE_RAISON_RETOUR_H
#define GESTIONNAIRE_RAISON_RETOUR_H

#include <QList>
#include <QMap>
#include <QUuid>
#include <QString>
#include <QDateTime>

struct RaisonRetour
{
    QUuid raisonId;
    QString code;
    QString nom;
    QString syncStatus;
    int version;
    QDateTime deletedAt;
    QDateTime createdAt;
    QDateTime updatedAt;

    RaisonRetour() : version(1) {}
};

class GestionnaireRaisonsRetour
{
public:
    GestionnaireRaisonsRetour();

    // Récupère la liste de raisons retour (offline d’abord)
    QList<RaisonRetour> obtenirRaisons(bool inclureSupprimees = false, bool forcerSync = false);

    // Manipule le cache local
    void ajouterOuMajRaison(const RaisonRetour& raison);
    void supprimerRaison(const QUuid& raisonId);

    // Synchro avec la BDD distante (update le cache mémoire)
    void synchroniserDepuisServeur();

    // Accès direct par id
    RaisonRetour trouverParId(const QUuid& raisonId) const;

private:
    void initialiserCache(); // charge une seule fois depuis la base, si offline il reste en cache
    QMap<QUuid, RaisonRetour> m_cacheRaisons;
    bool m_cacheInitialise;
};

#endif // GESTIONNAIRE_RAISON_RETOUR_H
