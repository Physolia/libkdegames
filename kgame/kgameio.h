/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/*
    $Id$
*/
#ifndef __KGAMEIO_H__
#define __KGAMEIO_H__


#include <qstring.h>
#include <qobject.h>

class KPlayer;
class KProcess;

/**
 *  The KGameIO class. This is the master class for
 *  creating IO game devices. You cannot use it directly.
 *  Either take one of the classes derived from it or
 *  you have to create your own IO class derived from it.
 *
 *  The idea behind this class is to provide a common interface
 *  for input devices into your game. By programming a KGameIO
 *  device you need not distinguish the actual IO in the game
 *  anymore. All work is done by the IO's. This allows very
 *  easy reuse in other games as well.
 */
class KGameIO : public QObject
{
  Q_OBJECT

public:
    /**
     * Constructs a KGameIO object
     */
   KGameIO();
   ~KGameIO();

    /**
     * Gives debug output of the game status
     */
   void Debug();

   /**
    * Identifies the KGameIO via the rtti function
    */
   enum IOMode {GenericIO=1,KeyIO=2,MouseIO=4,ProcessIO=8,ComputerIO=16};
   /**
    * Run time idendification. Predefined values are from IOMode
    * You MUST overwrite this in derived classes!
    *
    * @return rtti value
    */
   virtual int rtti() const = 0;  // Computer, network, local, ...
   /**
    * This function returns the player who owns this IO
    *
    * @return the player this IO device is plugged into
    */
   KPlayer *player() const {return mPlayer;}
   /**
    * Sets the player to which this IO belongs to. This
    * is done automatically when adding a device to a 
    * player
    *
    * @param p the player
    */
   void setPlayer(KPlayer *p) {mPlayer=p;}

   /** 
    * Init this device by setting the player and e.g. sending an
    * init message to the device. This initialisation message is
    * very useful for computer players as you can transmit the
    * game status to them and only update this status in the setTurn
    * commands.
    */
   virtual void initIO(KPlayer *p);

   /**
    * Message to clients. Needs to be overwritten to be
    * of use.
    *
    */
    virtual void sendSystemMessage(QDataStream &stream, int msgid, int receiver, int sender);

    /**
    * Message to clients. Needs to be overwritten to be
    * of use.
    *
    * You probably want to use this for your game messages instead of @ref
    * sendSystemMessage
    **/
    virtual void sendMessage(QDataStream &stream, int msgid, int receiver, int sender);
    
    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer
     *
     *  @param turn is true/false
     */
    virtual void notifyTurn(bool b);

    

private:  
   KPlayer *mPlayer;
};

/**
 *  The KGameKeyIO class. It is used to process keyboard input
 *  from a widget and create moves for the player it belongs to.
 */
class KGameKeyIO : public KGameIO
{
  Q_OBJECT
    
public:
    /**
     * Create a keyboard input devices. All keyboards
     * inputs of the given widgets are passed through a signal
     * handler @ref #signalKeyEvent and can be used to generate
     * a valid move for the player.
     *
     * @param parent The parents widget whose keyboard events * should be grabbed
     */
    KGameKeyIO(QWidget *parent);
    /**
     * The idendification of the IO
     *
     * @return KeyIO
     */
    int rtti() const;

signals:
      /**
       * Signal handler for keyboard events. This function is called
       * on every keyboard event. If appropriate it can generate a
       * move for the player the device belongs to. If this is done
       * and the event is eaten eatevent needs to be set to true.
       * What move you generate (i.e. what you write to the stream)
       * is totally up to you as it will not be evaluated but forwared
       * to the player's/game's  input move function
       * Example:
       * <pre>
       * KPlayer *player=input->player(); // Get the player
       * Q_INT32 key=e->key();
       * stream << key;
       * eatevent=true;
       * </pre>
       *
       * @param io the IO device we belong to
       * @param stream the stream where we write our move into
       * @param m The QKeyEvent we can evaluate
       * @param eatevent set this to true if we processed the event
       */
      void signalKeyEvent(KGameIO *,QDataStream &stream,QKeyEvent *m,bool &eatevent);

protected:
       bool eventFilter( QObject *o, QEvent *e );
};

/**
 *  The KMouseKeyIO class. It is used to process mouse input
 *  from a widget and create moves for the player it belongs to.
 */
class KGameMouseIO : public KGameIO
{
  Q_OBJECT
    
public:
    /**
     * Creates a mouse IO device. It captures all mouse
     * event of the given widget and forwards them to the
     * signal handler @ref #signalMouseEvent.
     *
     * @param parent the parent widget whose events shoudl be captured
     * @param trackmouse enables mouse tracking (gives mouse move events)
     */
    KGameMouseIO(QWidget *parent,bool trackmouse=false);
    /**
     * Manually activate or deactivate mouse tracking
     *
     * @param b true = tracking on
     */
    void setMouseTracking(bool b);
    /**
     * The idendification of the IO
     *
     * @return MouseIO
     */
    int rtti() const; 

signals:
      /**
       * Signal handler for mouse events. This function is called
       * on every mouse event. If appropriate it can generate a
       * move for the player the device belongs to. If this is done
       * and the event is eaten eatevent needs to be set to true.
       * See alsp the analogous @ref #signalKeyEvent
       * Example:
       * <pre>
       * KPlayer *player=input->player(); // Get the player
       * Q_INT32 button=e->button();
       * stream << button;
       * eatevent=true;
       * </pre>
       *
       * @param io the IO device we belong to
       * @param stream the stream where we write our move into
       * @param m The QMouseEvent we can evaluate
       * @param eatevent set this to true if we processed the event
       */
      void signalMouseEvent(KGameIO *,QDataStream &stream,QMouseEvent *m,bool &eatevent);

protected:
      bool eventFilter( QObject *o, QEvent *e );

};


