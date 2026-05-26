#pragma once
#include "../../core/entities/Grade.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <QSqlQuery>

class RepositoryGrade
{
public:
    RepositoryGrade();

    bool create(Grade& grade);
    bool update(Grade& grade);
    bool remove(const QUuid& gradeId); // soft delete
    Grade getById(const QUuid& gradeId);
    QList<Grade> getAll();

    QString getLastError() const;

private:
    QString m_lastError;
    void mapRow(QSqlQuery& query, Grade& grade);
};
