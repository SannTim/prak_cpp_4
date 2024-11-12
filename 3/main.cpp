#include <iostream>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

enum FunctionType {
    IDENT,
    CONST,
    POWER,
    EXP,
    POLYNOMIAL,
    UNKNOWN
};

class TFunction;
using TFunctionPtr = std::shared_ptr<TFunction>;

std::string doubleToString(double num) {
    if (std::floor(num) == num) {
        return std::to_string(static_cast<int>(num));
	}
    std::ostringstream stream;
    stream << num;
    return stream.str();
}

class TFunction {
public:
    virtual double Evaluate(double x) const = 0;
    virtual double Derivative(double x) const = 0;
    virtual std::string ToString() const = 0;
    virtual ~TFunction() = default;
    
    double operator()(double x) const { return Evaluate(x); }
    double GetDerivative(double x) const { return Derivative(x); }
    
    static TFunctionPtr Create(const std::string& type, double param = 0.0, const std::vector<double>& coeffs = {});
};

class IdentityFunc : public TFunction {
public:
    double Evaluate(double x) const override { return x; }
    double Derivative(double x) const override { return 1.0; }
    std::string ToString() const override { return "x"; }
};

class ConstFunc : public TFunction {
public:
    explicit ConstFunc(double value) : value_(value) {}
    double Evaluate(double x) const override { return value_; }
    double Derivative(double x) const override { return 0.0; }
    std::string ToString() const override { return doubleToString(value_); }

private:
    double value_;
};


// функция возведения в степень
class PowerFunc : public TFunction {
public:
    explicit PowerFunc(double exponent) : exponent_(exponent) {}
    double Evaluate(double x) const override { return std::pow(x, exponent_); }
    double Derivative(double x) const override { return exponent_ * std::pow(x, exponent_ - 1); }
    std::string ToString() const override {return "x^" + doubleToString(exponent_);}

private:
    double exponent_;
};


class ExpFunc : public TFunction {
public:
    double Evaluate(double x) const override { return std::exp(x); }
    double Derivative(double x) const override { return std::exp(x); }
    std::string ToString() const override { return "exp(x)"; }
};

class PolynomialFunc : public TFunction {
public:
    explicit PolynomialFunc(const std::vector<double>& coefficients) : coefficients_(coefficients) {}

    double Evaluate(double x) const override {
        double result = 0.0;
        double x_pow = 1.0;
        for (double coef : coefficients_) {
            result += coef * x_pow;
            x_pow *= x;
        }
        return result;
    }

    double Derivative(double x) const override {
        double result = 0.0;
        double x_pow = 1.0;
        for (size_t i = 1; i < coefficients_.size(); ++i) {
            result += i * coefficients_[i] * x_pow;
            x_pow *= x;
        }
        return result;
    }

    std::string ToString() const override {
        bool all_zero = true;
        std::ostringstream oss;

        for (size_t i = 0; i < coefficients_.size(); ++i) {
            if (coefficients_[i] != 0) {
                if (i > 0 && !all_zero && coefficients_[i] > 0) oss << " + ";
                    oss << coefficients_[i];
                if (i > 0) oss << "*x^" << doubleToString(i);
				all_zero = false;
            }
        }

        return all_zero ? "0" : oss.str();
    }

private:
    std::vector<double> coefficients_;
};


class AddFunc : public TFunction {
public:
    AddFunc(TFunctionPtr lhs, TFunctionPtr rhs) : lhs_(lhs), rhs_(rhs) {}

    double Evaluate(double x) const override {
        return lhs_->Evaluate(x) + rhs_->Evaluate(x);
    }

    double Derivative(double x) const override {
        return lhs_->Derivative(x) + rhs_->Derivative(x);
    }

    std::string ToString() const override {
        return lhs_->ToString() + " + " + rhs_->ToString();
    }

private:
    TFunctionPtr lhs_, rhs_;
};

class SubFunc : public TFunction {
public:
    SubFunc(TFunctionPtr lhs, TFunctionPtr rhs) : lhs_(lhs), rhs_(rhs) {}

    double Evaluate(double x) const override {
        return lhs_->Evaluate(x) - rhs_->Evaluate(x);
    }

    double Derivative(double x) const override {
        return lhs_->Derivative(x) - rhs_->Derivative(x);
    }

