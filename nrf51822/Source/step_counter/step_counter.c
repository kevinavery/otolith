#include <stdint.h>
#include <stdlib.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "app_gpiote.h"
#include "simple_uart.h"
#include "acc_driver.h"
#include "step_counter.h"
#include "util.h"



static measurements data;
static const int size =  SAMPLE_SIZE;
static acc_data_t acc_arr[size];
static int collected_data;

static app_gpiote_user_id_t  step_counter_gpiote_user;



int max_of(int a, int b) {
  return (a > b) ? a : b; 
}

int min_of(int a, int b) {
  return (a < b) ? a : b; 
}

void filter(acc_data_t * acc_data_array, int size)
{
  int i;
  for(i =0; i < size - 3; i++)
  {
    acc_data_array[i].x =  (acc_data_array[i].x + acc_data_array[i + 1].x + acc_data_array[i + 2].x + acc_data_array[i + 3].x) / 4;
    acc_data_array[i].y =  (acc_data_array[i].y + acc_data_array[i + 1].y + acc_data_array[i + 2].y + acc_data_array[i + 3].y) / 4;
    acc_data_array[i].z =  (acc_data_array[i].z + acc_data_array[i + 1].z + acc_data_array[i + 2].z + acc_data_array[i + 3].z) / 4;
  }
	acc_data_array[size- 3].x =  (acc_data_array[size- 3].x + acc_data_array[size- 2].x + acc_data_array[size- 1].x) / 3;
	acc_data_array[size- 3].y =  (acc_data_array[size- 3].y + acc_data_array[size- 2].y + acc_data_array[size- 1].y) / 3;
	acc_data_array[size- 3].z =  (acc_data_array[size- 3].z + acc_data_array[size- 2].z + acc_data_array[size- 1].z) / 3;
	
	acc_data_array[size- 2].x =  (acc_data_array[size- 2].x + acc_data_array[size- 1].x) / 2;
	acc_data_array[size- 2].y =  (acc_data_array[size- 2].y + acc_data_array[size- 1].y) / 2;
	acc_data_array[size- 2].z =  (acc_data_array[size- 2].z + acc_data_array[size- 1].z) / 2;	
	
}

void set_acc_data(acc_data_t *data, int x, int y, int z) {
  data->x = x;
  data->y = y;
  data->z = z;
}
int max_axis_offset(int dx, int dy, int dz)
{
  if(dx > dy) {
    if(dx > dz)
        return X;
      else
        return Z;
  } else {
    if(dy > dz)
      return Y;
    else
      return Z;
  }
}
void get_max_min(measurements *measure, acc_data_t *data, int size) {
  acc_data_t max, min;
  int i = 0;

  set_acc_data(&max, data[i].x,
                     data[i].y, 
                     data[i].z);
    
  set_acc_data(&min, data[i].x, 
                     data[i].y, 
                     data[i].z);
  
  for(i = 1; i < size; i++) {
    set_acc_data(&max, max_of(max.x, data[i].x),
                       max_of(max.y, data[i].y), 
                       max_of(max.z, data[i].z));
    
    set_acc_data(&min, min_of(min.x, data[i].x), 
                       min_of(min.y, data[i].y), 
                       min_of(min.z, data[i].z));
  }

  measure->axis = max_axis_offset(max.x - min.x, max.y - min.y, max.z - min.z);
  measure->max =  GET_FIELD(&max, measure->axis);
  measure->min =  GET_FIELD(&min, measure->axis);
  measure->threshold = (measure->max + measure->min) / 2;
	measure->precision = max_of(abs((measure->max - measure->min) / 8), MIN_PRECISION);
}

int get_steps(int steps) {
  float max_steps, min_steps;
  float secs = (SAMPLE_SIZE / SAMPLE_RATE);
  max_steps = MAX_STEP_FREQ * secs;
  min_steps = MIN_STEP_FREQ * secs;
 return (steps <= max_steps && steps >= min_steps) ? steps : 0;
}

int count_steps(measurements *measure, acc_data_t *acc_data_array, int size) {
  int i, last_sample_index, sample_above, above_taken, sample_below, below_taken, result, steps, thresh;
  sample_above = measure->min;
  above_taken  = 0;
  sample_below = measure->max;
  below_taken  = 0;
  thresh = measure->threshold;
  steps = 0;
  last_sample_index = 0;

  for(i = 0; i < size; i++) {
    result = GET_FIELD((acc_data_array + i), measure->axis);
    
    if((result > thresh && !above_taken) || (result > sample_above && above_taken)) {
      // take a sample above the thresh
      sample_above = result;
      above_taken = 1;

    } else if((result < thresh) && above_taken && (sample_above - result) > measure->precision) {
      sample_below = result;
      below_taken = 1;
    }
    if(below_taken && above_taken) {
      if(((sample_above - sample_below) > measure->precision)) { 
          if(((i - last_sample_index) >= MIN_SAMPlES_BETWEEN)) {
            steps++;
            last_sample_index = i;
         } 
      }
      above_taken = 0;
      below_taken = 0;
    }
  }

  return get_steps(steps);
}

