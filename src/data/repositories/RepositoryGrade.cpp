#include "RepositoryGrade.h"
#include <QSqlError>
#include <QDateTime>

RepositoryGrade::RepositoryGrade() {}

bool RepositoryGrade::create(Grade& grade)
{
    QSqlQuery query;
    if (grade.gradeId().isNull()) grade.setGradeId(QUuid::createUuid());
    grade.setSyncStatus("PENDING");
    grade.setVersion(1);
    grade.setCreatedAt(QDateTime::currentDateTime());
    grade.setUpdatedAt(grade.createdAt());
    grade.setDeletedAt(QDateTime());

    query.prepare(R"(
        INSERT INTO grade
        (grade_id, nom, description, sync_status, version, created_at, updated_at, deleted_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(grade.gradeId());
    query.addBindValue(grade.nom());
    query.addBindValue(grade.description());
    query.addBindValue(grade.syncStatus());
    query.addBindValue(grade.version());
    query.addBindValue(grade.createdAt());
    query.addBindValue(grade.updatedAt());
    query.addBindValue(grade.deletedAt());

    bool ok = query.exec();
    if (!ok)
        m_lastError = query.lastError().text();
    return ok;
}

bool RepositoryGrade::update(Grade& grade)
{
    QSqlQuery query;
    grade.setSyncStatus("PENDING");
    grade.setVersion(grade.version() + 1);
    grade.setUpdatedAt(QDateTime::currentDateTime());

    query.prepare(R"(
        UPDATE grade SET
            nom=?,
            description=?,
            sync_status=?,
            version=?,
            updated_at=?
        WHERE grade_id=?
    )");
    query.addBindValue(grade.nom());
    query.addBindValue(grade.description());
    query.addBindValue(grade.syncStatus());
    query.addBindValue(grade.version());
    query.addBindValue(grade.updatedAt());
    query.addBindValue(grade.gradeId());

    bool ok = query.exec();
    if (!ok)
        m_lastError = query.lastError().text();
    return ok;
}

bool RepositoryGrade::remove(const QUuid& gradeId)
{
    QSqlQuery query;
    QDateTime now = QDateTime::currentDateTime();
    query.prepare(R"(
        UPDATE grade SET
            deleted_at=?,
            sync_status=?,
            version=version+1,
            updated_at=?
        WHERE grade_id=?
    )");
    query.addBindValue(now);
    query.addBindValue("PENDING");
    query.addBindValue(now);
    query.addBindValue(gradeId);

    bool ok = query.exec();
    if (!ok)
        m_lastError = query.lastError().text();
    return ok;
}

Grade RepositoryGrade::getById(const QUuid& gradeId)
{
    Grade grade;
    QSqlQuery query;
    query.prepare("SELECT * FROM grade WHERE grade_id=? AND deleted_at IS NULL");
    query.addBindValue(gradeId);
    if (query.exec() && query.next()) {
        mapRow(query, grade);
    }
    return grade;
}

QList<Grade> RepositoryGrade::getAll()
{
    QList<Grade> result;
    QSqlQuery query("SELECT * FROM grade WHERE deleted_at IS NULL ORDER BY nom");
    while (query.next()) {
        Grade grade;
        mapRow(query, grade);
        result.append(grade);
    }
    return result;
}

void RepositoryGrade::mapRow(QSqlQuery& query, Grade& grade)
{
    grade.setGradeId(query.value("grade_id").toUuid());
    grade.setNom(query.value("nom").toString());
    grade.setDescription(query.value("description").toString());
    grade.setSyncStatus(query.value("sync_status").toString());
    grade.setVersion(query.value("version").toInt());
    grade.setCreatedAt(query.value("created_at").toDateTime());
    grade.setUpdatedAt(query.value("updated_at").toDateTime());
    grade.setDeletedAt(query.value("deleted_at").toDateTime());
}

QString RepositoryGrade::getLastError() const { return m_lastError; }
