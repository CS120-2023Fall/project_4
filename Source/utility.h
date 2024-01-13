#include <string>
#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include  <ostream>
#define MAX_WRITE_DATA_SIZE 24000

//file operator
inline std::vector<bool> Read_bits(const std::string& path)//read_bits_from file
{
	FILE* file = fopen(path.c_str(), "r");
	if (file == NULL)
	{
		std::string new_path = "../" + path;
		file = fopen(new_path.c_str(), "r");

	}
	if (file == NULL) {
		exit(-1);
	}


	std::vector<bool> bits;
	while (1)
	{
		char head[100];
		int break_info = fscanf(file, "%c", head);
		if (break_info == EOF)
		{
			break;
		}
		if (head[0] == '\n')continue;
		bool bit = (bool)atoi(head);
		bits.push_back(bit);
	}
	return bits;
}
inline std::vector<bool> Read_bits_from_bin(const std::string& path) {
	std::ifstream f(path.c_str(), std::ios::binary | std::ios::in);
	std::ofstream o("INPUT_bin.txt", std::ios::out);
	std::string line;
	std::vector<bool>bits;
	char c;
	while (f.get(c)) {
		for (int j = 7; j >= 0; j--) {
			bool bit = static_cast<bool> ((c >> j) & 1);
			bits.push_back(bit);
			o << (int)bit;
		}
	}
	return bits;
}

inline void Tranlate_from_A_bin_To_B_Bin(const std::string& path, const std::string& b_path) {
	std::ifstream f(path.c_str(), std::ios::binary | std::ios::in);
	std::ofstream o(b_path.c_str(), std::ios::binary|std::ios::out);
	std::string line;
	std::vector<bool>bits;
	char c;
	while (f.get(c)) {
		unsigned  t = 0;
		for (int j = 7; j >= 0; j--) {
			bool bit = static_cast<bool>(c >> j) & 1;
			t += bit * (1 << (7 - j));
			bits.push_back(bit);
		}
		o << (char)c;
	}
}
inline void Write_bin(std::vector<bool>bits,const std::string& path) {
	std::ofstream o(path.c_str(), std::ios::binary| std::ios::out);
	o.write((const char*)&(bits[0]), bits.size()/8);
	o.close();
}
//inline std::vector<bool> from_symbols_to_bits(const std::vector<unsigned int >symbols, int seperation_num) {
//	std::vector<bool>bits;
//	int size = symbols.size();
//	for (auto& symbol : symbols) {
//		for (int offset = seperation_num - 1; offset >= 0; offset--) {
//			bool bit = bool(symbol >> offset & 1);
//			bits.push_back(bit);
//		}
//	}
//	return bits;
//}


// //write to a format matlab can read
inline void Write(const std::string& path, const std::vector<float>& data) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE* file = fopen(path.c_str(), "w");
	fprintf(file, "[");
	for (int i = 0; i < size; i++) {
		fprintf(file, "%f,", data[i]);


	}
	fprintf(file, "]");
	fclose(file);
}
inline void Write(const std::string& path, const std::vector<double>& data, std::string name) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE* file = fopen(path.c_str(), "w");

	fprintf(file, "%s=[", name.c_str());
	for (int i = 0; i < size; i++) {
		fprintf(file, "%f,", data[i]);


	}
	fprintf(file, "];");
	fclose(file);
}

inline void Write(const std::string &path, const std::deque<double> &data) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE *file = fopen(path.c_str(), "w");
	fprintf(file, "%d\n", size);
	fprintf(file, "[");
	for (int i = 0; i < size; i++) {
		fprintf(file, "%f,", data[i]);
	}
	fprintf(file, "]");
	fclose(file);
}

inline void Write(const std::string& path, const std::vector<double>& data) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE* file = fopen(path.c_str(), "w");
	fprintf(file, "[");
	for (int i = 0; i < size; i++) {
		fprintf(file, "%f,", data[i]);
	}
	fprintf(file, "]");
	fclose(file);
}
inline void Write(const std::string& path, const std::vector<bool>& data) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE* file = fopen(path.c_str(), "w");
	for (int i = 0; i < data.size(); i++) {
		fprintf(file, "%d", data[i]);


	}
	fclose(file);
}
inline void Write(const std::string& path, const std::vector<int>& data) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE* file = fopen(path.c_str(), "w");
	for (int i = 0; i < data.size(); i++) {
		fprintf(file, "%d", data[i]);
	}
	fclose(file);
}
inline void Write(const std::string &path, const std::deque<int> &data) {
	auto size = (data.size() > MAX_WRITE_DATA_SIZE) ? MAX_WRITE_DATA_SIZE : data.size();
	FILE *file = fopen(path.c_str(), "w");
	for (int i = 0; i < data.size(); i++) {
		fprintf(file, "%d", data[i]);
	}
	fclose(file);
}
inline void Write(const std::string& path, float* data, unsigned int size) {
	FILE* file = fopen(path.c_str(), "w");
	fprintf(file, "[");
	for (int i = 0; i < size; i++) {
		fprintf(file, "%f ", data[i]);
	}
	fprintf(file, "]");
	fclose(file);
}

