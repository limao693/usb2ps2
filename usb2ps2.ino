/* MAX3421E USB Host controller LCD/keyboard demonstration */

#include "Max3421e.h"
#include "Usb.h"
#include "FidPS2Host.h"

/* keyboard data taken from configuration descriptor */
#define KBD_ADDR        1
#define KBD_EP          1
#define KBD_IF          0
#define EP_MAXPKTSIZE   8
#define EP_POLL         0x0a
/**/
//******************************************************************************
//  macros to identify special charaters(other than Digits and Alphabets)
//******************************************************************************
#define BANG        (0x1E)
#define AT          (0x1F)
#define POUND       (0x20)
#define DOLLAR      (0x21)
#define PERCENT     (0x22)
#define CAP         (0x23)
#define AND         (0x24)
#define STAR        (0x25)
#define OPENBKT     (0x26)
#define CLOSEBKT    (0x27)

#define RETURN      (0x28)
#define ESCAPE      (0x29)
#define BACKSPACE   (0x2A)
#define TAB         (0x2B)
#define SPACE       (0x2C)
#define HYPHEN      (0x2D)
#define EQUAL       (0x2E)
#define SQBKTOPEN   (0x2F)
#define SQBKTCLOSE  (0x30)
#define BACKSLASH   (0x31)
#define SEMICOLON   (0x33)
#define INVCOMMA    (0x34)
#define TILDE       (0x35)
#define COMMA       (0x36)
#define PERIOD      (0x37)
#define FRONTSLASH  (0x38)
#define DELETE      (0x4c)
/**/
/* Modifier masks. One for both modifiers */
#define SHIFT       0x22
#define CTRL        0x11
#define ALT         0x44
#define GUI         0x88
/**/
/* "Sticky keys */
#define CAPSLOCK    (0x39)
#define NUMLOCK     (0x53)
#define SCROLLLOCK  (0x47)
/* Sticky keys output report bitmasks */
#define bmNUMLOCK       0x01
#define bmCAPSLOCK      0x02
#define bmSCROLLLOCK    0x04
/**/
/* flag of functiong buttion is press*/
bool FLAG_SHIFT = false;
bool FLAG_CTRL = false;
bool FLAG_ALT = false;
bool FLAG_GUI = false;
int FUNPS2CODE[8] = {0x14, 0x12, 0x11, 0x1F, 0x14, 0x59, 0x11, 0x27};

EP_RECORD ep_record[ 2 ];  //endpoint record structure for the keyboard

char new_buf[ 8 ] = { 0 };      //keyboard buffer
char old_buf[ 8 ] = { 0 };  //last poll
/* Sticky key state */
bool numLock = false;
bool capsLock = false;
bool scrollLock = false;
bool line = false;

void setup();
void loop();

MAX3421E Max;
USB Usb;

uint8_t K[255], KE[255];

void setup() {
  setup_keymaps();
  Serial.begin( 9600 );
  Serial.println("Start");
  Max.powerOn();
  fid_ps2h_init(4, 2);
  delay( 200 );
}

void loop() {
  Max.Task();
  Usb.Task();
  if ( Usb.getUsbTaskState() == USB_STATE_CONFIGURING ) { //wait for addressing state
    kbd_init();
    Usb.setUsbTaskState( USB_STATE_RUNNING );
  }
  if ( Usb.getUsbTaskState() == USB_STATE_RUNNING ) { //poll the keyboard
    kbd_poll();
  }
}
/* Initialize keyboard */
void kbd_init( void )
{
  byte rcode = 0;  //return code
  /**/
  /* Initialize data structures */
  ep_record[ 0 ] = *( Usb.getDevTableEntry( 0, 0 )); //copy endpoint 0 parameters
  ep_record[ 1 ].MaxPktSize = EP_MAXPKTSIZE;
  ep_record[ 1 ].Interval  = EP_POLL;
  ep_record[ 1 ].sndToggle = bmSNDTOG0;
  ep_record[ 1 ].rcvToggle = bmRCVTOG0;
  Usb.setDevTableEntry( 1, ep_record );              //plug kbd.endpoint parameters to devtable
  /* Configure device */
  rcode = Usb.setConf( KBD_ADDR, 0, 1 );
  if ( rcode ) {
    Serial.print("Error attempting to configure keyboard. Return code :");
    Serial.println( rcode, HEX );
    while (1); //stop
  }
  /* Set boot protocol */
  rcode = Usb.setProto( KBD_ADDR, 0, 0, 0 );
  if ( rcode ) {
    Serial.print("Error attempting to configure boot protocol. Return code :");
    Serial.println( rcode, HEX );
    while ( 1 ); //stop
  }
  delay(2000);
  Serial.println("Keyboard initialized");
}

