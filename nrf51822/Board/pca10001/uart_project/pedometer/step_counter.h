#ifndef ADXL345_h
#define ADXL345_h

#define SAMPLE_SIZE 100.0
#define SAMPLE_RATE 50.0
#define PI 3.14159265
#define X 0
#define Y 4
#define Z 8
#define GET_FIELD(acc_data_p, field) *((int*)((char*)(acc_data_p) + field))
#define MAX_STEP_FREQ 5.0
#define MIN_STEP_FREQ 0.5

// data array contains an array of structs
// typedef struct {
//   int x;
//   int y;
//   int z;
// } acc_data_t;

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