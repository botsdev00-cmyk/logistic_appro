#include "EntreeStock.h"

EntreeStock::EntreeStock()
    : m_entreeStockId(QUuid::createUuid()),
      m_quantite(0),
      m_source(Source::Production),
      m_date(QDateTime::currentDateTime())
{
}

EntreeStock::~EntreeStock()
{
}

QString EntreeStock::sourceToString(Source source)
{
    switch (source) {
        case Source::Production:
            return "production";
        case Source::Retour:
            return "retour";
        case Source::Ajustement:
            return "ajustement";
        default:
            return "production";
    }
}

EntreeStock::Source EntreeStock::stringToSource(const QString& str)
{
    QString lower = str.toLower();
    if (lower == "retour") return Source::Retour;
    if (lower == "ajustement") return Source::Ajustement;
    return Source::Production;
}

QString EntreeStock::getSourceLabel() const
{
    switch (m_source) {
        case Source::Production:
            return "Production";
        case Source::Retour:
            return "Retour";
        case Source::Ajustement:
            return "Ajustement";
        default:
            return "Inconnu";
    }
}