#pragma once
#include <QString>
#include <QStringList>

// Reihenfolge: de en es fr it pt nl pl tr ru ja zh
QStringList languageCodes();
QStringList languageNames();  // Eigennamen (Deutsch, English, Español, ...)
void setLanguage(const QString& code);
QString currentLanguage();
QString T(const char* key);
