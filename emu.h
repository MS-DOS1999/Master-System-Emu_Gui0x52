#ifndef EMU_H
#define EMU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SFML/Config.h>
#include <SFML/System.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>

#include "oldSize.h"
#include "bitUtils.h"
#include "z80.h"
#include "tms.h"

int DEBUG;

int UNOP;

/*Region	    Maps to
  $0000-$03ff	ROM (unpaged)
  $0400-$3fff	ROM mapper slot 0
  $4000-$7fff	ROM mapper slot 1
  $8000-$bfff	ROM/RAM mapper slot 2
  $c000-$dfff	System RAM
  $e000-$ffff	System RAM (mirror)
  $fff8	      3D glasses control
  $fff9-$fffb	3D glasses control (mirrors)
  $fffc	      Cartridge RAM mapper control
  $fffd	      Mapper slot 0 control
  $fffe	      Mapper slot 1 control
  $ffff	      Mapper slot 2 control
*/
byte smsMemory[0x10000];
byte gameMemory[0x100000]; //un jeu SMS fait max 1 mega byte
byte ramBank[0x2][0x4000];
int isCodeMaster; //ce fabricant à une particularité à lui tout seul, on doit donc savoir si c'ets lui qui est chargé
int oneMegaCartridge;
int ramBankNumber;

byte slot0Page;
byte slot1Page;
byte slot2Page;

unsigned int FPS;

sfRenderWindow* window;

void EMU_Init();
void EMU_Update();
void EMU_Render(sfImage* screenImg, sfTexture* screenTex, sfSprite* screenSpr);
void EMU_LoadRom(const char* romName);
int EMU_IsCodeMaster();
void EMU_WriteMem(word address, byte data);
void EMU_SetPaging(word address, byte data);
void EMU_SetPagingCodeMaster(word address, byte data);
byte EMU_ReadMem(word address);
byte EMU_ReadIO(byte address);
void EMU_WriteIO(byte address, byte data);

#endif
