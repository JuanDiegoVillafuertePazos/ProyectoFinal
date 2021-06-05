/*
 * File:   Simulacion.c
 * Author: Medardo
 *
 * Created on 2 de junio de 2021, 11:06 PM
 */

#include <xc.h>
#pragma config FOSC=INTRC_NOCLKOUT //Oscilador interno sin salida
#pragma config WDTE=OFF           //Reinicio repetitivo del pic
#pragma config PWRTE=OFF           //no espera de 72 ms al iniciar el pic
#pragma config MCLRE=OFF          //El pin MCLR se utiliza como entrada/salida
#pragma config CP=OFF             //Sin protección de código
#pragma config CPD=OFF            //Sin protección de datos
    
#pragma config BOREN=OFF //Sin reinicio cuando el input voltage es inferior a 4V
#pragma config IESO=OFF  //Reinicio sin cambio de reloj de interno a externo
#pragma config FCMEN=OFF //Cambio de reloj externo a interno en caso de fallas
#pragma config LVP=OFF    //Programación en low voltage apagada
    
//CONFIGURATION WORD 2
#pragma config WRT=OFF //Proteccion de autoescritura por el programa desactivada
#pragma config BOR4V=BOR40V //Reinicio abajo de 4V 
#define _XTAL_FREQ 8000000 //frecuencia de 8 MHz
//variables
//124
unsigned char pot0, pot1;
unsigned char contpot0, contpot1;
unsigned char hab0, hab1;
//----------------------interrupciones------------------------------------------
unsigned char numresets(unsigned char potbang){
    unsigned char tmr0resets;
    if (potbang <19)
    {
        tmr0resets = 2;
    }
    else if (potbang > 18 && potbang < 40)
    {
        tmr0resets =  3;
    }
    else if (potbang > 39 && potbang < 61 )
    {
        tmr0resets =  5;
    }
    else if (potbang > 60 && potbang < 82)
    {
        tmr0resets =  6;
    }
    else if (potbang > 81 && potbang < 103 )
    {
        tmr0resets =  7;
    }
    else if (potbang > 102 && potbang < 131)
    {
        tmr0resets =  8;
    }
    else if (potbang > 130 && potbang < 152)
    {
        tmr0resets =  9;
    }
    else if (potbang > 151 &&  potbang < 173)
    {
        tmr0resets =  10;
    }
    else if (potbang > 172 && potbang < 194)
    {
        tmr0resets =  11;
    }
    else if ( potbang > 193 && potbang < 215)
    {
        tmr0resets =  12;
    }
    else if (potbang > 214 && potbang < 236)
    {
        tmr0resets =  14;
    }
    else  if (potbang > 236)
    {
        tmr0resets =  15;
    }
    return tmr0resets;
}

void __interrupt() isr(void){    // only process timer-triggered interrupts
    //interrupcion del adc
    if (ADIF == 1) {
        //multiplexacion de canales para el adc
        switch (ADCON0bits.CHS){
            case 12:
                CCPR1L = (ADRESH>>1)+124;//para que el servo1 pueda girar 180 g
                ADCON0bits.CHS = 10; //se cambia al canal del segundo pot
                break;
                
            case 10:
                CCPR2L = (ADRESH>>1)+124;//para que el servo0 pueda girar 180 g
                ADCON0bits.CHS = 8;//se cambia a canal del primer pot
                break;
            
            case 8:
                pot0 = numresets(ADRESH>>1);//para que el servo0 pueda girar 180
                ADCON0bits.CHS = 9;//se cambia a canal del primer pot
                break;
                
            case 9:
                pot1 = numresets(ADRESH>>1);//para que el servo0 pueda girar 180
                ADCON0bits.CHS = 12;//se cambia a canal del primer pot
                break;
            
        }
        __delay_us(50);   //delay de 50 us
        PIR1bits.ADIF = 0;//interrupcion de adc
        ADCON0bits.GO = 1;//inicio de la siguiente conversión
    }
    if (INTCONbits.T0IF == 1){
        TMR0 = 234; //para que el tmr0 se reinicie cada 22 us
        if (contpot0 == pot0){
            PORTDbits.RD0 = 0;
            hab0 = 0;
            contpot0 = 0;}
        if (contpot1 == pot1){
            PORTDbits.RD1 = 0;
            hab1 = 0;
            contpot1 = 0;}
        
        if (hab0 == 1){
            contpot0++;}
        
        if (hab1 == 1){
            contpot1++;}
      
        INTCONbits.T0IF = 0;
    }
    if (PIR1bits.TMR1IF == 1){
        TMR1L = 192; //reinicio cada 20 ms
        TMR1H = 99;
        PORTB = pot0;
        hab0 = 1;
        hab1 = 1;
       
        PORTDbits.RD0 = 1;
        PORTDbits.RD1 = 1;
       
        PIR1bits.TMR1IF = 0;}}

