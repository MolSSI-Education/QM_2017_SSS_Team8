#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <string>
#include <iostream>

namespace py = pybind11;

void print_arg(const std::string & s)
{
	std::cout << "String argument: " << s << std::endl;
}


long factorial(long n)
{
	if(n < 0)
		throw std::runtime_error("n needs to >= 0 for factorial");
	
	long fac = 1;
	for(long i = n; i > 0; i--)
		fac *= i;

	return fac;
}


long binomial_coefficient(long n, long k)
{
	if(n < k)
		throw std::runtime_error("n < k in binomial coefficient");

	return factorial(n)/(factorial(k)*factorial(n-k));
}


long double_factorial(long n)
{
	if (n < -1)
		throw std::runtime_error("n needs to >= -1 for double factorial");

	long fac = 1;

	for(long i = n; i > 0; i-=2)
		fac *= i;

	return fac; 
}


double dot_product(const std::vector<double> & v1, const std::vector<double> & v2)
{
	if(v1.size() != v2.size())
		throw std::runtime_error("Vectors are of different lengths");
	
	if(v1.size() == 0)
		throw std::runtime_error("Zero-length vector");

	double dot = 0.0;
	for(size_t i = 0; i < v1.size(); i++)
		dot += v1[i] * v2 [i];

	return dot;
}


double dot_product_numpy(py::array_t<double> v1, py::array_t<double> v2)
{
	py::buffer_info v1_info = v1.request();
	py::buffer_info v2_info = v2.request();

	if(v1_info.ndim != 1)
		throw std::runtime_error("v1 is not a vector");
	if(v2_info.ndim != 1)
		throw std::runtime_error("v2 is not a vector");
	if(v1_info.shape[0] != v2_info.shape[0])
		throw std::runtime_error("Vectors are of different lengths");

	double dot = 0.0;

	const double * v1_data = static_cast<double *>(v1_info.ptr);
	const double * v2_data = static_cast<double *>(v2_info.ptr);

	for(size_t i = 0; i < v1_info.shape[0]; i++)
		dot += v1_data[i] * v2_data[i];

	return dot;
}


py::array_t<double> dgemm_numpy(double alpha, py::array_t<double> A, 
								py::array_t<double> B)
{
	py::buffer_info A_info = A.request();
	py::buffer_info B_info = B.request();

	if(A_info.ndim != 2)
		throw std::runtime_error("A is not a matrix");
	if(B_info.ndim != 2)
		throw std::runtime_error("B is not a matrix");
	if(A_info.shape[1] != B_info.shape[0])
		throw std::runtime_error("Row os A != columns of B");

	size_t C_nrows = A_info.shape[0];
	size_t C_ncols = B_info.shape[1];
	size_t n_k = A_info.shape[1]; //same as B_info.shape[0]

	const double * A_data = static_cast<double *>(A_info.ptr);
	const double * B_data = static_cast<double *>(B_info.ptr);

	std::vector<double> C_data(C_nrows * C_ncols);

	for(size_t i = 0; i < C_nrows; i++)
	{
		for(size_t j = 0; j < C_ncols; j++)
		{
			double val = 0.0;
			for (size_t k = 0; k < n_k; k++)
			{
				val += alpha * A_data[i*n_k + k] * B_data[k * C_ncols + j];
			}
			C_data[i*C_ncols + j] = val;
		}
	}

	py::buffer_info Cbuf =
		{
			C_data.data(),
			sizeof(double),
			py::format_descriptor<double>::format(),
			2,
			{ C_nrows, C_ncols },
			{ C_ncols * sizeof(double), sizeof(double)}
		};

	return py::array_t<double>(Cbuf);
}


PYBIND11_PLUGIN(basic_mod)
{
	py::module m("basic_mod", "Ben's basic module");

	m.def("print_arg", &print_arg, "Prints the passed arg");
	m.def("factorial", &factorial, "Computes n!");
	m.def("binomial_coefficient", &binomial_coefficient, "Computes binomial coefficient");
	m.def("double_factorial", &double_factorial, "Computes n!!");
	m.def("dot_product", &dot_product);
	m.def("dot_product_numpy", &dot_product_numpy);
	m.def("dgemm_numpy", &dgemm_numpy);

	return m.ptr();
}
