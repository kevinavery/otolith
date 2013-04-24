#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int SAMPLE_SIZE = 25;
int SAMPLE_RATE = 50;
#define PI 3.14159265
#define X 0
#define Y 4
#define Z 8
#define GET_FIELD(acc_data_p, field) *((int*)((char*)(acc_data_p) + field))
// #define MAX(a, b) (a > b) ? a : b
// #define MIN(a, b) (a < b) ? a : b


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

inline int MAX(int a , int b)
{
  return (a > b) ? a : b; 
}

inline int MIN(int a , int b)
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
    set_acc_data(&max, MAX(max.x, data[i].x),
                       MAX(max.y, data[i].y), 
                       MAX(max.z, data[i].z));
    
    set_acc_data(&min, MIN(min.x, data[i].x), 
                       MIN(min.y, data[i].y), 
                       MIN(min.z, data[i].z));
  }

  printf("MAXX: %d\n", max.x);
  printf("MINX: %d\n", min.x);

  measure->axis = max_axis_offset(max.x - min.x, max.y - min.y, max.z - min.z);
  // measure->max =  GET_FIELD(&max, measure->axis);
  // measure->min =  GET_FIELD(&min, measure->axis);
  measure->max =  GET_FIELD(&max, 0);
  measure->min =  GET_FIELD(&min, 0);
  measure->threshold = (measure->max + measure->min)/2;

}

int main() {
  measurements measure;
  acc_data_t *acc_arr = malloc(sizeof(acc_data_t) * SAMPLE_SIZE);  
  fake_acc_data_array(acc_arr, SAMPLE_SIZE, 5);
  
  filter(acc_arr, SAMPLE_SIZE);
  get_max_min(&measure, acc_arr, SAMPLE_SIZE);
  
  print_measure_data(&measure);

  return 0;
}