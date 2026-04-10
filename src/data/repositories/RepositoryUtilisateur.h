#ifndef REPOSITORYUTILISATEUR_H
#define REPOSITORYUTILISATEUR_H

#include "IRepository.h"
#include "../entities/Utilisateur.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryUtilisateur : public IRepository<Utilisateur>
{
public:
    RepositoryUtilisateur();

    bool create(const Utilisateur& entity) override;
    Utilisateur getById(const QUuid& id) override;
    QList<Utilisateur> getAll() override;
    bool update(const Utilisateur& entity) override;
    bool remove(const QUuid& id) override;

    QList<Utilisateur> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    Utilisateur getByUsername(const QString& username);
    Utilisateur getByEmail(const QString& email);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYUTILISATEUR_H