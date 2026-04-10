#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <QList>
#include <QUuid>
#include <QString>

template<typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;

    // CRUD operations
    virtual bool create(const T& entity) = 0;
    virtual T getById(const QUuid& id) = 0;
    virtual QList<T> getAll() = 0;
    virtual bool update(const T& entity) = 0;
    virtual bool remove(const QUuid& id) = 0;

    // Search operations
    virtual QList<T> search(const QString& criterion) = 0;
    virtual bool exists(const QUuid& id) = 0;

    // Error handling
    virtual QString getLastError() const = 0;
};

#endif // IREPOSITORY_H