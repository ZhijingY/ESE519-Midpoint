#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "pio_i2c.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

#include "ws2812.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "registers.h"

// #include "sequencer.h"

#include <stdlib.h>
#include "hardware/structs/bus_ctrl.h"

const uint TUBE_PIN_1 = 22;
const uint TUBE_PIN_2 = 22;
const uint DRAW_PIN = 21;
const uint MOVE_PIN = 23;
const uint SDA_PIN = 22;
const uint SCL_PIN = 23;


const int addr = 0x39;
PIO pio = pio0;
uint sm = 1;

enum States
{
    IDLE,
    ADJUST,
    CYCLE_DOWN,
    DRAW,
    CYCLE_UP,
    END
};

struct Drinks {
   int ingre_num;
   int ingredient[4];
   int position[4];
};

static void APDS9960_reset() {
    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    uint8_t buf[] = {0x80, 0x27};
    pio_i2c_write_blocking(pio, sm, addr, buf, 2, false);
}

static void receive_command(uint8_t *command) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    uint8_t buffer[1];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    uint8_t val = 0x9C;
    pio_i2c_write_blocking(pio, sm, addr, &val, 1, true); // true to keep master control of bus
    pio_i2c_read_blocking(pio, sm, addr, buffer, 1, false);

    *command = buffer[0];
}

void move_tube(int distance_unit) {
    gpio_put(MOVE_PIN, 1);
    sleep_ms(distance_unit);
    gpio_put(MOVE_PIN, 0);
}

void drop_tube(int mode) {
    if(mode == 1) {
        gpio_put(TUBE_PIN_1, 0);
        gpio_put(TUBE_PIN_2, 1);
        sleep_ms(5000);
        gpio_put(TUBE_PIN_2, 0);
    } else {
        gpio_put(TUBE_PIN_1, 1);
        gpio_put(TUBE_PIN_2, 0);
        sleep_ms(5000);
        gpio_put(TUBE_PIN_1, 0);
    }
}

void suck(int amount) {
    // Keep the pump on for a amount of time
    gpio_put(DRAW_PIN, 1);
    sleep_ms(amount);
    gpio_put(DRAW_PIN, 0);
}

void main_core1() {
    // ...
}

int main() {

    stdio_init_all();
    uint offset = pio_add_program(pio, &i2c_program);
    i2c_program_init(pio, sm, offset, SDA_PIN, SCL_PIN);
    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C))

    gpio_init(TUBE_PIN_1);
    gpio_init(TUBE_PIN_2);
    gpio_set_dir(TUBE_PIN_1, GPIO_OUT);
    gpio_set_dir(TUBE_PIN_2, GPIO_OUT);
    gpio_put(TUBE_PIN_1, 0);
    gpio_put(TUBE_PIN_2, 0);

    gpio_init(DRAW_PIN);
    gpio_set_dir(DRAW_PIN, GPIO_OUT);
    gpio_put(DRAW_PIN, 0);

    gpio_init(MOVE_PIN);
    gpio_set_dir(MOVE_PIN, GPIO_OUT);
    gpio_put(MOVE_PIN, 0);

    enum States state = IDLE;
    enum States next_state = state;
    int command = 0;
    int pos_before = 0;
    struct Drinks drink1;
    struct Drinks drink2;
    int iteration = 0;

    drink1.ingre_num = 3;
    drink1.ingredient[0] = 30;
    drink1.ingredient[1] = 20;
    drink1.ingredient[2] = 30;
    drink1.position[0] = 1;
    drink1.position[1] = 2;
    drink1.position[2] = 3;

    drink2.ingre_num = 2;
    drink2.ingredient[0] = 10;
    drink2.ingredient[1] = 30;
    drink2.position[0] = 3;
    drink2.position[1] = 2;

    while (true) {     
        if(state == IDLE) {receive_command(&command);}

        switch (state)
        {
        case IDLE:
            if(command != 0) {
                next_state = ADJUST
            }
            break;
        
        case ADJUST:
            if(iteration == 0) {
                pos_before = 0;
            } else {
                pos_before = (command == 1) ? drink1.position[iteration-1] : drink2.position[iteration-1];
            }

            if(command == 1) {
                move_tube(drink1.position[iteration] - pos_before); // move the tube by fixed distance
            } else if(command == 2) {
                move_tube(drink2.position[iteration] - pos_before); // move the tube by fixed distance
            }

            next_state = CYCLE_DOWN;

            break;

        case CYCLE_DOWN:
            drop_tube(1);
            next_state = DRAW;

            break;

        case DRAW:
            if(command == 1) {
                suck(drink1.ingredient[iteration]); // move the tube by fixed distance
            } else if(command == 2) {
                suck(drink2.ingredient[iteration]); // move the tube by fixed distance
            }
            next_state = CYCLE_UP;

            break;

        case CYCLE_UP:
            drop_tube(-1);
            next_state = END;

            break;

        case END:
            if(command == 1) {
                next_state = (iteration < drink1.ingre_num - 1) ? ADJUST : IDLE;
            } else if(command == 2) {
                next_state = (iteration < drink2.ingre_num - 1) ? ADJUST : IDLE;
            }
            iteration++;
            sleep_ms(100);
            break;

        default:
            break;
        }
        state = next_state;
    }
}