/* Poll keyboard and print result */
/* buffer starts at position 2, 0 is modifier key state and 1 is irrelevant */
void kbd_poll( void )
{
  char i;
  boolean samemark = true;
  static char leds = 0;
  byte rcode = 0;     //return code
  byte usb_data;
  /* poll keyboard */
  rcode = Usb.inTransfer( KBD_ADDR, KBD_EP, 8, new_buf );
  if ( rcode != 0 ) {
    return;
  }//if ( rcode..
//  if (new_buf[0] != old_buf[0])
//  {
//    Ps2FunButtonPress(new_buf[0]);
//    Ps2FunButtonRelease(new_buf[0]);
//  }

  for (int i = 0; i < 8; i++) {
    if (((new_buf[0] >> i) & 1) != ((old_buf[0] >> i) & 1)) {
      if  ((new_buf[0] >> i) & 1) {
        usb_data = FUNPS2CODE[i];
        Serial.println(usb_data, HEX);
        if ((i == 0) || (i == 1) || (i == 2) || (i == 5)) {
          fid_ps2h_write(usb_data);
        } else {
          fid_ps2h_write(0xE0);
          fid_ps2h_write(usb_data);
        }
      } else {
        usb_data = FUNPS2CODE[i];
        
        if ((i == 0) || (i == 1) || (i == 2) || (i == 5)) {
          fid_ps2h_write(0xF0);
          fid_ps2h_write(usb_data);
        } else {
          fid_ps2h_write(0xE0);
          fid_ps2h_write(0xF0);
          fid_ps2h_write(usb_data);
        }
      }
    }
  }

  for ( i = 2; i < 8; i++ ) {
    //    if ( new_buf[ i ] == 0 ) { //end of non-empty space
    //      break;
    //    }
    if ( buf_compare_with_old( new_buf[ i ] ) == false ) {  //if new key
      switch ( new_buf[ i ] ) {
        case CAPSLOCK:
          capsLock = ! capsLock;
          leds = ( capsLock ) ? leds |= bmCAPSLOCK : leds &= ~bmCAPSLOCK;       // set or clear bit 1 of LED report byte
          Ps2ButtonPress(new_buf[ i ]);
          //sendToPs2(new_buf[ i ], new_buf[ 0 ]);
          break;
        case NUMLOCK:
          numLock = ! numLock;
          leds = ( numLock ) ? leds |= bmNUMLOCK : leds &= ~bmNUMLOCK;           // set or clear bit 0 of LED report byte
          Ps2ButtonPress(new_buf[ i ]);
          //sendToPs2(new_buf[ i ], new_buf[ 0 ]);
          break;
        case SCROLLLOCK:
          scrollLock = ! scrollLock;
          leds = ( scrollLock ) ? leds |= bmSCROLLLOCK : leds &= ~bmSCROLLLOCK;   // set or clear bit 2 of LED report byte
          Ps2ButtonPress(new_buf[ i ]);
          //sendToPs2(new_buf[ i ], new_buf[ 0 ]);
          break;
        case DELETE:
          line = false;
          Ps2ButtonPress(new_buf[ i ]);
          //sendToPs2(new_buf[ i ], new_buf[ 0 ]);
          break;
        case RETURN:
          line = ! line;
          Ps2ButtonPress(new_buf[ i ]);
          //sendToPs2(new_buf[ i ], new_buf[ 0 ]);
          break;
        default:
          Ps2ButtonPress(new_buf[ i ]);
          //sendToPs2(new_buf[ i ], new_buf[ 0 ]);
      }//switch( new_buf[ i ...

      rcode = Usb.setReport( KBD_ADDR, 0, 1, KBD_IF, 0x02, 0, &leds );
      if ( rcode ) {
        Serial.print("Set report error: ");
        Serial.println( rcode, HEX );
      }//if( rcode ...
    }//if( buf_compare( new_buf[ i ] ) == false ...

    if ( buf_compare_with_new( old_buf[ i ] ) == false ) {  //if reles key
      Ps2ButtonRelease(old_buf[ i ]);
    }
  }//for( i = 2...

  for ( i = 0; i < 8; i++ ) {                   //copy new buffer to old
    old_buf[ i ] = new_buf[ i ];
  }
}

