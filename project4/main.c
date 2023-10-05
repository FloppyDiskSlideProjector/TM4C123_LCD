#define RCGCGPIO (*((volatile unsigned long *) 0x400FE608))

//LCD
//PORTA 2,3,4
#define GPIODEN_PORTA (*((volatile unsigned long *) 0x4000451C))
#define GPIODIR_PORTA (*((volatile unsigned long *) 0x40004400))
#define GPIODATA_PORTA (*((volatile unsigned long*) 0x40004070))//1110000

//PORTB 0,1,2,3,4,5,6,7
#define GPIODEN_PORTB (*((volatile unsigned long *) 0x4000551C))
#define GPIODIR_PORTB (*((volatile unsigned long *) 0x40005400))
#define GPIODATA_PORTB (*((volatile unsigned long *) 0x400053FC))//1111111100

//instruction, put this into portB
#define LCD_clear_display 0x01
#define LCD_return_home 0x02
#define LCD_display_shift_right 0x05
#define LCD_cursor_line1_position0 0x80//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#define LCD_function_set 0x38 //110011 1,8bit,0line,0x8,1,1
#define LCD_display_cursor_blink 0x0F //display: On, cursor: On, blink: Off
#define LCD_display_cursor_blink_init 0x08
#define LCD_entry_mode_set 0x6 //no shift, cursor move right
#define LCD_cursor_line2_position 0xC0 //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


void LCD_setup();
void LCD_write_IR(unsigned char inst);
void LCD_write_DR(unsigned char data);
void LCD_print_int(int data);
void Delay(unsigned int t);


//////////////////////////////////////////////////////////////////
//keypad

unsigned char keypad[4][4] = {{'1' ,'2' ,'3' ,'A' }, {'4' ,'5' ,'6' ,'B' }, {'7' ,'8' ,'9' ,'C' }, {'*' ,'0' ,'#' ,'D' }};
#define GPIOCR_PORTE (*((volatile unsigned long *) 0x40024524))
#define GPIOCR_PORTC (*((volatile unsigned long *) 0x40006524))
#define GPIOPDR_PORTE (*((volatile unsigned long *) 0x40024514))
#define GPIOPDR_PORTC (*((volatile unsigned long *) 0x40006514))
#define GPIOEN_PORTE (*((volatile unsigned long *) 0x4002451C))
#define GPIOEN_PORTC (*((volatile unsigned long *) 0x4000651C))
#define GPIODATA_PORTE (*((volatile unsigned long *) 0x4002403C))
#define GPIODATA_PORTC (*((volatile unsigned long *) 0x400063C0))
#define GPIOLOCK_PORTE (*((volatile unsigned long *) 0x40024520))
#define GPIOLOCK_PORTC (*((volatile unsigned long *) 0x40006520))
#define GPIODIR_PORTE  (*((volatile unsigned long *) 0x40024400))
#define GPIODIR_PORTC  (*((volatile unsigned long *) 0x40006400))


void KEY_PAD_setup();
char KEY_PAD_current_key();
char KEY_PAD_input();

///////////////////////////////////////////////////////////////
#define INITIAL_STATE 0
#define A_STATE 1
#define B_STATE 2
#define DISPLAY_STATE 3



/**
 * main.c
 */
int main(void)
{
    LCD_setup();
    KEY_PAD_setup();
    char state;
    char input;
    int A = 0;
    int B = 0;
    int i;

    while(1){
        switch(state){
        case INITIAL_STATE:
            A = 0;
            B = 0;
            LCD_write_IR(LCD_clear_display);
            LCD_write_IR(LCD_return_home);
            state = A_STATE;
            break;

        case A_STATE:
            for(i=0;i<8;++i){
                input = KEY_PAD_input();
                state = B_STATE;
                switch(input){
                case '*':
                    i = 8;//break the for loop
                    break;

                case '1' ... '9':
                    A = A*10 + (input - '0');
                    LCD_write_DR(input);
                    break;

                case 'C':
                    state = INITIAL_STATE;
                    i = 8;
                    break;

                case '#':
                    state = DISPLAY_STATE;
                    i = 8;
                    break;

                default:
                    --i;//ignore
                    break;
                }
            }
            break;

        case B_STATE:
            //clear top row @@@@@@@@@@@@@@@@@@
            LCD_write_IR(LCD_clear_display);
            LCD_write_IR(LCD_return_home);
            for(i=0;i<8;++i){
                input = KEY_PAD_input();
                state = DISPLAY_STATE;
                switch(input){
                case '#':
                    i = 8;
                    break;

                case '1' ... '9':
                    B = B*10 + (input - '0');
                    LCD_write_DR(input);
                    break;

                case 'C':
                    state = INITIAL_STATE;
                    break;

                default:
                    --i;//ignore
                    break;
                }

            }
            break;


        case DISPLAY_STATE:
            //move cursor to botton left
            //print
            //wait for input 'C'
            LCD_write_IR(LCD_cursor_line2_position);
            LCD_print_int(A*B);
            while(KEY_PAD_input()!='C');
            state = INITIAL_STATE;
            break;
        }
	}
}

