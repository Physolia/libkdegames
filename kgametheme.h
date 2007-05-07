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

#ifndef _KGAMETHEME_H_
#define _KGAMETHEME_H_

#include <libkdegames_export.h>

class KGameThemePrivate;
class QString;
class QPixmap;

/**
 * @short Class for managing highscore tables
 *
 * Essentially just a wrapper around a .desktop theme file. Load a file with
 * load() and then access its properties.
 * 
 * @author Mauricio Piacentini
 **/
class KDEGAMES_EXPORT KGameTheme 
{
    public:
        KGameTheme();
        ~KGameTheme();

        /**
         * Load the default theme file. Called "default.desktop"
         * @return true if the theme files and properties could be loaded
         */
        bool loadDefault();
        /**
         * Load a specific theme file.
         * @param file the name of the theme file. e.g. "classic.desktop"
         * @return true if the theme files and properties could be loaded
         */
        bool load(const QString &file);
        /// @return the full path of the .desktop file
        QString path() const;
        /// @return just the "*.desktop" part
        QString fileName() const;
        /// @return the full path of the svg file
        QString graphics() const;
        /// @return a preview pixmap
        QPixmap preview() const;
        /**
         * Possible keys:
         * - Name
         * - Author
         * - Description
         * - AuthorEmail
         * @param key the key of the wanted property
         * @return the data realted to 'key'
         */
        QString authorProperty(const QString &key) const;
    
    private:
        friend class KGameThemePrivate;
        KGameThemePrivate *const d;
};

#endif
