/* **************************************************************************
                           KGame class
                           -------------------
    begin                : 1 January 2001
    copyright            : (C) 2001 by Martin Heni and Andreas Beckermann
    email                : martin@heni-online.de and b_mann@gmx.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/
/*
    $Id$
*/
#ifndef __KGAME_H_
#define __KGAME_H_

#include <qstring.h>
#include <qlist.h>
#include <qintdict.h>
#include <krandomsequence.h>

#include "kgamenetwork.h"

class KPlayer;
class KGamePropertyBase;
class KGamePropertyHandlerBase;

class KGamePrivate;

/**
 * The KGame class is the central game object. A game basically
 * consists of following features:
 * - Player handling (add, remove,...)
 * - Game status (end,start,pause,...)
 * - load/save
 * - Move (and message) handling 
 * - Network connection (for KGameNetwork)
 *
 * Example:
 * <pre>
 * KGame *game=new KGame;
 * </pre>
 *
 * Note: I think KGame and KGameNetwork should
 *       be merged into one class
 *
 * @short The main KDE game object
 * @author Martin Heni <martin@heni-online.de>
 * @version $Id$
 *
 */
class KGame : public KGameNetwork
{
  Q_OBJECT

public:
    typedef QList<KPlayer> KGamePlayerList;

    /** 
     * Create a KGame object
     */
    KGame(int cookie=42,QObject* parent=0);
    ~KGame();

    /**
     * Gives debug output of the game status
     */
    virtual void Debug();


    /**
     * Game status: kind of unused at the moment
     */
    enum GameStatus {Run,Pause,End,Abort};

    // Properties
    /**
     * Returns a list of all active players 
     *
     * @return the list of players
     */
    KGamePlayerList *playerList() { return &mPlayerList; }

    /**
     * The same as @ref playerList but returns a const pointer.
     **/
    const KGamePlayerList *playerList() const { return &mPlayerList; }

    /**
     * Returns a list of all inactive players 
     *
     * @return the list of players
     */
    KGamePlayerList *inactivePlayerList() {return &mInactivePlayerList;}

    /**
     * The same as @ref inactivePlayerList but returns a const pointer.
     **/
    const KGamePlayerList *inactivePlayerList() const {return &mInactivePlayerList;}
    
    /**
     * Is the game running
     *
     * @return true/false
     */
    bool isRunning() const;

    // Player handling
    /**
     * Returns the player object for a given player id
     *
     * @param id Player id
     * @return player object
     */
    KPlayer *findPlayer(int id) const;


    /**
     * Note that @ref KPlayer::save must be implemented properly, as well as
     * @ref KPlayer::rtti!
     * This will only send a message to all clients. The player is _not_ added
     * directly!
     * See also @ref signalPlayerInput which will be emitted as soon as the
     * player really has been added.
     *
     * Note that an added player will first get into a "queue" and won't be in
     * the game. It will be added to the game as soon as systemAddPlayer is
     * called what will happen as soon as IdAddPlayer is received.
     *
     * Note: you probably want to connect to @ref signalPlayerJoinedGame for
     * further initialization!
     * @param newplayer The player you want to add. KGame will send a message to
     * all clients and add the player using @ref systemAddPlayer
     **/
    void addPlayer(KPlayer* newplayer) { addPlayer(newplayer, 0); }
    
    /**
     * Sends a message over the network, msgid=IdRemovePlayer.
     *
     * As soon as this message is received by @ref networkTransmission @ref
     * systemRemovePlayer is called and the player is removed.
     **/
    //AB: TODO: make sendMessage to return if the message will be able to be
    //sent, eg if a socket is connected, etc. If sendMessage returns false
    //remove the player directly using systemRemovePlayer
    bool removePlayer(KPlayer * player) { return removePlayer(player, 0); }

    /**
     * sends activate player: interal use only?
     */
    bool activatePlayer(KPlayer *player);

    /**
     * sends inactivate player: interal use only?
     */
    bool inactivatePlayer(KPlayer *player);

