// qd.cpp : Defines the exported functions for the DLL application.
//
#include "header.h"
#include "mcadHelper.h"
#include "qd/c_dd.h"
#include "qd/c_qd.h"
#include "qd/dd_real.h"
#include "qd/qd_real.h"
#include <complex>
#include <functional>
#include <ComplexTraits.h>
#include "utils.h"
#include <vector>
#include <iomanip>

using namespace mcadHelper;
namespace {
	auto NOT_A_QD_VALUE = registerErrorMessage("must be a 4 element real vector");
	auto NOT_A_DD_OR_QD_VALUE = registerErrorMessage("must be a 2 or 4 element real vector");

	auto OUT_OF_MEMORY = registerErrorMessage("out of memory");
	auto MUST_BE_REAL = registerErrorMessage("must be real");
	auto INVALID_ARGUMENT = registerErrorMessage("invalid argument");
	auto DIVIDE_BY_ZERO = registerErrorMessage("divide by 0");
	auto MUST_BE_INT = registerErrorMessage("must be a integer");
	auto MUST_BE_POS_INT = registerErrorMessage("must be a positive integer");
	auto UNKNOWN_CONST = registerErrorMessage("unrecognized constant");
	auto NOT_A_QUAD_DOUBLE_LIST = registerErrorMessage("must be a 4xn real matrix.");
}

MCFN(qd_read, "s", "read a string representation of a quad-double into a double-double value",
	 LPCOMPLEXARRAY result, LPCMCSTRING s)
{
	double qd[4]{ 0.0, 0.0, 0.0, 0.0};
	c_qd_read(s->str, qd);
	if(!std::isfinite(qd[0]))
		return MAKELRESULT(INVALID_ARGUMENT, 1);

	if(!allocQD(result, qd))
		return MAKELRESULT(OUT_OF_MEMORY, 0);

	return 0;
}
END_FN;

MCFN(qd_write, "a", "converts a quad-double to a string ",
	 LPMCSTRING result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);

	const auto allocSize = 73;
	const auto precision = 64;

	result->str = MathcadAllocate(allocSize);

	if (!result->str)
	{
		return MAKELRESULT(OUT_OF_MEMORY, 0);
	}

	result->str[allocSize - 1] = '\0';
	c_qd_swrite(a->hReal[0], precision, result->str, allocSize);

	//ISQD(a, 1);

	//result->str = MathcadAllocate(73);

	//if (!result->str)
	//{
	//	return MAKELRESULT(OUT_OF_MEMORY, 0);
	//}

	//result->str[72] = '\0';

	//qd_real(a->hReal[0]).write(result->str, 73, 64, false, true);

	return 0;
}
END_FN;

//MCFN(dd_cwrite, "a", "converts a complex double-double to a string ",
//	 LPMCSTRING result, LPCCOMPLEXARRAY a)
//{
//	// ISDD(a, 1);
//
//	// result->str = MathcadAllocate(41 + 3 + 41);
//
//	char re[41];
//	char im[41];
//
//	double d[2] = {a->hImag[0][0], a->hImag[0][1]};
//
//	auto isNeg = d[0] < 0.0;
//
//	if (isNeg)
//	{
//		d[0] = -a->hImag[0][0];
//		d[1] = -a->hImag[0][1];
//	}
//	
//
//	// c_dd_swrite(a->hReal[0], 32, re, 41);
//	dd_real(a->hReal[0]).write(re, 41, 32, false, true);
//	dd_real(d).write(im, 41, 32, false, true);
//	// c_dd_swrite(d, 32, im, 41);
//
//	std::string str{ re };
//
//	if (isNeg)
//		str += " - ";
//	else
//		str += " + ";
//
//	str += im;
//	str += "i";
//
//	result->str = MathcadAllocate(str.size()+1);
//	result->str[str.size()] = '\0';
//	if (!result->str)
//	{
//		return MAKELRESULT(OUT_OF_MEMORY, 0);
//	}
//	strcpy_s(result->str, str.size(), str.c_str());
//
//	return 0;
//}
//END_FN;

