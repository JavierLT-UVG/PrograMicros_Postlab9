/*
 * Archivo:         Prelab8.c
 * Dispositivo:     PIC16F887
 * Autor:           Francisco Javier López
 *
 * Programa:        Control de 2 servos por medio de potenciómetros
 * Hardware:        2 Pot en PortE, 2 Servo en PortC
 * 
 * Creado: 10 de octubre de 2021
 * Última Modificación: 10 de octubre de 2021
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 8000000

//============================================================================
//============================ VARIABLES GLOBALES ============================
//============================================================================
uint8_t cont;                   // Variable contadora 
uint8_t adresh_aux;             // Variable auxiliar

//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void config_tmr0(void);
void config_adc(void);
void config_pwm1(void);
void config_pwm2(void);
void config_int(void);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    if(PIR1bits.ADIF)         // Si la bandera está encendida, entrar
    {
        if(ADCON0bits.CHS == 5)                 // Canal es 5
        {
            CCPR1L = (ADRESH>>1)+118;
            CCP1CONbits.DC1B1 = ADRESH & 0b01;
            CCP1CONbits.DC1B0 = (ADRESH>>7);
        }else if(ADCON0bits.CHS == 6)           // Canal es 6
        {
            CCPR2L = (ADRESH>>1)+118;           //124
            CCP1CONbits.DC1B1 = ADRESH & 0b01;
            CCP1CONbits.DC1B0 = (ADRESH>>7);
        }else                                   // Canal es 7
        {
            adresh_aux = ADRESH;
        }
        PIR1bits.ADIF = 0;
    }else if(INTCONbits.T0IF)    // Si la bandera está encendida, entrar
    {
        cont = cont + 1;
        
        if (cont >= adresh_aux)
        {
            PORTDbits.RD0 = 0;
        } else
        {
            PORTDbits.RD0 = 1;
        }
        
        TMR0 = 225;
        INTCONbits.T0IF = 0;   
    }
}

//============================================================================
//=================================== MAIN ===================================
//============================================================================
void main(void) 
{
    config_io();
    config_reloj();
    config_adc();
    config_pwm1();
    config_pwm2();
    config_int();
    config_tmr0();
    
    
    ADCON0bits.GO = 1;                  // Activar ciclo de conversión
    while(1)
    {
        if(ADCON0bits.GO == 0)          // Si la última conversión ya terminó, entrar
        {
            if(ADCON0bits.CHS == 5)     // Si el último canal fue 5, cambiar a 6
                ADCON0bits.CHS = 6;
            else if(ADCON0bits.CHS == 6) // Si el último canal fue 6, cambiar a 7
                ADCON0bits.CHS = 7;
            else                        // Si no fue ni 5 ni 6 (7), cambiar a 5
                ADCON0bits.CHS = 5;     
            
            __delay_us(50);             // Delay para no interrumpir conversión
            ADCON0bits.GO = 1;          // Iniciar nueva conversión
        }
        
    }
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)            // Configuración de entradas y salidas
{
    ANSEL   =   0b11100000;     // Puertos analógicos (AN7, AN6 y AN5)
    ANSELH  =   0;              // Puertos digitales
    
    TRISD   =   0;              // PuertoD como salida 
    PORTD   =   0;
    return;
}

void config_reloj(void)         // Configuración del oscilador
{
    OSCCONbits.IRCF2 = 1;       // 8MHz
    OSCCONbits.IRCF1 = 1;       // 
    OSCCONbits.IRCF0 = 1;       // 
    OSCCONbits.SCS = 1;         // Reloj interno
    return;
}

void config_tmr0(void)
{
    OPTION_REGbits.T0CS = 0;  // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
    OPTION_REGbits.T0SE = 0;  // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
    OPTION_REGbits.PSA = 0;   // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the Timer0
    OPTION_REGbits.PS2 = 0;   // bits 2-0  PS2:PS0: Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS0 = 0;
    TMR0 = 225;             // preset for timer register
    return;
}

void config_adc(void)
{
    ADCON1bits.ADFM = 0;        // Justificación a la izquierda
    ADCON1bits.VCFG0 = 0;       // Vss como referencia
    ADCON1bits.VCFG1 = 0;       // Vdd como referencia
    
    ADCON0bits.ADCS = 0b10;     // Fosc/32
    ADCON0bits.CHS = 5;         // Selección del canal 5
    ADCON0bits.ADON = 1;        // ADC encendido
    __delay_us(50);             // Delay de 50us
    return;
}

void config_pwm1(void)
{
    TRISCbits.TRISC2 = 1;           // RC2/CCP1 como entrada
    PR2 = 250;                      // Periodo
    CCP1CONbits.P1M = 0;
    CCP1CONbits.CCP1M = 0b1100;
    
    CCPR1L = 0b1111;                //  Duty cycle  
    CCP1CONbits.DC1B = 0;
    
    PIR1bits.TMR2IF = 0;            // TMR2
    T2CONbits.T2CKPS = 0b11;
    T2CONbits.TMR2ON = 1;
    
    while(PIR1bits.TMR2IF == 0);
    PIR1bits.TMR2IF = 0;
    
    TRISCbits.TRISC2 = 0;           // RC2 como salida
}

void config_pwm2(void)
{
    TRISCbits.TRISC1 = 1;           // RC1/CCP2 como entrada
    CCP2CONbits.CCP2M = 0b1100;
    
    CCPR2L = 0b1111;                //  Duty cycle  
    CCP2CONbits.DC2B0 = 0;
    CCP2CONbits.DC2B1 = 0;
    
    TRISCbits.TRISC1 = 0;           // RC1 como salida
}

void config_int(void)           // Configuración de interrupciones
{
    INTCONbits.GIE  = 1;        // Activar interrupciones
    INTCONbits.PEIE = 1;        // Activar interrupciones periféricas
    INTCONbits.T0IE = 1;        // Activar interrupcion de Tmr0
    INTCONbits.TMR0IF = 0;
    PIE1bits.ADIE = 1;          // Activar interrupción de ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera de ADC
    return;
}