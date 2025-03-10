/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001-2002 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMECHAT_H__
#define __KGAMECHAT_H__

// own
#include "../kchatbase.h"
#include "kdegamesprivate_export.h"
// Qt
#include <QString>

class KPlayer;
class KGame;
class KGamePropertyBase;

class KGameChatPrivate;

/**
 * \class KGameChat kgamechat.h <KGame/KGameChat>
 *
 * @short A Chat widget for KGame-based games
 *
 * Call @ref setFromPlayer() first - this will be used as the "from" part of
 * every message you will send. Otherwise it won't work! You can also use the
 * fromPlayer parameter in the constructor though...
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class KDEGAMESPRIVATE_EXPORT KGameChat : public KChatBase
{
    Q_OBJECT

public:
    /**
     * Construct a @ref KGame chat widget on @p game that used @p msgid for
     * the chat message. The @p fromPlayer is the local player (see @ref
     * setFromPlayer).
     */
    KGameChat(KGame *game, int msgid, KPlayer *fromPlayer, QWidget *parent, KChatBaseModel *model = nullptr, KChatBaseItemDelegate *delegate = nullptr);

    /**
     * @overload
     * To make use of this widget you need to call @ref setFromPlayer
     * manually.
     */
    KGameChat(KGame *game, int msgId, QWidget *parent, KChatBaseModel *model = nullptr, KChatBaseItemDelegate *delegate = nullptr);

    /**
     * @overload
     * This constructs a widget that is not usable. You must call at least
     * setGame, setFromPlayer and setMessageId manually.
     */
    explicit KGameChat(QWidget *parent);

    ~KGameChat() override;

    enum SendingIds {
        SendToGroup = 1
    };

    /**
     * This sets the fromPlayer to @p player. The fromPlayer is the
     * player that will appear as "from" when you send messages through this
     * widget.
     * @param player The player of this widget
     */
    void setFromPlayer(KPlayer *player);

    KPlayer *fromPlayer() const;

    /**
     * Set the @ref KGame object for this chat widget. All messages will be
     * sent through this object. You don't have to implement any send
     * functions, just call this function, call @ref setFromPlayer and be
     * done :-)
     * @param g The @ref KGame object the messages will be sent through
     */
    void setKGame(KGame *g);

    KGame *game() const;

    /**
     * @return The id of the messages produced by KGameChat. The id will be
     * used in @ref KGame as parameter msgid in the method @ref KGame::sendMessage
     */
    int messageId() const;

    /**
     * Change the message id of the chat widget. It is recommended that you
     * don't use this but prefer the constructor instead, but in certain
     * situations (such as using this widget in Qt designer) it may be
     * useful to change the message id.
     *
     * See also @ref messageId
     */
    void setMessageId(int msgid);

    /**
     * reimplemented from @ref KChatBase
     * @return @ref KPlayer::name() for the player set by @ref setFromPlayer
     */
    QString fromName() const override;

public Q_SLOTS:
    void addMessage(const QString &fromName, const QString &text) override
    {
        KChatBase::addMessage(fromName, text);
    }
    virtual void addMessage(int fromId, const QString &text);

    void slotReceiveMessage(int, const QByteArray &, quint32 receiver, quint32 sender);

protected:
    /**
     * @param id The ID of the sending entry, as returned by @ref KChatBase sendingEntry
     * @return True if the entry "send to all" was selected, otherwise false
     */
    bool isSendToAllMessage(int id) const;

    /**
     * Used to indicate whether a message shall be sent to a group of
     * players. Note that this was not yet implemented when this doc was
     * written so this description might be wrong. (FIXME)
     * @param id The ID of the sending entry, as returned by @ref KChatBase sendingEntry
     * @return True if the message is meant to be sent to a group (see @ref KPlayer::group
     * ), e.g. if "send to my group" was selected.
     */
    bool isToGroupMessage(int id) const;

    /**
     * Used to indicate whether the message shall be sent to a single player
     * only. Note that you can also call @ref isSendToAllMessage and @ref
     * isToGroupMessage - if both return false it must be a player message.
     * This behaviour might be changed later - so don't depend on it.
     *
     * See also toPlayerId
     * @param id The ID of the sending entry, as returned by
     * @ref KChatBase sendingEntry
     * @return True if the message shall be sent to a special player,
     * otherwise false.
     */
    bool isToPlayerMessage(int id) const;

    /**
     * @param id The ID of the sending entry, as returned by
     * @ref KChatBase sendingEntry
     * @return The ID of the player (see KPlayer::id) the sending entry
     * belongs to. Note that the parameter id is an id as returned by
     * @ref KChatBase sendingEntry and the id this method returns is a
     * KPlayer ID. If @ref isToPlayerMessage returns false this method
     * returns -1
     */
    int playerId(int id) const;

    /**
     * @param playerId The ID of the KPlayer object
     * @return The ID of the sending entry (see KChatBase) or -1 if
     * the player id was not found.
     */
    int sendingId(int playerId) const;

    /**
     * @return True if the player with this ID was added before (see
     * slotAddPlayer)
     */
    bool hasPlayer(int id) const;

    /**
     * @param name The name of the added player
     * @return A string that will be added as sending entry in @ref
     * KChatBase. By default this is "send to name" where name is the name
     * that you specify. See also KChatBase::addSendingEntry
     */
    virtual QString sendToPlayerEntry(const QString &name) const;

protected Q_SLOTS:
    /**
     * Unsets a KGame object that has been set using setKGame
     * before. You don't have to call this - this is usually done
     * automatically.
     */
    void slotUnsetKGame();

    void slotPropertyChanged(KGamePropertyBase *, KPlayer *);
    void slotAddPlayer(KPlayer *);
    void slotRemovePlayer(KPlayer *);

    /**
     * Called when KPlayer::signalNetworkData is emitted. The message
     * gets forwarded to slotReceiveMessage if @p me equals
     * fromPlayer.
     */
    void slotReceivePrivateMessage(int msgid, const QByteArray &buffer, quint32 sender, KPlayer *me);

protected:
    void returnPressed(const QString &text) override;

private:
    void init(KGame *g, int msgid);

private:
    Q_DECLARE_PRIVATE_D(KChatBase::d, KGameChat)
};

#endif
