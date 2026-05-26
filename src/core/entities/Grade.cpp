#include "Grade.h"

Grade::Grade() : m_gradeId(QUuid::createUuid()) {}

QUuid Grade::gradeId() const { return m_gradeId; }
void Grade::setGradeId(const QUuid& id) { m_gradeId = id; }

QString Grade::nom() const { return m_nom; }
void Grade::setNom(const QString& nom) { m_nom = nom; }

QString Grade::description() const { return m_description; }
void Grade::setDescription(const QString& desc) { m_description = desc; }

QString Grade::syncStatus() const { return m_syncStatus; }
void Grade::setSyncStatus(const QString& status) { m_syncStatus = status; }

int Grade::version() const { return m_version; }
void Grade::setVersion(int version) { m_version = version; }

QDateTime Grade::createdAt() const { return m_createdAt; }
void Grade::setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

QDateTime Grade::updatedAt() const { return m_updatedAt; }
void Grade::setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

QDateTime Grade::deletedAt() const { return m_deletedAt; }
void Grade::setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
