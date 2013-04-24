#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SAMPLE_SIZE 25
#define SAMPLE_RATE 50
#define PI 3.14159265
#define X 0
#define Y 4
#define Z 8
#define GET_FIELD(acc_data_p, field) *((int*)((char*)(acc_data_p) + field))
#define MAX_STEP_FREQ 5.0
#define MIN_STEP_FREQ 0.5


// data array contains an array of structs
typedef struct {
  int x;
  int y;
  int z;
} acc_data_t;

typedef struct {
  int threshold;
  int max;
  int min;
  int axis;
} measurements;

inline int max_of(int a, int b)
{
  return (a > b) ? a : b; 
}

inline int min_of(int a, int b)
{
  return (a < b) ? a : b; 
}

void fake_acc_data_array(acc_data_t *acc_data_array, int size, double freq) {
  int i;
  for(i = 0; i < size; i++) {
    acc_data_array[i].x = 512.0 * sin((freq/SAMPLE_RATE) * i * 2 * PI);
    acc_data_array[i].y = 256.0 * sin((freq/SAMPLE_RATE) * i * 2 * PI);
    acc_data_array[i].z = 128.0 * sin((freq/SAMPLE_RATE) * i * 2 * PI);
  }
}
 
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

void print_acc_data_array(acc_data_t* acc_data_array, int size) {
  int i;
  for(i = 0; i < size; i++) {
    printf("x: %d ", GET_FIELD((acc_data_array + i), X));
    printf("y: %d ", GET_FIELD((acc_data_array + i), Y));
    printf("z: %d \n", GET_FIELD((acc_data_array + i), Z));
  } 
}

void print_measure_data(measurements* measure) {
    printf("AXIS: %d ", measure->axis);
    printf("MAX: %d ", measure->max);
    printf("MIN: %d \n", measure->min);
    printf("THRESHOLD: %d \n", measure->threshold);
}

void set_acc_data(acc_data_t *data, int x, int y, int z) {
  data->x = x;
  data->y = y;
  data->z = z;
}
int max_axis_offset(int dx, int dy, int dz)
{
  printf("DX: %d ", dx);
  printf("DY: %d ", dy);
  printf("DZ: %d \n", dz);


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
  measure->max =  GET_FIELD(&max, 0);
  measure->min =  GET_FIELD(&min, 0);
  measure->threshold = (measure->max + measure->min) >> 1;

}

int get_steps(int steps) {
  int max_steps, min_steps;
  max_steps = MAX_STEP_FREQ * (SAMPLE_SIZE / SAMPLE_RATE);
  min_steps = MIN_STEP_FREQ * (SAMPLE_SIZE / SAMPLE_RATE);
  return (steps <= max_steps && steps >= min_steps) ? steps : 0;
}

int count_steps(measurements *measure, acc_data_t *acc_data_array, int size) {
  int i, sample_new, sample_old, result, steps, last_step;
  sample_new = measure->min;
  sample_old = measure->min;
  steps = 0;

  for(i = 0; i < size; i++) {
    result = GET_FIELD((acc_data_array + i), measure->axis);
    if(result > measure->threshold) {
      // data is above the threshold, shift into new
      sample_old = sample_new;
      sample_new = result;
      // look to see if new is less than old (we are on a down slope)
      if(sample_new < sample_old) {
        // we are on a down slope
        steps++;
        printf("Counted step i:%d Sample_old:%d Sample_new:%d\n", i, sample_old, sample_new);
      }
    } else {
      // move new into old
      sample_old = sample_new;
    }
  }

  return get_steps(steps);
}

int main() {
  measurements measure;
  acc_data_t *acc_arr = malloc(sizeof(acc_data_t) * SAMPLE_SIZE);  
  fake_acc_data_array(acc_arr, SAMPLE_SIZE, 5);
  filter(acc_arr, SAMPLE_SIZE);
  get_max_min(&measure, acc_arr, SAMPLE_SIZE);
  printf("counted %d steps\n", count_steps(&measure, acc_arr, SAMPLE_SIZE));
  print_measure_data(&measure);
  return 0;
}