void Ps2FunButtonPress(byte mod)
{
  Serial.println(mod, HEX);
  Serial.println(" ");
  Serial.println((mod & SHIFT) == true);
  if ((mod & SHIFT) && (FLAG_SHIFT == false)) {
    Serial.println("shift");
    FLAG_SHIFT = true;
    fid_ps2h_write(0x12); //0x12: Left shift scan code
  }
  if ((mod & CTRL) && (FLAG_CTRL == false)) {
    FLAG_CTRL == true;
    fid_ps2h_write(0x14); //0x14: Left control scan code
  }
  if ((mod & ALT) && (FLAG_ALT == false)) {
    FLAG_ALT == true;
    fid_ps2h_write(0x11); //0x11: Left Alt scan code
  }
  if ((mod & GUI) && (FLAG_GUI == false)) {
    FLAG_GUI == true;
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0x1F); //0x1F: Left GUI scan code
  }
}

void Ps2FunButtonRelease(byte mod)
{
  Serial.println(mod, HEX);
  Serial.println(" ");

  if ((FLAG_GUI == true) && (mod & GUI == false)) {
    FLAG_GUI == false;
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x1F); //0x1F: Left GUI scan code
  }
  if ((FLAG_ALT == true) && (mod & ALT == false)) {
    FLAG_ALT == false;
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x11); //0x14: Left control scan code
  }
  if ((FLAG_CTRL == true) && (mod & CTRL == false)) {
    FLAG_CTRL == false;
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x14);
  }
  if ((FLAG_SHIFT == true) && ((mod & SHIFT) == false)) {
    Serial.println("release shift");
    FLAG_SHIFT == false;
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x12); //0x12: Left shift scan code
  }
}

void Ps2ButtonPress(byte usb_data)
{
  uint8_t ps_data;
  ps_data = K[usb_data];
  Serial.println(ps_data, HEX);
  Serial.println(" ");

//  if (mod & SHIFT) {
//    fid_ps2h_write(0x12); //0x12: Left shift scan code
//  }
//  if (mod & CTRL) {
//    fid_ps2h_write(0x14); //0x14: Left control scan code
//  }
//  if (mod & ALT) {
//    fid_ps2h_write(0x11); //0x11: Left Alt scan code
//  }
//  if (mod & GUI) {
//    fid_ps2h_write(0xE0);
//    fid_ps2h_write(0x1F); //0x1F: Left GUI scan code
//  }

  if ((usb_data >= 0x49 && usb_data <= 0x52) || usb_data == 0x54 || usb_data == 0x58 || usb_data == 0x65 || usb_data == 0x66)
  {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(ps_data);
  } else {
    fid_ps2h_write(ps_data);
  }
}