void LCD_setup(){
    RCGCGPIO |= 0x3;//000011 Port A and Port B
    GPIODEN_PORTA = 0x1C;//11100
    GPIODEN_PORTB = 0xFF;//11111111
    GPIODIR_PORTA = 0x1C;//11100
    GPIODIR_PORTB = 0xFF;//11111111

    GPIODATA_PORTA = 0x00;
    GPIODATA_PORTB = 0x00;
    Delay(200);

    GPIODATA_PORTA = 0x10;//EN:1, R/W:0, RS:0, 0, 0
    GPIODATA_PORTB = LCD_function_set;
    Delay(5);
    GPIODATA_PORTA = 0x00;
    GPIODATA_PORTB = 0x00;
    Delay(5);
    LCD_write_IR(LCD_function_set);
    LCD_write_IR(LCD_function_set);
    LCD_write_IR(LCD_function_set);
    LCD_write_IR(LCD_display_cursor_blink_init);
    LCD_write_IR(LCD_clear_display);
    LCD_write_IR(LCD_entry_mode_set);
    LCD_write_IR(LCD_display_cursor_blink);
    LCD_write_IR(LCD_cursor_line1_position0);
}

void Delay(unsigned int t){//ms delay
    int i;
    int j;
    for(i=0;i<t;i++){
        for(j=0;j<16000;++j){
        }
    }
}

void LCD_write_IR(unsigned char inst){
    GPIODATA_PORTA = 0x10;//EN:1, R/W:0, RS:0, 0, 0
    GPIODATA_PORTB = inst;
    Delay(5);
    GPIODATA_PORTA = 0x00;
    GPIODATA_PORTB = 0x00;
    Delay(5);

    /*
    if(inst < 4){//clear display
        Delay(2);
    }else{
        Delay(1)//entry mode set
    }
    */
}

void LCD_write_DR(unsigned char data){
    GPIODATA_PORTA = 0x14;//10100
    GPIODATA_PORTB = data;
    Delay(2);
    GPIODATA_PORTA = 0x00;
    GPIODATA_PORTB = 0x00;
    Delay(2);
}

void LCD_print_int(int data){
    char str[16];

    int i = 0;
    int j = 0;
    while(data != 0){
        str[i] = data%10+'0';
        data = data/10;
        ++i;
    }
    for(j = i-1;j>=0;--j){
        LCD_write_DR(str[j]);
    }
}

void KEY_PAD_setup(){
    RCGCGPIO |= 0x14; //port E and C
    GPIOCR_PORTE |= 0x0F;
    GPIOCR_PORTC |= 0xF0;
    GPIODIR_PORTE |= 0x0F;
    GPIODIR_PORTC |= 0x00;
    GPIOPDR_PORTE |= 0x0F;
    GPIOPDR_PORTC |= 0xF0;
    GPIOEN_PORTE |= 0x0F;
    GPIOEN_PORTC |= 0xF0;
}

char KEY_PAD_current_key(){
    int i = 0;
    int j = 0;
    for(i = 0; i < 4; i++){
        GPIODATA_PORTE = (0x01 << i);
        for(j = 0; j < 4; j++){
            if((GPIODATA_PORTC & 0xF0) == (0x10 << j))
                return keypad[i][j];
        }
    }
    return '\0';
}

char KEY_PAD_input(){
    char c;
    while(KEY_PAD_current_key()=='\0');
    c = KEY_PAD_current_key();
    while(KEY_PAD_current_key()!='\0');
    return c;
}