MCFN(qd_format, "a,precision", "converts a quad-double string with precision",
	 LPMCSTRING result, LPCCOMPLEXARRAY a, LPCCOMPLEXSCALAR precision)
{
	ISQD(a, 1);
	ISD(precision, 2);
	
	auto n = std::floor(precision->real);
	if (n != precision->real)
	{
		return MAKELRESULT(MUST_BE_INT, 2);
	}


	auto aa = makeQD(a);

	auto s = aa.to_string(static_cast<int>(n), 0, std::ios_base::scientific, false, true);
	const auto sz = s.size();
	result->str = MathcadAllocate(static_cast<unsigned int>(sz + 1));
	if (!result->str)
	{
		return MAKELRESULT(OUT_OF_MEMORY, 0);
	}
	result->str[sz] = '\0';
	if (sz > 0)
	{
		strncpy_s(result->str, sz+1, s.c_str(), sz);
	}

	

	//auto nn = static_cast<unsigned>(n);

	//result->str = MathcadAllocate(nn+8);

	//if (!result->str)
	//{
	//	return MAKELRESULT(OUT_OF_MEMORY, 0);
	//}

	//result->str[nn+7] = '\0';

	//c_qd_swrite(a->hReal[0], nn, result->str, nn+8);

	return 0;

}
END_FN;

