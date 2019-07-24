/***************************************************************************
 *   Copyright 2019 Alexander Potashev <aspotashev@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "virtualfileqt-openal.h"

VirtualFileQt::VirtualFileQt(const QString &path)
    : m_file(path)
{
}

bool VirtualFileQt::open()
{
    return m_file.open(QIODevice::ReadOnly);
}

int64_t VirtualFileQt::getFileLen() const
{
    return static_cast<int64_t>(m_file.size());
}

int64_t VirtualFileQt::seek(int64_t offset, int whence)
{
    switch (whence) {
        case SEEK_SET:
            return m_file.seek(static_cast<quint64>(offset)) ? 0 : -1;
        case SEEK_CUR:
            return m_file.seek(m_file.pos() + static_cast<quint64>(offset)) ? 0 : -1;
        case SEEK_END:
            return m_file.seek(m_file.size() + static_cast<quint64>(offset)) ? 0 : -1;
        default:
            return -1;
    }
}

int64_t VirtualFileQt::read(void *ptr, int64_t count)
{
    return static_cast<int64_t>(m_file.read(static_cast<char*>(ptr), static_cast<qint64>(count)));
}

int64_t VirtualFileQt::write(const void *, int64_t)
{
    // Writing is not supported.
    return 0;
}

int64_t VirtualFileQt::tell()
{
    return static_cast<int64_t>(m_file.pos());
}

sf_count_t sf_get_filelen(void *user_data)
{
    return static_cast<sf_count_t>(VirtualFileQt::get(user_data)->getFileLen());
}

sf_count_t sf_seek(sf_count_t offset, int whence, void *user_data)
{
    return static_cast<sf_count_t>(VirtualFileQt::get(user_data)->seek(static_cast<int64_t>(offset), whence));
}

sf_count_t sf_read(void *ptr, sf_count_t count, void *user_data)
{
    return static_cast<sf_count_t>(VirtualFileQt::get(user_data)->read(ptr, static_cast<int64_t>(count)));
}

sf_count_t sf_write(const void *ptr, sf_count_t count, void *user_data)
{
    return static_cast<sf_count_t>(VirtualFileQt::get(user_data)->write(ptr, static_cast<int64_t>(count)));
}

sf_count_t sf_tell(void *user_data)
{
    return static_cast<sf_count_t>(VirtualFileQt::get(user_data)->tell());
}

SF_VIRTUAL_IO sfVirtualIO = {
    sf_get_filelen,
    sf_seek,
    sf_read,
    sf_write,
    sf_tell,
};

SF_VIRTUAL_IO &VirtualFileQt::getSndfileVirtualIO()
{
    return sfVirtualIO;
}

VirtualFileQt *VirtualFileQt::get(void *user_data)
{
    return static_cast<VirtualFileQt*>(user_data);
}