void Ps2ButtonRelease(byte usb_data)
{
  uint8_t ps_data;
  ps_data = K[usb_data];
  Serial.println(ps_data, HEX);
  Serial.println(" ");

  if ((usb_data >= 0x49 && usb_data <= 0x52) || usb_data == 0x54 || usb_data == 0x58 || usb_data == 0x65 || usb_data == 0x66)
  {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps_data);
  } else {
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps_data);
  }

//  if (mod & GUI) {
//    fid_ps2h_write(0xE0);
//    fid_ps2h_write(0xF0);
//    fid_ps2h_write(0x1F); //0x1F: Left GUI scan code
//  }
//  if (mod & ALT) {
//    fid_ps2h_write(0xF0);
//    fid_ps2h_write(0x11); //0x14: Left control scan code
//  }
//  if (mod & CTRL) {
//    fid_ps2h_write(0xF0);
//    fid_ps2h_write(0x14);
//  }
//  if (mod & SHIFT) {
//    fid_ps2h_write(0xF0);
//    fid_ps2h_write(0x12); //0x12: Left shift scan code
//  }

}

void sendToPs2(byte usb_data, byte mod)
{

  Serial.println(usb_data, HEX);

  if (mod & SHIFT) {
    fid_ps2h_write(0x12); //0x12: Left shift scan code
  }
  if (mod & CTRL) {
    fid_ps2h_write(0x14); //0x14: Left control scan code
  }
  if (mod & ALT) {
    fid_ps2h_write(0x11); //0x11: Left Alt scan code
  }
  if (mod & GUI) {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0x1F); //0x1F: Left GUI scan code
  }
  //un_function_press(usb_data);
  if (usb_data >= 0x04 && usb_data <= 0x1D) {//Letters a-z
    if (capsLock == true) { //upper case
      fid_ps2h_write(0x12); //Left Shift scan code set 2
      un_function_press(usb_data);
      fid_ps2h_write(0xF0);
      fid_ps2h_write(0x12);
    } else {
      un_function_press(usb_data);
    }
  } else {
    un_function_press(usb_data);
  }


  if (mod & GUI) {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x1F); //0x1F: Left GUI scan code
  }
  if (mod & ALT) {
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x11); //0x14: Left control scan code
  }
  if (mod & CTRL) {
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x14);
  }
  if (mod & SHIFT) {
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x12); //0x12: Left shift scan code
  }
}

/*scan code has two format to unpress. input: ps2 set 2 scan code*/
void un_function_press(byte usb_data) {
  uint8_t ps_data;
  ps_data = K[usb_data];
  Serial.println(ps_data, HEX);
  Serial.println( " ");

  if ((usb_data >= 0x49 && usb_data <= 0x52) || usb_data == 0x54 || usb_data == 0x58 || usb_data == 0x65 || usb_data == 0x66)
  {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(ps_data);
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps_data);
  }
  else {
    fid_ps2h_write(ps_data);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps_data);
  }
}

/* compare byte against bytes in old buffer */
bool buf_compare_with_old( byte data )
{
  char i;
  for ( i = 2; i < 8; i++ ) {
    if ( old_buf[ i ] == data ) {
      return ( true );
    }
  }
  return ( false );
}

/* compare byte against bytes in new buffer */
bool buf_compare_with_new( byte data )
{
  char i;
  for ( i = 2; i < 8; i++ ) {
    if ( new_buf[ i ] == data ) {
      return ( true );
    }
  }
  return ( false );
}

