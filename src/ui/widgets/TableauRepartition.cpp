#include "TableauRepartition.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "../../core/entities/Repartition.h"
#include "../../core/entities/ArticleRepartition.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryRoute.h"

TableauRepartition::TableauRepartition(QWidget* parent)
    : QTableWidget(parent),
      m_gestionnaire(std::make_unique<GestionnaireRepartition>())
{
    initialiserColonnes();
    creerContextMenu();
    setStyleSheet(
        "QTableWidget { background-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }"
    );
}

TableauRepartition::~TableauRepartition() {}

void TableauRepartition::initialiserColonnes()
{
    setColumnCount(6);
    setHorizontalHeaderLabels({"Équipe", "Route", "Date", "Montant attendu", "Statut", "Actions"});

    // REDIMENSIONNEMENT égal de chaque colonne (stretch = même taille)
    QHeaderView* header = horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
}
// Helper pour obtenir le nom d'une équipe/route via leur UUID
QString getNomEquipe(const QUuid& id) {
    RepositoryEquipe repo;
    auto eq = repo.getById(id);
    return eq.getNom();
}
QString getNomRoute(const QUuid& id) {
    RepositoryRoute repo;
    auto r = repo.getById(id);
    return r.getNom();
}

void TableauRepartition::chargerDonnees()
{
    rafraichir();
}

void TableauRepartition::rafraichir()
{
    clearContents();
    setRowCount(0);
    remplirTableau();
}

void TableauRepartition::remplirTableau()
{
    QList<Repartition> repartitions = m_gestionnaire->obtenirRepartitionsEnCours();
    setRowCount(repartitions.size());
    int row = 0;
    for (const auto& rep : repartitions) {
        setItem(row, 0, new QTableWidgetItem(getNomEquipe(rep.getEquipeId())));
        setItem(row, 1, new QTableWidgetItem(getNomRoute(rep.getRouteId())));
        setItem(row, 2, new QTableWidgetItem(rep.getDateRepartition().toString(Qt::ISODate)));
        setItem(row, 3, new QTableWidgetItem(QString::number(rep.getMontantCashAttendu(), 'f', 2)));
        setItem(row, 4, new QTableWidgetItem(rep.getStatutLabel()));

        QTableWidgetItem* itemId = new QTableWidgetItem(rep.getRepartitionId().toString());
        itemId->setData(Qt::UserRole, rep.getRepartitionId().toString());
        setItem(row, 5, itemId);

        ++row;
    }
    mettreEnEvidence();
}

void TableauRepartition::filtrerParStatut(const QString& statut)
{
    for (int row = 0; row < rowCount(); ++row) {
        QString itemStatut = item(row, 4)->text();
        bool visible = statut == "Tous" || itemStatut == statut;
        setRowHidden(row, !visible);
    }
}

void TableauRepartition::mettreEnEvidence()
{
    for (int row = 0; row < rowCount(); ++row) {
        QString statut = item(row, 4)->text();
        QColor bg = Qt::white;
        if (statut == "En cours") bg = QColor("#d1ecf1");
        else if (statut == "Complétée") bg = QColor("#d4edda");
        for (int col = 0; col < columnCount(); ++col)
            if (item(row, col)) item(row, col)->setBackground(bg);
    }
}

void TableauRepartition::creerContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction("Afficher détails", this, &TableauRepartition::afficherDetailsRepartition);
        menu.addAction("Afficher articles", this, &TableauRepartition::afficherArticles);
        menu.addAction("Changer statut", this, &TableauRepartition::changerStatut);
        menu.addAction("Imprimer bon PDF", this, &TableauRepartition::imprimerBon);
        menu.exec(mapToGlobal(pos));
    });
}

QUuid TableauRepartition::repartitionIdFromRow(int row) const
{
    if (row < 0) return QUuid();
    auto item = this->item(row, 5);
    if (!item) return QUuid();
    return QUuid(item->data(Qt::UserRole).toString());
}

void TableauRepartition::afficherDetailsRepartition()
{
    int row = currentRow();
    if (row < 0) return;
    QUuid id = repartitionIdFromRow(row);
    if (id.isNull()) return;

    Repartition repartition = m_gestionnaire->obtenirRepartition(id, true);
    if (repartition.getRepartitionId().isNull()) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger la répartition.");
        return;
    }

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Détails de la répartition");
    QVBoxLayout* layout = new QVBoxLayout(dlg);

    layout->addWidget(new QLabel(QString("<b>Equipe :</b> %1 &nbsp; <b>Route :</b> %2")
                                .arg(getNomEquipe(repartition.getEquipeId()))
                                .arg(getNomRoute(repartition.getRouteId()))));
    layout->addWidget(new QLabel(QString("<b>Date :</b> %1 &nbsp; <b>Statut :</b> %2")
                                .arg(repartition.getDateRepartition().toString(Qt::ISODate))
                                .arg(repartition.getStatutLabel())));

    QTableWidget* tbl = new QTableWidget(dlg);
    const auto& arts = repartition.getArticles();
    tbl->setColumnCount(6);
    tbl->setRowCount(arts.size());
    tbl->setHorizontalHeaderLabels(QStringList() << "Produit" << "Vente" << "Cadeau" << "Dégustation" << "Total" << "Observation");

    for (int i = 0; i < arts.size(); ++i) {
        const ArticleRepartition& art = arts.at(i);
        tbl->setItem(i, 0, new QTableWidgetItem(art.getProduitId().toString()));
        tbl->setItem(i, 1, new QTableWidgetItem(QString::number(art.getQuantiteVente())));
        tbl->setItem(i, 2, new QTableWidgetItem(QString::number(art.getQuantiteCadeau())));
        tbl->setItem(i, 3, new QTableWidgetItem(QString::number(art.getQuantiteDegustation())));
        tbl->setItem(i, 4, new QTableWidgetItem(QString::number(art.getQuantiteTotale())));
        tbl->setItem(i, 5, new QTableWidgetItem(art.getObservation()));
    }

    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(tbl);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton* btnFermer = new QPushButton("Fermer", dlg);
    connect(btnFermer, &QPushButton::clicked, dlg, &QDialog::accept);
    btnLayout->addWidget(btnFermer);
    layout->addLayout(btnLayout);

    dlg->setLayout(layout);
    dlg->resize(600, 350);
    dlg->exec();
    delete dlg;
}

void TableauRepartition::changerStatut()
{
    // À développer selon workflow
}

void TableauRepartition::afficherArticles()
{
    afficherDetailsRepartition();
}

void TableauRepartition::imprimerBon()
{
    int row = currentRow();
    if (row < 0) return;
    QUuid id = repartitionIdFromRow(row);
    if (id.isNull()) return;
    QString fichier = QFileDialog::getSaveFileName(this, "Exporter PDF", "bon_commande.pdf", "PDF (*.pdf)");
    if (fichier.isEmpty()) return;
    QString err;
    if (!m_gestionnaire->imprimerBonCommande(id, fichier, &err))
        QMessageBox::warning(this, "Erreur impression", err);
    else
        QMessageBox::information(this, "PDF Généré", "Le bon de commande a été exporté dans :\n" + fichier);
}