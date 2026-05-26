#include "Employee.h"

Employee::Employee() : m_employeId(QUuid::createUuid()) {}

QUuid Employee::employeId() const { return m_employeId; }
void Employee::setEmployeId(const QUuid& id) { m_employeId = id; }

QString Employee::nom() const { return m_nom; }
void Employee::setNom(const QString& nom) { m_nom = nom; }

QString Employee::prenom() const { return m_prenom; }
void Employee::setPrenom(const QString& prenom) { m_prenom = prenom; }

QUuid Employee::gradeId() const { return m_gradeId; }
void Employee::setGradeId(const QUuid& id) { m_gradeId = id; }

QDate Employee::dateNaissance() const { return m_dateNaissance; }
void Employee::setDateNaissance(const QDate& date) { m_dateNaissance = date; }

QString Employee::email() const { return m_email; }
void Employee::setEmail(const QString& email) { m_email = email; }

QString Employee::telephone() const { return m_telephone; }
void Employee::setTelephone(const QString& tel) { m_telephone = tel; }

QDate Employee::dateEmbauche() const { return m_dateEmbauche; }
void Employee::setDateEmbauche(const QDate& date) { m_dateEmbauche = date; }

QString Employee::syncStatus() const { return m_syncStatus; }
void Employee::setSyncStatus(const QString& status) { m_syncStatus = status; }

int Employee::version() const { return m_version; }
void Employee::setVersion(int version) { m_version = version; }

QDateTime Employee::createdAt() const { return m_createdAt; }
void Employee::setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

QDateTime Employee::updatedAt() const { return m_updatedAt; }
void Employee::setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

QDateTime Employee::deletedAt() const { return m_deletedAt; }
void Employee::setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
