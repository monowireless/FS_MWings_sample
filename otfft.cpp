#include <complex>
// OTFFT - http://wwwa.pikara.ne.jp/okojisan/stockham/optimization1.html
//    Copyright(c) OK Ojisan(Takuya OKAHISA)

// 2022 modified by Mono Wireless Inc. 

#include "otfft.hpp"


// some functions
static inline float ot_cos(float v) { return ::cosf(v); }
static inline double ot_cos(double v) { return ::cos(v); }

static inline float ot_sin(float v) { return ::sinf(v); }
static inline double ot_sin(double v) { return ::sin(v); }

template<typename T>
void OTFFT::fft0(int n, int s, bool eo, std::complex<T>* x, std::complex<T>* y)
{
    const int m = n / 2;
    const T theta0 = 2 * 3.14159265358979323846264338327950288 / n;

    if (n == 2) {
        std::complex<T>* z = eo ? y : x;
        for (int q = 0; q < s; q++) {
            const std::complex<T> a = x[q + 0];
            const std::complex<T> b = x[q + s];
            z[q + 0] = a + b;
            z[q + s] = a - b;
        }
    }
    else if (n >= 4) {
        for (int p = 0; p < m; p++) {
            const std::complex<T> wp = std::complex<T>(ot_cos(p * theta0), -ot_sin(p * theta0));
            for (int q = 0; q < s; q++) {
                const std::complex<T> a = x[q + s * (p + 0)];
                const std::complex<T> b = x[q + s * (p + m)];

                y[q + s * (2 * p + 0)] = a + b;

                // y[q + s * (2 * p + 1)] = (a - b) * wp;
                T ab_r = a.real() - b.real();
                T ab_i = a.imag() - b.imag();
                y[q + s * (2 * p + 1)] = std::complex<T>(ab_r*wp.real()-ab_i*wp.imag(), ab_r*wp.imag()+ab_i*wp.real());
            }
        }
        fft0(n / 2, 2 * s, !eo, y, x);
    }
}

template<typename T>
void OTFFT::fft(int N, std::complex<T>* x, std::complex<T>* y) // FFT
{
    fft0(N, 1, 0, x, y);
    for (int k = 0; k < N; k++) {
      x[k] /= N;
    }
}

template<typename T>
void OTFFT::ifft(int N, std::complex<T>* x, std::complex<T>* y) // I-FFT
{
    for (int p = 0; p < N; p++) x[p] = conj(x[p]);
    fft0(N, 1, 0, x, y);
    for (int k = 0; k < N; k++) x[k] = conj(x[k]);
}

void OTFFT::abs(int N, std::complex<float>*in, float* out) {
  for(int i = 0; i < N/2; i++) {
    out[i] = sqrtf(in[i].imag()*in[i].imag() + in[i].real()*in[i].real());
  }
}

void OTFFT::abs(int N, std::complex<double>*in, double* out) {
  for(int i = 0; i < N/2; i++) {
    out[i] = sqrt(in[i].imag()*in[i].imag() + in[i].real()*in[i].real());
  }
}

// instantiation
template void OTFFT::fft<float>(int, std::complex<float>*, std::complex<float>*);
template void OTFFT::ifft<float>(int, std::complex<float>*, std::complex<float>*);
template void OTFFT::fft<double>(int, std::complex<double>*, std::complex<double>*);
template void OTFFT::ifft<double>(int, std::complex<double>*, std::complex<double>*);