///////////////////////////////Addition
MCFN(qd_add, "a,b", "adds quad-double \"a\" to quad-double \"b\"", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{	
	ISDDQD(a, 1);
	ISDDQD(b, 2);
	const auto success = (a->rows == 2
						  ? b->rows == 2
						  ? copy(result, qd_real{ dd_real{ a->hReal[0] } + dd_real{ b->hReal[0] } })
						  : copy(result, dd_real{ a->hReal[0] } + qd_real{ b->hReal[0] })
						  : b->rows == 2
						  ? copy(result, qd_real{ a->hReal[0] } + dd_real{ b->hReal[0] })
						  : copy(result, qd_real{ a->hReal[0] } + qd_real{ b->hReal[0] })
						  );

//	const auto success = copy(result, qd_real{ a->hReal[0] } + qd_real{ b->hReal[0] });
	return (success) ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

//MCFN(dd_cadd, "a,b", "adds double-double a to double-double b", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a_, LPCCOMPLEXARRAY b_)
//{
//
//	std::complex<dd_real> a{ dd_real{a_->hReal[0][0], a_->hReal[0][1]},  dd_real{a_->hImag[0][0], a_->hImag[0][1]} };
//	std::complex<dd_real> b{ dd_real{b_->hReal[0][0], b_->hReal[0][1]},  dd_real{b_->hImag[0][0], b_->hImag[0][1]} };
//
//	std::complex<dd_real> c = a + b;
//
//	MathcadArrayAllocate(result, 2, 1, true, true);
//	result->hReal[0][0] = c.real().x[0];
//	result->hReal[0][1] = c.real().x[1];
//	result->hImag[0][0] = c.imag().x[0];
//	result->hImag[0][1] = c.imag().x[1];
//
//	std::complex<double> foo{ 3.5, 1.3 };
//	auto bar = a * foo;
//	auto foobar = foo * a;
//
//
//
//	// c_dd_add(a->hReal[0], b->hReal[0], result->hReal[0]);
//	return 0;
//}
//END_FN;

MCFN(qd_add_qd_d, "a,b", "adds quad-double a to a double b",
	 DD result, CDD a, LPCCOMPLEXSCALAR b)
{
	ISQD(a, 1);
	ISD(b, 2);
	
	ALLOCQD(result);
	c_qd_add_qd_d(a->hReal[0], b->real, result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_add_d_qd, "a,b", "adds double a to a double-double b",
	 DD result, LPCCOMPLEXSCALAR a, CDD b )
{
	ISD(a, 1);
	ISQD(b, 2);

	ALLOCQD(result);
	c_qd_add_d_qd(a->real, b->hReal[0], result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_add_d_d, "a,b", "adds double a to a double b returning a double-double result",
	 DD result, LPCCOMPLEXSCALAR a, LPCCOMPLEXSCALAR b )
{
	ISD(a, 1);
	ISD(b, 2);
	double tmp[4] = { b->real, 0.0, 0.0, 0.0 };

	ALLOCQD(result);

	c_qd_add_d_qd(a->real, tmp, result->hReal[0]);
	return 0L;
}
END_FN;


///////////////////////////////Subtraction

MCFN(qd_sub, "a,b", "subtracts quad-double \"a\" from quad-double \"b\"", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISDDQD(a, 1);
	ISDDQD(b, 2);

	//const auto success = copy(result, qd_real{ a->hReal[0] } - qd_real{ b->hReal[0] });
	const auto success = (a->rows == 2
						  ? b->rows == 2
						  ? copy(result, qd_real{ dd_real{ a->hReal[0] } - dd_real{ b->hReal[0] } })
						  : copy(result, dd_real{ a->hReal[0] } - qd_real{ b->hReal[0] })
						  : b->rows == 2
						  ? copy(result, qd_real{ a->hReal[0] } - dd_real{ b->hReal[0] })
						  : copy(result, qd_real{ a->hReal[0] } - qd_real{ b->hReal[0] })
						  );

	return (success) ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_sub_qd_d, "a,b", "subtracts quad-double a from double b",
	 DD result, CDD a, LPCCOMPLEXSCALAR b)
{
	ISQD(a, 1);
	ISD(b, 2);

	ALLOCQD(result);
	c_qd_sub_qd_d(a->hReal[0], b->real, result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_sub_d_qd, "a,b", "subtracts double a from quad-double b",
	 DD result, LPCCOMPLEXSCALAR a, CDD b)
{
	ISD(a, 1);
	ISQD(b, 2);

	ALLOCQD(result);
	c_qd_sub_d_qd(a->real, b->hReal[0], result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_sub_d_d, "a,b", "subtracts double a from double b returning a quad-double result",
	 DD result, LPCCOMPLEXSCALAR a, LPCCOMPLEXSCALAR b)
{
	ISD(a, 1);
	ISD(b, 2);
	double tmp[4] = { a->real, 0.0, 0.0, 0.0 };

	ALLOCQD(result);
	c_qd_sub_qd_d(tmp, b->real, result->hReal[0]);
	return 0L;
}
END_FN;

///////////////////////////////Multiplication

MCFN(qd_mul, "a,b", "multiplies quad-double \"a\" by quad-double \"b\"", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISDDQD(a, 1);
	ISDDQD(b, 2);
	//const auto success = copy(result, qd_real{ a->hReal[0] } * qd_real{ b->hReal[0] });
	const auto success = (a->rows == 2
						  ? b->rows == 2
						  ? copy(result, qd_real{ dd_real{ a->hReal[0] } * dd_real{ b->hReal[0] } })
						  : copy(result, dd_real{ a->hReal[0] } * qd_real{ b->hReal[0] })
						  : b->rows == 2
						  ? copy(result, qd_real{ a->hReal[0] } * dd_real{ b->hReal[0] })
						  : copy(result, qd_real{ a->hReal[0] } * qd_real{ b->hReal[0] })
						  );

	return (success) ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_mul_qd_d, "a,b", "multiplies quad-double a by double b",
	 DD result, CDD a, LPCCOMPLEXSCALAR b)
{
	ISQD(a, 1);
	ISD(b, 2);

	ALLOCQD(result);
	c_qd_mul_qd_d(a->hReal[0], b->real, result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_mul_d_qd, "a,b", "multiplies double a by quad-double b",
	 DD result, LPCCOMPLEXSCALAR a, CDD b)
{
	ISD(a, 1);
	ISQD(b, 2);

	ALLOCQD(result);
	c_qd_mul_d_qd(a->real, b->hReal[0], result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_mul_d_d, "a,b", "multiplies double a by double b returning a quad-double result",
	 DD result, LPCCOMPLEXSCALAR a, LPCCOMPLEXSCALAR b)
{
	ISD(a, 1);
	ISD(b, 2);
	double tmp[4] = { b->real, 0.0, 0.0, 0.0 };

	ALLOCQD(result);
	c_qd_mul_d_qd(a->real, tmp, result->hReal[0]);
	return 0L;
}
END_FN;


///////////////////////////////Division

MCFN(qd_div, "a,b", "divides quad-double \"a\" by quad-double \"b\"", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISDDQD(a, 1);
	ISDDQD(b, 2);
	DIVZ(b, 2);
//	const auto success = copy(result, qd_real{ a->hReal[0] } / qd_real{ b->hReal[0] });
	const auto success = (a->rows == 2
						  ? b->rows == 2
						  ? copy(result, qd_real{ dd_real{ a->hReal[0] } / dd_real{ b->hReal[0] } })
						  : copy(result, dd_real{ a->hReal[0] } / qd_real{ b->hReal[0] })
						  : b->rows == 2
						  ? copy(result, qd_real{ a->hReal[0] } / dd_real{ b->hReal[0] })
						  : copy(result, qd_real{ a->hReal[0] } / qd_real{ b->hReal[0] })
						  );

	return (success) ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);

}
END_FN;

MCFN(qd_rem, "a,b", "returns the remainder of dividing quad-double \"a\" by quad-double \"b\"", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	DIVZ(b, 2);
	const auto success = copy(result, drem(qd_real{ a->hReal[0] }, qd_real{ b->hReal[0] }));
	//const auto success = (a->rows == 2
	//					  ? b->rows == 2
	//					  ? copy(result, dd_real{ a->hReal[0] } / dd_real{ b->hReal[0] })
	//					  : copy(result, dd_real{ a->hReal[0] } / qd_real{ b->hReal[0] })
	//					  : b->rows == 2
	//					  ? copy(result, qd_real{ a->hReal[0] } / dd_real{ b->hReal[0] })
	//					  : copy(result, qd_real{ a->hReal[0] } / qd_real{ b->hReal[0] })
	//					  );

	return (success) ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);

}
END_FN;

MCFN(qd_div_qd_d, "a,b", "divides quad-double a by double b",
	 DD result, CDD a, LPCCOMPLEXSCALAR b)
{
	ISQD(a, 1);
	ISD(b, 2);
	DIVZ(b, 2);
	ALLOCQD(result);
	c_qd_div_qd_d(a->hReal[0], b->real, result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_div_d_qd, "a,b", "divides double a by quad-double b",
	 DD result, LPCCOMPLEXSCALAR a, CDD b)
{
	ISD(a, 1);
	ISQD(b, 2);
	DIVZ(b, 2);
	ALLOCQD(result);
	c_qd_div_d_qd(a->real, b->hReal[0], result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_div_d_d, "a,b", "divides double a by double b returning a quad-double result",
	 DD result, LPCCOMPLEXSCALAR a, LPCCOMPLEXSCALAR b)
{
	ISD(a, 1);
	ISD(b, 2);
	DIVZ(b, 2);
	ALLOCQD(result);

	double tmp[4] = { a->real, 0.0, 0.0, 0.0 };

	c_qd_div_qd_d(tmp, b->real, result->hReal[0]);
	return 0L;
}
END_FN;

MCFN(qd_fmod, "a,b", "divides double-doubleor quad=double \"a\" by double-double or quad-double \"b\" and returns the remainder", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	DIVZ(b, 2);
	const auto success = copy(result, fmod(qd_real{ a->hReal[0] }, qd_real{ b->hReal[0] }));
	//const auto success = (a->rows == 2
	//					  ? b->rows == 2
	//					  ? copy(result, fmod(dd_real{ a->hReal[0] } , dd_real{ b->hReal[0] }))
	//					  : copy(result, fmod(dd_real{ a->hReal[0] } , qd_real{ b->hReal[0] }))
	//					  : b->rows == 2
	//					  ? copy(result, fmod(qd_real{ a->hReal[0] } , dd_real{ b->hReal[0] }))
	//					  : copy(result, fmod(qd_real{ a->hReal[0] } , qd_real{ b->hReal[0] }))
	//					  );

	return (success) ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);

}
END_FN;
//
//
//MCFN(dd_const, "c", "looks up constants and returns the double-double value", LPCOMPLEXARRAY result, LPCCOMPLEXSCALAR a)
//{
//	if (fabs(a->real - 3.141592653589793116) < 1e-3)
//	{
//		copy(result, dd_real::_pi);
//	}
//	else if (fabs(a->real - 6.28318530717958620) < 1e-3)
//	{
//		copy(result, dd_real::_2pi);
//	}
//	else if (fabs(a->real - 2.35619449019234480) < 1e-3)
//	{
//		copy(result, dd_real::_3pi4);
//	}
//	else if (fabs(a->real - 1.57079632679489660) < 1e-3)
//	{
//		copy(result, dd_real::_pi2);
//	}
//	else if (fabs(a->real - 0.78539816339744828) < 1e-3)
//	{
//		copy(result, dd_real::_pi4);
//	}
//	else if (fabs(a->real - 2.71828182845904510) < 1e-3)
//	{
//		copy(result, dd_real::_e);
//	}
//	else if (fabs(a->real - 0.69314718055994529) < 1e-3)
//	{
//		copy(result, dd_real::_log2);
//	}
//	else if (fabs(a->real - 2.30258509299404590) < 1e-3)
//	{
//		copy(result, dd_real::_log10);
//	}
//	else
//	{
//		return MAKELRESULT(UNKNOWN_CONST, 0);
//	}
//	return 0L;
//}
//END_FN;

MCFN(qd_const, "c", "looks up constants and returns the quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXSCALAR a)
{
	if (fabs(a->real - 3.141592653589793116) < 1e-3)
	{
		copy(result, qd_real::_pi);
	}
	else if (fabs(a->real - 6.28318530717958620) < 1e-3)
	{
		copy(result, qd_real::_2pi);
	}
	else if (fabs(a->real - 2.35619449019234480) < 1e-3)
	{
		copy(result, qd_real::_3pi4);
	}
	else if (fabs(a->real - 1.57079632679489660) < 1e-3)
	{
		copy(result, qd_real::_pi2);
	}
	else if (fabs(a->real - 0.78539816339744828) < 1e-3)
	{
		copy(result, qd_real::_pi4);
	}
	else if (fabs(a->real - 2.71828182845904510) < 1e-3)
	{
		copy(result, qd_real::_e);
	}
	else if (fabs(a->real - 0.69314718055994529) < 1e-3)
	{
		copy(result, qd_real::_log2);
	}
	else if (fabs(a->real - 2.30258509299404590) < 1e-3)
	{
		copy(result, qd_real::_log10);
	}
	else
	{
		return MAKELRESULT(UNKNOWN_CONST, 0);
	}
	return 0L;
}
END_FN;

MCFN(qd_sin, "a", "computes the sin of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, sin(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_cos, "a", "computes the cos of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, cos(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_tan, "a", "computes the tan of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, tan(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_asin, "a", "computes the arcsin of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, asin(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_acos, "a", "computes the arccos of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, acos(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_atan, "a", "computes the arctan of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, atan(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_atan2, "y,x", "computes arctan of quad-double values y/x and returns the correct quadrant.", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY y, LPCCOMPLEXARRAY x)
{
	ISQD(y, 1);
	ISQD(x, 2);
	copy(result, atan2( makeQD(y),makeQD(x) ));
	return 0L;
}
END_FN;

MCFN(qd_sinh, "a", "computes the sinh of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, sinh(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_cosh, "a", "computes the cosh of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, cosh(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_tanh, "a", "computes the tanh of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, tanh(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_asinh, "a", "computes the arc-sinh of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, asinh(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_acosh, "a", "computes the arc-cosh of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, acosh(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_atanh, "a", "computes the arc-tanh of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, atanh(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_exp, "a", "computes the exponential of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, exp(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_log, "a", "computes the natural log of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, log(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_log10, "a", "computes the base-10 log of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, log10(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_abs, "a", "computes the absolute value of a quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	copy(result, fabs(makeQD(a)));
	return 0L;
}
END_FN;

MCFN(qd_lt, "a,b", "returns a < b of quad-double values", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	auto lThan =  (makeQD(a) < makeQD(b));
	//if (isDD(a))
	//{
	//	if (isDD(b))
	//	{
	//		lThan = (makeDD(a) < makeDD(b));
	//	}
	//	else
	//	{
	//		lThan = (makeDD(a) < makeQD(b));
	//	}
	//}
	//else if(isDD(b))
	//{
	//	lThan = (makeQD(a) < makeDD(b));
	//}
	//else
	//{
	//	lThan = (makeQD(a) < makeQD(b));
	//}
	
	result->real = lThan ? 1.0 : 0.0;
	result->imag = 0.0;

	return 0L;
}
END_FN;

MCFN(qd_lte, "a,b", "returns a <= b of quad-double values", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	auto lThan = (makeQD(a) <= makeQD(b));;
	//if (isDD(a))
	//{
	//	if (isDD(b))
	//	{
	//		lThan = (makeDD(a) <= makeDD(b));
	//	}
	//	else
	//	{
	//		lThan = (makeDD(a) <= makeQD(b));
	//	}
	//}
	//else if (isDD(b))
	//{
	//	lThan = (makeQD(a) <= makeDD(b));
	//}
	//else
	//{
	//	lThan = (makeQD(a) <= makeQD(b));
	//}

	result->real = lThan ? 1.0 : 0.0;
	result->imag = 0.0;

	return 0L;
}
END_FN;


MCFN(qd_gt, "a,b", "returns a > b of quad-double values", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	auto lThan = (makeQD(a) > makeQD(b));

	result->real = lThan ? 1.0 : 0.0;
	result->imag = 0.0;

	return 0L;

	//ISDDQD(a, 1);
	//ISDDQD(b, 2);
	//auto lThan = false;
	//if (isDD(a))
	//{
	//	if (isDD(b))
	//	{
	//		lThan = (makeDD(a) > makeDD(b));
	//	}
	//	else
	//	{
	//		lThan = (makeDD(a) > makeQD(b));
	//	}
	//}
	//else if (isDD(b))
	//{
	//	lThan = (makeQD(a) > makeDD(b));
	//}
	//else
	//{
	//	lThan = (makeQD(a) > makeQD(b));
	//}

	//result->real = lThan ? 1.0 : 0.0;
	//result->imag = 0.0;

	//return 0L;
}
END_FN;

MCFN(qd_gte, "a,b", "returns a >= b of quad-double values", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	auto lThan = (makeQD(a) >= makeQD(b));

	result->real = lThan ? 1.0 : 0.0;
	result->imag = 0.0;

	return 0L;

	//ISDDQD(a, 1);
	//ISDDQD(b, 2);
	//auto lThan = false;
	//if (isDD(a))
	//{
	//	if (isDD(b))
	//	{
	//		lThan = (makeDD(a) >= makeDD(b));
	//	}
	//	else
	//	{
	//		lThan = (makeDD(a) >= makeQD(b));
	//	}
	//}
	//else if (isDD(b))
	//{
	//	lThan = (makeQD(a) >= makeDD(b));
	//}
	//else
	//{
	//	lThan = (makeQD(a) >= makeQD(b));
	//}

	//result->real = lThan ? 1.0 : 0.0;
	//result->imag = 0.0;

	//return 0L;
}
END_FN;

MCFN(qd_eq, "a,b", "returns a == b of quad-double values", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	auto lThan = (makeQD(a) == makeQD(b));

	result->real = lThan ? 1.0 : 0.0;
	result->imag = 0.0;

	return 0L;

	//ISDDQD(a, 1);
	//ISDDQD(b, 2);
	//auto lThan = false;
	//if (isDD(a))
	//{
	//	if (isDD(b))
	//	{
	//		lThan = (makeDD(a) == makeDD(b));
	//	}
	//	else
	//	{
	//		lThan = (makeDD(a) == makeQD(b));
	//	}
	//}
	//else if (isDD(b))
	//{
	//	lThan = (makeQD(a) == makeDD(b));
	//}
	//else
	//{
	//	lThan = (makeQD(a) == makeQD(b));
	//}

	//result->real = lThan ? 1.0 : 0.0;
	//result->imag = 0.0;

	//return 0L;
}
END_FN;

MCFN(qd_neq, "a,b", "returns a != b of quad-double values", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a, LPCCOMPLEXARRAY b)
{
	ISQD(a, 1);
	ISQD(b, 2);
	auto lThan = (makeQD(a) != makeQD(b));

	result->real = lThan ? 1.0 : 0.0;
	result->imag = 0.0;

	return 0L;

	//ISDDQD(a, 1);
	//ISDDQD(b, 2);
	//auto lThan = false;
	//if (isDD(a))
	//{
	//	if (isDD(b))
	//	{
	//		lThan = (makeDD(a) != makeDD(b));
	//	}
	//	else
	//	{
	//		lThan = (makeDD(a) != makeQD(b));
	//	}
	//}
	//else if (isDD(b))
	//{
	//	lThan = (makeQD(a) != makeDD(b));
	//}
	//else
	//{
	//	lThan = (makeQD(a) != makeQD(b));
	//}

	//result->real = lThan ? 1.0 : 0.0;
	//result->imag = 0.0;

	//return 0L;
}
END_FN;

MCFN(qdrand, "unused", "returns a random quad-double value", LPCOMPLEXARRAY result, LPCCOMPLEXSCALAR)
{
	copy(result, qdrand());
	return 0L;
}
END_FN;

MCFN(qd_nint, "a", "rounds quad-double value to nearest integer", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	const auto success = copy(result, nint(makeQD(a)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_aint, "a", "truncates quad-double value towards 0", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	const auto success = copy(result, aint(makeQD(a)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_floor, "a", "computes the floor of quad-double value \"a\".", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	const auto success = copy(result, floor(makeQD(a)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_ceil, "a", "computes the floor of quad-double value \"a\".", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	const auto success = copy(result, ceil(makeQD(a)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_ldexp, "x,exp", "Multiplies a quad-double point value x by the number 2 raised to the exp power \"a\".", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXSCALAR exp)
{
	ISQD(a, 1);
	ISINT(exp, 2);
	const auto success = copy(result, ldexp(makeQD(a), static_cast<int>(exp->real)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_pow, "x,y", "Raises quad-double value x to quad-double power y", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY x, LPCCOMPLEXARRAY y)
{
	ISQD(x, 1);
	ISQD(y, 2);


	const auto xx = makeQD(x);
	const auto yy = makeQD(y);
	auto success = false;
	if (xx.is_zero())
	{
		if (yy.is_zero())
		{
			success = copy(result, qd_real{ 1.0 });
		}
		else
		{
			success = copy(result, qd_real{ 0.0 });
		}
	}
	else
	{
		success = copy(result, pow(xx, yy));
	}

	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_npwr, "x,exp", "Raises quad-double value x to integer power y", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXSCALAR exp)
{
	ISQD(a, 1);
	ISINT(exp, 2);
	const auto success = copy(result, npwr(makeQD(a), static_cast<int>(exp->real)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_nroot, "x,n", "Computesthe n'th root of quad-double value x", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a, LPCCOMPLEXSCALAR n)
{
	ISQD(a, 1);

	if (!isInt(n) || n->real < 1.0)
		return MAKELRESULT(MUST_BE_POS_INT, 2);

	const auto success = copy(result, nroot(makeQD(a), static_cast<int>(n->real)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_sqrt, "a", "computes the square root of quad-double value \"a\".", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	const auto success = copy(result, sqrt(makeQD(a)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_sqr, "a", "efficient square of quad-double value \"a\".", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	const auto success = copy(result, sqr(makeQD(a)));
	return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
}
END_FN;

MCFN(qd_isnan, "a", "tests if quad-double value \"a\" is a NaN.", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	result->imag = 0.0;
	result->real = makeQD(a).isnan() ? 1.0 : 0.0;
	return 0L;
}
END_FN;

MCFN(qd_isinf, "a", "tests if quad-double value \"a\" is an infinity.", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	result->imag = 0.0;
	result->real = makeQD(a).isinf() ? 1.0 : 0.0;
	return 0L;
}
END_FN;

MCFN(qd_isfinite, "a", "tests if quad-double value \"a\" is finite.", LPCOMPLEXSCALAR result, LPCCOMPLEXARRAY a)
{
	ISQD(a, 1);
	result->imag = 0.0;
	result->real = makeQD(a).isfinite() ? 1.0 : 0.0;
	return 0L;
}
END_FN;

MCFN(qd_polyeval, "c,x", "Evaluates the given n-th degree polynomial at \"x\". The quad-double coreeifients are storeed in a 2 row by n column matrix", LPCOMPLEXARRAY result, LPCCOMPLEXARRAY c, LPCCOMPLEXARRAY x)
{
	if (c->hImag != nullptr)
		return MAKELRESULT(MUST_BE_REAL, 1);
	if (c->rows != 4)
		return MAKELRESULT(NOT_A_QUAD_DOUBLE_LIST, 1);

	ISQD(x, 2);

	try
	{
		std::vector<qd_real> vec{};
		vec.reserve(c->rows);
		for (auto i = 0U; i < c->cols; ++i)
		{
			vec.emplace_back(c->hReal[i]);
		}

		const auto success = copy(result, polyeval(vec.data(), static_cast<int>(vec.size() - 1), makeQD(x)));

		return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
	}
	catch (const std::bad_alloc &)
	{
		return MAKELRESULT(OUT_OF_MEMORY, 0);
	}
}
END_FN;

MCFN(qd_polyroot, "c,x0,max_iter,eps", "Given an n-th degree quad-double polynomial, finds a root close to the given guess \
x0. Note that this uses simple Newton iteration scheme, and does not work for multiple roots. Will run for maximum of iter iterations. \
eps is the error thereshold. Pass eps=0.0 to use build-in eps.",
LPCOMPLEXARRAY result, LPCCOMPLEXARRAY c, LPCCOMPLEXARRAY x, LPCCOMPLEXSCALAR iter, LPCCOMPLEXSCALAR eps)
{
	if (c->hImag != nullptr)
		return MAKELRESULT(MUST_BE_REAL, 1);
	if (c->rows != 4)
		return MAKELRESULT(NOT_A_QUAD_DOUBLE_LIST, 1);

	ISINT(iter, 3);
	ISD(eps, 4);
	ISQD(x, 2);

	try
	{
		std::vector<qd_real> vec{};
		vec.reserve(c->rows);
		for (auto i = 0U; i < c->cols; ++i)
		{
			vec.emplace_back(c->hReal[i]);
		}

		const auto success = copy(result, polyroot(vec.data(), static_cast<int>(vec.size() - 1), makeQD(x), static_cast<int>(iter->real), eps->real));

		return success ? 0L : MAKELRESULT(OUT_OF_MEMORY, 0);
	}
	catch (const std::bad_alloc &)
	{
		return MAKELRESULT(OUT_OF_MEMORY, 0);
	}
}
END_FN;
