#include "emu.h"

int DEBUG;
int UNOP;

byte smsMemory[0x10000];
byte gameMemory[0x100000]; //un jeu SMS fait max 1 mega byte
byte ramBank[0x2][0x4000];
int isCodeMaster; //ce fabricant à une particularité à lui tout seul, on doit donc savoir si c'ets lui qui est chargé
int oneMegaCartridge;
int ramBankNumber;

unsigned long int cyclesThisUpdate;

byte slot0Page;
byte slot1Page;
byte slot2Page;

byte joypadPortOne;
byte joypadPortTwo;

unsigned int FPS;

sfRenderWindow* window;
sfEvent event;
SN soundChip;

int main(int argc, char *argv[])
{
  DEBUG = 0;
  char deb[6];
  strcpy(deb, "debug");

  char unop[6];
  strcpy(unop, "unop");

  if(argc == 2)
  {
    if(strcmp(argv[1], deb) == 0)
    {
      DEBUG = 1;
      freopen("output.txt", "w", stdout);
    }
    else if(strcmp(argv[1], unop) == 0)
    {
      UNOP = 1;
    }
  }

  SDL_Init(SDL_INIT_AUDIO);

  sfVideoMode mode = {256, 240, 32};

  window = sfRenderWindow_create(mode, "Gui0x52 - Master System Emulator", sfClose, NULL);

  sfClock* clock;

  clock = sfClock_create();

  sfImage* screenImg;
  sfTexture* screenTex;
  sfSprite* screenSpr;

  screenImg = sfImage_createFromColor(256, 240, sfBlack);
  screenTex = sfTexture_createFromImage(screenImg, NULL);
  screenSpr = sfSprite_create();
  sfSprite_setTexture(screenSpr, screenTex, sfTrue);
  sfSprite_setPosition(screenSpr, (sfVector2f){ 0.0f, 0.0f });

  EMU_Init();
  EMU_LoadRom("zexdoc.sms");

  const double VdpUpdateInterval = 1000/FPS;

  double lastFrameTime = 0;

  while(sfRenderWindow_isOpen(window)) //emu loop
  {

    while (sfRenderWindow_pollEvent(window, &event))
    {
      /* Close window : exit */
      if (event.type == sfEvtClosed)
      {
          sfRenderWindow_close(window);
      }

      EMU_HandleInput();
    }

    double currentTime = sfTime_asMilliseconds(sfClock_getElapsedTime(clock));

    //on draw une frame toutes les 1/60 secondes
    if((lastFrameTime + VdpUpdateInterval) <= currentTime)
    {
      lastFrameTime = currentTime;
      
      EMU_Update();

      EMU_Render(screenImg, screenTex, screenSpr);
    }
  }

  return 0;
}

void EMU_Init()
{
  memset(&smsMemory, 0, sizeof(smsMemory));
  memset(&gameMemory, 0, sizeof(gameMemory));
  memset(&ramBank, 0, sizeof(ramBank));
  memset(&DAATable, 0, sizeof(DAATable));
  memset(&ZSPTable, 0, sizeof(ZSPTable));

  registerAF.reg = 0x0000;
  registerBC.reg = 0x0000;
  registerDE.reg = 0x0000;
  registerHL.reg = 0x0000;
  registerAFShadow.reg = 0x0000;
  registerBCShadow.reg = 0x0000;
  registerDEShadow.reg = 0x0000;
  registerHLShadow.reg = 0x0000;

  registerIX.reg = 0x0000;
  registerIY.reg = 0x0000;

  registerI = 0;
  registerR = 0;

  programCounter = 0;
  stackPointer = 0xDFF0;

  smsMemory[0xFFFF] = 2;
  smsMemory[0xFFFE] = 1;

  IFF1 = 0;
  IFF2 = 0;
  Halted = 0;
  ResetInt = 0;
  ExecuteReset = 0;
  IntMode = 1;
  EIPending = 0;

  isCodeMaster = 0;
  oneMegaCartridge = 0;
  ramBankNumber = -1;

  slot0Page = 0;
  slot1Page = 1;
  slot2Page = 2;

  joypadPortOne = 0xFF;
  joypadPortTwo = 0xFF;

  tmsIsPal = 0;

  //FPS = tmsIsPal ? 50 : 60;
  FPS = 1000000;
  InitDAATable();
  TMS_Init();
  soundChip.SN_Reset();

}

void EMU_Update()
{
  //SMS (Main) clock 10.738580 Mhz
  //Z80 clock 3.58 Mhz, 3 fois plus lent que la SMS
  //VDP clock 5.36 Mhz, 2 fois plus lent que la SMS
  //Sound clock 3.58 Mhz, pareil que le Z80

  TMS_ResetScreen();
  cyclesThisUpdate = 0;
  //on emule donc le nombre de clicks dans SmsClicksPerFrame par frame
  while(!TMS_GetRefresh())
  {
    int z80Clicks;
    if(Halted)
    {
      z80Clicks = 4;
    }
    else
    {
      z80Clicks = Z80_ExecuteInstruction();
    }

    Z80_UpdateInterrupts();

    z80Clicks *= 2;

    cyclesThisUpdate += z80Clicks;

    float vdpClicks = z80Clicks;
    vdpClicks /= 2;

    float soundClicks = z80Clicks;
    soundClicks /= 3;

    TMS_Update(vdpClicks);

    soundChip.SN_Update(soundClicks);

  }

}

