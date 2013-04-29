#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "step_counter.h"
#include "util.h"

int max_of(int a, int b)
{
  return (a > b) ? a : b; 
}

int min_of(int a, int b)
{
  return (a < b) ? a : b; 
}

// void fake_acc_data_array(acc_data_t *acc_data_array, int size, float freq) {
//   int i;
//   for(i = 0; i < size; i++) {
//     acc_data_array[i].x = 512.0 * sin((freq/SAMPLE_RATE) * i * 2 * PI);
//     acc_data_array[i].y = 256.0 * sin((freq/SAMPLE_RATE) * i * 2 * PI);
//     acc_data_array[i].z = 128.0 * sin((freq/SAMPLE_RATE) * i * 2 * PI);
//   }
// }
 
void filter(acc_data_t * acc_data_array, int size)
{
  int i;
  for(i =0; i < size - 3; i++)
  {
    acc_data_array[i].x =  (acc_data_array[i].x + acc_data_array[i + 1].x + acc_data_array[i + 2].x + acc_data_array[i + 3].x) >> 2;
    acc_data_array[i].y =  (acc_data_array[i].y + acc_data_array[i + 1].y + acc_data_array[i + 2].y + acc_data_array[i + 3].y) >> 2;
    acc_data_array[i].z =  (acc_data_array[i].z + acc_data_array[i + 1].z + acc_data_array[i + 2].z + acc_data_array[i + 3].z) >> 2;
  }
}

// void print_acc_data_array(acc_data_t* acc_data_array, int size) {
//   int i;
//   for(i = 0; i < size; i++) {
//     printf("x: %d ", GET_FIELD((acc_data_array + i), X));
//     printf("y: %d ", GET_FIELD((acc_data_array + i), Y));
//     printf("z: %d \n", GET_FIELD((acc_data_array + i), Z));
//   }
// }

// void print_measure_data(measurements* measure) {
//     printf("AXIS: %d", measure->axis);
//     printf(" MAX: %d", measure->max);
//     printf(" MIN: %d", measure->min);
//     printf("  THRESHOLD: %d", measure->threshold);
//     printf("  PRECISION: %d\n", measure->precision);
// }

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
  measure->threshold = (measure->max + measure->min) >> 1;
  measure->precision = abs(((measure->max - measure->min) >> 4));
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

    }// else {
    //   printf("Below Precision: \r\n");
    // }
    
    if(below_taken && above_taken) {
      //if(((sample_above - sample_below) > measure->precision)) { 
          if(((i - last_sample_index) >= MIN_SAMPlES_BETWEEN)) {
            steps++;
            last_sample_index = i;
          } else {
            printf("Below MIN SAMPLES BETWEEN: \r\n");
          }
      //}// else {
      //  printf("Below Precision: \r\n");
      // }
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
  int interval = 10;
  int steps = 0;
  int i;
  //bool consec = true;
  sample_new = GET_FIELD((acc_data_array), measure->axis);

  for(i = 1; i < size; i++) {
    result = GET_FIELD((acc_data_array + i), measure->axis);
    (acc_data_array + i)->step_location = 0; //csv stuff
    sample_old = sample_new;
    interval++;
    if(abs(sample_new - result) > measure->precision) {
      sample_new = result;
    }
    
    if((sample_old > measure->threshold) && (sample_new < measure->threshold)) {
          if((interval > 10) && (interval < 100)) {            
            steps++;
            (acc_data_array + i)->step_location = result; //csv stuff
            interval = 0;
          } else {
            // printf("INVALID interval: %d\r\n", interval);
          }
    }
  }

  return get_steps(steps);
}