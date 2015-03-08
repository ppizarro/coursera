/*
 * Simple_Client.cpp
 *
 *  Created on: May 6, 2013
 *      Author: hlima
 */
#include "Tipos.h"
#include "Objeto.h"
#include "Tcp.h"
#include "string.h"


int AppMain(int argc, char * argv[])
{
  //CTcp Cliente((char*)"192.168.160.160",50000);
  CTcp Cliente((char*)"127.0.0.1",50000);

  Cliente.Conecta();
  char Buffer[100];
  while(1) {
    snprintf(Buffer, "Agora vai...\n", 100);
    Cliente.Write(Buffer,strlen(Buffer));
    int Lidos = Cliente.Get(Buffer,sizeof(Buffer));
    write(1,Buffer,Lidos);
  }
  Cliente.Close();

  return 0;
}

