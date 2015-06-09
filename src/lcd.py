import pylcdlib  
lcd = pylcdlib.lcd(0x21,0)  
lcd.lcd_puts("Raspberry Pi",1)  #display "Raspberry Pi" on line 1  
lcd.lcd_puts("  Take a byte!",2)  #display "Take a byte!" on line 2  