    /**
     * Set the maximal number of players. After this is
     * reached no more players can be added
     *
     * @param maxnumber maximal number of players
     * @param transmit command over network
     */
    void setMaxPlayers(uint maxnumber);
    /**
     * What is the maximal number of players?
     *
     * @return maximal number of players
     */
    int maxPlayers() const;
    /**
     * Set the minimal number of players. A game can not be started
     * withless player resp. is paused when already running
     *
     * @param minnumber minimal number of players
     * @param transmit command over network
     */
    void setMinPlayers(uint minnumber);

    /**
     * What is the minimal number of players?
     *
     * @return minimal number of players
     */
    uint minPlayers() const;

    /**
     * Returns how many players are plugged into the game
     *
     * @return number of players
     */
    uint playerCount() const;

    /**
     * Select the next player in a turn based game. In an asynchronous game this
     * function has no meaning. Overwrite this function for your own game sequence.
     * Per default it selects the next player in the playerList
     */
    virtual bool nextPlayer(KPlayer *last,bool exclusive=true);

    // Input events
    /**
     * Called by @ref KPlayer to send a player input to the @ref
     * KGameCommunicationServer.
     **/
    virtual bool sendPlayerInput(QDataStream &msg,KPlayer *player,int sender=0);

    /**
     * Called when a player input arrives from @ref KGameCommunicationServer.
     * TODO: docu!!
     **/
    virtual bool playerInput(QDataStream &msg,KPlayer *player,int sender=0);

    // load/save
    /**
     * Load a saved game, from file OR network. This function has
     * to be overwritten or you need to connect to the load signal
     * if you have game data other than KGameProperty.
     * For file load you should reset() the game before any load attempt
     * to make sure you load into an clear state.
     *
     * @param stream a data stream where you can stream the game from
     *
     * @return true?
     */
    virtual bool load(QDataStream &stream);
    /**
     * Save a game to a file OR to network. Otherwise the same as 
     * the load function
     *
     * @param stream a data stream to load the game from
     * @param saveplayers If true then all players wil be saved too
     *
     * @return true?
     */
    virtual bool save(QDataStream &stream,bool saveplayers=true);

    /**
     * Resets the game, i.e. puts it into a state where everything
     * can be started from, e.g. a load game
     * Right now it does only need to delete all players
     *
     * @return true on success
     */
    virtual bool reset();


    // Game sequence
    /**
     * returns the game status, ie running,pause,ended,...
     *
     * @return game status
     */
    int gameStatus() const;
    /**
     * sets the game status
     *
     * @param status the new status
     * @param transmit command over network
     */
    virtual void setGameStatus(int status);

    //AB: docu: see KPlayer
    bool addProperty(KGamePropertyBase* data);

    /**
     * Caled by KGameProperty only! Internal function!
     **/
    void sendProperty(QDataStream& s, bool isPublic = true);
    /**
      * Called by KGameProperty only! Internal function!
     **/
    void emitSignal(KGamePropertyBase *me);
    /**
     * This is called by @ref KPlayer::sendProperty only! Internal function!
     **/
    void sendPlayerProperty(QDataStream& s, int playerId, bool isPublic = true);
    
    KGamePropertyBase* findProperty(int id) const;

    /**
     * Returns a pointer to the game's KRandomSequence. This sequence is
     * identical for all network players!
     *
     * @return KRandomSequence pointer
     */
    KRandomSequence *random() {return &mRandom;}


    /**
     * Sends a network message msg with a given msg id msgid to all players of
     * a given group
     *
     * @param msg the message which will be send. See messages.txt for contents
     * @param msgid an id for this message
     * @param group the group of the receivers
     * @return true if worked
     */
    bool sendGroupMessage(QDataStream &msg, int msgid, int sender, const QString& group);
    bool sendGroupMessage(const QString& msg, int msgid, int sender, const QString& group);