void EMU_Render(sfImage* screenImg, sfTexture* screenTex, sfSprite* screenSpr)
{

  if(!ScreenDisabled)
  {
    if(tmsHeight == NUM_RES_VERT_SMALL)
    {
      for(int y = 0; y < 192; y++)
      {
        for(int x = 0; x < 256; x++)
        {
          sfColor color;
          color.r = screenSmall[x][y][0];
          color.g = screenSmall[x][y][1];
          color.b = screenSmall[x][y][2];
          color.a = 255;
          sfImage_setPixel(screenImg, x, y, color);
        }
      }
    }
    else if(tmsHeight == NUM_RES_VERT_MED)
    {
      for(int y = 0; y < 224; y++)
      {
        for(int x = 0; x < 256; x++)
        {
          sfColor color;
          color.r = screenMedium[x][y][0];
          color.g = screenMedium[x][y][1];
          color.b = screenMedium[x][y][2];
          color.a = 255;
          sfImage_setPixel(screenImg, x, y, color);
        }
      }
    }
    else if(tmsHeight == NUM_RES_VERT_HIGH)
    {
      for(int y = 0; y < 240; y++)
      {
        for(int x = 0; x < 256; x++)
        {
          sfColor color;
          color.r = screenHigh[x][y][0];
          color.g = screenHigh[x][y][1];
          color.b = screenHigh[x][y][2];
          color.a = 255;
          sfImage_setPixel(screenImg, x, y, color);
        }
      }
    }
    sfTexture_updateFromImage(screenTex, screenImg, 0, 0);
    sfSprite_setTexture(screenSpr, screenTex, sfTrue);

    sfRenderWindow_clear(window, sfBlack);
    sfRenderWindow_drawSprite(window, screenSpr, NULL);
    sfRenderWindow_display(window);
  }

  
}

void EMU_LoadRom(const char* romName)
{
  FILE *gameRom = NULL;

  gameRom = fopen(romName, "rb");

  //on recup la taille
  fseek(gameRom, 0L, SEEK_END);
  long endPos = ftell(gameRom);
  fclose(gameRom);

  gameRom = fopen(romName, "rb");

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

  fread(gameMemory, 1, 0x100000, gameRom);
  oneMegaCartridge = (endPos > 0x80000) ? 1 : 0;

  //on copy les trois première page de jeu dans la memoire de la sms
  memcpy(&smsMemory[0x0], &gameMemory[0x0], 0xC000);

  smsMemory[0xFFFE] = 0x01;
  smsMemory[0xFFFF] = 0x02;

  isCodeMaster = EMU_IsCodeMaster();

  if(isCodeMaster)
  {
    EMU_SetPagingCodeMaster(0x0, 0);
    EMU_SetPagingCodeMaster(0x4000, 1);
    EMU_SetPagingCodeMaster(0x8000, 0);
  }
}