//convert the bits
//inline std::vector<bool> Read_bits_from_string(const std::string& str)//read_bits_from_string
//{
//	std::vector<bool> bits;
//	for (auto& i : str)
//	{
//		bool bit = (i == '1');
//		bits.push_back(bit);
//	}
//	return bits;
//}
//inline unsigned int from_bits_vector_to_unsigned_int(const std::vector<bool>& bits)//convert bits to unsigned int
//{
//	unsigned int size = bits.size();
//	unsigned int l = 1 << (size - 1);
//	unsigned int sum = 0;
//	for (int i = 0; i < size; i++)
//	{
//		sum += l * bits[i];
//		l = l >> 1;
//	}
//	return sum;
//}
//std::vector<bool> from_uint8_t_to_bits_vector(uint8_t data) {
//	std::vector<bool> bits;
//	for (int i = 7; i >= 0; i--) {
//		bool bit = bool((data >> i) & 1);
//		bits.push_back(bit);
//	}
//	return bits;
//}
//std::vector<unsigned int> translate_from_bits_vector_to_unsigned_int_vector(const std::vector<bool>& bits, unsigned int seperation_num)//convert bits to unsigned int with num seperation
//{
//	int size = bits.size();
//	std::vector<unsigned int > translated;
//	for (int i = 0; i < size; i += seperation_num) {
//		std::vector<bool> slice_of_bits;
//
//		for (int j = 0; j < seperation_num; j++) {
//			int index = i + j;
//			slice_of_bits.push_back(bits[index]);
//		}
//		unsigned int translate_num = from_bits_vector_to_unsigned_int(slice_of_bits);
//		translated.push_back(translate_num);
//	}
//	return translated;
//}
//inline std::vector<bool> from_unsigned_int_to_bits_vector(unsigned int num)//convert int to_bits
//{
//	std::vector<bool> bits_inverse;
//	unsigned int l = num;
//	while (l)
//	{
//		bool bit = (bool)(l & 1);
//		l = l >> 1;
//		bits_inverse.push_back(bit);
//	}
//
//	std::vector<bool> bits;
//	for (int i = bits_inverse.size() - 1; i >= 0; i--)
//	{
//		bits.push_back(bits_inverse[i]);
//	}
//	return bits;
//}
//inline std::vector<bool> from_unsigned_int_to_bits_vector_filled_with_zero(unsigned int num, int size) {
//
//	std::vector<bool> bits_origin = from_unsigned_int_to_bits_vector(num);
//	std::vector<bool>bits;
//	if (size >= bits_origin.size()) {
//		for (int i = 0; i < size - bits_origin.size(); i++) {
//			bits.push_back(0);
//		}
//	}
//	for (int i = 0; i < bits_origin.size(); i++) {
//		bits.push_back(bits_origin[i]);
//	}
//	return bits;
//}
//try to return a vector size of size,filled with high 0