class KGameProcessIOPrivate;
/**
 *  The KProcessIO class. It is used to create a computer player
 *  via a separate process and communicate transparetly with it.
 *  Its counterpart is the @ref KGameProcess class which needs
 *  to be used by the computer player. See its documentation
 *  for the definition of the computer player.
 */
class KGameProcessIO : public KGameIO
{
  Q_OBJECT
    
public:
    /** 
     * Creates a computer player via a separate process. The process
     * name is given as fully qualified filename. 
     *
     * @param name the filename of the process to start
     */
    KGameProcessIO(const QString& name);

    /**
     * Deletes the process input devices 
     */
    ~KGameProcessIO();

    /**
     * The idendification of the IO
     *
     * @return ProcessIO
     */
    int rtti() const;
    
    /**
     * Send a message to the process. This is analogous to the sendMessage
     * commands of KGame. It will result in a signal of the computer player
     * on which you can react in the process player.
     *
     * @param stream  - the actual data
     * @param msgid - the id of the message
     * @param receiver - not used
     * @param sender - who send the message
     */
    void sendMessage(QDataStream &stream,int msgid, int receiver, int sender);

    /**
     * Send a system message to the process. This is analogous to the sendMessage
     * commands of KGame. It will result in a signal of the computer player
     * on which you can react in the process player.
     *
     * @param stream  - the actual data
     * @param msgid - the id of the message
     * @param receiver - not used
     * @param sender - who send the message
     */
    void sendSystemMessage(QDataStream &stream, int msgid, int receiver, int sender);

    /** 
     * Init this device by setting the player and e.g. sending an
     * init message to the device. Calling this function will emit
     * the IOAdded signal on which you can react. 
     * This function is called automatically when adding the IO to
     * a player.
     */
    void initIO(KPlayer *p);

    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer. You can react on the signalPrepareTurn to
     *  prepare a message for the process
     *
     *  @param turn is true/false
     */
    void notifyTurn(bool b);

  protected:
    /**
     * Combined function for all message handling */
    void sendAllMessages(QDataStream &stream,int msgid, int receiver, int sender, bool usermsg);
  protected slots:
    void receivedMessage(const QByteArray& receiveBuffer);

  
signals:
  /**
   * A computer query message is received. This is a 'dummy'
   * message sent by the process if it needs to communicate
   * with us. It is not forwarded over the network.
   * Racting to this message allows you to 'answer' questions
   * of the process, e.g. sending addition data which the process
   * needs to calculate a move.
   */
  void signalProcessQuery(QDataStream &stream,KGameProcessIO *me);

  /**
  * Signal generated when the computer player is about to perform a turn. 
  * The datastream has to be filled with user data for the computer player
  * to make a proper move. What you write into the stream is completely
  * up to you. The stream will be received by the corresponding process
  * which emits a signal on which you can react to retrieve the data from
  * the stream.
  *
  * @param the KGameIO object itself
  * @param the stream into which themove will be written
  * @param the argument of setTurn
  * @param set this to false if no move should be generated
  */
  void signalPrepareTurn(QDataStream &,bool,KGameIO *,bool &);

  /**
  * Signal generated when the computer player is added. 
  * You can use this to communicated with the process and
  * e.g. send initialisation information to the process.
  *
  * @param the KGameIO object itself
  * @param the stream into which themove will be written
  * @param the player itself
  * @param set this to false if no move should be generated
  */
  void signalIOAdded(KGameIO *,QDataStream &,KPlayer *p,bool &);


protected:

private:

  KGameProcessIOPrivate* d;
};

/**
 *  The KComputerIO class. It is used to create a LOCAL computer player
 *  and communicate transparetly with it. 
 *  Question: Is this needed or is it overwritten anyway for a real game?
 */
class KGameComputerIO : public KGameIO
{
  Q_OBJECT
    
public:
    /** 
     * Creates a LOCAL computer player 
     *
     */
    KGameComputerIO();

    ~KGameComputerIO();
    int rtti() const;
    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer
     *
     *  @param turn is true/false
     */
    void notifyTurn(bool b);

public slots:
  
signals:
      /**
      * Signal generated when the computer player is activated. 
      * The datastream has to be filled with a move
      *
      * @param the KGameIO object itself
      * @param the stream into which themove will be written
      * @param the argument of setTurn
      * @param set this to false if no move should be generated
      */
      void signalPrepareTurn(QDataStream &,bool,KGameIO *,bool &);

protected:

private:
};


#endif
