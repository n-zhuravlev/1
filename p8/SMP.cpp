#include <iostream>
#include <omp.h>
#define MIN_CHUNK_SIZE 1
#define ARRAY_MAX_NUM 10000
#define ARRAY_MAX_SIZE 50
#include <ctime>

template<typename T>
void MergeSort(T *input_array, size_t length);

template<typename T>
void Parallel_MergeSort(T *input_array, size_t length);

using namespace std;


int main(int argc, char *argv[])
{
    size_t *as_array = new size_t[ARRAY_MAX_SIZE];
    srand(time(0));
    for(size_t i = 0; i < ARRAY_MAX_SIZE; ++i) as_array[i] = rand() % ARRAY_MAX_NUM;
    cout << "\n\nInput array:" << endl;
    for(size_t i = 0; i < ARRAY_MAX_SIZE; ++i) cout << as_array[i] << ' ';
    cout << "\n\n" << endl;
    // параллельная сортировка
    Parallel_MergeSort(as_array, ARRAY_MAX_SIZE);
    cout << "Parallel Merge sort:" << endl;
    for(size_t i = 0; i < ARRAY_MAX_SIZE; ++i) cout << as_array[i] << ' ';
    cout << "\n\n" << endl;
    
    // сортировка
    srand(time(0));
    for(size_t i = 0; i < ARRAY_MAX_SIZE; ++i) as_array[i] = rand() % ARRAY_MAX_NUM;
    MergeSort(as_array, ARRAY_MAX_SIZE);
    cout << "Ordinary Merge sort:" << endl;
    for(size_t i = 0; i < ARRAY_MAX_SIZE; ++i) cout << as_array[i] << ' ';
    cout << '\n' << endl;

    delete[] as_array;

    return 0;
}


template<typename T>
void Parallel_MergeSort(T *input_array, size_t length)
{
    T *buffer = new T[length];
    // ставим указатели на начальный массив и временное хранилище чтобы в случае необходимости быстро менять местами их
    T *input = input_array, *output = buffer;

    for(size_t chunk_size = MIN_CHUNK_SIZE; chunk_size < length; chunk_size *= 2){

        #pragma omp parallel num_threads(4)
        {
            T *output_ptr;
            T *end_first_block, *end_second_block, *ptr_first, *ptr_second;
            #pragma omp for
            for(size_t current_chunk = 0; current_chunk < length; current_chunk += 2*chunk_size){

                output_ptr = output + current_chunk;
                ptr_first = input + current_chunk;
                end_first_block = ptr_second = ptr_first + chunk_size;

                if(input + length > end_first_block){
                    end_second_block = min(end_first_block + chunk_size, input + length);
                    while(ptr_first != end_first_block && ptr_second != end_second_block) *output_ptr++ = *ptr_first <= *ptr_second ? *ptr_first++ : *ptr_second++;
                    while(ptr_first != end_first_block) *output_ptr++ = *ptr_first++;
                    while(ptr_second != end_second_block) *output_ptr++ = *ptr_second++;

                }else{
                    while(ptr_first != input + length) *output_ptr++ = *ptr_first++;
                }
            }

        }
        swap(input, output);
    }
    if(input != input_array){
        for(size_t i = 0; i < length; ++i) input_array[i] = input[i];
    }
    delete[] buffer;
    return;
}


template<typename T>
void MergeSort(T *input_array, size_t length)
{
    T *buffer = new T[length];
    // ставим указатели на начальный массив и временное хранилище чтобы в случае необходимости быстро менять местами их
    T *input = input_array, *output = buffer;
    T *output_ptr;
    T *end_first_block, *end_second_block, *ptr_first, *ptr_second;

    for(size_t chunk_size = MIN_CHUNK_SIZE; chunk_size < length; chunk_size *= 2){

        for(size_t current_chunk = 0; current_chunk < length; current_chunk += 2*chunk_size){

            output_ptr = output + current_chunk;
            ptr_first = input + current_chunk;
            end_first_block = ptr_second = ptr_first + chunk_size;

            if(input + length > end_first_block){
                end_second_block = min(end_first_block + chunk_size, input + length);
                while(ptr_first != end_first_block && ptr_second != end_second_block) *output_ptr++ = *ptr_first <= *ptr_second ? *ptr_first++ : *ptr_second++;
                while(ptr_first != end_first_block) *output_ptr++ = *ptr_first++;
                while(ptr_second != end_second_block) *output_ptr++ = *ptr_second++;

            }else{
                while(ptr_first != input + length) *output_ptr++ = *ptr_first++;
            }
        }
        swap(input, output);
    }
    if(input != input_array){
        for(size_t i = 0; i < length; ++i) input_array[i] = input[i];
    }
    delete[] buffer;
    return;
}
