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

#ifndef __KGAMEPROPERTYARRAY_H_
#define __KGAMEPROPERTYARRAY_H_

#include <qdatastream.h>
#include <kdebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"


template<class type>
class KGamePropertyArray : public QArray<type>, public KGamePropertyBase
{
public:
  KGamePropertyArray() :QArray<type>(), KGamePropertyBase()
  {
    //kdDebug(11001) << "KGamePropertyArray init" << endl;
  }
  KGamePropertyArray( int size )
  {
    resize(size);
  }
  KGamePropertyArray( const KGamePropertyArray<type> &a ) : QArray<type>(a)
  {
    send();
  }
  bool  resize( uint size )
  {
    if (size!=QArray<type>::size())
    {
      bool a=true;
      QByteArray b;
      QDataStream s(b, IO_WriteOnly);
      if (policy()==PolicyClean || policy()==PolicyDirty)
      {
        KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdResize);
        s << size ;
        if (mOwner)  mOwner->sendProperty(s);
      }
      if (policy()==PolicyLocal || policy()==PolicyDirty)
      {
        a=QArray<type>::resize(size);
      }
      return a;
    }
    else return true;
  }

  void setAt(uint i,type data)
  {
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdAt);
      s << i ;
      s << data;
      if (mOwner)  mOwner->sendProperty(s);
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::at(i)=data;
    }
    //kdDebug(11001) << "KGamePropertyArray setAt send COMMAND for id="<<id() << " type=" << 1 << " at(" << i<<")="<<data  << endl;
  }

  type at( uint i ) const
  {
    return QArray<type>::at(i);
  }
  type operator[]( int i ) const
  {
    return QArray<type>::at(i);
  }

  KGamePropertyArray<type> &operator=(const KGamePropertyArray<type> &a)
  {
    return assign(a);
  }

  bool  truncate( uint pos )
  {
    return resize(pos);
  }
  bool  fill( const type &data, int size = -1 )
  {
    bool r=true;
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      r=QArray<type>::fill(data,size);
    }
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdFill);
      s << data;
      s << size ;
      if (mOwner)  mOwner->sendProperty(s);
    }
    return r;
  }
  KGamePropertyArray<type>& assign( const KGamePropertyArray<type>& a )
  {
// note: send() has been replaced by sendProperty so it might be broken now!
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::assign(a);
    }
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    return *this;
  }
  KGamePropertyArray<type>& assign( const type *a, uint n )
  {
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::assign(a,n);
    }
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    return *this;
  }
  KGamePropertyArray<type>& duplicate( const KGamePropertyArray<type>& a )
  {
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::duplicate(a);
    }
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    return *this;
  }
  KGamePropertyArray<type>& duplicate( const type *a, uint n )
  {
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::duplicate(a,n);
    }
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    return *this;
  }
  KGamePropertyArray<type>& setRawData( const type *a, uint n )
  {
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::setRawData(a,n);
    }
      if (policy()==PolicyClean || policy()==PolicyDirty)
      {
        sendProperty();
    }
    return *this;
  }
  void sort()
  {
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QArray<type>::sort();
    }
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdSort);
      if (mOwner)  mOwner->sendProperty(s);
    }
  }

	void load(QDataStream& s)
	{
    kdDebug(11001) << "KGamePropertyArray load " << id() << endl;
    type data;
    for (unsigned int i=0;i<QArray<type>::size();i++) {s >> data;  QArray<type>::at(i)=data;}
		if (isEmittingSignal()) emitSignal();
  }
	void save(QDataStream &s)
	{
    kdDebug(11001) << "KGamePropertyArray save "<<id() << endl;
    for (unsigned int i=0;i<QArray<type>::size();i++) s << at(i);
	}

  void command(QDataStream &s,int cmd,bool)
  {
    KGamePropertyBase::command(s, cmd);
    kdDebug(11001) << "Array id="<<id()<<" got command ("<<cmd<<") !!!" <<endl; 
    switch(cmd)
    {
      case CmdAt:
      {
        uint i;
        type data;
        s >> i >> data;
        QArray<type>::at(i)=data;
        kdDebug(11001) << "CmdAt:id="<<id()<<" i="<<i<<" data="<<data <<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdResize:
      {
        uint size;
        s >> size;
        kdDebug(11001) << "CmdResize:id="<<id()<<" oldsize="<<QArray<type>::size()<<" newsize="<<size <<endl; 
        if (QArray<type>::size()!=size) resize(size);
        break;
      }
      case CmdFill:
      {
        int size;
        type data;
        s >> data >> size;
        kdDebug(11001) << "CmdFill:id="<<id()<<"size="<<size <<endl; 
        QArray<type>::fill(data,size);
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdSort:
      {
        kdDebug(11001) << "CmdSort:id="<<id()<<endl; 
        QArray<type>::sort();
        break;
      }
      default: 
        kdDebug(11001) << "Error in KPropertyArray::command: Unknown command " << cmd << endl;
    }
  }


};

#endif
