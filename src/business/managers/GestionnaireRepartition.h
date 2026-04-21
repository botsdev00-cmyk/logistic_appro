#pragma once

#include <QObject>
#include <QUuid>
#include <QDate>
#include <QString>
#include <QList>
#include <memory>
#include "../../core/entities/Repartition.h"
#include "../../core/entities/ArticleRepartition.h"

class GestionnaireRepartition
{
public:
    GestionnaireRepartition();

    QUuid creerRepartition(const QUuid& equipeId, const QUuid& routeId, const QDate& date, const QUuid& utilisateurId);

    bool ajouterArticle(const ArticleRepartition& article);
    bool marquerEnCours(const QUuid& repartitionId);
    bool marquerCompletee(const QUuid& repartitionId);
    bool annulerRepartition(const QUuid& repartitionId);
    Repartition obtenirRepartition(const QUuid& repartitionId, bool avecArticles = false);

    QList<Repartition> obtenirRepartitionsEnCours();
    QList<Repartition> obtenirRepartitionsParEquipe(const QUuid& equipeId);

    bool verifierQuantitesDisponibles(const QUuid& repartitionId, QString* err = nullptr);

    bool imprimerBonCommande(const QUuid& repartitionId, const QString& cheminFichier, QString* erreur = nullptr);

    QString getDernierErreur() const { return m_dernierErreur; }

    // Ajouté pour la mise à jour du montant attendu
    bool mettreAJourMontantAttendu(const QUuid& repartitionId, double montant);

private:
    void clearErreur() { m_dernierErreur.clear(); }

    QString m_dernierErreur;
};