    /**
     * This will either forward an incoming message to a specified player 
     * (see @ref KPlayer::networkTransmission) or
     * handle the message directly (e.g. if msgif==IdRemovePlayer it will remove
     * the (in the stream) specified player). If both is not possible (i.e. the
     * message is user specified data) the signal @ref signalNetworkData is
     * emitted.
     * @param msgid Specifies the kind of the message. See messages.txt for
     * further information
     * @param stream The message that is being sent
     * @param receiver The is of the player this message is for. 0 For broadcast.
     * @param sender
     * @param the client from which we received the transmission - hardly used
     **/
    virtual void networkTransmission(QDataStream &stream,int msgid,int receiver,int sender, Q_UINT32 clientID);




protected slots:
    /**
     * This slot is called whenever the connection to a client is lost (ie the
     * signal @ref KGameNetwork::signalConnectionLost is called) and will remove
     * the players from that client.
     * @param client The client the connection has been lost to
     **/
    void slotConnectionLost(Q_UINT32 clientID);
    /**
     * Prepare the call of next() after a QTimer event to allow QT Event processing
     */
    void prepareNext();

signals:
    /**
     * A player input occured. This is the most important function
     * as the given message will contain the current move made by
     * the given player.
     * Example:
     * <pre>
     * void MyClass::slotPlayerInput(QDataStream &msg,KPlayer *player)
     * {
     *   Q_INT32 move;
     *   msg >>  move;
     *   kdDebug() << "  Player " << player->id() << " moved to " << move <<
     *   endl;
     * }
     * </pre>
     *
     * @param msg the move message
     * @param player the player who did the move
     */
    void signalPlayerInput(QDataStream &msg,KPlayer *player);
    /**
     * unused
     */
    void signalMessage(QDataStream &msg);
    /**
     * the game will be loaded from the given stream. We better
     * fill our data.
     *
     * @param stream the load stream
     */
    void signalLoad(QDataStream &stream);
    /**
     * the game will be saved to the given stream. We better
     * put our data.
     *
     * @param stream the save stream
     */
    void signalSave(QDataStream &stream);

    /**
     * We got an user defined update message. This is usually done
     * by a sendData in a inherited KGame Object which defines its
     * own methods and has to syncronise them over the network.
     * Reaction to this is usually a call to a KGame function.
     */
    void signalNetworkData(int msgid,const QByteArray& buffer,int receiver,int sender);

    /**
     * We got an network message. this can be used to notify us that something
     * changed. What changed can be seen in the message id. Whether this is
     * the best possible method to do this is unclear...
     */
    void signalMessageUpdate(int msgid,int receiver,int sender);
    /**
     * a player left the game because of a broken connection!
     *
     * @param player the player who left the game
     */
    void signalPlayerLeftGame(KPlayer *player);
    /**
     * a player joined the game
     *
     * @param player the player who joined the game
     *
     */
    void signalPlayerJoinedGame(KPlayer *player);

    /**
    * This signal is emmited if the KGame needs to create a new player.
    * Currently this happens only over a network connection. Doing nothing
    * will create a default KPlayer. If you want to have your own player
    * you have to create one with the given rtti here.
    *
    * @param player is the player object which got created
    * @param rtti is the type of the player (0 means default KPlayer)
    * @param io is the 'or'ed rtti of the KGameIO's
    */
    void signalCreatePlayer(KPlayer *&player,int rtti,int io,bool isvirtual,KGame *me);

    /**
    * This signal is emmited if a player property changes its value and
    * the property is set to notify this change
    *
    */
    void signalPropertyChanged(KGamePropertyBase *property,KGame *me);

    /**
     * Is emitted after a call to gameOver() returns a nonm zero
     * return code. This code is forwarded to this signal as 'status'.
     *
     * @param status the return code of gameOver()
     * @param current the player who did the last move
     * @param a pointer to the KGame object
     */
    void signalGameOver(int status,KPlayer *current,KGame *me);



protected:
    /**
     * Updates the ids of the players. Called when a connection is changed, e.g.
     * when you connect from a local game to a network game.
     **/
    void updatePlayerIds();
    
    /**
     * inactivates player. Use @ref inactivatePlayer instead!
     */
    bool systemInactivatePlayer(KPlayer *player);

    /**
     * activates player. Use @ref activatePlayer instead!
     */
    bool systemActivatePlayer(KPlayer *player);

