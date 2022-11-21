#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

void app_main(void)
{
    // printf("Timer: %lld μs\n", esp_timer_get_time());
    // printf("portTICK_PERIOD_MS :  %d\n", portTICK_PERIOD_MS);
    // printf("portTICK_RATE_MS : %d\n", portTICK_RATE_MS);

    printf("Timer for 1 millisecond\n");
  long long int Timer1 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer1);  
  vTaskDelay(1/ portTICK_PERIOD_MS);
  long long int Timer2 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer2);  
  float diff1 = Timer2 - Timer1;
  printf("Difference: %f ms\n", diff1/1000); 

  printf("\n");
  printf("Timer for 10 milliseconds\n");
  long long int Timer3 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer3);  
  vTaskDelay(10/ portTICK_PERIOD_MS);
  long long int Timer4 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer4);  
  float diff2 = Timer4 - Timer3;
  printf("Difference: %f ms\n", diff2/1000); 

  printf("\n");
  printf("Timer for 100 milliseconds\n");
  long long int Timer5 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer5);  
  vTaskDelay(100/ portTICK_PERIOD_MS);
  long long int Timer6 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer6);  
  float diff3 = Timer6 - Timer5;
  printf("Difference: %f ms\n", diff3/1000); 

  printf("\n");
  printf("Timer for 1000 milliseconds\n");
  long long int Timer7 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer7);  
  vTaskDelay(1000/ portTICK_PERIOD_MS);
  long long int Timer8 = esp_timer_get_time();
  printf("Timer: %lld μs\n", Timer8);  
  float diff4 = Timer8 - Timer7;
  printf("Difference: %f ms\n", diff4/1000); 
}
