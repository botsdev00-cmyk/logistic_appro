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

QUuid GestionnaireRepartition::creerRepartition(const QUuid& equipeId, const QUuid& routeId, const QDate& date, const QUuid& utilisateurId)
{
    clearErreur();
    Repartition repartition;
    repartition.setRepartitionId(QUuid::createUuid());
    repartition.setEquipeId(equipeId);
    repartition.setRouteId(routeId);
    repartition.setDateRepartition(date);
    repartition.setStatut(Repartition::Statut::EnCours); 
    repartition.setCreePar(utilisateurId);
    repartition.setMontantCashAttendu(0.0); // Initialisé à 0, sera mis à jour plus loin

    RepositoryRepartition repo;
    if (!repo.create(repartition)) {
        m_dernierErreur = repo.getLastError();
        return QUuid();
    }
    return repartition.getRepartitionId();
}

bool GestionnaireRepartition::ajouterArticle(const ArticleRepartition& article)
{
    clearErreur();

    // 1. Vérification du stock via la vue v_statut_stock
    QSqlQuery query;
    query.prepare("SELECT quantite_disponible FROM stock_soldes WHERE produit_id = :id");
    query.bindValue(":id", article.getProduitId().toString(QUuid::WithoutBraces));
    
    if (query.exec() && query.next()) {
        int dispo = query.value(0).toInt();
        int totalDemande = article.getQuantiteTotale();
        
        if (totalDemande > dispo) {
            m_dernierErreur = QString("Stock insuffisant pour ce produit (Disponible : %1, Demandé : %2)")
                                .arg(dispo).arg(totalDemande);
            return false;
        }
    } else {
        m_dernierErreur = "Erreur lors de la lecture du stock ou produit introuvable.";
        return false;
    }

    // 2. Si le stock est OK, on procède à l'ajout
    RepositoryArticleRepartition repo;
    if (!repo.create(article)) {
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
    repartition.setDateRetour(QDate::currentDate());
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
    if (!repo.remove(repartitionId)) {
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

// Nouvelle méthode pour le calcul et la mise à jour du montant attendu
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
    return repo.update(r);
}