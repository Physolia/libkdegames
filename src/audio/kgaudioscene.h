/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef KGAUDIOSCENE_H
#define KGAUDIOSCENE_H

// own
#include <libkdegames_export.h>
// Qt
#include <QPointF>

/**
 * @namespace KgAudioScene
 *
 * This class exposes general properties of the audio playback context. Actual
 * sounds are represented in this context by KgSound instances.
 *
 * The audio scene basically consists of a listener. The position of this
 * listener is relevant when sounds are played at certain positions: The audio
 * channels will then be balanced to make the sound appear to come from that
 * direction.
 *
 * Because there can ogly be one listener, all methods in this class are static.
 *
 * @warning Not all functionally exposed by the API of this class is guaranteed
 *          to be available on the compiled KgAudio backend. Check
 *          KgAudioScene::capabilities() if in doubt.
 */
namespace KgAudioScene
{
	///This enumeration represents capabilities which may not be provided by
	///every KgAudio backend.
	enum Capability
	{
		///Playback starts as soon as KgSound::start is called.
		SupportsLowLatencyPlayback = 1 << 0,
		SupportsPositionalPlayback = 1 << 1,
	};
	Q_DECLARE_FLAGS(Capabilities, Capability)

	///@return which capabilities are supported by the compiled KgAudio backend
	KDEGAMES_EXPORT Capabilities capabilities();

	///@return the position of the listener
	KDEGAMES_EXPORT QPointF listenerPos();
	///Sets the position of the listener. The default is (0.0, 0.0), the
	///point of origin.
	///@note Effective only if positional playback is supported.
	KDEGAMES_EXPORT void setListenerPos(const QPointF& pos);
	///@return the master volume for sounds outputted by TagaroAudio
	KDEGAMES_EXPORT qreal volume();
	///Sets the master volume for sounds outputted by TagaroAudio. The
	///default is 1.0, which means no volume change, compared to the
	///original sounds. 0.0 means that all sounds are muted.
	KDEGAMES_EXPORT void setVolume(qreal volume);

	///@returns whether an error was detected in the audio backend
	///
	///Since KgAudio is typically used by games where audio is not an absolutely
	///vital part of the gameplay, we do not need to fail if sound does not work,
	///over even make some sort of deep analysis why something did not work. The
	///user will notice missing sound, and advanced users may investigate
	///the kWarning() messages. That is usually enough. If not, use this method.
	///
	///The state of hasError() may theoretically change while the application
	///runs, but in practice, this is very unlikely. (The only tricky part is
	///typically the initial allocation of resources.)
	///
	///@sa KgSound::hasError()
	KDEGAMES_EXPORT bool hasError();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KgAudioScene::Capabilities)

#endif // KGAUDIOSCENE_H
