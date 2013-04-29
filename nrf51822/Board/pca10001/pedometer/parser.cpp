#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "step_counter.c"

measurements data;
int collected_data;
int total_steps = 0;
acc_data_t* acc_arr;
using namespace std;
int total_samples = 0;
int line_num = 0;
void parse_file(string filename)
{
	string line, word;

	ifstream file(filename.c_str());
	if(file) 
	{
		while(getline(file, line, '\r')) {
			total_samples++;
		}
		file.clear();
		file.seekg(0, file.beg);
		acc_arr = new acc_data_t[total_samples];
		
		while(getline(file, line, '\r')) {
			if(line_num > 0) {			
				if(line[line.length() == '\r'])
					line.erase(line.length());

				int index = 0;
				std::vector<std::string> strs;
				boost::algorithm::split(strs, line, boost::is_any_of(","));
				
				int tmp;
				istringstream buffer0(strs[0]);
				istringstream buffer1(strs[1]);
				istringstream buffer2(strs[2]);

				buffer0 >> tmp;
				acc_arr[line_num].x = tmp;
				buffer1 >> tmp;
				acc_arr[line_num].y = tmp;
				buffer2 >> tmp;
				acc_arr[line_num].z = tmp;
			}
			line_num++;
		
		}
	file.close();
	}
	else
		cout << "Error: Could not find file "<< filename << endl;
	
}

int print_csv(measurements *measure, acc_data_t *acc) {
	ofstream myfile;
  myfile.open("plot.csv", ios::out | ios::app);
  
  for(int i = 0; i < SAMPLE_SIZE; i++) {
  	myfile << (acc + i)->x << ", ";
  	myfile << (acc + i)->y << ", ";
  	myfile << (acc + i)->z << ", ";
  	myfile << (acc + i)->step_location << ", ";
  	myfile << measure->threshold << ", ";
  	myfile << measure->precision << ", ";
  	myfile << measure->max << ", ";
  	myfile << measure->min << ", ";
  	myfile << measure->axis << ", ";
  	myfile << total_steps << "\n";
  }

  myfile.close();
}

int print_csv_header() {
	ofstream myfile;
  myfile.open("plot.csv");
	myfile << "X, ";
	myfile << "Y, ";
	myfile << "Z, ";
	myfile << "step, ";
	myfile << "Thresh, ";
	myfile << "Prec, ";
	myfile << "Max, ";
	myfile << "Min, ";
	myfile << "Axis, ";
	myfile << "Total Steps \n";
  myfile.close();
}

int main (int argc, char* argv[]) {
	int temp_steps;
	if(argc < 2)
		cout << "Provide file " << endl;
	if(argc > 2) {
		istringstream buf(argv[2]);
		buf >> SAMPLE_SIZE;
	}


	parse_file(argv[1]);
	measurements measure;
	print_csv_header();
    int i;
  for(i = 0; i< total_samples - SAMPLE_SIZE; i+=SAMPLE_SIZE) {
		filter((acc_arr + i), SAMPLE_SIZE);
		get_max_min(&measure, (acc_arr + i), SAMPLE_SIZE);
		temp_steps = count_steps1(&measure, (acc_arr + i), SAMPLE_SIZE);
		total_steps += temp_steps;				
	}

	filter((acc_arr + total_samples - 1), (total_samples - i));
	get_max_min(&measure, (acc_arr + total_samples - 1), (total_samples - i));
	total_steps += count_steps1(&measure, (acc_arr + total_samples - 1), (total_samples - i));
	std::cout << "Total Steps: " << total_steps << std::endl;
	// std::cout << "Sample Size: " << SAMPLE_SIZE << std::endl;
	return 0;
}
