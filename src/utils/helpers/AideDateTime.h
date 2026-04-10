#ifndef AIDEDATETIME_H
#define AIDEDATETIME_H

#include <QString>
#include <QDate>
#include <QTime>
#include <QDateTime>

/**
 * @class AideDateTime
 * @brief Classe utilitaire pour la manipulation et formatage des dates/heures
 * 
 * Fournit des méthodes statiques pour formater les dates, heures et datetimes
 * selon les normes de l'application SEMULIKI ERP.
 */
class AideDateTime
{
public:
    /**
     * @brief Formate une date au format français (jj/mm/aaaa)
     * @param date La date à formater
     * @return La date formatée en chaîne de caractères
     */
    static QString formatDate(const QDate& date);

    /**
     * @brief Formate une date au format ISO (aaaa-mm-jj)
     * @param date La date à formater
     * @return La date formatée en chaîne de caractères
     */
    static QString formatDateISO(const QDate& date);

    /**
     * @brief Formate une heure (hh:mm:ss)
     * @param time L'heure à formater
     * @return L'heure formatée en chaîne de caractères
     */
    static QString formatTime(const QTime& time);

    /**
     * @brief Formate une heure courte (hh:mm)
     * @param time L'heure à formater
     * @return L'heure formatée en chaîne de caractères
     */
    static QString formatTimeShort(const QTime& time);

    /**
     * @brief Formate une date et heure complètes (jj/mm/aaaa hh:mm:ss)
     * @param dateTime La date et heure à formater
     * @return La date et heure formatées en chaîne de caractères
     */
    static QString formatDateTime(const QDateTime& dateTime);

    /**
     * @brief Formate une date et heure courtes (jj/mm/aaaa hh:mm)
     * @param dateTime La date et heure à formater
     * @return La date et heure formatées en chaîne de caractères
     */
    static QString formatDateTimeShort(const QDateTime& dateTime);

    /**
     * @brief Formate une date et heure en ISO (aaaa-mm-jj hh:mm:ss)
     * @param dateTime La date et heure à formater
     * @return La date et heure formatées en ISO
     */
    static QString formatDateTimeISO(const QDateTime& dateTime);

    /**
     * @brief Convertit une chaîne de caractères en QDate
     * @param dateStr La date en chaîne (format jj/mm/aaaa ou aaaa-mm-jj)
     * @return La date convertie ou QDate invalide si erreur
     */
    static QDate parseDate(const QString& dateStr);

    /**
     * @brief Convertit une chaîne de caractères en QTime
     * @param timeStr L'heure en chaîne (format hh:mm:ss ou hh:mm)
     * @return L'heure convertie ou QTime invalide si erreur
     */
    static QTime parseTime(const QString& timeStr);

    /**
     * @brief Convertit une chaîne de caractères en QDateTime
     * @param dateTimeStr La date et heure en chaîne
     * @return La date et heure converties ou QDateTime invalide si erreur
     */
    static QDateTime parseDateTime(const QString& dateTimeStr);

    /**
     * @brief Obtient le nombre de jours entre deux dates
     * @param debut La date de début
     * @param fin La date de fin
     * @return Le nombre de jours (positif si fin > début)
     */
    static int daysBetween(const QDate& debut, const QDate& fin);

    /**
     * @brief Obtient le nombre de jours restants jusqu'à une date
     * @param date La date cible
     * @return Le nombre de jours (négatif si la date est passée)
     */
    static int daysUntil(const QDate& date);

    /**
     * @brief Ajoute un nombre de jours à une date
     * @param date La date de base
     * @param jours Nombre de jours à ajouter
     * @return La nouvelle date
     */
    static QDate addDays(const QDate& date, int jours);

    /**
     * @brief Ajoute un nombre de mois à une date
     * @param date La date de base
     * @param mois Nombre de mois à ajouter
     * @return La nouvelle date
     */
    static QDate addMonths(const QDate& date, int mois);

    /**
     * @brief Obtient le premier jour du mois
     * @param date Une date du mois
     * @return Le premier jour du mois
     */
    static QDate firstDayOfMonth(const QDate& date);

    /**
     * @brief Obtient le dernier jour du mois
     * @param date Une date du mois
     * @return Le dernier jour du mois
     */
    static QDate lastDayOfMonth(const QDate& date);

    /**
     * @brief Obtient le premier jour de l'année
     * @param date Une date de l'année
     * @return Le premier jour de l'année
     */
    static QDate firstDayOfYear(const QDate& date);

    /**
     * @brief Obtient le dernier jour de l'année
     * @param date Une date de l'année
     * @return Le dernier jour de l'année
     */
    static QDate lastDayOfYear(const QDate& date);

    /**
     * @brief Vérifie si une année est bissextile
     * @param year L'année à vérifier
     * @return true si bissextile, false sinon
     */
    static bool isLeapYear(int year);

    /**
     * @brief Obtient le nom du jour de la semaine
     * @param date La date
     * @param abbreviated true pour le nom court (ex: Lun), false pour complet (ex: Lundi)
     * @return Le nom du jour
     */
    static QString dayName(const QDate& date, bool abbreviated = false);

    /**
     * @brief Obtient le nom du mois
     * @param month Le numéro du mois (1-12)
     * @param abbreviated true pour le nom court, false pour complet
     * @return Le nom du mois
     */
    static QString monthName(int month, bool abbreviated = false);

    /**
     * @brief Obtient la date actuelle
     * @return La date du jour
     */
    static QDate today();

    /**
     * @brief Obtient l'heure actuelle
     * @return L'heure actuelle
     */
    static QTime currentTime();

    /**
     * @brief Obtient la date et heure actuelle
     * @return La date et heure actuelle
     */
    static QDateTime now();

private:
    /**
     * @brief Constructeur privé (classe utilitaire statique)
     */
    AideDateTime() = default;
};

#endif // AIDEDATETIME_H