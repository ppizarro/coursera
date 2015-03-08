/*
 * Simple_Server.cpp
 *
 *  Created on: May 6, 2013
 *      Author: hlima
 */

#include "Tipos.h"
#include "Objeto.h"
#include "Tcp.h"
#include "string.h"


int AppMain(int argc, char * argv[])
{ int Porta = 50000;
  CSocket_ Soquete1;

  if(!Soquete1.OpenSocket())     // cria novo socket
  { write(1,"Falha na criacao do servidor TCP socket()",0);
    return 0;
  }
  if(!Soquete1.Bind(Porta))      // associa o socket a uma porta
  { write(1,"Falha na criacao do servidor TCP bind(%d)",Porta);
    return 0;
  }
  if(!Soquete1.Listen())         // habilita leitura na porta
  { write(1,"Falha na criacao do servidor TCP listen(%d)",Porta);
    return 0;
  }

  while(1)
  { char    Ip[80];
    SOCKET  ClientSocket = Soquete1.Accept(Ip);          // aguarda até chegar nova conexão

    write(1,Ip,strlen(Ip));
    CTcp Soquete2(ClientSocket);

    char Buffer[100];
    int Size = Soquete2.Get(Buffer,sizeof(Buffer));
    if(Size > 0)
    { write(1,Buffer,Size);
      Soquete2.Write(Buffer,Size);
    }
    Soquete2.CloseSocket();
  }
  Soquete1.CloseSocket();

  return 0;
}

