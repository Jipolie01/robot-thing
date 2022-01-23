#include "base.hpp"
#include "esp_vfs_dev.h"
#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_vfs_fat.h"


void command_line_base::init_console(){
  fflush(stdout);
  fsync(fileno(stdout));
  setvbuf(stdin, NULL, _IONBF, 0);
  /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
  esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  /* Move the caret to the beginning of the next line on '\n' */
  esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
  const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .use_ref_tick = true
  };

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_length = 256,
            .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        linenoiseSetDumbMode(1);
    }
}

void command_line_base::parse_servo(std::string line){
  uint8_t index_first_bracket = line.find('[');
  uint8_t index_last_bracket = line.find(']');
  uint8_t index_comma = line.find(',');

  uint8_t servo_slot = std::stoi(line.substr(index_first_bracket+1, index_comma - index_first_bracket));
  uint8_t degrees = std::stoi(line.substr(index_comma+1, index_last_bracket - index_comma));
  
  //std::cout << std::endl << "Setting servo-slot: " << (int)servo_slot << " to " << (int)degrees << " degrees" << std::endl;
  queue_to_use->add({servo_slot, degrees}, TickType_t(100));
}

void command_line_base::parse_movement(std::string line){
  while(true){
    uint8_t index = line.find_first_of('[');
    if(line.find_first_of('[') == std::string::npos){ break; }
    uint8_t other_index = line.find_first_of(']');
    if(line.find_first_of(']') == std::string::npos){ break; }
    parse_servo(line.substr(index, (other_index - index) + 1));
    line.erase(0, other_index+1);
  }
}

command_line_base::command_line_base(queue<servo_command, 20> * queue_to_use):
  task("console"),
  prompt("[robot] >"),
  queue_to_use(queue_to_use)
{
  init_console();
}


void command_line_base::run(void * user_data){
  std::cout << "Hello! Internal robot command line at your service!\n";
  for(;;){
    char * line = linenoise(prompt.c_str());
    std::cout << std::endl << line << std::endl;

    if(std::string(line).find("servo") != std::string::npos){
      parse_servo(line);
    }else if(std::string(line).find("move") != std::string::npos){
      parse_movement(line);
    }

    if(line == NULL){continue; /*TODO: Check if continue also works*/}

    linenoiseFree(line);
    delay(200);
  }
}