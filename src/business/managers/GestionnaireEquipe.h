#ifndef GESTIONNAIREEQUIPE_H
#define GESTIONNAIREEQUIPE_H

#include <QString>
#include <QUuid>
#include <QList>
#include <optional>
#include "../../core/entities/Equipe.h"

class GestionnaireEquipe
{
public:
    GestionnaireEquipe();
    ~GestionnaireEquipe();

    // CRUD
    QUuid creerEquipe(const QString& nom, const QString& nomChef, const QUuid& createdBy, bool estActif = true);
    bool modifierEquipe(const Equipe& equipe, const QUuid& updatedBy);
    bool supprimerEquipe(const QUuid& equipeId);

    std::optional<Equipe> obtenirEquipe(const QUuid& equipeId) const;
    QList<Equipe> listerEquipes() const;
    QList<Equipe> rechercherEquipes(const QString& criterion) const;

    // Offline-first
    QList<Equipe> obtenirEquipesPendantes() const;
    QList<Equipe> obtenirEquipesConflit() const;
    QList<Equipe> obtenirEquipesDepuisVersion(int version) const;

    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREEQUIPE_H
