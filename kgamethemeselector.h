/*
    Copyright (C) 2006 Mauricio Piacentini  <mauricio@tabuleiro.com>

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

#ifndef __KGAMETHEMESELECTOR_H_
#define __KGAMETHEMESELECTOR_H_

#include <QtCore/QMap>
#include <QtGui/QWidget>

#include <libkdegames_export.h>

class KGameTheme;
class KConfigSkeleton;

class KDEGAMES_EXPORT KGameThemeSelector : public QWidget
{
    Q_OBJECT
    public:
        KGameThemeSelector( QWidget* parent, KConfigSkeleton* config );
    
    private:
        class KGameThemeSelectorPrivate;
        KGameThemeSelectorPrivate* const d;
    
    public Q_SLOTS:
        void updatePreview();
        void openKNewStuffDialog();
};

#endif