int EMU_IsCodeMaster()
{
  word checksum = smsMemory[0x7FE7] << 8;
  checksum |= smsMemory[0x7FE6];

  if(checksum == 0x0)
  {
    return 0;
  }

  word compute = 0x10000 - checksum;

  word answer = smsMemory[0x7FE9] << 8;
  answer |= smsMemory[0x7FE8];

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
      EMU_SetPagingCodeMaster(address, data);
    }
  }

  //ici c'est la Rom, donc Read only Mem, on écrit pas dessus
  if(address < 0x8000)
  {
    return;
  }
  else if(address < 0xC000)
  {
    
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
  if(address >= 0xFFFC)
  {
    byte page = oneMegaCartridge ? data & 0x3F : data & 0x1F;

    smsMemory[address - 0x2000] = data;

    if(address == 0xFFFC)
    {
      //slot 2, Ram ou Rom ?
      if(BIT_ByteCheck(data, 3)) //RAM
      {
        //on swap quel bank ?
        ramBankNumber = BIT_ByteCheck(data, 2) ? 1 : 0;
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
}

void EMU_SetPagingCodeMaster(word address, byte data)
{
  BIT_ByteClear(&data, 7);
  BIT_ByteClear(&data, 6);
  BIT_ByteClear(&data, 5);
  byte page = data;

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

  return smsMemory[address];
}

byte EMU_ReadIO(byte address)
{
  if(address < 0x40)
  {
    return 0xFF;
  }

  if((address >= 0x40) && (address <= 0x7F))
  {
    if((address % 2) == 0)
    {
      return VCounter;
    }
    
    return 0;
  }

  if((address >= 0x80) && (address <= 0xBF))
  {
    if((address % 2) == 0)
    {
      return TMS_ReadDataPort();
    }

    return TMS_GetStatus();
  }

  switch(address)
  {
    case 0xBD: return TMS_GetStatus(); break;
    case 0xBE: return TMS_ReadDataPort(); break;
    case 0xBF: return TMS_GetStatus(); break;
    case 0x7E: return VCounter; break;
    case 0x7F:
    {
      word mod = HCounter & 511;
      mod >>= 1;
      byte res = mod & 0xFFFF;
      return res;
      break;
    }
    case 0xC0: return joypadPortOne; break; //joypadPort1
    case 0xC1: return joypadPortTwo; break; //joypadPort2
    case 0xDC: return joypadPortOne; break; //joypadPort1
    case 0xDD: return joypadPortTwo; break; //joypadPort2
    default : return 0xFF; break;
  }
}

void EMU_WriteIO(byte address, byte data)
{
  if(address < 0x40)
  {
    return;
  }

  if((address >= 0x40) && (address < 0x80))
  {
    soundChip.SN_WriteData(cyclesThisUpdate, data);
    return;
  }

  switch(address)
  {
    case 0xBD: TMS_WriteAddress(data); break;
    case 0xBE: TMS_WriteDataPort(data); break;
    case 0xBF: TMS_WriteAddress(data); break;
    default: break;
  }
}

void EMU_JoypadKeyPressed(int port, int key)
{
  if(port == 1)
  {
    BIT_ByteClear(&joypadPortOne, key);
  }
  else
  {
    BIT_ByteClear(&joypadPortTwo, key);
  }
}

void EMU_JoypadKeyReleased(int port, int key)
{
  if(port == 1)
  {
    BIT_ByteSet(&joypadPortOne, key);
  }
  else
  {
    BIT_ByteSet(&joypadPortTwo, key);
  }
}

void EMU_ResetButton()
{
  if(!ExecuteReset)
  {
    ResetInt = 1;
  }

  EMU_JoypadKeyPressed(2, 4);
}

void EMU_HandleInput()
{
  
  if(event.type == sfEvtKeyPressed)
  {
    int key = -1;
    int port = 1;
    
    //player1
    if(event.key.code == sfKeyK){
      key = 4;
    }
    else if(event.key.code == sfKeyL){
      key = 5;
    }
    else if(event.key.code == sfKeySpace){
      EMU_ResetButton();
    }
    else if(event.key.code == sfKeyD){
      key = 3;
    }
    else if(event.key.code == sfKeyQ){
      key = 2;
    }
    else if(event.key.code == sfKeyZ){
      key = 0;
    }
    else if(event.key.code == sfKeyS){
      key = 1;
    }
    //player2
    else if(event.key.code == sfKeyNumpad2)
    {
      port = 2;
      key = 2;
    }
    else if(event.key.code == sfKeyNumpad3)
    {
      port = 2;
      key = 3;
    }
    else if(event.key.code == sfKeyRight)
    {
      port = 2;
      key = 1;
    }
    else if(event.key.code == sfKeyLeft)
    {
      port = 2;
      key = 0;
    }
    else if(event.key.code == sfKeyUp)
    {
      port = 1;
      key = 6;
    }
    else if(event.key.code == sfKeyDown)
    {
      port = 1;
      key = 7;
    }
    
    if (key != -1)
    {
      EMU_JoypadKeyPressed(port, key);
    }
  }
  else if(event.type == sfEvtKeyReleased)
  {
    int key = -1;
    int port = 1;
    
    //player1
    if(event.key.code == sfKeyK){
      key = 4;
    }
    else if(event.key.code == sfKeyL){
      key = 5;
    }
    else if(event.key.code == sfKeySpace){
      port = 2;
      key = 4;
    }
    else if(event.key.code == sfKeyD){
      key = 3;
    }
    else if(event.key.code == sfKeyQ){
      key = 2;
    }
    else if(event.key.code == sfKeyZ){
      key = 0;
    }
    else if(event.key.code == sfKeyS){
      key = 1;
    }
    //player2
    else if(event.key.code == sfKeyNumpad2)
    {
      port = 2;
      key = 2;
    }
    else if(event.key.code == sfKeyNumpad3)
    {
      port = 2;
      key = 3;
    }
    else if(event.key.code == sfKeyRight)
    {
      port = 2;
      key = 1;
    }
    else if(event.key.code == sfKeyLeft)
    {
      port = 2;
      key = 0;
    }
    else if(event.key.code == sfKeyUp)
    {
      port = 1; //(although marked as player 1 it is player 2 but using overlapped ports)
      key = 6;
    }
    else if(event.key.code == sfKeyDown)
    {
      port = 1; //(although marked as player 1 it is player 2 but using overlapped ports)
      key = 7;
    }
    
    if (key != -1)
    {
      EMU_JoypadKeyReleased(port, key);
    }
  }
}
