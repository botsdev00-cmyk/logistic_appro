#pragma once
#include <QUuid>
#include <QString>
#include <QDateTime>

class Grade
{
public:
    enum SyncStatus { PENDING, SYNCED, CONFLICT };
    Grade();

    QUuid gradeId() const;
    void setGradeId(const QUuid& id);

    QString nom() const;
    void setNom(const QString& nom);

    QString description() const;
    void setDescription(const QString& desc);

    QString syncStatus() const;
    void setSyncStatus(const QString& status); // ("PENDING", "SYNCED", ...)

    int version() const;
    void setVersion(int version);

    QDateTime createdAt() const;
    void setCreatedAt(const QDateTime& dt);

    QDateTime updatedAt() const;
    void setUpdatedAt(const QDateTime& dt);

    QDateTime deletedAt() const;
    void setDeletedAt(const QDateTime& dt);

private:
    QUuid m_gradeId;
    QString m_nom;
    QString m_description;
    QString m_syncStatus = "PENDING";
    int m_version = 1;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
};
