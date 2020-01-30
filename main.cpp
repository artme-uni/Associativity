#include <iostream>
#include <fstream>

#define CACHE 2097152  // элементов в кэше, равном 8 мб
#define OFFSET 2097152 // смещение между фрагментами = размер баннки
// так как значение неизвестено, то берем равное размеру кэш память, которая кратна размеру банка

static inline unsigned long long rdtsc(void)  //подсчет времени в тактах
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}

//arr[i-ый элемент k-го блока] = i-ый элемент k+1-го блока
void set_arr(volatile int *arr, int segments_count, int elements_count)
{
    for (int k = 0; k < segments_count; k++)
    {
        for (int i = 0; i < elements_count; i++)
        {
            arr[i + OFFSET * k] = (i + OFFSET * (k + 1)) % (segments_count * OFFSET);

            if (k == (segments_count - 1))
                arr[i + OFFSET * k]++;
        }
    }

    arr[(elements_count - 1) + OFFSET * (segments_count - 1)] = 0;
}

//возвращает минимальное время прохода массива длины n
int pass(volatile int *arr, int n)
{
    volatile unsigned long long t1, t2;
    unsigned long long min_time = ULLONG_MAX;

    for (int i = 0; i < 5; i++)
    {
        t1 = rdtsc();
        for (int k = 0, i = 0; i < n; i++)
            k = arr[k];
        t2 = rdtsc();

        if (min_time > (t2 - t1))
        {
            min_time = (t2 - t1);
        }
    }

    return min_time;
}

//печать среднего времени обращения к элементу
void print_time(int segments_count, std::ostream &out)
{
    volatile int *arr = new int[OFFSET * segments_count]();

    int elements_count = CACHE / segments_count; // Суммарный объем фрагментов равен кэшу

    set_arr(arr, segments_count, elements_count);

    int time = pass(arr, elements_count * segments_count);

    out << segments_count << "\t" << time / (elements_count * segments_count) << std::endl;

    delete[] arr;
}

int main()
{
    std::ofstream out;
    out.open("/Users/twiggy/Desktop/output.txt");

    for (int i = 1; i <= 256; i++)
        print_time(i, out);

    out.close();

    return 0;
}