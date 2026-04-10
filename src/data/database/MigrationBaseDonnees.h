#ifndef MIGRATIONBASEDONNEES_H
#define MIGRATIONBASEDONNEES_H

#include <QString>
#include <QStringList>

class MigrationBaseDonnees
{
public:
    static bool executerMigrationsInitiales();
    static bool executerMigration(const QString& nomMigration);
    static bool verifierSchemaExiste();

private:
    static QString obtenirContenuMigration(const QString& nomMigration);
};

#endif // MIGRATIONBASEDONNEES_H