int count_steps1(measurements *measure, acc_data_t *acc_data_array, int size) {
  int sample_new;
  int sample_old;
  int result;
  int i;
	int steps = 0;
  sample_new = GET_FIELD((acc_data_array), measure->axis);

  for(i = 0; i < size; i++) {
    result = GET_FIELD((acc_data_array + i), measure->axis);
    sample_old = sample_new;
    measure->interval++;
    if(abs(sample_new - result) > measure->precision) {
      sample_new = result;
    }
    
    if((sample_old > measure->threshold) && (sample_new < measure->threshold)) {
          if((measure->interval > 10) && (measure->interval < 100)) {
						if(measure->temp_steps < MIN_CONSECUTIVE_STEPS)	{
								measure->temp_steps++;
						} else if(measure->temp_steps == MIN_CONSECUTIVE_STEPS)	{
								steps += ++measure->temp_steps;
						} else if(measure->temp_steps > MIN_CONSECUTIVE_STEPS) {
								steps++;
						}
						measure->interval = 0;
          } else {
							if(measure->interval > 100) {
								measure->temp_steps = 0;
								measure->interval = 0;
							}
          }
    }
  }
  return steps;
}


void print_measure_data(measurements* measure) {
    mlog_print("STEPS: " , measure->total_steps);
    mlog_print(" T_STEPS: " , measure->temp_steps);
    mlog_print(" AXIS: ", measure->axis);
    mlog_print(" MAX: ", measure->max);
    mlog_print(" MIN: ", measure->min);
    mlog_print(" INTER: ", measure->interval);
    mlog_print(" THRESH: ", measure->threshold);
    mlog_print(" PREC: ", measure->precision);
    mlog_str("\r\n");
}

void print_csv(int num_step) {
  int i;
  for(i = 0; i < collected_data; i++)
  { 
    mlog_num(acc_arr[i].x);
    mlog_str(",");
    mlog_num(acc_arr[i].y);
    mlog_str(",");
    mlog_num(acc_arr[i].z);
    mlog_str(",");
    mlog_num(data.axis);
    mlog_str(",");
    mlog_num(data.max);
    mlog_str(",");
    mlog_num(data.min);
    mlog_str(",");
    mlog_num(data.threshold);
    mlog_str(",");
    mlog_num(data.precision);
    mlog_str(",");
    mlog_num(num_step);
    mlog_str("\r\n");
  }
}
void print_csv_header() {
    mlog_str("X");
    mlog_str(",");
    mlog_str("Y");
    mlog_str(",");
    mlog_str("Z");
    mlog_str(",");
    mlog_str("AXIS");
    mlog_str(",");
    mlog_str("MAX");
    mlog_str(",");
    mlog_str("MIN");
    mlog_str(",");
    mlog_str("THRESH");
    mlog_str(",");
    mlog_str("PREC");
    mlog_str(",");
    mlog_str("STEPS");
    mlog_str("\r\n");
}

void print_acc_data_array(acc_data_t* acc_data_array, int size) {
  int i;
  for(i = 0; i < size; i++) {
    mlog_print("x: ", acc_data_array[i].x);
    mlog_print(" y: ", acc_data_array[i].y);
    mlog_println(" z: ", acc_data_array[i].z);
  }
}

int fill_data(acc_data_t* acc_array) {
    int max, temp;
    if(collected_data >= SAMPLE_SIZE) {
        collected_data = 0;
    }
    
    temp = collected_data + FIFO_SAMPLES;
    max = (temp < SAMPLE_SIZE) ? temp : SAMPLE_SIZE;
    for(; collected_data < max; collected_data++) {
        update_acc_data(acc_array + collected_data);
    }
    if(collected_data >= SAMPLE_SIZE)
        return 1;
    return 0;
}

/** GPIOTE interrupt handler.
* Triggered on motion interrupt pin input low-to-high transition.
*/
// void GPIOTE_IRQHandler(void)
void on_fifo_full_event(uint32_t event_pins_low_to_high,
                                   uint32_t event_pins_high_to_low)
{
    if ((event_pins_low_to_high >> 7) & 1)
    {
        //simple_uart_putstring("Handling\r\n");
        int steps;
        if(fill_data(acc_arr)) {        
            filter(acc_arr, SAMPLE_SIZE); 
            get_max_min(&data, acc_arr, SAMPLE_SIZE);
            steps = count_steps1(&data, acc_arr, SAMPLE_SIZE);
            data.total_steps += steps;
            print_measure_data(&data);
        }
        // Event causing the interrupt must be cleared
        NRF_GPIOTE->EVENTS_IN[0] = 0;
    }
}

void step_counter_init()
{
    //NVIC_DisableIRQ(GPIOTE_IRQn);
    data.interval = 10;
    data.temp_steps = 0;
    //mlog_init();
    //mlog_str("Waiting for Key...\r\n");
    //uint8_t cr = simple_uart_get();
    //mlog_str("Starting after key...\r\n");

    uint32_t mask = 1 << 7;
    app_gpiote_user_register(&step_counter_gpiote_user, mask, 0, on_fifo_full_event);
    //NVIC_EnableIRQ(GPIOTE_IRQn);
    
    // Configure fifo interrupt pin
    nrf_gpio_cfg_input(FIFO_INTERRUPT_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
    
    // Configure GPIOTE channel 0 to generate event when 
    // MOTION_INTERRUPT_PIN_NUMBER goes from Low to High
    nrf_gpiote_event_config(0, FIFO_INTERRUPT_PIN_NUMBER, NRF_GPIOTE_POLARITY_LOTOHI);
    
    // Enable interrupt for NRF_GPIOTE->EVENTS_IN[0] event
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Msk;

    app_gpiote_user_enable(step_counter_gpiote_user);

    acc_init();
}