    /**
     * Adds a player to the game
     *
     * Use @ref addPlayer to send @ref KGameMessage::IdAddPlayer. As soon as
     * this Id is received this function is called, where the player (see @ref
     * KPlayer::rtti) is added as well as its properties (see @ref KPlayer::save
     * and @ref KPlayer::load)
     *
     * This method calls the overloaded @ref systemAddPlayer with the created
     * player as argument. That method will really add the player.
     * If you need to do some changes to your newly added player just connect to
     * @ref signalPlayerJoinedGame
     */
    void systemAddPlayer(QDataStream& stream);

    /**
     * Finally adds a player to the game and therefore to the list.
     **/
    void systemAddPlayer(KPlayer* newplayer);

    /**
     * Removes a player from the game
     *
     * Use @ref removePlayer to send @ref KGameMessage::IdRemovePlayer. As soon
     * as this Id is received systemRemovePlayer is called and the player is
     * removed directly.
     *
     **/
    void systemRemovePlayer(KPlayer* player);
    
    /**
     * This slot is called when a new client has connected to the game. This
     * member function will transmit e.g. all players to that client, as well as
     * all properties of these players (at least if they have been added by
     * @ref KPlayer::addProperty) so that the client will finally have the same
     * status as the master. You want to overwrite this function if you expand
     * KGame by any properties which have to be known by all clients.
     *
     * Only the MASTER is allowed to call this.
     * @param clientID The ID of the message client which has connected
     **/
    virtual void negotiateNetworkGame(Q_UINT32 clientID);

    /**
     * syncronise the random numbers with all network clients
     */
    void syncRandom();

    /**
     * Well what do you think will this member function set?
     * @param id Uff - What is this parameter for? I don't know ;-)
     **/
    virtual void setGameId(int id);
    void deletePlayers();
    void deleteInactivePlayers();

    /**
     * Check whether the game is over.
     *
     * @param the player who made the last move
     * @return anything else but 0 is considered as game over
     *
     */
    virtual int gameOver(KPlayer *player);

    /**
     * Load a saved game, from file OR network. Internal
     *
     * @param stream a data stream where you can stream the game from
     * @param is it a network call -> make players virtual
     *
     * @return true?
     */
    virtual bool loadgame(QDataStream &stream,bool network);

    KGamePropertyHandlerBase* dataHandler();

private:
    //AB: this is to hide the "receiver" parameter from the user. It shouldn't be
    //used if possible (except for init).
    /**
     * This is an overloaded function. Id differs from the public one only in
     * its parameters:
     *
     * @param receiver The Client that will receive the message. You will hardly
     * ever need this. It it internally used to initialize a newly connected
     * client.
     **/
    void addPlayer(KPlayer* newplayer, int receiver);

    /**
     * Just the same as the public one except receiver:
     * @param receiver 0 for broadcast, otherwise the receiver. Should only be
     * used in special circumstances and not outside KGame.
     **/
    bool removePlayer(KPlayer * player, int receiver);

    /**
     * Helping function - game negotiation
     **/
    void setupGame(QDataStream& msg, int sender);

    /**
     * Helping function - game negotiation
     **/
    void setupGameContinue(QDataStream& msg, int sender);

    /**
     * Helping function - game negotiation
     **/
    void gameReactivatePlayer(QDataStream& msg, int sender);

    /**
     * Removes a player from all list, removes the @ref KGame pointer from the
     * @ref KPlayer but does not delete the player. Used by (e.g.) @ref
     * systemRemovePlayer
     * @return True if the player has been removed, false if the current is not
     * found
     **/
    bool systemRemove(KPlayer* player);

    /**
     * Prepare a player for being added. Put all data about a player into the
     * stream so that it can be sent to the @ref KGameCommunicationServer using
     * @ref addPlayer (e.g.)
     *
     * This function ensures that the code for adding a player is the same in
     * @ref addPlayer as well as in @ref negotiateNetworkGame
     * @param receiver The owner of the player
     **/
    void savePlayer(QDataStream& stream,KPlayer* player, int receiver=0);
    
private:
    KRandomSequence mRandom;

    KGamePlayerList mPlayerList;
    KGamePlayerList mInactivePlayerList;

    KGamePrivate* d;
};

#endif
