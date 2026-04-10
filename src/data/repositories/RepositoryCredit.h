#ifndef REPOSITORYCREDIT_H
#define REPOSITORYCREDIT_H

#include "IRepository.h"
#include "../entities/Credit.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryCredit : public IRepository<Credit>
{
public:
    RepositoryCredit();

    bool create(const Credit& entity) override;
    Credit getById(const QUuid& id) override;
    QList<Credit> getAll() override;
    bool update(const Credit& entity) override;
    bool remove(const QUuid& id) override;

    QList<Credit> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<Credit> getByClient(const QUuid& clientId);
    QList<Credit> getOverdueCredits();
    QList<Credit> getByStatut(const Credit::Statut& statut);
    double getTotalAmount(const Credit::Statut& statut);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYCREDIT_H