    std::string ToString() const override {
        return lhs_->ToString() + " - (" + rhs_->ToString() + ")";
    }

private:
    TFunctionPtr lhs_, rhs_;
};

class MulFunc : public TFunction {
public:
    MulFunc(TFunctionPtr lhs, TFunctionPtr rhs) : lhs_(lhs), rhs_(rhs) {}

    double Evaluate(double x) const override {
        return lhs_->Evaluate(x) * rhs_->Evaluate(x);
    }

    double Derivative(double x) const override {
        return lhs_->Evaluate(x) * rhs_->Derivative(x) + lhs_->Derivative(x) * rhs_->Evaluate(x);
    }

    std::string ToString() const override {
        return "(" + lhs_->ToString() + ") * (" + rhs_->ToString() + ")";
    }

private:
    TFunctionPtr lhs_, rhs_;
};

class DivFunc : public TFunction {
public:
	DivFunc(TFunctionPtr lhs, TFunctionPtr rhs) : lhs_(lhs), rhs_(rhs) {}

    double Evaluate(double x) const override {
		double rhsval = rhs_->Evaluate(x);
		if (rhsval == 0) throw std::invalid_argument("Division by zero");
        return lhs_->Evaluate(x) / rhs_->Evaluate(x);
    }

    double Derivative(double x) const override {
		double divis = rhs_->Evaluate(x)*rhs_->Evaluate(x);
		if (divis == 0) throw std::invalid_argument("Division by zero");
        return (lhs_->Derivative(x) * rhs_->Evaluate(x) - lhs_->Evaluate(x) * rhs_->Derivative(x)) / (divis);
    }

    std::string ToString() const override {
        return "(" + lhs_->ToString() + ") / (" + rhs_->ToString() + ")";
    }

private:
    TFunctionPtr lhs_, rhs_;
};


TFunctionPtr operator+(TFunctionPtr lhs, TFunctionPtr rhs) {
    return std::make_shared<AddFunc>(lhs, rhs);
}

TFunctionPtr operator-(TFunctionPtr lhs, TFunctionPtr rhs) {
    return std::make_shared<SubFunc>(lhs, rhs);
}

TFunctionPtr operator*(TFunctionPtr lhs, TFunctionPtr rhs) {
    return std::make_shared<MulFunc>(lhs, rhs);
}

TFunctionPtr operator/(TFunctionPtr lhs, TFunctionPtr rhs) {
    return std::make_shared<DivFunc>(lhs, rhs);
}


FunctionType TypeFromString(const std::string& type) {
    static const std::unordered_map<std::string, FunctionType> type_map = {
        {"ident", IDENT},
        {"const", CONST},
        {"power", POWER},
        {"exp", EXP},
        {"polynomial", POLYNOMIAL}
    };

    auto it = type_map.find(type);
    return (it != type_map.end()) ? it->second : UNKNOWN;
}

TFunctionPtr TFunction::Create(const std::string& type, double param, const std::vector<double>& coeffs) {
    switch (TypeFromString(type)) {
        case IDENT:
            return std::make_shared<IdentityFunc>();
        case CONST:
            return std::make_shared<ConstFunc>(param);
        case POWER:
            return std::make_shared<PowerFunc>(param);
        case EXP:
            return std::make_shared<ExpFunc>();
        case POLYNOMIAL:
            return std::make_shared<PolynomialFunc>(coeffs);
        default:
            throw std::logic_error("Unknown function type");
    }
}

class FunctionFactory {
public:
    std::shared_ptr<TFunction> Create(const std::string& type, const std::vector<double>& coeffs) const {
        return TFunction::Create(type, 0, coeffs);
	}
    std::shared_ptr<TFunction> Create(const std::string& type, double param = 0) const {
        return TFunction::Create(type, param);
    }
};

double FindRootByGradientDescent(
    TFunctionPtr func,
    double initial_guess = 0,
    double learning_rate = 0.01,
    int max_iterations = 10000,
	double tolerance = 1e-6){

    double x = initial_guess;
    for (int i = 0; i < max_iterations; ++i) {
        double value = func->Evaluate(x);
        double gradient = func->Derivative(x);
        if (std::abs(value) < tolerance) {
            return x;
        }
        if (std::abs(gradient) < 1e-10) {
            break; // остановка для слишком малого градиента
        }
        x = x - learning_rate * value / gradient;
    }
    return x;
}
