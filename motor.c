/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>

#include "pico/stdlib.h" // standard c librarry from pico
#include "hardware/pio.h" // chip specific library
#include "hardware/clocks.h" // chip specific library


// int main() {

//     stdio_init_all();

//     gpio_init(29);
//     gpio_init(28);
//     gpio_set_dir(29, GPIO_OUT);
//     gpio_set_dir(28, GPIO_OUT);
//     gpio_put(29, 0);
//     gpio_put(28, 0);

//     while(1){
//         gpio_put(29, 0);    
//         gpio_put(28, 1);
//         sleep_ms(5000);

//         gpio_put(29, 1);
//         gpio_put(28, 0);
//         sleep_ms(5000);

//         gpio_put(29, 0);
//         gpio_put(28, 0);
//         sleep_ms(5000);
//     }

//     return 0;
// }

//50HZ, 4.8V-6V, need level shifter, 
//duty is double type value between 0 and 1
void setPWM(double duty, int gpio_port, int activeTime){
    int time = (int)((duty+1)*1000);
    
    int i = 0;
    for(i;i<(activeTime/20);i++){
        gpio_put(gpio_port, 1);
        sleep_us(time);
        gpio_put(gpio_port, 0);
        sleep_us(20000-time);
    }
}

void setPWM(double duty, int gpio_port){
    int time = (int)((duty+1)*1000);
    
    gpio_put(gpio_port, 1);
    sleep_us(time);
    gpio_put(gpio_port, 0);
    sleep_us(20000-time);
    
}

int main() {

    stdio_init_all();

    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 0);

    int counter = 150;
    int i = 0;

    while(1){  

        
        i = 0;
        for(i;i<counter;i++){
            //60hz square wave
            setPWM(0.3, 0);
        }
        sleep_ms(3000);

        i = 0;
        for(i;i<counter;i++){
            //60hz square wave
            setPWM(0.6, 0);
        }
        sleep_ms(3000);

        i = 0;
        for(i;i<counter;i++){
            //60hz square wave
            setPWM(1, 0);
        }
        sleep_ms(3000);


    }

    return 0;
}


