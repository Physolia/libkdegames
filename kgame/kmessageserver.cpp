/* **************************************************************************
                           KMessageServer Class
                           --------------------------
    begin                : 31 March 2001
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
#include <qiodevice.h>
#include <qbuffer.h>
#include <qlist.h>
#include <qqueue.h>
#include <qtimer.h>
#include <qvaluelist.h>

#include <kdebug.h>

#include "kmessageio.h"
#include "kmessageserver.h"

// --------------- internal class KMessageServerSocket

KMessageServerSocket::KMessageServerSocket (Q_UINT16 port, QObject *parent)
  : QServerSocket (port, 0, parent)
{
}

KMessageServerSocket::~KMessageServerSocket ()
{
}

void KMessageServerSocket::newConnection (int socket)
{
  emit newClientConnected (new KMessageSocket (socket));
}

// ---------------- class for storing an incoming message

class MessageBuffer
{
  public:
    MessageBuffer (Q_UINT32 clientID, const QByteArray &messageData)
      : id (clientID), data (messageData)
    {}
    ~MessageBuffer () {}
    Q_UINT32 id;
    QByteArray data;
};

// ---------------- KMessageServer's private class

class KMessageServerPrivate
{
public:
  KMessageServerPrivate()
    : mMaxClients (-1), mGameId (1), mUniqueClientNumber (1), mAdminID (0), mServerSocket (0)
  {
    mClientList.setAutoDelete (true);
    mClientList.setAutoDelete (true);
  }

  int mMaxClients;
  int mGameId;
  Q_UINT16 mCookie;
  Q_UINT32 mUniqueClientNumber;
  Q_UINT32 mAdminID;

  KMessageServerSocket* mServerSocket;

  QList <KMessageIO> mClientList;
  QQueue <MessageBuffer> mMessageQueue;
  QTimer mTimer;
};


// ------------------ KMessageServer

KMessageServer::KMessageServer (Q_UINT16 cookie,QObject* parent)
  : QObject(parent, 0)
{
  d = new KMessageServerPrivate;
  d->mCookie=cookie;
  connect (&(d->mTimer), SIGNAL (timeout()),
           this, SLOT (processOneMessage()));
  kdDebug(11001) << "CREATE(KMessageServer="
		<< this
		<< ") cookie="
		<< d->mCookie
		<< " sizeof(this)="
		<< sizeof(KMessageServer)
		<< endl;
}

KMessageServer::~KMessageServer()
{
  kdDebug(11001) << "DESTRUCT(KMessageServer=" << this <<")" << endl;
  Debug();
  stopNetwork();
  deleteClients();
  delete d;
  kdDebug(11001) << "DESTRUCT(KMessageServer=" << this <<") done" << endl;
}

//------------------------------------- TCP/IP server stuff

bool KMessageServer::initNetwork (Q_UINT16 port)
{
  kdDebug(11001) << "KMessageServer::initNetwork" << endl;

  if (d->mServerSocket)
  {
    kdDebug (11001) << "KMessageServer::initNetwork: We were already offering connections!" << endl;
    delete d->mServerSocket;
  }

  d->mServerSocket = new KMessageServerSocket (port);

  if (!d->mServerSocket || !d->mServerSocket->ok())
  {
    kdError(11001) << "KMessageServer::initNetwork: Serversocket::ok() == false" << endl;
    delete d->mServerSocket;
    d->mServerSocket=0;
    return false;
  }

  kdDebug (11001) << "KMessageServer::initNetwork: Now listening to port "
                  << d->mServerSocket->port() << endl;
  connect (d->mServerSocket, SIGNAL (newClientConnected (KMessageIO*)),
           this, SLOT (addClient (KMessageIO*)));
  return true;
}

Q_UINT16 KMessageServer::serverPort () const
{
  if (d->mServerSocket)
    return d->mServerSocket->port();
  else
    return 0;
}

void KMessageServer::stopNetwork()
{
  if (d->mServerSocket) 
  {
    delete d->mServerSocket;
    d->mServerSocket = 0;
  }
}

bool KMessageServer::isOfferingConnections() const
{
  return d->mServerSocket != 0;
}

//----------------------------------------------- adding / removing clients

void KMessageServer::addClient (KMessageIO* client)
{
  QByteArray msg;

  // maximum number of clients reached?
  if (d->mMaxClients >= 0 && d->mMaxClients <= clientCount())
  {
    kdError (11001) << "KMessageServer::addClient: Maximum number of clients reached!" << endl;
    return;
  }

  // give it a unique ID
  client->setId (uniqueClientNumber());

  // connect its signals
  connect (client, SIGNAL (connectionBroken()),
           this, SLOT (removeBrokenClient()));
  connect (client, SIGNAL (received (const QByteArray &)),
           this, SLOT (getReceivedMessage (const QByteArray &)));

  // Tell everyone about the new guest
  // Note: The new client doesn't get this message!
  QDataStream (msg, IO_WriteOnly) << Q_UINT32 (EVNT_CLIENT_CONNECTED) << client->id();
  broadcastMessage (msg);

  // add to our list
  d->mClientList.append (client);

  // tell it its ID
  QDataStream stream (msg, IO_WriteOnly);
  stream << Q_UINT32 (ANS_CLIENT_ID) << client->id();
  client->send (msg);

  if (clientCount() == 1)
    // if it is the first client, it becomes the admin
    setAdmin (client->id());
  else
  {
    // otherwise tell it who is the admin
    QDataStream (msg, IO_WriteOnly) << Q_UINT32 (ANS_ADMIN_ID) << adminID();
    sendMessage (client->id(), msg);
  }

  emit clientConnected (client);
}

void KMessageServer::removeClient (KMessageIO* client, bool purpose)
{
  Q_UINT32 clientID = client->id();
  if (!d->mClientList.removeRef (client))
  {
    kdError(11001) << "KMessageServer::removeClient: Deleting client that wasn't added before!" << endl;
    return;
  }

  // tell everyone about the removed client
  QByteArray msg;
  QDataStream (msg, IO_WriteOnly) << Q_UINT32 (EVNT_CLIENT_DISCONNECTED) << client->id() << (Q_INT8)purpose;
  broadcastMessage (msg);

  // If it was the admin, select a new admin.
  if (clientID == adminID())
  {
    if (!d->mClientList.isEmpty())
      setAdmin (d->mClientList.first()->id());
    else
      setAdmin (0);
  }
}

void KMessageServer::deleteClients()
{
  d->mClientList.clear();
  d->mAdminID = 0;
}

void KMessageServer::removeBrokenClient ()
{
  if (!sender()->inherits ("KMessageIO"))
  {
    kdError (11001) << "KMessageServer::removeBrokenClient: sender of the signal was not a KMessageIO object!" << endl;
    return;
  }

  KMessageIO *client = (KMessageIO *) sender();

  emit connectionLost (client);
  removeClient (client, false);
}


void KMessageServer::setMaxClients(int c)
{
  d->mMaxClients = c;
}

int KMessageServer::maxClients() const
{
  return d->mMaxClients;
}

int KMessageServer::clientCount() const
{
  return d->mClientList.count();
}

QValueList <Q_UINT32> KMessageServer::clientIDs () const
{
  QValueList <Q_UINT32> list;
  QListIterator <KMessageIO> iter (d->mClientList);
  while (*iter)
    list.append ((*iter)->id());
  return list;
}

KMessageIO* KMessageServer::findClient (Q_UINT32 no) const
{
  if (no == 0)
    no = d->mAdminID;

  QListIterator <KMessageIO> iter (d->mClientList);
  while (*iter)
  {
    if ((*iter)->id() == no)
      return (*iter);
    ++iter;
  }
  return 0;
}

Q_UINT32 KMessageServer::adminID () const
{
  return d->mAdminID;
}

void KMessageServer::setAdmin (Q_UINT32 adminID)
{
  // Trying to set the the client that is already admin => nothing to do
  if (adminID == d->mAdminID)
    return;

  if (adminID > 0 && findClient (adminID) == 0)
  {
    kdWarning (11001) << "Trying to set a new admin that doesn't exist!" << endl;
    return;
  }

  QByteArray msg;
  QDataStream (msg, IO_WriteOnly) << Q_UINT32 (ANS_ADMIN_ID) << adminID;

  // Tell everyone about the new master
  broadcastMessage (msg);
}


//------------------------------------------- ID stuff

Q_UINT32 KMessageServer::uniqueClientNumber() const
{
  return d->mUniqueClientNumber++;
}

// --------------------- Messages ---------------------------

void KMessageServer::broadcastMessage (const QByteArray &msg)
{
  for (QListIterator <KMessageIO> iter (d->mClientList); *iter; ++iter)
    (*iter)->send (msg);
}

void KMessageServer::sendMessage (Q_UINT32 id, const QByteArray &msg)
{
  KMessageIO *client = findClient (id);
  if (client)
    client->send (msg);
}

void KMessageServer::sendMessage (const QValueList <Q_UINT32> &ids, const QByteArray &msg)
{
  for (QValueListConstIterator <Q_UINT32> iter = ids.begin(); iter != ids.end(); ++iter)
    sendMessage (*iter, msg);
}

void KMessageServer::getReceivedMessage (const QByteArray &msg)
{
  if (!sender() || !sender()->inherits("KMessageIO"))
  {
    kdError (11001) << "KMessageServer::processReceivedMessage: slot was not called from KMessageIO!" << endl;
    return;
  }
  KMessageIO *client = (KMessageIO *) sender();
  Q_UINT32 clientID = client->id();
  d->mMessageQueue.enqueue (new MessageBuffer (clientID, msg));
  if (!d->mTimer.isActive())
    d->mTimer.start(0); // AB: should be , TRUE i guess
}

void KMessageServer::processOneMessage ()
{
  // This shouldn't happen, since the timer should be stopped before. But only to be sure!
  if (d->mMessageQueue.isEmpty())
  {
    d->mTimer.stop();
    return;
  }

  MessageBuffer *msg_buf = d->mMessageQueue.head();

  Q_UINT32 clientID = msg_buf->id;
  QBuffer in_buffer (msg_buf->data);
  in_buffer.open (IO_ReadOnly);
  QDataStream in_stream (&in_buffer);

  QByteArray out_msg;
  QBuffer out_buffer (out_msg);
  out_buffer.open (IO_WriteOnly);
  QDataStream out_stream (&out_buffer);

  bool unknown = false;

  Q_UINT32 messageID;
  in_stream >> messageID;
  switch (messageID)
  {
    case REQ_BROADCAST:
      out_stream << Q_UINT32 (MSG_BROADCAST) << clientID;
      // FIXME, compiler bug?
      // this should be okay, since QBuffer is subclass of QIODevice! :
      // out_buffer.writeBlock (in_buffer.readAll());
      out_buffer.QIODevice::writeBlock (in_buffer.readAll());
      broadcastMessage (out_msg);
      break;

    case REQ_FORWARD:
      {
        QValueList <Q_UINT32> clients;
        in_stream >> clients;
        out_stream << Q_UINT32 (MSG_FORWARD) << clientID << clients;
        // see above!
        out_buffer.QIODevice::writeBlock (in_buffer.readAll());
        sendMessage (clients, out_msg);
      }
      break;

    case REQ_CLIENT_ID:
      out_stream << Q_UINT32 (ANS_CLIENT_ID) << clientID;
      sendMessage (clientID, out_msg);
      break;

    case REQ_ADMIN_ID:
      out_stream << Q_UINT32 (ANS_ADMIN_ID) << d->mAdminID;
      sendMessage (clientID, out_msg);
      break;

    case REQ_ADMIN_CHANGE:
      if (clientID == d->mAdminID)
      {
        Q_UINT32 newAdmin;
        in_stream >> newAdmin;
        setAdmin (newAdmin);
      }
      break;

    case REQ_REMOVE_CLIENT:
      if (clientID == d->mAdminID)
      {
        QValueList <Q_UINT32> client_list;
        in_stream >> client_list;
        for (QValueListIterator <Q_UINT32> iter = client_list.begin(); iter != client_list.end(); ++iter)
        {
          KMessageIO *client = findClient (*iter);
          if (client)
            removeClient (client);
          else
            kdWarning (11001) << "KMessageServer::processReceivedMessage: removing non-existing clientID" << endl;
        }
      }
      break;

    case REQ_MAX_NUM_CLIENTS:
      if (clientID == d->mAdminID)
      {
        Q_INT32 maximum_clients;
        in_stream >> maximum_clients;
        setMaxClients (maximum_clients);
      }
      break;

    case REQ_CLIENT_LIST:
      {
        QValueList <Q_UINT32> client_list;
        for (QListIterator <KMessageIO> iter (d->mClientList); *iter; ++iter)
          client_list.append ((*iter)->id());
        out_stream << Q_UINT32 (ANS_CLIENT_LIST) << client_list;
        sendMessage (clientID, out_msg);
      }
      break;

    default:
      unknown = true;
  }

  // check if all the data has been used
  if (!unknown && !in_buffer.atEnd())
    kdWarning (11001) << "KMessageServer::processReceivedMessage: Extra data received for message ID " << messageID << endl;

  emit messageReceived (msg_buf->data, clientID, unknown);

  if (unknown)
    kdWarning (11001) << "KMessageServer::processReceivedMessage: received unknown message ID " << messageID << endl;

  // remove the message, since we are ready with it
  d->mMessageQueue.remove();
  if (d->mMessageQueue.isEmpty())
    d->mTimer.stop();
}

void KMessageServer::Debug()
{
   kdDebug(11001) << "------------------- KNETWORKGAME -------------------------" << endl;
   kdDebug(11001) << "MaxClients :   " << maxClients() << endl;
   kdDebug(11001) << "NoOfClients :  " << clientCount() << endl;
   kdDebug(11001) << "---------------------------------------------------" << endl;
}
#include "kmessageserver.moc"
