/*
    Copyright (C) 2007 Mauricio Piacentini   <mauricio@tabuleiro.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "kgametheme.h"

#include <KStandardDirs>
#include <KLocale>
#include <KConfig>
#include <KSvgRenderer>
#include <KComponentData>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QDebug>
#include <QtGui/QPixmap>

class KGameThemePrivate
{
    public:
        KGameThemePrivate(){}
        
        QMap<QString, QString> authorproperties;
        QString fullPath;
        QString fileName; ///< just e.g. "default.desktop"
        QString graphics;
        QPixmap preview;
        KSvgRenderer svg;
};

KGameTheme::KGameTheme()
    : d(new KGameThemePrivate)
{
    static bool _inited = false;
    if (_inited)
        return;
    KGlobal::dirs()->addResourceType("gametheme", KStandardDirs::kde_default("data") + KGlobal::mainComponent().componentName() + "/themes/");

    KGlobal::locale()->insertCatalog("kmines");
    _inited = true;
}

KGameTheme::~KGameTheme() {
    delete d;
}

bool KGameTheme::loadDefault()
{
    return load("default.desktop");
}

#define kThemeVersionFormat 1

bool KGameTheme::load(const QString &fileName) {

    QString filePath = KStandardDirs::locate("gametheme", fileName);
    qDebug() << "Attempting to load .desktop at " << filePath;
    if (filePath.isEmpty()) {
        return false;
    }
    
    QString graphicsPath;

    // verify if it is a valid file first and if we can open it
    QFile themefile(filePath);
    if (!themefile.open(QIODevice::ReadOnly)) {
      return (false);
    }
    themefile.close();

    KConfig themeconfig(filePath, KConfig::OnlyLocal);
    KConfigGroup group = themeconfig.group("KGameTheme");

    d->authorproperties.insert("Name", group.readEntry("Name"));// Returns translated data
    d->authorproperties.insert("Author", group.readEntry("Author"));
    d->authorproperties.insert("Description", group.readEntry("Description"));
    d->authorproperties.insert("AuthorEmail", group.readEntry("AuthorEmail"));

    //Version control
    int themeversion = group.readEntry("VersionFormat",0);
    //Format is increased when we have incompatible changes, meaning that older clients are not able to use the remaining information safely
    if (themeversion > kThemeVersionFormat) {
        return false;
    }

    QString graphName = group.readEntry("FileName");
    graphicsPath = KStandardDirs::locate("gametheme", graphName);
    d->graphics = graphicsPath;
    if (graphicsPath.isEmpty()) return (false);

    d->svg.load(graphicsPath);
    if (!d->svg.isValid()) {
        qDebug() << "could not load svg";
        return( false );
    }
    
    QString previewName = group.readEntry("Preview");
    graphicsPath = KStandardDirs::locate("gametheme", previewName);
    d->preview = QPixmap(graphicsPath);

    d->fileName = fileName;
    d->fullPath = filePath;
    return true;
}

QString KGameTheme::path() const {
    return d->fullPath;
}

QString KGameTheme::fileName() const {
    return d->fileName;
}

QString KGameTheme::graphics() const {
  return d->graphics;
}

QPixmap KGameTheme::preview() const {
  return d->preview;
}

QString KGameTheme::authorProperty(const QString &key) const {
    return d->authorproperties[key];
}
