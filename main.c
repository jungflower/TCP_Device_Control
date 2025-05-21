#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "device_control.h"

void print_help() {
    printf("Command list:\n");
    printf("  led_on                 - Turn LED on\n");
    printf("  led_off                - Turn LED off\n");
    printf("  led_brightness [0|1|2] - Set LED brightness (0=low,1=mid,2=high)\n");
    printf("  buzzer_on              - Turn buzzer on\n");
    printf("  buzzer_off             - Turn buzzer off\n");
    printf("  read_light             - Read light sensor value\n");
    printf("  auto_led               - Auto control LED based on light sensor\n");
    printf("  segment_disp [0-9]     - Display number on 7-segment\n");
    printf("  segment_countdown [n]  - Countdown on 7-segment from n\n");
    printf("  help                   - Show this help message\n");
    printf("  exit                   - Exit program\n");
}

int main() {

    if (wiringPiSetup() == -1) {
        printf("Failed to initialize wiringPi\n");
        return 1;
    }

    printf("Device Control CLI Test Started\n");
    print_help();

    char input[100];
    while (1) {
        printf("\nEnter command> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break; // Input error, exit loop
        }

        // Remove newline character
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "led_on") == 0) {
            led_on();
            printf("LED turned ON\n");
        }
        else if (strcmp(input, "led_off") == 0) {
            led_off();
            printf("LED turned OFF\n");
        }
        else if (strncmp(input, "led_brightness ", 15) == 0) {
            int level = input[15] - '0';
            if (level >= 0 && level <= 2) {
                led_set_brightness(level);
                printf("LED brightness set to %d\n", level);
            } else {
                printf("Invalid brightness level (0~2 allowed)\n");
            }
        }
        else if (strcmp(input, "buzzer_on") == 0) {
            buzzer_on();
            printf("Buzzer turned ON\n");
        }
        else if (strcmp(input, "buzzer_off") == 0) {
            buzzer_off();
            printf("Buzzer turned OFF\n");
        }
        else if (strcmp(input, "read_light") == 0) {
            int val = read_light_sensor();
            printf("Light sensor value: %d\n", val);
        }
        else if (strcmp(input, "auto_led") == 0) {
            auto_led_control_by_light();
            printf("Auto LED control by light sensor executed\n");
        }
        else if (strncmp(input, "segment_disp ", 13) == 0) {
            int num = input[13] - '0';
            if (num >= 0 && num <= 9) {
                segment_display(num);
                printf("7-segment displayed number %d\n", num);
            } else {
                printf("Number must be between 0 and 9\n");
            }
        }
        else if (strncmp(input, "segment_countdown ", 18) == 0) {
            int num = atoi(input + 18);
            if (num > 0) {
                segment_countdown(num);
                printf("7-segment countdown started from %d\n", num);
            } else {
                printf("Please enter a positive integer\n");
            }
        }
        else if (strcmp(input, "help") == 0) {
            print_help();
        }
        else if (strcmp(input, "exit") == 0) {
            printf("Exiting program\n");
            break;
        }
        else {
            printf("Unknown command. Type 'help' for the command list\n");
        }
    }

    return 0;
}