#include "RepositoryEmployee.h"
#include <QSqlError>
#include <QDateTime>

RepositoryEmployee::RepositoryEmployee() {}

bool RepositoryEmployee::create(Employee& emp)
{
    QSqlQuery query;
    if (emp.employeId().isNull()) emp.setEmployeId(QUuid::createUuid());
    emp.setSyncStatus("PENDING");
    emp.setVersion(1);
    emp.setCreatedAt(QDateTime::currentDateTime());
    emp.setUpdatedAt(emp.createdAt());
    emp.setDeletedAt(QDateTime());

    query.prepare(R"(
        INSERT INTO employee
        (employe_id, nom, prenom, grade_id, date_naissance, email, telephone, date_embauche, sync_status, version, created_at, updated_at, deleted_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(emp.employeId());
    query.addBindValue(emp.nom());
    query.addBindValue(emp.prenom());
    query.addBindValue(emp.gradeId());
    query.addBindValue(emp.dateNaissance());
    query.addBindValue(emp.email());
    query.addBindValue(emp.telephone());
    query.addBindValue(emp.dateEmbauche());
    query.addBindValue(emp.syncStatus());
    query.addBindValue(emp.version());
    query.addBindValue(emp.createdAt());
    query.addBindValue(emp.updatedAt());
    query.addBindValue(emp.deletedAt());

    bool ok = query.exec();
    if (!ok)
        m_lastError = query.lastError().text();
    return ok;
}

bool RepositoryEmployee::update(Employee& emp)
{
    QSqlQuery query;
    emp.setSyncStatus("PENDING");
    emp.setVersion(emp.version() + 1);
    emp.setUpdatedAt(QDateTime::currentDateTime());

    query.prepare(R"(
        UPDATE employee SET
            nom=?,
            prenom=?,
            grade_id=?,
            date_naissance=?,
            email=?,
            telephone=?,
            date_embauche=?,
            sync_status=?,
            version=?,
            updated_at=?
        WHERE employe_id=?
    )");
    query.addBindValue(emp.nom());
    query.addBindValue(emp.prenom());
    query.addBindValue(emp.gradeId());
    query.addBindValue(emp.dateNaissance());
    query.addBindValue(emp.email());
    query.addBindValue(emp.telephone());
    query.addBindValue(emp.dateEmbauche());
    query.addBindValue(emp.syncStatus());
    query.addBindValue(emp.version());
    query.addBindValue(emp.updatedAt());
    query.addBindValue(emp.employeId());

    bool ok = query.exec();
    if (!ok)
        m_lastError = query.lastError().text();
    return ok;
}

bool RepositoryEmployee::remove(const QUuid& employeId)
{
    QSqlQuery query;
    QDateTime now = QDateTime::currentDateTime();
    query.prepare(R"(
        UPDATE employee SET
            deleted_at=?,
            sync_status=?,
            version=version+1,
            updated_at=?
        WHERE employe_id=?
    )");
    query.addBindValue(now);
    query.addBindValue("PENDING");
    query.addBindValue(now);
    query.addBindValue(employeId);

    bool ok = query.exec();
    if (!ok)
        m_lastError = query.lastError().text();
    return ok;
}

Employee RepositoryEmployee::getById(const QUuid& employeId)
{
    Employee emp;
    QSqlQuery query;
    query.prepare("SELECT * FROM employee WHERE employe_id=? AND deleted_at IS NULL");
    query.addBindValue(employeId);
    if (query.exec() && query.next()) {
        mapRow(query, emp);
    }
    return emp;
}

QList<Employee> RepositoryEmployee::getAll()
{
    QList<Employee> result;
    QSqlQuery query("SELECT * FROM employee WHERE deleted_at IS NULL ORDER BY nom, prenom");
    while (query.next()) {
        Employee emp;
        mapRow(query, emp);
        result.append(emp);
    }
    return result;
}

void RepositoryEmployee::mapRow(QSqlQuery& query, Employee& emp)
{
    emp.setEmployeId(query.value("employe_id").toUuid());
    emp.setNom(query.value("nom").toString());
    emp.setPrenom(query.value("prenom").toString());
    emp.setGradeId(query.value("grade_id").toUuid());
    emp.setDateNaissance(query.value("date_naissance").toDate());
    emp.setEmail(query.value("email").toString());
    emp.setTelephone(query.value("telephone").toString());
    emp.setDateEmbauche(query.value("date_embauche").toDate());
    emp.setSyncStatus(query.value("sync_status").toString());
    emp.setVersion(query.value("version").toInt());
    emp.setCreatedAt(query.value("created_at").toDateTime());
    emp.setUpdatedAt(query.value("updated_at").toDateTime());
    emp.setDeletedAt(query.value("deleted_at").toDateTime());
}

QString RepositoryEmployee::getLastError() const { return m_lastError; }