void main(void) {
    //configuraciones
    //configuracion reloj
    OSCCONbits.IRCF = 0b0111;//0111, Frecuencia de reloj 8 MHz
    OSCCONbits.SCS   = 1;//reloj interno
    //configuracion in out
    ANSELH = 0b11111111; //Pines digitales
    ANSEL=0;
    
    TRISA  = 0; //RA0 y RA1 como inputs y los demas como outputs
    TRISB  = 0xff;
    TRISC  = 0;
    TRISD  = 0;
    TRISE  = 0;
    
    PORTA  = 0;//se limpian los puertos
    PORTB  = 0;
    PORTC  = 0;
    PORTD  = 0;
    PORTE  = 0;
    contpot0 = 0;
    contpot1 = 0;
    
    pot0 = 0;
    pot1 = 0;
    
    //configuracion adc
    ADCON0bits.ADCS = 2;//10 se selecciona Fosc/32 para conversion 4us full TAD
    ADCON0bits.CHS0 = 0;//se selecciona el canal AN0
    ADCON1bits.VCFG1 = 0;//se ponen los voltajes de referencia internos del PIC
    ADCON1bits.VCFG0 = 0;//0V a 5V
    ADCON1bits.ADFM = 0; //se justifica a la izquierda, vals más significativos
    ADCON0bits.ADON = 1;//se enciende el adc
    __delay_us(50);   //delay de 50 us
    //configuracion pwm
    //ccp1
    TRISCbits.TRISC2 = 1;      //CCP1 como entrada;
    PR2 = 250;                 //valor para que el periodo pwm sea 2 ms 
    CCP1CONbits.P1M = 0;       //config pwm
    CCP1CONbits.CCP1M = 0b1100;
    CCPR1L = 0x0f;             //ciclo de trabajo inicial
    CCP1CONbits.DC1B = 0;
    //ccp2
    TRISCbits.TRISC1 = 1;      //CCP2 como entrada;
    CCP2CONbits.CCP2M = 0b1100;//config pwm
    CCPR2L = 0x0f;             //ciclo de trabajo inicial
    CCP2CONbits.DC2B1 = 0;
    //configuracion tmr0
    OPTION_REGbits.T0CS = 0; //reloj interno (low to high)
    OPTION_REGbits.PSA  = 0; //prescaler 
    OPTION_REGbits.PS   = 0; //2
    //reset tmr0
    TMR0 = 234; //para que el tmr0 se reinicie cada 22 us
    INTCONbits.T0IF = 0; //baja la bandera de interrupcion del tmr0
    //configuracion tmr1
    TMR1L = 192; //reinicio cada 20 ms
    TMR1H = 99;
    T1CONbits.T1CKPS = 0; //prescale 1
    T1CONbits.TMR1CS = 0;
    T1CONbits.TMR1ON = 1;
    PIR1bits.TMR1IF = 0;
    //configuracion tmr2
    PIR1bits.TMR2IF = 0; //se apaga la bandera de interrupcion del tmr2
    T2CONbits.T2CKPS = 0b11;//prescaler 1:16
    T2CONbits.TMR2ON = 1;//se enciende el tmr2
    while(PIR1bits.TMR2IF == 0);//esperar un ciclo de tmr2
    PIR1bits.TMR2IF = 0;
    TRISCbits.TRISC2 = 0;//out pwm2
    TRISCbits.TRISC1 = 0;//out pwm1
    //configuracion interrupciones
    INTCONbits.T0IE = 1;
    PIE1bits.TMR1IE = 1;
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;   //se habilitan las interrupciones por adc
    INTCONbits.PEIE = 1; //se habilitan las interrupciones de los perifericos
    INTCONbits.GIE  = 1; //se habilitan las interrupciones globales
    ADCON0bits.GO = 1;  //se comienza la conversion adc
    while (1){}      
}