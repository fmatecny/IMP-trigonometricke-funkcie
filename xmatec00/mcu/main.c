/**
 * main.c - trichonometricke funkcie (sin, cos, tang)
 * Autor zmien: Fratnisek Matecny, xmatec00
 * Podiel zmien: 80%
 * Uskutocnene zmeny: vytvorenie funkcie signed_degree(), cordic()
 * 			    upravenie funkcie keyboard_idle() - povodne funkcia vypisovala nacitane znaky na displej
 *			    funkcia main() bola zachovana
 **/
/*******************************************************************************
   main.c: LCD + keyboard demo
   Copyright (C) 2012 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Michal Bidlo <bidlom AT fit.vutbr.cz>

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$


*******************************************************************************/

#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>
#include <stdio.h>

#define CORDIC_N 16		  //16 iteracii

#define cordic_1K 0x000026DD  //konstanta K
#define pi 0x0000C90F		  //pi
#define MUL 16384.000000	  //2^14
//degree*PI/180*MUL
//degree*0x0000011b
//tabulka 
int cordic_ctab [] = {0x00003243, 0x00001DAC, 0x00000FAD, 0x000007F5, 
					  0x000003FE, 0x000001FF, 0x000000FF, 0x0000007F, 
					  0x0000003F, 0x0000001F, 0x0000000F, 0x00000007, 
					  0x00000003, 0x00000001, 0x00000000, 0x00000000, };

char last_ch = 0;		//naposledy nacitany znak
char char_cnt = 0;		//pocet znakov na displeji
char fce = 0;			//funkcia - sin, cos alebo tang
unsigned int num = 0;	//nacitany uhol
int signd = 1;			//znamienko

void print_user_help(void) { }

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

void fpga_initialized() { }


/**
 * funkcia nastavi znamienko vysledku
 * vypocet uhla do I. kvadrantu t.j. <0, 90>
 */
void signed_degree(){

	signd = 1;

	//rotácia uhlu tak aby bol medzi 0-360 stupnov
	for (;num > 360; num -= 360);

	switch (fce){
		
		//sinus
		case 'A':
					if (num > 180)
					{
						num -= 180;
						signd = -1;
					}		
		
					if (num > 90)
					{
						num = 90 - (num-90);
					}
					break;
		
		//cosinus
		case 'B': 	
					if (num > 180)
					{
						num -= 180;
						signd *= -1;
					}		
		
					if (num > 90)
					{
						num = 90 - (num-90);
						signd *= -1;
					}
					break;
		
		//tangens			
		case 'C':
					if (num > 180)
					{
						num -= 180;
						signd = 1;
					}		
		
					if (num > 90)
					{
						num = 90 - (num-90);
						signd *= -1;
					}
					break;
					
		default: 
				break;
	}
}

/**
 * algoritmus cordic - Q2.14
 */
void cordic(unsigned int degree)
{
  char abc[16];
unsigned int foo = 0;

  //prepocet uhlu na radiany a prevod do Q2.14
  degree = degree*3.141593/180*MUL;


  int n = 16;
  int k, tx, ty, tz;
  int x=cordic_1K,y=0,z=degree;
	
  n = (n>CORDIC_N) ? CORDIC_N : n;
  
  //cordic
  for (k = 0; k < n; ++k)
  {

   if (z < 0)
   	{
	tx = x + (y>>k);
    	ty = y - (x>>k);
    	tz = z + (cordic_ctab[k]);
	}
   else{
    	tx = x - (y>>k);
    	ty = y + (x>>k);
    	tz = z - (cordic_ctab[k]);
	}

    x = tx; y = ty; z = tz;
  }  

	//formatovanie vypisu sinusu
	if (fce == 'A')
	{
		
		if (y>10736)	//redukcia proti preteceniu
		{
			y = y/10;
			sprintf(abc,"%d.%04u", y/16384,(unsigned int) (y%16384)*100000/16383);
		}
		else {
			sprintf(abc,"%d.%05u", y/16384,(unsigned int) (y%16384)*100000/16383);
			}
			
	}//formatovanie vypisu cosinusu
	else if (fce == 'B')
	{
		
		if(x>10736)	//redukcia proti preteceniu
		{
			x = x/10;
			sprintf(abc,"%d.%04u", x/16384, (unsigned int) x%16384*100000/16383);
		}
		else {
			sprintf(abc,"%d.%05u", x/16384, (unsigned int) x%16384*100000/16383);
			}
			
	}//formatovanie vypisu tangensu
	else {
		
			if (num > 33)	//redukcia proti preteceniu
			{
				foo = (unsigned int) ((y%16384)/10)*100000/x;
				if (foo > 9999){foo = foo - (y/x)*10000;}

				sprintf(abc,"%d.%04u", y/x, foo);
			}
			else {
				sprintf(abc,"%d.%05u", y/x, (unsigned int) (y%16384)*100000/x);
				}
				
		}

	//vypis znamienka
	if (signd < 0) {LCD_append_char('-');}
	
	//vypis na dislej
	LCD_append_string(abc);
}

/**
 * funkcia nacitava znaky zadane na klavesnici Fitkitu
 */
int keyboard_idle()
{
  char ch;
  ch = key_decode(read_word_keyboard_4x4());		//nacitanie a dekodovanie znaku
  if (ch != last_ch) // osetrenie drzania tracitka
  {
    last_ch = ch;
    if (ch != 0) // ak bolo stlacene tlacitko
    {
      if ((char_cnt > 15) || (fce == 0)) {		//vymazanie displeju
         LCD_clear();
         char_cnt = 0;
      }
	
	if (ch == '*'){				//vymazanie displeja a reset hodnot
		LCD_clear();
         	char_cnt = 0;
		fce = 0;
		num = 0;
		}

	if (fce == 0)		//urcenie funkcie - sin, cos, tang
	{
		if (ch == 'A') {
			LCD_append_string("sin(");
			char_cnt += 4;
			fce = 'A';
		}
		else if (ch == 'B') {
			LCD_append_string("cos(");
			char_cnt += 4;
			fce = 'B';
		}
		else if (ch == 'C') {
			LCD_append_string("tang(");
			char_cnt += 6;
			fce = 'C';
		}
	}
	else if (ch == '#') //rovna sa
		{
			LCD_clear();
			
			//osetrenie hodnot tangensu
			if ((fce == 'C') && ((num%90) == 0))
				{
				LCD_append_string("error");
				}
				else {
					signed_degree();		//urcenie znamienka
					cordic(num);			//vypocet
				}
				
			//nulovanie premennych do pociatocneho stavu
			fce = 0;
			num = 0;
		}
	else if ((ch >= '0') && (ch <= '9'))		//nacitanie cisel(stupnov) z klavesnice
		{	
			LCD_append_char(ch);
			if (num == 0)
			{
				num = ch-48;
			}
			else {
				num = num*10 + (ch-48);
				}
		}
	else if (fce != 0)		//ak je zadane nieco ine ako cislo
		{		
			LCD_clear();
			char_cnt = 0;
			LCD_append_string("zadaj len cisla");
			num = 0;
			fce = 0;
		}
	
    }
  }
  
  return 0;
}

/**
 * Hlavna funkcia
 */
int main(void)
{

  initialize_hardware();
  keyboard_init();
  LCD_init();
  LCD_clear();

  while (1)
  {
    keyboard_idle();  // obsluha klavesnice
    terminal_idle();  // obsluha terminalu
  }         
}