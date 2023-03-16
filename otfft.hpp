// OTFFT - http://wwwa.pikara.ne.jp/okojisan/stockham/optimization1.html
//    Copyright(c) OK Ojisan(Takuya OKAHISA)

// 2022 modified by Mono Wireless Inc. 

#include <complex>
#include <cmath>

namespace OTFFT {
	/**
	 * Core procedure of FFT/IFFT, which is recursively called.
	 * 
	 * \param n		length of data
	 * \param s		stride
	 * \param eo	if eo == 0 or falsem, then x is output, else if eo == 1 or true then y is output.
	 * \param x		input data for FFT (output if eo == 0)
	 * \param y		working buffer (output if eo == 1)
	 */
	template <typename T>
	void fft0(int n, int s, bool eo, std::complex<T>* x, std::complex<T>* y);

	/**
	 * perform FFT with prealocatted working buffer.
	 * 
	 * \param N  FFT length (2^N)
	 * \param x  Input/Output buffers (complex_t array of 2^N)
	 * \param y  Working buffers (complex_t array of 2^N)
	 */
	template <typename T>
	void fft(int N, std::complex<T>* x, std::complex<T>* y);

	/**
	 * perform I-FFT.
	 * 
	 * \param N  FFT length (2^N)
	 * \param x  Input/Output buffers (complex_t array of 2^N)
   * \param y  Working buffers (complex_t array of 2^N)
	 */
	template<typename T>
  void ifft(int N, std::complex<T>* x, std::complex<T>* y); // I-FFT

  // abs 2/N items
  void abs(int N, std::complex<double>*in, double* out);
  void abs(int N, std::complex<float>*in, float* out);

}


