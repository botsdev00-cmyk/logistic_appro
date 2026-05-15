#include "TableauRetoursEnAttente.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../data/repositories/RepositoryRepartition.h"
#include "../../core/entities/Produit.h"
#include <QHeaderView>
#include <QDebug>
#include <QVariant>
#include <QMetaType>

Q_DECLARE_METATYPE(RetourStock)

TableauRetoursEnAttente::TableauRetoursEnAttente(QWidget* parent)
    : QTableWidget(parent)
{
    setColumnCount(4);
    setHorizontalHeaderLabels({"Article", "Quantité", "Date création", "Équipe"});
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(this, &QTableWidget::cellDoubleClicked, [this](int row, int){
        if (row < 0 || row >= rowCount()) return;
        RetourStock retour = item(row, 0)->data(Qt::UserRole).value<RetourStock>();
        emit ligneRetourClicked(retour);
    });
}

void TableauRetoursEnAttente::setRetoursEnAttente(const QList<RetourStock>& retours)
{
    clearContents();
    setRowCount(retours.size());

    RepositoryProduit repoProduit;
    RepositoryRepartition repoRep;
    RepositoryEquipe repoEquipe;

    for (int i = 0; i < retours.size(); ++i) {
        const auto& r = retours[i];

        // Article
        auto produitOpt = repoProduit.getById(r.getProduitId());
        QString nomProduit = produitOpt ? produitOpt->getNom() : "(Inconnu)";
        QTableWidgetItem* itemArt = new QTableWidgetItem(nomProduit);

        // Quantité
        QTableWidgetItem* itemQte = new QTableWidgetItem(QString::number(r.getQuantite()));

        // Date
        QTableWidgetItem* itemDate = new QTableWidgetItem(r.getDate().toString("dd/MM/yyyy HH:mm"));

        // Équipe
        QString nomEquipe = "(Aucune)";
        QUuid repId = r.getRepartitionId();
        if (!repId.isNull()) {
            auto rep = repoRep.getById(repId); // rep est un objet
            if (!rep.getEquipeId().isNull()) {
                auto optEq = repoEquipe.getById(rep.getEquipeId());
                if (optEq)
                    nomEquipe = optEq->getNom();
            }
        }
        QTableWidgetItem* itemEquipe = new QTableWidgetItem(nomEquipe);

        setItem(i, 0, itemArt);
        setItem(i, 1, itemQte);
        setItem(i, 2, itemDate);
        setItem(i, 3, itemEquipe);

        itemArt->setData(Qt::UserRole, QVariant::fromValue(r));
    }
}