void setup_keymaps() {
  K[0x04] = 0x1C;
  K[0x05] = 0x32;
  K[0x06] = 0x21;
  K[0x07] = 0x23;
  K[0x08] = 0x24;
  K[0x09] = 0x2B;
  K[0x0A] = 0x34;
  K[0x0B] = 0x33;
  K[0x0C] = 0x43;
  K[0x0D] = 0x3B;
  K[0x0E] = 0x42;
  K[0x0F] = 0x4B;
  K[0x10] = 0x3A;
  K[0x11] = 0x31;
  K[0x12] = 0x44;
  K[0x13] = 0x4D;
  K[0x14] = 0x15;
  K[0x15] = 0x2D;
  K[0x16] = 0x1B;
  K[0x17] = 0x2C;
  K[0x18] = 0x3C;
  K[0x19] = 0x2A;
  K[0x1A] = 0x1D;
  K[0x1B] = 0x22;
  K[0x1C] = 0x35;
  K[0x1D] = 0x1A;
  K[0x1E] = 0x16;
  K[0x1F] = 0x1E;
  K[0x20] = 0x26;
  K[0x21] = 0x25;
  K[0x22] = 0x2E;
  K[0x23] = 0x36;
  K[0x24] = 0x3D;
  K[0x25] = 0x3E;
  K[0x26] = 0x46;
  K[0x27] = 0x45;
  K[0x28] = 0x5A;
  K[0x29] = 0x76;
  K[0x2A] = 0x66;
  K[0x2B] = 0x0D;
  K[0x2C] = 0x29;
  K[0x2D] = 0x4E;
  K[0x2E] = 0x55;
  K[0x2F] = 0x54;
  K[0x30] = 0x5B;
  K[0x31] = 0x5D;
  K[0x33] = 0x4C;
  K[0x34] = 0x52;
  K[0x35] = 0x0E;
  K[0x36] = 0x41;
  K[0x37] = 0x49;
  K[0x38] = 0x4A;
  K[0x39] = 0x58;
  K[0x3A] = 0x5;
  K[0x3B] = 0x6;
  K[0x3C] = 0x4;
  K[0x3D] = 0x0C;
  K[0x3E] = 0x3;
  K[0x3F] = 0x0B;
  K[0x40] = 0x83;
  K[0x41] = 0x0A;
  K[0x42] = 0x1;
  K[0x43] = 0x9;
  K[0x44] = 0x78;
  K[0x45] = 0x7;
  K[0x46] = 0x7C;
  K[0x47] = 0x7E;
  K[0x49] = 0x70;
  K[0x4A] = 0x6C;
  K[0x4B] = 0x7D;
  K[0x4C] = 0x71;
  K[0x4D] = 0x69;
  K[0x4E] = 0x7A;
  K[0x4F] = 0x74;
  K[0x50] = 0x6B;
  K[0x51] = 0x72;
  K[0x52] = 0x75;
  K[0x53] = 0x77;
  K[0x54] = 0x4A;
  K[0x55] = 0x7C;
  K[0x56] = 0x7B;
  K[0x57] = 0x79;
  K[0x58] = 0x5A;
  K[0x59] = 0x69;
  K[0x5A] = 0x72;
  K[0x5B] = 0x7A;
  K[0x5C] = 0x6B;
  K[0x5D] = 0x73;
  K[0x5E] = 0x74;
  K[0x5F] = 0x6C;
  K[0x60] = 0x75;
  K[0x61] = 0x7D;
  K[0x62] = 0x70;
  K[0x63] = 0x71;
  K[0x64] = 0x61;
  K[0x67] = 0x0F;
  K[0xE0] = 0x14;
  K[0xE1] = 0x12;
  K[0xE2] = 0x11;
  K[0xE3] = 0x58;
  K[0xE5] = 0x59;
  KE[0x46] = 0x7c;
  KE[0x49] = 0x70;
  KE[0x4A] = 0x6C;
  KE[0x4B] = 0x7D;
  KE[0x4C] = 0x71;
  KE[0x4D] = 0x69;
  KE[0x4E] = 0x7A;
  KE[0x4F] = 0x74;
  KE[0x50] = 0x6B;
  KE[0x51] = 0x72;
  KE[0x52] = 0x75;
  KE[0x54] = 0x4A;
  KE[0x58] = 0x5A;
  KE[0x65] = 0x2F;
  KE[0xE3] = 0x1F;
  KE[0xE4] = 0x14;
  KE[0xE6] = 0x11;
  KE[0xE7] = 0x27;
}
