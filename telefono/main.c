#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

char arreglo [4][4]={{'1' , '4', '7', '*'}, {'2', '5', '8', '0'}, {'3', '6', '9', '#'}, {'A', 'B' , 'C', 'D'}};  //valores en hexadecimal de los numeros
int numt[10],
    numt1[10]={'5','5','1','1','1','1','1','1','1','1'};
int i,k,z,valido, pantalla, ren,c, col, j,cel, recibe,potencia=0x30, pulso=0,delay,msmocall,edo;
int contnumeros=0;
char pvar, var, Rx[80],mensaje_mem[80],recibido[32],envio[32];
char puerta[14]={'P','U','E','R','T','A',' ','A','B','I','E','R','T','A'},error[5]={'E','R','R','O','R'};

char letras[10][5][4] = {
    {"0"},      // 0
    {"1"},      // 1
    {"A", "B", "C"},      // 2
    {"D", "E", "F"},      // 3
    {"G", "H", "I"},      // 4
    {"J", "K", "L"},      // 5
    {"M", "N", "O"},      // 6
    {"P", "Q", "R", "S"}, // 7
    {"T", "U", "V"},      // 8
    {"W", "X", "Y", "Z"}  // 9
};
//////////////////INVERSOR///////////////////
const int p [11][21]= {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                      {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
                      {0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0},
                      {0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0},
                      {0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
                      {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
                      {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
                      {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0},
                      {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
                      {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
                      {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}};

int output=0, ele=0, pola=0;//pola es el ciclo, si pola es 0, el ciclo es positivo, si es 1, es negativo



#define CR 0x0d //'T'
#define LF 0x0a //'T'
#define CTRL_Z  0x1a


static const  uint8_t AT[]="AT;";
static const  uint8_t ATD[]="ATD5627649910;";
static const  uint8_t CSQ[]="AT+CSQ;";
static const  uint8_t call[]="ATD";
static const  uint8_t CMGF[]="at+cmgf?";
static const  uint8_t CMGF_1[]="at+cmgf=1";
static const  uint8_t ATCMGS[]="at+cmgs=\"5627649910\"";//manda mensaje/////////////////////Revisar las comillas
static const  uint8_t MSJ[]="MODEM MENSAJE";
static const  uint8_t ATA[]="ATA";
static const  uint8_t ATH[]="ATH";
static const  uint8_t RING[]="RING";
static const  uint8_t PUNTOYCOMA[]=";";



uint8_t d_uint8Dato,d_uint8Dat;


unsigned char Valcon,Temp,Temp2;

#define bitset(Var,bitno) ((Var) |= 1 << (bitno))
#define bitclr(Var,bitno) ((Var) &= ~(1 << (bitno)))

#define PORTDIS GPIO_PORTK_DATA_R
#define PORTCON GPIO_PORTM_DATA_R

//bits de control en PORTM
#define RS   0x00
#define R_W  0x01
#define E    0x02

#define BIT7 0X80


#define DISP_ON 0X0F
#define CLR_DISP 0x01

void INILCD(void);
void limpia_d();
void BUSY (void);
void BUSY1 (void);
void _E(void);
void ESCDAT (unsigned char c);
void ESCCOM (unsigned char d);
void CLS(void);
void BYTEDIS(unsigned char DByte);
void HOME(void);
void AND491(void);
void LEECON(void);
void configura(void);
void detect (void);
void llamada(void);
void UART6_INT (void);
void salidapot(void);
void recibe_mensaje(void);



int main(void)
{
    uint32_t g_ui32SysClock;
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |//se acerca al valor mÃ¡s cercano a la frecuencia que le pedimos
                                                 SYSCTL_OSC_MAIN |
                                                 SYSCTL_USE_PLL |
                                                 SYSCTL_CFG_VCO_480), 50000000);//

    UART0_INI();
    UART6_INI();


    configura();
    INILCD();

    while(1){
        if(Rx[3]=='G') {
            i=0;
            z=0;
            pantalla=1;
        }
        if((GPIO_PORTQ_DATA_R&0x0F)==0x0F){
            i=0;
            col=-1;
            ren=-1;
            //var="";
        }
        else{
          detect();
           salida();
           //tellaman();
        }
        if (recibe==1){
            recibe_mensaje();
        }
    }
    return 0;
}



void configura(){
    // Habilitar el reloj del puerto  H Y Q
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7 | SYSCTL_RCGCGPIO_R9 | SYSCTL_RCGCGPIO_R11 | SYSCTL_RCGCGPIO_R14 ;

    // Esperar a que los relojes de los puertos estén listos
    while ((SYSCTL_PRGPIO_R & (SYSCTL_PRGPIO_R7 | SYSCTL_RCGCGPIO_R9 | SYSCTL_RCGCGPIO_R11 | SYSCTL_PRGPIO_R14 )) != (SYSCTL_PRGPIO_R7 | SYSCTL_RCGCGPIO_R9 | SYSCTL_RCGCGPIO_R11 | SYSCTL_PRGPIO_R14)) {}

    SYSCTL_RCGCGPIO_R |= 0X1400; // Habilitar el reloj del puerto L, N
    k=123;
    //CONFIGURA EL TIMMER 3
    SYSCTL_RCGCTIMER_R = 0x08; //ACTIVA RELOJ TIMER 3
    k=1234; //TIEMPO PARA ESTABILIZAR RELOJES
    //CONFIGURAMOS TIMER 3 SUBTIMER A
    TIMER3_CTL_R = 0X0; //DESCATIVA EL TIMER A UTILIZAR
    TIMER3_CFG_R = 0X04; //ACTIVAMOS LA FUNCION DE 16 BITS DEL TIMER
    TIMER3_TAMR_R = 0X02; //CUENTA HACIA ABAJO
    TIMER3_TAILR_R = 0X1970; //CUENTA PRECARGADA
    TIMER3_ICR_R = 0x1; //LIMPIA BANDERA DE INTERRUPCION
    TIMER3_IMR_R = 0X01; //TIMEOUT
    TIMER3_TAPR_R = 0X0; //PRESCALADO
    NVIC_EN1_R= 1<<(35-32); //ACTIVA INTERRUPCION TIMER 3
    TIMER3_CTL_R |= 0X01; //ACTIVA TIMER

    //Configuración puerto L

    GPIO_PORTL_DIR_R &= ~0X03;
    GPIO_PORTL_PUR_R = 0x03;
    GPIO_PORTL_IM_R = 0x00;
    GPIO_PORTL_IS_R &= ~0x00;        // PL es sensible por flanco
    GPIO_PORTL_IBE_R &= ~0x03;       // PL no es sensible a dos flancos
    GPIO_PORTL_IEV_R &= ~0x03;       // PL detecta eventos de flanco de bajada
    GPIO_PORTL_ICR_R = 0x03; //Limpia interrupciones
    GPIO_PORTL_IM_R |= 0x03; //Habilita interrupciones
    GPIO_PORTL_DEN_R = 0x03; //Habilita puerto L
    NVIC_EN1_R |= 0x200000; //Habilita interrupciiones en NVIC (PL)

    // Configura pines del puerto H como salidas (TECLADO)
    GPIO_PORTH_AHB_DIR_R |= 0x0F;
    GPIO_PORTH_AHB_DEN_R |= 0X0F;

    //Configura pines del puerto K  (PANTALLA)
    GPIO_PORTK_DATA_R=0X00; //AHB
    GPIO_PORTK_DEN_R=0XFF;
    GPIO_PORTK_DIR_R=0XFF;

    //Configura pines del puerto M (PANTALLA)
    GPIO_PORTM_DATA_R|=0X00;
    GPIO_PORTM_DEN_R |=0X07;
    GPIO_PORTM_DIR_R |=0X07;

    // Configurar pines del puerto Q como entradas (TECLADO)
    GPIO_PORTQ_DIR_R &= ~0X0F;
    GPIO_PORTQ_DEN_R |= 0X0F;
    GPIO_PORTQ_PUR_R |= 0X0F;

}

void detect (void){
    SysCtlDelay (56000);//20ms
    //delay tiempos de rebote de 15 [ms], entonces con 20[ms] es suficiente

    if((GPIO_PORTQ_DATA_R&0x0F)!=0x0F){
        if((GPIO_PORTQ_DATA_R&0x0F)==0x0E&&(i!=1)){
            ren=0;
        }
        else if((GPIO_PORTQ_DATA_R&0x0F)==0x0D&&(i!=1)){
            ren=1;
        }
        else if((GPIO_PORTQ_DATA_R&0x0F)==0x0B&&(i!=1)){
            ren=2;
        }
        else if((GPIO_PORTQ_DATA_R&0x0F)==0x07&&(i!=1)){
            ren=3;
        }
        else{
                //mas de una tecla pulsada
        }

        //SE MODIFICA EL ESTADO DEL PUERTO H A 0
        GPIO_PORTH_AHB_DATA_R &= 0XF0;
        //ENCENDEMOS LAS ENTRADAS H3, H2, H1
        GPIO_PORTH_AHB_DATA_R |= 0X0E;
        //evaluamos si el puerto sigue siendo diferente de 0x0F
        if(((GPIO_PORTQ_DATA_R&0x0F)!=0x0F)&&(i!=1)){
            col=0;
            i=1;
            var=arreglo [ren][col];
            j++;
        }
        GPIO_PORTH_AHB_DATA_R &= 0XF0;
        GPIO_PORTH_AHB_DATA_R |= 0X0D;
        if(((GPIO_PORTQ_DATA_R&0x0F)!=0x0F)&&i!=1){
            col=1;
            i=1;
            var=arreglo [ren][col];
            j++;
        }
        GPIO_PORTH_AHB_DATA_R &= 0XF0;
        GPIO_PORTH_AHB_DATA_R |= 0X0B;
        if(((GPIO_PORTQ_DATA_R&0x0F)!=0x0F)&&i!=1){
            col=2;
            i=1;
            var=arreglo [ren][col];
            j++;
        }
        GPIO_PORTH_AHB_DATA_R &= 0XF0;
        GPIO_PORTH_AHB_DATA_R |= 0X07;
        if(((GPIO_PORTQ_DATA_R&0x0F)!=0x0F)&&i!=1){
            col=3;
            i=1;
            var=arreglo [ren][col];
            j++;
        }
        GPIO_PORTH_AHB_DATA_R &= 0XF0;
        if(var=='*'){
            ESCCOM(CLR_DISP);
        }
        else if(var=='A'){
            llamada();
        }
        else if (var=='B'){
            mensaje();
        }
        else if (var=='C'){
            modem();
        }
        else if (var=='D'){
           contesta();
               }
        else if (var=='#'){
           cuelga();
               }

        else{
            ESCDAT(var);
        }
        while((GPIO_PORTQ_DATA_R&0x0F)!=0x0F){
        }
    }
}

void INILCD(void)  //INICIALIZA EL DISPLAY A DOS LINEAS, 4 BITS DE INTERFAZ, CURSOR Y DESTELLO.
{
    //PRIMER METODO DE INICIALIZACION
    PORTCON=0X00;
    PORTDIS=0X38; // **D7-D4 = 0011

    SysCtlDelay(533000);                 //Delay 100ms
    _E();                               //toggle E
    SysCtlDelay (53000);                   //10ms
    _E();                               //toggle E
    SysCtlDelay (53000);                   //10ms
    ;PORTDIS=0X38;
    _E();                               //toggle E
    SysCtlDelay (53000);                   //10ms
    ESCCOM(0X38);   //8 BITS , 2 LINEAS, 5X7

    ESCCOM(DISP_ON);  // ENCIENDE DISPLAY CON CURSOR Y DESTELLO


    ESCCOM(CLR_DISP); // BORRAR DISPLAY

    ESCCOM(0X06); // AL ESCRIBIR EL CURSOR SE INCREMENTA Y SE DESPLAZA
    ESCCOM(0X38); // //8 BITS , 2 LINEAS, 5X7 REQUERIDO POR LOS DISPLAYS CLONES
}

void _E(void)  //GENERA UN PULSO DE 1.25 uS EN LA TERMINAL E DEL DISPLAY
{
    bitset(PORTCON,E);        // E=1
    SysCtlDelay (3);  //retraso de 500 nS
    bitclr(PORTCON,E);  // E=0
}

void ESCDAT (unsigned char c)  //ESCRIBE UN DATO ALFANUMERICO AL DISPLAY
{
    //ENVIA dato
    PORTDIS = c;

    bitset(PORTCON,RS);    //ENVIA A REGISTRO DE DATOS
    _E();                //TOGGLE _E()
    BUSY1(); //PREGUNTA POR LA BANDERA DE BUSY
    bitclr(PORTCON,RS);    //niveles de control a 0
    }



void ESCCOM (unsigned char d)// ENVIA UN COMANDO AL REGISTRO DE CONTROL DEL LCD
{

    PORTDIS = d;               //envia CMD al LCD
    bitclr(PORTCON,RS);               //direcciona registro de controldel  LCD
    _E();                           //toggle E
    BUSY1(); //VERIFICA LA BANDERA DE BUSY
    }



void CLS(void)
{ ESCCOM(0x01);
     //AND491( );
        }

void HOME(void)
{
        pantalla=1;
        ESCCOM(CLR_DISP);

        ESCCOM(0X80);

        ESCDAT(0X00); //nulo
        ESCDAT(0x4C); //L
        ESCDAT(0x4C); //L
        ESCDAT(0x41); //A
        ESCDAT(0x4D); //M
        ESCDAT(0x41); //A
        ESCDAT(0x44); //D
        ESCDAT(0x41); //A
        ESCCOM(0x88); //
        ESCDAT(0X50); //P
        ESCDAT(0x55); //U
        ESCDAT(0x4C); //L
        ESCDAT(0x53); //S
        ESCDAT(0x41); //A
        ESCCOM(0x8E); //
        ESCDAT(0x31); //1

        ESCCOM(0XC0);//CURSOR A 2DO RENGLON

        ESCDAT(0X4D); //M
        ESCDAT(0x45); //E
        ESCDAT(0x4E); //N
        ESCDAT(0x53); //S
        ESCDAT(0x41); //A
        ESCDAT(0x4A); //J
        ESCDAT(0x45); //E
        ESCCOM(0xC8); //
        ESCDAT(0X50); //P
        ESCDAT(0x55); //U
        ESCDAT(0x4C); //L
        ESCDAT(0x53); //S
        ESCDAT(0x41); //A
        ESCCOM(0xCE); //
        ESCDAT(0x32); //2
        ESCDAT(0x20); //
}

void AND491(void)  // GENERA LOS CORRIMIENTOS PARA ALINEAR COMO EN
    {   ESCCOM (0X1C);  //EL AND491 PARA LOS DISPLAYS GENERICOS
        ESCCOM (0X1C);
        ESCCOM (0X1C);
        ESCCOM (0X1C);
    }



void BUSY (void)  //PREGUNTA POR EL ESTADO DE LA BANDERA BUSY Y ESPERA HASTA QUE SEA CERO
{
    do LEECON( );
    while ((Valcon & BIT7) != 0);
    }


void   LEECON(void)
// LEE EL VALOR DEL REGISTRO DE CONTROL DEL DISPLAY Y REGRESA EL CONTENIDO EN VALCON
    {   PORTDIS=0;
        GPIO_PORTK_DIR_R=0x00; //PORTK como entrada
        bitset(PORTCON,R_W); // LEER PUERTO DE CONTROL
         bitset(PORTCON,E);      //ACTIVA E
         SysCtlDelay(2); //espera 384 nS
         Temp=PORTDIS; // LEE PARTE ALTA DEL BUS DE DATOS
         bitclr(PORTCON,E);
         bitclr(PORTCON,RS);
         bitclr(PORTCON,R_W);
         GPIO_PORTK_DIR_R=0XFF; //REGRESA A LA CONDICION ORIGINAL DEL PUERTO K A SALIDA
          Valcon=Temp;
                }


void BYTEDIS(unsigned char DByte) //escribe un byte a pantalla

{ Temp2=DByte;
    Temp2=Temp2>>4;
    if (Temp2<=0x09)
        Temp2+=0x30;
        else
        Temp2+=0x37;
            ESCDAT(Temp2);
        Temp2=DByte&0x0f;
        if (Temp2<=0x09)
        Temp2+=0x30;
        else
        Temp2+=0x37;
            ESCDAT(Temp2);

}

void BUSY1(void)
    {SysCtlDelay(10066);}

void llamada(){
    ESCCOM(CLR_DISP);
    SysCtlDelay (53000);

    ESCCOM(0X80);
    ESCDAT(0x48);//H
    ESCDAT(0x61);//a
    ESCDAT(0x63);//c
    ESCDAT(0x69);//i
    ESCDAT(0x65);//e
    ESCDAT(0x6E);//n
    ESCDAT(0x64);//d
    ESCDAT(0x6F);//o

    ESCCOM(0XC0);
    ESCDAT(0x4C);//L
    ESCDAT(0x6C);//l
    ESCDAT(0x61);//a
    ESCDAT(0x6D);//m
    ESCDAT(0x61);//a
    ESCDAT(0x64);//d
    ESCDAT(0x61);//a
              sendComandInitModem();//Aquí escribimos en UART6
              UART6_Lee_dato();
              UART6_Lee_dato();
              ESCDAT(d_uint8Dato);


}

void mensaje(){
    ESCCOM(CLR_DISP);
    SysCtlDelay (53000);

    ESCCOM(0X80);
    ESCDAT(0x4D);//M
    ESCDAT(0x65);//e
    ESCDAT(0x6E);//n
    ESCDAT(0x73);//s
    ESCDAT(0x61);//a
    ESCDAT(0x6A);//j
    ESCDAT(0x65);//e

    ESCCOM(0XC0);
    ESCDAT(0x45);//E
    ESCDAT(0x6E);//n
    ESCDAT(0x76);//v
    ESCDAT(0x69);//i
    ESCDAT(0x61);//a
    ESCDAT(0x64);//d
    ESCDAT(0x6F);//o
    ComandMensajeModem();

    //RETRASO PARA BORRAR PANTALLA Y LUEGO MOSTRAR EL MENSAJE ENVIADO
}

void modem(){
    ESCCOM(CLR_DISP);
    SysCtlDelay (53000);
    ESCCOM(0X80);//Línea de arriba
    ESCDAT(0x41);//A
    ESCDAT(0x54);//T
    ESCDAT(0x3A);//:
    send_Inst(AT);
    do{
        UART6_Lee_dato();
    }while(d_uint8Dato!='O');
    ESCDAT(d_uint8Dato);//M
    UART6_Lee_dato();
    ESCDAT(d_uint8Dato);//M


    ESCCOM(0XC0);//Línea de abajo
    ESCDAT(0x43);//C
    ESCDAT(0x53);//S
    ESCDAT(0x51);//Q
    ESCDAT(0x3A);//:
    send_Inst(CSQ);

   /*for(i=0; i<13; i++){
           UART6_Lee_dato();
       }*/
    do{
        UART6_Lee_dato();
    }while(d_uint8Dato!=':');

    ESCDAT(d_uint8Dato);//M
    UART6_Lee_dato();
    ESCDAT(d_uint8Dato);//M
    UART6_Lee_dato();
    ESCDAT(d_uint8Dato);//M
    UART6_Lee_dato();
    ESCDAT(d_uint8Dato);//M
    UART6_Lee_dato();
    ESCDAT(d_uint8Dato);//M
    UART6_DR_R=0B00000000; // ES BORRAR EL REGISTRO DE DATOS PARA QUE
    //CUANDO VOLVAMOS A PRESIONAR, SE VUELVA A LLENAR
    //Y MODIFIQUE

}

void contesta(){
    send_Inst(ATA);

}

void cuelga(){
    send_Inst(ATH);
    d_uint8Dato=0;
    contnumeros=0;

    ESCCOM(CLR_DISP);
    SysCtlDelay (53000);
    ESCCOM(0X80);//Línea de arriba
    ESCDAT(0x42);//B
    ESCDAT(0x6F);//o
    ESCDAT(0x72);//r
    ESCDAT(0x72);//r
    ESCDAT(0x61);//a
    ESCDAT(0x6E);//n
    ESCDAT(0x64);//d
    ESCDAT(0x6F);//o
    ESCDAT(0x5F);//_
    ESCDAT(0X64);//d
    ESCDAT(0x61);//a
    ESCDAT(0x74);//t
    ESCDAT(0x6f);//o
    ESCDAT(0x73);//s

    SysCtlDelay (5300000);
    ESCCOM(0XC0);
    ESCDAT(0x46);//F
    ESCDAT(0x69);//i
    ESCDAT(0x6E);//n
    ESCDAT(0x5F);//_
    ESCDAT(0x6C);//l
    ESCDAT(0x6C);//l
    ESCDAT(0x61);//a
    ESCDAT(0x6D);//m
    ESCDAT(0x61);//a
    ESCDAT(0x64);//d
    ESCDAT(0x61);//a

}

//////////////////////////////////////////////////////////////////////////////////////////

//Modificado a las 9:26

UART0_INI(void){
SYSCTL_RCGCUART_R |=0X0001; //HABILITAR UART0
SYSCTL_RCGCGPIO_R |=0X0001; //HABILITAR PUERTO A
UART0_CTL_R &=~0X0001;  //DESHABILITAR UART
UART0_IBRD_R = 26 ; //IBDR=int(50000000/16*115200))= int(27.1267)
UART0_FBRD_R =2 ; //FBRD= round(0.1267*64 =8)
UART0_LCRH_R =0X0070; //8 BITS, HABILITAR FIFO
UART0_CTL_R= 0X0301 ; //HABILITAR RXE, TXE Y UART
GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R&0XFFFFFF00)+0X00000011; //UART
GPIO_PORTA_AHB_AMSEL_R &= ~0X03; //DESHABILITAR FUNCION ANLOGICA EN PA0-1
GPIO_PORTA_AHB_AFSEL_R |= 0X03; //HABILITAR FUNCION ALTERNA EN PA0-1
GPIO_PORTA_AHB_DEN_R |= 0X03; //HABILITAR FINCION I/O DIGITAL
}

///////////////UART6
UART6_INI(void)
{
SYSCTL_RCGCUART_R |=0X0040; //HABILITAR UART6   0000.0000.0100.0000
SYSCTL_RCGCGPIO_R |=0X2000; //HABILITAR PUERTO P 0010.0000.0000.0000
UART6_CTL_R &=~0X0001;  //DESHABILITAR UART
UART6_IBRD_R = 26 ; //IBDR=int(50000000/16*115200))= int(27.1267)
UART6_FBRD_R =2 ; //FBRD= round(0.1267*64 =8)
UART6_LCRH_R =0X0070; //8 BITS, HABILITAR FIFO
UART6_CTL_R= 0X0301 ; //HABILITAR RXE, TXE Y UART
GPIO_PORTP_PCTL_R = (GPIO_PORTA_AHB_PCTL_R&0XFFFFFF00)+0X00000011; //UART //bit0y1
GPIO_PORTP_AMSEL_R &= ~0X03; //DESHABILITAR FUNCION ANLOGICA EN PA0-1
GPIO_PORTP_AFSEL_R |= 0X03; //HABILITAR FUNCION ALTERNA EN PA0-1
GPIO_PORTP_DEN_R |= 0X03; //HABILITAR FINCION I/O DIGITAL
NVIC_EN1_R |=0x8000000;
}

//esperar hasta que se reciba un dato
char UART0_Lee_dato(void)
{
//while((UART0_FR_R&0X0010)!=0); //ESPERAR A QUE RXFE SEA CERO
d_uint8Dat=((char)(UART0_DR_R&0xff));
}

///////////////////////
char UART6_Lee_dato(void)
{
while((UART6_FR_R&0X0010)!=0); //ESPERAR A QUE RXFE SEA CERO
d_uint8Dato=((char)(UART6_DR_R&0xff));
}

//////////////////////
char UART0_Escribe_dato(char dato)
{
while   ((UART0_FR_R&0X0020)!=0); // espera a que TXFF sea cero
UART0_DR_R=dato;
}

////////////////
char UART6_Escribe_dato(char dato1)
{
while   ((UART6_FR_R&0X0020)!=0); // espera a que TXFF sea cero
UART6_DR_R=dato1;
}

///////////////ENTER////////////////////////////////////
void ENTER(void){
    UART6_Escribe_dato(CR);
    UART6_Escribe_dato(LF);
}

//////////////////////////////////INICIALIZAR MODEM Y HACER LLAMADA//////////////////////////7
void sendComandInitModem(void){//Inicializar modulo
    send_Inst(ATD);
}


////////////////ENVIAR MENSAJE/////////////////7
void ComandMensajeModem(void){//Mensaje modulo

    /////
    send_Inst(CMGF);
    SysCtlDelay(10066000);

    send_Inst(CMGF_1);
    SysCtlDelay(10066000);
    send_Inst(ATCMGS);
    SysCtlDelay(10066000);

        while (i==0) //ESPERA ">"
                   {
                   if ((d_uint8Dato=='>'))
                             i=1;}
        int p=0;
        while (MSJ[p]!=0X00){
        UART6_Escribe_dato(MSJ[p++]);}

        UART6_Escribe_dato (CTRL_Z); //FIN DE MENSAJE
        UART6_Escribe_dato(CR);
        UART6_Escribe_dato(LF);
        i=0;
        d_uint8Dato=0x00;


    /////


}

void Inicializar(void){//Inicializar
    send_Inst(AT);
    send_Inst(CSQ);
}

////////////////////INSTRUCCIONES/////////////////////////7
void send_Inst(const uint8_t comando[]){
    int i=0;
    while(comando[i]!=0x00){
        UART6_Escribe_dato(comando[i++]);
    }
    ENTER();
}

//////////////LLAMADA//////////////////////////
void salida(){
    contnumeros++;
   //Aquí ir escribiendo lo que va digitando
    //Mandarle al puerto serial los digitos

    SysCtlDelay(53000);

    //send_Inst_llamada(call);
    if(var!='A' && var!='B' && var!='C' && var!='D' && var!='*' && var!='#'){
        UART6_Escribe_dato(var);
        SysCtlDelay(53000);//Para el rebote
        UART6_Lee_dato();
    }

    if(var=='*'){
      send_Inst_llamada(call);
    }

    if(contnumeros==12){
        send_Inst(PUNTOYCOMA);
        ENTER();
        contnumeros=0;
        ESCCOM(0XC0);//Línea de abajo
        ESCDAT(0x4C);//L
        ESCDAT(0x6C);//l
        ESCDAT(0x61);//a
        ESCDAT(0x6D);//m
        ESCDAT(0x61);//a
        ESCDAT(0x6E);//n
        ESCDAT(0x64);//d
        ESCDAT(0x6F);//o

    }



}

void send_Inst_llamada(const uint8_t comando[]){
    int i=0;
    while(comando[i]!=0x00){
        UART6_Escribe_dato(comando[i++]);
    }
}



MENSAJEPRED (void){
    SysCtlDelay (320000);
    k=GPIO_PORTL_DATA_R;
    switch(k){

    case 0:
        ESCCOM(CLR_DISP);
        ESCCOM(0X80);
        ESCDAT ((char)(0x20));
        for(z=0;z<5;z++){
            ESCDAT ((char)(error[z]));
        }
        SysCtlDelay (32000000);
        break;
    case 1:

        break;
    case 2:
        ESCCOM(CLR_DISP);
        ESCCOM(0X80);
        ESCDAT ((char)(0x20));  //
        ESCDAT ((char)(0x45));  // E
        ESCDAT ((char)(0x4E));  // N
        ESCDAT ((char)(0x56));  // V
        ESCDAT ((char)(0x49));  // I
        ESCDAT ((char)(0x41));  // A
        ESCDAT ((char)(0x20));  //
        ESCDAT ((char)(0x4D));  // M
        ESCDAT ((char)(0x45));  // E
        ESCDAT ((char)(0x4E));  // N
        ESCDAT ((char)(0x53));  // S
        ESCDAT ((char)(0x41));  // A
        ESCDAT ((char)(0x4A));  // J
        ESCDAT ((char)(0x45));  // E
        ESCDAT ((char)(0x20));  //
        ESCDAT ((char)(0x41));  //A
        ESCCOM(0XC0);//CURSOR A 2DO RENGLON
        for(z=0;z<10;z++){
            ESCDAT ((char)(numt1[z]));
        }
        SysCtlDelay (32000000);
        ESCCOM(CLR_DISP);
        ESCCOM(0X80);
        ESCDAT (0x20); //
        ESCDAT (0x50); //P
        ESCDAT (0x55); //U
        ESCDAT (0x45); //E
        ESCDAT (0x52); //R
        ESCDAT (0x54); //T
        ESCDAT (0x41); //A
        ESCDAT (0x20); //
        ESCDAT (0x41); //A
        ESCDAT (0x42); //B
        ESCDAT (0x49); //I
        ESCDAT (0x45); //E
        ESCDAT (0x52); //R
        ESCDAT (0x54); //T
        ESCDAT (0x41); //A
        ESCDAT (0x2E); //.
        ESCDAT (0x2E); //.

        UART6_Escribe_dato((char)(0x41)); //A
        UART6_Escribe_dato((char)(0x54)); //T
        UART6_Escribe_dato((char)(0x2B)); //+
        UART6_Escribe_dato((char)(0x43)); //C
        UART6_Escribe_dato((char)(0x4D)); //M
        UART6_Escribe_dato((char)(0x47)); //G
        UART6_Escribe_dato((char)(0x46)); //F
        UART6_Escribe_dato((char)(0x3D)); //=
        UART6_Escribe_dato((char)(0x31)); //1
        UART6_Escribe_dato((char)(0x3B)); //;
        UART6_Escribe_dato((char)(0x0a)); //
        UART6_Escribe_dato((char)(0x0d)); //

        for (delay = 0; delay < 10000000; delay++) {}

        UART6_Escribe_dato((char)(0x41)); //A
        UART6_Escribe_dato((char)(0x54)); //T
        UART6_Escribe_dato((char)(0x2B)); //+
        UART6_Escribe_dato((char)(0x43)); //C
        UART6_Escribe_dato((char)(0x4D)); //M
        UART6_Escribe_dato((char)(0x47)); //G
        UART6_Escribe_dato((char)(0x53)); //S
        UART6_Escribe_dato((char)(0x3D)); //=
        UART6_Escribe_dato((char)(0x22)); //"

        //Se envia numero
        for(z=0;z<10;z++){
            UART6_Escribe_dato((char)numt1[z]);
        }

        UART6_Escribe_dato((char)(0x22)); //"
        UART6_Escribe_dato((char)(0x0a)); //
        UART6_Escribe_dato((char)(0x0d)); //

        for (delay = 0; delay < 100000; delay++) {}

        for(z=0;z<14;z++){
            UART6_Escribe_dato((char)puerta[z]);
        }

        UART6_Escribe_dato((char)(0x1a)); //
        UART6_Escribe_dato((char)(0x0a)); //
        UART6_Escribe_dato((char)(0x0d)); //

        for (delay = 0; delay < 100000; delay++) {}
        UART6_Escribe_dato((char)(0x0a)); //
        UART6_Escribe_dato((char)(0x0d)); //

        valido=0;
        limpia_d();
        HOME();
        break;
    }

    GPIO_PORTL_ICR_R=0x03; //Limpiamos la bandera de recepción de datos
}



void salidapot () {
        if (pola==0)
        {
            output=p[pulso][ele];
            if (output==0){
                GPIO_PORTN_DATA_R = 0x0;
            }else{
                GPIO_PORTN_DATA_R = 0x04;
            }
            if (ele<20)
                ele++;
            else{
                ele=0;
                pola=pola+1;    //Cuando termina de mandar el arreglo cambia de polaridad
            }
        }
        if (pola==1)
            {
            output=p[pulso][ele];
            if (output==0){
                GPIO_PORTN_DATA_R = 0x0;
            }else{
                GPIO_PORTN_DATA_R = 0x08;
            }
            if (ele<20)
                ele++;
            else{
                ele=0;
                pola=pola-1;    //Cuando termina de mandar el arreglo cambia de polaridad
            }
            }

    TIMER3_ICR_R = 0x1; //Limpia bandera de interrupcion
}

void UART6_INT (){
    d_uint8Dato=((char)(UART6_DR_R&0xFF));
    if(d_uint8Dato=='+'){
        for(z=0;z<80;z++){
            Rx[z]=0; //Se limpia el arreglo de recepción
        }
    }
    if((d_uint8Dato=='+')||(valido==1)){
        valido=1;
        Rx[i]=d_uint8Dato;

        if((Rx[7]=='+')&&(i>6 && i<17)){
            msmocall=1;
        }

        if((i>49)&&(d_uint8Dato==10)){
            valido=0;
            recibe=1;
        }
        i++;
    }
    if (msmocall == 1){
    if(i>49&&d_uint8Dato==10){
        z=0;
        k=0;
        i=0;

        while(Rx[z]!=10){
            z++;
        }
        z++;
        pvar=Rx[z];
        while(Rx[z]!=10){
            recibido[i]=Rx[z];
            z++;
            i++;
        }
        if(Rx[7]=='+'){
            for (z=0;z<10;z++){
                numt[z]=Rx[z+10];
            }
        }else{
            for (z=0;z<10;z++){
                numt[z]=Rx[z+9];
            }
        }
            switch(pvar){

            case'A':
                edo=1;
                break;

            case 'E':
                edo=0;
                break;

            case 'I':
                if (pulso<=3){
                    pulso++;
                }
                break;
            case 'R':
                if (pulso>0){
                    pulso=pulso-1;
                }
                break;
            }
            switch (pulso){
                case 0:
                    potencia=0x30;
                    break;
                case 1:
                    potencia=0x31;
                    break;
                case 2:
                    potencia=0x32;
                    break;
                case 3:
                    potencia=0x33;
                    break;
                default:
                    potencia=0x45;
                }
            for(z=0;z<80;z++){
                Rx[z]=0;
            }
        }

    }
    UART6_ICR_R=0x0F; //Limpiamos la bandera de recepción de datos
}
void limpia_d(){
    j=0;
    c=-1;
    i=0;
    for(z=0;z<10;z++){
        numt[z]=0;
    }
    for(z=0;z<32;z++){
        envio[z]=0;
    }
}
void recibe_mensaje(){
    ESCCOM(CLR_DISP);
    ESCCOM(0X80);

    ESCDAT((char)(0x20));  //
    ESCDAT((char)(0x4E));  // N
    ESCDAT((char)(0x55));  // U
    ESCDAT((char)(0x45));  // E
    ESCDAT((char)(0x56));  // V
    ESCDAT((char)(0x4F));  // O
    ESCDAT((char)(0x20));  //
    ESCDAT((char)(0x20));  //
    ESCDAT((char)(0x4D));  // M
    ESCDAT((char)(0x45));  // E
    ESCDAT((char)(0x4E));  // N
    ESCDAT((char)(0x53));  // S
    ESCDAT((char)(0x41));  // A
    ESCDAT((char)(0x4A));  // J
    ESCDAT((char)(0x45));  // E

    ESCCOM(0XC0);//CURSOR A 2DO RENGLON

    for(z=0;z<10;z++){
        ESCDAT((char)(numt[z]));
    }

    SysCtlDelay(32000000);

    ESCCOM(CLR_DISP);
    ESCCOM(0X80);

    ESCDAT(0x20);  //
    for(z=0;z<(i-1);z++){
        if(z==16){
            ESCCOM(0XC0);
        }
        ESCDAT(recibido[z]);
    }
    SysCtlDelay(32000000);
    for(z=0;z<32;z++){
        recibido[z]=0;
    }
    UART6_ICR_R=0x0F;
    i=0;
    recibe=0;
    HOME();
}
