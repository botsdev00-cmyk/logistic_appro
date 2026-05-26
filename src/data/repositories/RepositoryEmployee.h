#pragma once
#include "../../core/entities/Employee.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <QSqlQuery>

class RepositoryEmployee
{
public:
    RepositoryEmployee();

    bool create(Employee& emp);
    bool update(Employee& emp);
    bool remove(const QUuid& employeId); // soft delete
    Employee getById(const QUuid& employeId);
    QList<Employee> getAll();

    QString getLastError() const;

private:
    QString m_lastError;
    void mapRow(QSqlQuery& query, Employee& emp);
};
