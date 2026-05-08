#ifndef REPOSITORYCREDIT_H
#define REPOSITORYCREDIT_H

#include "../../core/entities/Credit.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>

class RepositoryCredit
{
public:
    RepositoryCredit();

    // CRUD & gestion de cycle de vie
    bool create(const Credit& entity);
    bool update(const Credit& entity);
    bool logicalDelete(const QUuid& id);
    std::optional<Credit> getById(const QUuid& id) const;
    QList<Credit> getAll() const;

    // Recherche/filtrage métier
    QList<Credit> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;
    QList<Credit> getByClient(const QUuid& clientId) const;
    QList<Credit> getOverdueCredits() const;
    QList<Credit> getByStatut(const Credit::Statut& statut) const;
    double getTotalAmount(const Credit::Statut& statut) const;

    // OFFLINE-FIRST / API/MOBILE
    QList<Credit> getPendingSync() const;
    QList<Credit> getSinceVersion(int minVersion) const;

    QString getLastError() const { return m_dernierErreur; }
private:
    QString m_dernierErreur;
};

#endif // REPOSITORYCREDIT_H