//template<typename T>
//std::vector<T> connect(const std::vector<T>& A, const std::vector<T>& B) {//connect two vector
//	std::vector<T> data;
//	for (int i = 0; i < A.size(); i++) {
//		data.push_back(A[i]);
//	}
//	for (int i = 0; i < B.size(); i++) {
//		data.push_back(B[i]);
//	}
//
//	return data;
//}
// insert B to A at start_index[0,start_index) B,[start_index,A[A.size()-1]]
//template<typename T>
//std::vector<T>insert(const std::vector<T>& A, const std::vector<T>& B, int start_index) {
//	std::vector<T>data;
//	for (int i = 0; i < start_index; i++) {
//		data.push_back(A[i]);
//	}
//	for (int i = 0; i < B.size(); i++) {
//		data.push_back(B[i]);
//
//	}
//	for (int i = start_index; i < A.size(); i++) {
//		data.push_back(A[i]);
//	}
//	return data;
//}
//inline std::string to_string(const std::vector<bool>& bits)
//{
//	std::string ans;
//	for (int i = 0; i < bits.size(); i++)
//	{
//		if (bits[i] == 0)
//		{
//			ans += "0";
//		}
//		else
//		{
//			ans += "1";
//		}
//	}
//	return ans;
//}
//inline std::string to_string_filled_with_zero(const std::vector<bool>& bits, unsigned int total_bits)
//{
//	std::string bits_string = to_string(bits);
//	if (total_bits <= bits.size())
//	{
//		return bits_string;
//	}
//	std::string ans(total_bits - bits.size(), '0');
//	ans += bits_string;
//	return ans;
//}
//inline unsigned char* unsigned_int_to_unsigned_char_star(unsigned int num, int& size) {
//	unsigned int test = num;
//	int count = 0;
//	while (test != 0)
//	{
//		test = test >> 8;
//		count++;
//	}
//	unsigned char* data = new unsigned char[count];
//	test = num;
//	for (int i = 0; i < count; i++) {
//		data[count - 1 - i] = (unsigned char)(test & 0xFF);
//		test = test >> 8;
//	}
//	size = count;
//	return data;
//
//}
//convert the vector
//template<typename T>
//T* from_vector_to_pointer(const std::vector<T>& data) {
//	T* new_data = new T[data.size()];
//	for (int i = 0; i < data.size(); i++) {
//		new_data[i] = data[i];
//	}
//	return new_data;
//}
//template<typename T>
//std::vector<T> vector_from_start_to_end(const std::vector<T>& data, unsigned int start, unsigned int end) {
//	std::vector<T> new_data;
//	for (int i = start; i < end; i++) {
//		new_data.push_back(data[i]);
//
//	}
//	return new_data;
//}
//the same as matlab do
//float sum(const std::vector<float>& data) {
//	float sum = 0;
//	for (int i = 0; i < data.size(); i++) {
//		sum += data[i];
//	}
//	return sum;
//}
//template<typename T>
//std::vector<T> smooth(const std::vector<T>& data, int smooth_order) {
//	std::vector<T> smoothed = data;
//	float sum = 0;
//	if (smooth_order % 2 == 0) {
//		smooth_order += 1;
//	}
//	int range = (smooth_order + 1) / 2;
//	for (int i = 0; i < data.size(); i++) {
//		int left_range = i - range;
//		int right_range = i + range;
//		if (left_range < 0) {
//			left_range = 0;
//		}
//		if (right_range > data.size() - 1) {
//			right_range = data.size() - 1;
//
//		}
//		float s = 0;
//		for (int j = left_range; j <= right_range; j++) {
//			s += smoothed[j];
//		}
//		s /= (right_range - left_range + 1);
//		smoothed[i] = s;
//
//	}
//	return smoothed;
//}
//float* smooth_float(float* data, unsigned int size) {
//	float* smoothed = new float[size];
//	float sum = 0;
//
//	for (int i = 0; i < size; i++) {
//		sum += data[i];
//		smoothed[i] = sum / (i + 1);
//
//	}
//	return smoothed;
//}

//template<typename T1, typename T2>
//double calculateCorrelationShip(const std::vector<T1>& data1, const std::vector<T2>& data2, int start_index_of_data1, int start_index_of_data2, int num) {
//	//first calculate E(data1) and E(data2)
//	double data1_mean, data2_mean;
//	double sum_of_data_1 = 0, sum_of_data_2 = 0;
//	for (int i = 0; i < num; i++) {
//		int index_data1 = start_index_of_data1 + i;
//		int index_data2 = start_index_of_data2 + i;
//		sum_of_data_1 += data1[index_data1];
//		sum_of_data_2 += data2[index_data2];
//
//	}
//	data1_mean = sum_of_data_1 / num;
//	data2_mean = sum_of_data_2 / num;
//	double sigma_data1 = 0, sigma_data2 = 0;
//	double cov_data1_data2 = 0;
//	for (int i = 0; i < num; i++) {
//		int index_data1 = start_index_of_data1 + i;
//		int index_data2 = start_index_of_data2 + i;
//		sigma_data1 += (data1[index_data1] - data1_mean) * (data1[index_data1] - data1_mean);
//		sigma_data2 += (data2[index_data1] - data2_mean) * (data2[index_data1] - data2_mean);
//		cov_data1_data2 += (data1[index_data1] - data1_mean) * (data2[index_data1] - data2_mean);
//
//	}
//	double ans = (cov_data1_data2) / (std::sqrt(sigma_data1) * std::sqrt(sigma_data2));
//
//	return ans;
//
//}