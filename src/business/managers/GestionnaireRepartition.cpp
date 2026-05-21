#include "GestionnaireRepartition.h"
#include "../../data/repositories/RepositoryRepartition.h"
#include "../../data/repositories/RepositoryArticleRepartition.h"
#include <QTextDocument>
#include <QPrinter>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

GestionnaireRepartition::GestionnaireRepartition() {}

QUuid GestionnaireRepartition::creerRepartition(const QUuid& equipeId, const QUuid& routeId, const QDate& date, const QUuid& /*utilisateurId*/)
{
    clearErreur();

    // 1. Crée un nouvel UUID qui sera le même dans le C++ et la base
    QUuid repartitionId = QUuid::createUuid();

    // 2. Préparer l'objet Repartition avec toutes les données et l'UUID généré
    Repartition repartition;
    repartition.setRepartitionId(repartitionId);
    repartition.setEquipeId(equipeId);
    repartition.setRouteId(routeId);
    repartition.setDateRepartition(date);
    repartition.setStatut(Repartition::Statut::EnCours); // ou EnAttente selon logique applicative
    repartition.setMontantCashAttendu(0.0);
    repartition.setChefId(QUuid()); // ou renseigner si tu as un chef affecté
    repartition.setCreatedAt(QDateTime::currentDateTime());
    repartition.setUpdatedAt(QDateTime::currentDateTime());
    repartition.setDeletedAt(QDateTime());
    repartition.setVersion(1);
    repartition.setSyncStatus("PENDING");

    RepositoryRepartition repo;
    // 3. ENREGISTRER la répartition en BASE AVANT TOUT
    if (!repo.create(repartition)) {
        m_dernierErreur = repo.getLastError();
        return QUuid(); // !! renvoie un id nul si la sauvegarde a échoué
    }

    // 4. Retourner l'identifiant _créé et inséré_
    return repartitionId;
}

bool GestionnaireRepartition::ajouterArticle(const ArticleRepartition& article)
{
    clearErreur();

    // 1. Vérification du stock via la vue stock_soldes
    QSqlQuery query;
    query.prepare("SELECT quantite_disponible FROM stock_soldes WHERE produit_id = :id");
    query.bindValue(":id", article.getProduitId().toString(QUuid::WithoutBraces));
    if (query.exec() && query.next()) {
        int dispo = query.value(0).toInt();
        int totalDemande = article.getQuantiteTotale();
        if (totalDemande > dispo) {
            m_dernierErreur = QString("Stock insuffisant pour ce produit (Disponible : %1, Demandé : %2)").arg(dispo).arg(totalDemande);
            return false;
        }
    } else {
        m_dernierErreur = "Erreur lors de la lecture du stock ou produit introuvable.";
        return false;
    }

    // 2. Création de l'article avec gestion du offline/audit
    ArticleRepartition art = article; // copie
    auto now = QDateTime::currentDateTime();
    art.setCreatedAt(now);
    art.setUpdatedAt(now);
    art.setDeletedAt(QDateTime());
    art.setVersion(1);
    art.setSyncStatus(ArticleRepartition::SyncStatus::PENDING);    // on suppose ici que le manager connaît l'utilisateur courant... à passer potentiellement en param
    // art.setCreatedBy(utilisateurId);
    // art.setUpdatedBy(utilisateurId);

    RepositoryArticleRepartition repo;
    if (!repo.create(art)) {
        m_dernierErreur = repo.getLastError();
        return false;
    }
    return true;
}

bool GestionnaireRepartition::marquerEnCours(const QUuid& repartitionId)
{
    clearErreur();
    RepositoryRepartition repo;
    Repartition repartition = repo.getById(repartitionId);

    if (repartition.getRepartitionId().isNull()) {
        m_dernierErreur = "Répartition non trouvée";
        return false;
    }
    if (!verifierQuantitesDisponibles(repartitionId, &m_dernierErreur)) {
        return false;
    }
    repartition.setStatut(Repartition::Statut::EnCours);
    repartition.setVersion(repartition.getVersion()+1);
    repartition.setSyncStatus("PENDING");
    repartition.setUpdatedAt(QDateTime::currentDateTime());
    if (!repo.update(repartition)) {
        m_dernierErreur = "Passage à En cours échoué : " + repo.getLastError();
        return false;
    }
    return true;
}

bool GestionnaireRepartition::marquerCompletee(const QUuid& repartitionId)
{
    clearErreur();
    RepositoryRepartition repo;
    Repartition repartition = repo.getById(repartitionId);

    if (repartition.getRepartitionId().isNull()) {
        m_dernierErreur = "Répartition non trouvée";
        return false;
    }
    repartition.setStatut(Repartition::Statut::Completee);
    // repartition.setDateRetour(QDate::currentDate()); // SUPPRIMÉ car n'existe pas
    repartition.setVersion(repartition.getVersion() + 1);
    repartition.setSyncStatus("PENDING");
    repartition.setUpdatedAt(QDateTime::currentDateTime());

    if (!repo.update(repartition)) {
        m_dernierErreur = "Passage à Complétée échoué : " + repo.getLastError();
        return false;
    }
    return true;
}

