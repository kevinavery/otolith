#ifndef STEP_COUNTER_h
#define STEP_COUNTER_h
// #include "acc_driver.h"
#define SAMPLE_SIZE 50.0
// int SAMPLE_SIZE = 50;
#define SAMPLE_RATE 50.0
#define PI 3.14159265
#define X 0
#define Y 2
#define Z 4
#define GET_FIELD(acc_data_p, field) *((int16_t*)((char*)(acc_data_p) + field))
#define MAX_STEP_FREQ 5.0
#define MIN_STEP_FREQ 0.5
#define MIN_SAMPlES_BETWEEN  10 // ((1 / MAX_STEP_FREQ) * SAMPLE_RATE)

// data array contains an array of structs
typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
  int16_t step_location;
} acc_data_t;

typedef struct {
  int threshold;
  int max;
  int min;
  int axis;
  int precision;
} measurements;

int max_of(int a, int b);
int min_of(int a, int b);
void fake_acc_data_array(acc_data_t *acc_data_array, int size, float freq);
void filter(acc_data_t * acc_data_array, int size);
void print_acc_data_array(acc_data_t* acc_data_array, int size);
void print_measure_data(measurements* measure);
void set_acc_data(acc_data_t *data, int x, int y, int z);
int max_axis_offset(int dx, int dy, int dz);
void get_max_min(measurements *measure, acc_data_t *data, int size);
int count_steps(measurements *measure, acc_data_t *acc_data_array, int size);

#endif
