#include "AideDateTime.h"
#include <QDebug>

QString AideDateTime::formatDate(const QDate& date)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative de formater une date invalide";
        return "";
    }
    return date.toString("dd/MM/yyyy");
}

QString AideDateTime::formatDateISO(const QDate& date)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative de formater une date invalide";
        return "";
    }
    return date.toString("yyyy-MM-dd");
}

QString AideDateTime::formatTime(const QTime& time)
{
    if (!time.isValid()) {
        qWarning() << "[WARNING] Tentative de formater une heure invalide";
        return "";
    }
    return time.toString("hh:mm:ss");
}

QString AideDateTime::formatTimeShort(const QTime& time)
{
    if (!time.isValid()) {
        qWarning() << "[WARNING] Tentative de formater une heure invalide";
        return "";
    }
    return time.toString("hh:mm");
}

QString AideDateTime::formatDateTime(const QDateTime& dateTime)
{
    if (!dateTime.isValid()) {
        qWarning() << "[WARNING] Tentative de formater un datetime invalide";
        return "";
    }
    return dateTime.toString("dd/MM/yyyy hh:mm:ss");
}

QString AideDateTime::formatDateTimeShort(const QDateTime& dateTime)
{
    if (!dateTime.isValid()) {
        qWarning() << "[WARNING] Tentative de formater un datetime invalide";
        return "";
    }
    return dateTime.toString("dd/MM/yyyy hh:mm");
}

QString AideDateTime::formatDateTimeISO(const QDateTime& dateTime)
{
    if (!dateTime.isValid()) {
        qWarning() << "[WARNING] Tentative de formater un datetime invalide";
        return "";
    }
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

QDate AideDateTime::parseDate(const QString& dateStr)
{
    if (dateStr.isEmpty()) {
        qWarning() << "[WARNING] Chaîne de date vide";
        return QDate();
    }

    // Essayer le format français d'abord
    QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
    if (date.isValid()) {
        return date;
    }

    // Essayer le format ISO
    date = QDate::fromString(dateStr, "yyyy-MM-dd");
    if (date.isValid()) {
        return date;
    }

    qWarning() << "[WARNING] Format de date non reconnu:" << dateStr;
    return QDate();
}

QTime AideDateTime::parseTime(const QString& timeStr)
{
    if (timeStr.isEmpty()) {
        qWarning() << "[WARNING] Chaîne d'heure vide";
        return QTime();
    }

    // Essayer le format long d'abord
    QTime time = QTime::fromString(timeStr, "hh:mm:ss");
    if (time.isValid()) {
        return time;
    }

    // Essayer le format court
    time = QTime::fromString(timeStr, "hh:mm");
    if (time.isValid()) {
        return time;
    }

    qWarning() << "[WARNING] Format d'heure non reconnu:" << timeStr;
    return QTime();
}

QDateTime AideDateTime::parseDateTime(const QString& dateTimeStr)
{
    if (dateTimeStr.isEmpty()) {
        qWarning() << "[WARNING] Chaîne de datetime vide";
        return QDateTime();
    }

    // Essayer le format français complet
    QDateTime dateTime = QDateTime::fromString(dateTimeStr, "dd/MM/yyyy hh:mm:ss");
    if (dateTime.isValid()) {
        return dateTime;
    }

    // Essayer le format français court
    dateTime = QDateTime::fromString(dateTimeStr, "dd/MM/yyyy hh:mm");
    if (dateTime.isValid()) {
        return dateTime;
    }

    // Essayer le format ISO
    dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss");
    if (dateTime.isValid()) {
        return dateTime;
    }

    qWarning() << "[WARNING] Format de datetime non reconnu:" << dateTimeStr;
    return QDateTime();
}

int AideDateTime::daysBetween(const QDate& debut, const QDate& fin)
{
    if (!debut.isValid() || !fin.isValid()) {
        qWarning() << "[WARNING] Tentative de calculer des jours avec des dates invalides";
        return 0;
    }
    return debut.daysTo(fin);
}

int AideDateTime::daysUntil(const QDate& date)
{
    return daysBetween(QDate::currentDate(), date);
}

QDate AideDateTime::addDays(const QDate& date, int jours)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'ajouter des jours à une date invalide";
        return QDate();
    }
    return date.addDays(jours);
}

QDate AideDateTime::addMonths(const QDate& date, int mois)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'ajouter des mois à une date invalide";
        return QDate();
    }
    return date.addMonths(mois);
}

QDate AideDateTime::firstDayOfMonth(const QDate& date)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'obtenir le premier jour avec une date invalide";
        return QDate();
    }
    return QDate(date.year(), date.month(), 1);
}

QDate AideDateTime::lastDayOfMonth(const QDate& date)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'obtenir le dernier jour avec une date invalide";
        return QDate();
    }
    return QDate(date.year(), date.month(), 1).addMonths(1).addDays(-1);
}

QDate AideDateTime::firstDayOfYear(const QDate& date)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'obtenir le premier jour de l'année avec une date invalide";
        return QDate();
    }
    return QDate(date.year(), 1, 1);
}

QDate AideDateTime::lastDayOfYear(const QDate& date)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'obtenir le dernier jour de l'année avec une date invalide";
        return QDate();
    }
    return QDate(date.year(), 12, 31);
}

bool AideDateTime::isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

QString AideDateTime::dayName(const QDate& date, bool abbreviated)
{
    if (!date.isValid()) {
        qWarning() << "[WARNING] Tentative d'obtenir le nom du jour avec une date invalide";
        return "";
    }

    const QStringList daysLong = {
        "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"
    };

    const QStringList daysShort = {
        "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"
    };

    int day = date.dayOfWeek() - 1; // Qt utilise 1-7, nous voulons 0-6
    if (day == 7) day = 6; // Dimanche (7) -> 6

    return abbreviated ? daysShort.at(day) : daysLong.at(day);
}

QString AideDateTime::monthName(int month, bool abbreviated)
{
    if (month < 1 || month > 12) {
        qWarning() << "[WARNING] Numéro de mois invalide:" << month;
        return "";
    }

    const QStringList monthsLong = {
        "Janvier", "Février", "Mars", "Avril", "Mai", "Juin",
        "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"
    };

    const QStringList monthsShort = {
        "Jan", "Fév", "Mar", "Avr", "Mai", "Juin",
        "Jul", "Aoû", "Sep", "Oct", "Nov", "Déc"
    };

    return abbreviated ? monthsShort.at(month - 1) : monthsLong.at(month - 1);
}

QDate AideDateTime::today()
{
    return QDate::currentDate();
}

QTime AideDateTime::currentTime()
{
    return QTime::currentTime();
}

QDateTime AideDateTime::now()
{
    return QDateTime::currentDateTime();
}