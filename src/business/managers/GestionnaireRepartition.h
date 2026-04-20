#ifndef GESTIONNAIREREPARTITION_H
#define GESTIONNAIREREPARTITION_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDate>
#include "../../core/entities/Repartition.h"
#include "../../core/entities/ArticleRepartition.h"

class GestionnaireRepartition
{
public:
    GestionnaireRepartition();

    // Opérations majeures
    QUuid creerRepartition(const QUuid& equipeId, const QUuid& routeId, const QDate& dateRepartition, const QUuid& utilisateurId);
    bool ajouterArticle(const ArticleRepartition& art);
    bool marquerEnCours(const QUuid& repartitionId);      // = Départ
    bool marquerCompletee(const QUuid& repartitionId);    // = Fin tournée
    bool annulerRepartition(const QUuid& repartitionId);

    // Lecture & performance
    Repartition obtenirRepartition(const QUuid& repartitionId, bool avecArticles = false);
    QList<Repartition> obtenirRepartitionsEnCours();
    QList<Repartition> obtenirRepartitionsParEquipe(const QUuid& equipeId);

    // Business logic
    bool verifierQuantitesDisponibles(const QUuid& repartitionId, QString* erreur = nullptr);

    // PDF/Bons de commande
    bool imprimerBonCommande(const QUuid& repartitionId, const QString& cheminFichier, QString* erreur = nullptr);

    // Gestion erreurs
    QString getDernierErreur() const { return m_dernierErreur; }
    void clearErreur() { m_dernierErreur.clear(); }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREREPARTITION_H