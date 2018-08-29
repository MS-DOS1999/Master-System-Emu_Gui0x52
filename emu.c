#include "emu.h"

int main()
{
  const double VdpUpdateInterval = 1000/60;
  double lastFrameTime = 0;
  while(1) //emu loop
  {
    double currentTime = //API getCurrentime();

    //on draw une frame toutes les 1/60 secondes
    if((lastFrameTime + VdpUpdateInterval) <= currentTime)
    {
      lastFrameTime = currentTime;
      EMU_Update();
    }
  }

  return 0;

}

void EMU_Update()
{
  //SMS (Main) clock 10.738580 Mhz
  //Z80 clock 3.58 Mhz, 3 fois plus lent que la SMS
  //VDP clock 5.36 Mhz, 2 fois plus lent que la SMS
  //Sound clock 3.58 Mhz, pareil que le Z80

  const double SmsClicksPerFrame = 10738580 / 60;
  unsigned int clicksInUpdate = 0;

  //on emule donc le nombre de clicks dans SmsClicksPerFrame par frame
  while(clicksInUpdate < SmsClicksPerFrame)
  {
    int z80Clicks = Z80_ExecuteInstruction();

    Z80_UpdateInterrupts();

    int smsClicks = z80ClockCycles * 3;

    float vdpClicks = smsClicks / 2;

    int soundClicks = z80Clicks;

    VDP_Update();
    Sound_Update();

    clicksInUpdate += smsClicks;
  }

}

void EMU_LoadRom(const char* romName)
{
  memset(smsMemory, 0, sizeof(smsMemory));
  memset(gameMemory, 0, sizeof(gameMemory));

  FILE *gameRom = NULL;

  gameRom = fopen(romName, "rb");

  //on recup la taille
  fseek(gameRom, 0L, SEEK_END);
  long endPos = ftell(gameRom);
  fclose(gameRom);

  gameRom = fopen(gameRom, "rb");

  //on check si il y a un header
  endPos = endPos % 0x4000;

  if(endPos == 512) //si oui on le skip
  {
    char header[1000];
    fread(header, 1, 512, gameRom);
  }

  if(endPos == 64)
  {
    char header[1000];
    fread(header, 1, 64, gameRom);
  }

  fread(gameMemory, 0, sizeof(gameMemory, 1, 0x100000, gameRom));
  oneMegaCartridge = (endPos > 0x80000) ? true : false;

  //on copy les trois première page de jeu dans la memoire de la sms
  memcpy(&smsMemory[0x0], &gameMemory[0x0], 0xC000);

  isCodeMaster = EMU_IsCodeMaster();
}

int EMU_IsCodeMaster()
{
  word checksum = smsMemory[0x7fe7] << 8;
  checksum |= smsMemory[0x7fe6];

  if(checksum == 0x0)
  {
    return 0;
  }

  word compute = 0x10000 - checksum;

  word answer = smsMemory[0x7fe9] << 8;
  answer |= smsMemory[0x7fe8];

  if(compute == answer)
  {
    return 1;
  }

  return 0;
}

void EMU_WriteMem(word address, byte data)
{
  //codemaster mem paging
  if(isCodeMaster)
  {
    if(address == 0x0 || address == 0x4000 || address == 0x8000)
    {
      EMU_SetPagingCodeMaster(address, byte);
    }
  }

  //ici c'est la Rom, donc Read only Mem, on écrit pas dessus
  if(address < 0x8000)
  {
    return;
  }
  else if(address < 0xC000)
  {
    byte memCtrlData = smsMemory[0xFFFC];
    if(ramBankNumber > -1)
    {
      ramBank[ramBankNumber][address-0x8000] = data;
    }
    else //si la Ram n'est pas activé, alors on est sur du read only
    {
      return;
    }
  }

  //après ça on est ok pour écrire dans la mem
  smsMemory[address] = data;

  //standard mem paging
  if(address >= 0xFFFC)
  {
    if(!isCodeMaster)
    {
      EMU_SetPaging(address, data);
    }
  }

  //on gère le mirroring
  if(address >= 0xC000 && address < 0xDFFC)
  {
    smsMemory[address+0x2000] = data;
  }

  if(address >= 0xE000)
  {
    smsMemory[address-0x2000] = data;
  }
}

void EMU_SetPaging(word address, byte data)
{
  byte page = oneMegaCartridge ? data & 0x3F : data & 0x1F;

  if(address == 0xFFFC)
  {
    //slot 2, Ram ou Rom ?
    if(BIT_ByteCheck(data, 3)) //RAM
    {
      //on swap quel bank ?
      BIT_ByteCheck(data, 2) ? ramBankNumber = 1 : ramBankNumber = 0;
    }
    else //ROM
    {
      ramBankNumber = -1;
    }
  }
  else if(address == 0xFFFD)
  {
    slot0Page = page;
  }
  else if(address == 0xFFFE)
  {
    slot1Page = page;
  }
  else if(address == 0xFFFF)
  {
    //on peut rom banking que si la ram n'est pas mappé ici
    if(!BIT_ByteCheck(smsMemory[0xFFFC], 3))
    {
      slot2Page = page;
    }
  }
}

void EMU_SetPagingCodeMaster(word address, byte data)
{
  byte page = oneMegaCartridge ? data & 0x3F : data & 0x1F;

  switch(address)
  {
    case 0x0:
      slot0Page = page;
      break;

    case 0x4000:
      slot1Page = page;
      break;

    case 0x8000:
      slot2Page = page;
      break;
  }
}

byte EMU_ReadMem(word address)
{
  //on ne lit pas les registres, du coup on lit depuis la mirrored 0xDFFC-0xDFFF
  if(address >= 0xFFFC)
  {
    address -= 0x2000;
  }
  //fixed mem dans le slot 0
  else if(!isCodeMaster && (address < 0x400))
  {
    return smsMemory[address];
  }
  //slot 0
  else if(address < 0x4000)
  {
    //on converti l'address physique en address de la page
    unsigned int pageAddr = address + (0x4000 * slot0Page);
    return gameMemory[pageAddr];
  }
  //slot 1
  else if(address < 0x8000)
  {
    unsigned int pageAddr = address + (0x4000 * slot1Page);
    //on remove l'offset
    pageAddr -= 0x4000;
    return gameMemory[pageAddr];
  }
  //slot2
  else if(address < 0xC000)
  {
    //ram ou rom ?
    if(ramBankNumber > -1)
    {
      return ramBank[ramBankNumber][address - 0x8000];
    }
    else
    {
      unsigned int pageAddr = address + (0x4000 * slot2Page);
      pageAddr -= 0x8000;
      return gameMemory[pageAddr];
    }
  }

  smsMemory[address];
}