bool GestionnaireRepartition::annulerRepartition(const QUuid& repartitionId)
{
    clearErreur();
    RepositoryRepartition repo;
    Repartition repartition = repo.getById(repartitionId);

    if (repartition.getRepartitionId().isNull()) {
        m_dernierErreur = "Répartition non trouvée";
        return false;
    }

    // Soft-delete
    repartition.setDeletedAt(QDateTime::currentDateTime());
    repartition.setSyncStatus("PENDING");
    repartition.setVersion(repartition.getVersion() + 1);
    repartition.setUpdatedAt(QDateTime::currentDateTime());

    if (!repo.update(repartition)) {
        m_dernierErreur = "Annulation échouée : " + repo.getLastError();
        return false;
    }

    return true;
}

Repartition GestionnaireRepartition::obtenirRepartition(const QUuid& repartitionId, bool avecArticles)
{
    RepositoryRepartition repo;
    Repartition repartition = repo.getById(repartitionId);
    if (avecArticles && !repartition.getRepartitionId().isNull()) {
        RepositoryArticleRepartition ra;
        repartition.setArticles(ra.getByRepartitionId(repartitionId));
    }
    return repartition;
}

QList<Repartition> GestionnaireRepartition::obtenirRepartitionsEnCours()
{
    RepositoryRepartition repo;
    return repo.getByStatut(Repartition::Statut::EnCours);
}

QList<Repartition> GestionnaireRepartition::obtenirRepartitionsParEquipe(const QUuid& equipeId)
{
    RepositoryRepartition repo;
    return repo.getByEquipe(equipeId);
}

bool GestionnaireRepartition::verifierQuantitesDisponibles(const QUuid& repartitionId, QString* err)
{
    clearErreur();
    RepositoryArticleRepartition repoArticles;
    QList<ArticleRepartition> articles = repoArticles.getByRepartitionId(repartitionId);

    QSqlDatabase db = QSqlDatabase::database();
    for (const ArticleRepartition& a : articles) {
        int total = a.getQuantiteTotale();
        if (total <= 0)
            continue;
        QSqlQuery q(db);
        q.prepare("SELECT disponible, quantite_actuelle, message FROM fn_check_stock_availability(?, ?)");
        q.addBindValue(a.getProduitId().toString());
        q.addBindValue(total);
        if (!q.exec() || !q.next() || !q.value(0).toBool()) {
            if (err) *err = q.value(2).toString();
            return false;
        }
    }
    return true;
}

bool GestionnaireRepartition::imprimerBonCommande(const QUuid& repartitionId, const QString& cheminFichier, QString* erreur)
{
    clearErreur();
    auto r = obtenirRepartition(repartitionId, true);
    if (r.getRepartitionId().isNull()) {
        if (erreur) *erreur = "Répartition introuvable";
        return false;
    }

    QTextDocument doc;
    QString html;
    html += "<h2>Bon de Commande - Répartition</h2>";
    html += QString("<p><b>Date :</b> %1<br>"
                    "<b>Équipe :</b> %2<br>"
                    "<b>Route :</b> %3<br>"
                    "<b>Statut :</b> %4</p>")
                .arg(r.getDateRepartition().toString(Qt::ISODate))
                .arg(r.getEquipeId().toString())
                .arg(r.getRouteId().toString())
                .arg(r.getStatutLabel());

    html += "<table border='1' cellspacing='0' cellpadding='3'><tr>"
            "<th>Produit</th><th>Qté Vente</th><th>Qté Cadeau</th><th>Qté Dégustation</th><th>Total</th><th>Obs.</th>"
            "</tr>";
    for (const ArticleRepartition& a : r.getArticles()) {
        html += QString("<tr>"
                        "<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td>"
                        "</tr>")
                    .arg(a.getProduitId().toString())
                    .arg(a.getQuantiteVente())
                    .arg(a.getQuantiteCadeau())
                    .arg(a.getQuantiteDegustation())
                    .arg(a.getQuantiteTotale())
                    .arg(a.getObservation().toHtmlEscaped());
    }
    html += "</table>";
    doc.setHtml(html);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(cheminFichier);
    doc.print(&printer);

    return true;
}

bool GestionnaireRepartition::mettreAJourMontantAttendu(const QUuid& repartitionId, double montant)
{
    clearErreur();
    RepositoryRepartition repo;
    Repartition r = repo.getById(repartitionId);
    if (r.getRepartitionId().isNull()) {
        m_dernierErreur = "Répartition non trouvée";
        return false;
    }
    r.setMontantCashAttendu(montant);
    r.setVersion(r.getVersion()+1);
    r.setSyncStatus("PENDING");
    r.setUpdatedAt(QDateTime::currentDateTime());
    return repo.update(r);
}
