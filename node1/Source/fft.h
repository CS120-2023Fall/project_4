#ifndef PROJECT1_FFT_H
#define PROJECT1_FFT_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <vector>
#include <assert.h>

using Complex = std::complex<double>;


void f(int i, int j, Complex w, std::vector<Complex>& fft_array) {
    Complex t = fft_array[i];
    fft_array[i] = fft_array[i] + w * fft_array[j];
    fft_array[j] = t - w * fft_array[j];
}

Complex w(int m, int N) {
    Complex c;
    c.real(cos(2 * acos(-1) * m / N));
    c.imag(sin(-2 * acos(-1) * m / N));
    return c;
}

int reverse(int n, int l) {
    int i = 0, r = 0, t;
    while (i < l) {
        t = n & 1;
        n = n >> 1;
        r += (t * pow(2, l - 1 - i));
        i++;
    }
    return r;
}

/**
 *
 * @param signal_to_process : The signal to implement fft.
 * @param fft_result : The result of fft, the spectrum.
 */
void fft(std::vector<double>& signal_to_process, std::vector<Complex>& fft_result,
    int num_samples) {
    int N, p = 0, k, l, i, t;
    while (num_samples > pow(2, p))
        p++;

    N=pow(2,p);
    //N = num_samples;

    for (i = 0; i < N; i++) {
        t = reverse(i, p);
        fft_result[i].real(signal_to_process[t]);
        fft_result[i].imag(0);
    }

    for (k = 1; k <= p; k++)
        for (l = 0; l < N / pow(2, k); l++)
            for (i = 0; i < pow(2, k - 1); i++)
                f(l * pow(2, k) + i, l * pow(2, k) + i + pow(2, k - 1), w(i, pow(2, k)),
                    fft_result);

}

#endif //PROJECT1_FFT_H
