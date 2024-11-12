#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "main.cpp"

using namespace testing;


FunctionFactory funcFactory;
// --------------------------------------------------------
// проверка создания, перевода в строку и вычисления базовой функции в точке
// --------------------------------------------------------
class BasicFunction: public Test{};
TEST_F(BasicFunction, IdentityFunction) {
    SCOPED_TRACE("Testing identity function");
    auto ident = funcFactory.Create("ident");
    EXPECT_EQ(ident->ToString(), "x");
    EXPECT_DOUBLE_EQ((*ident)(5), 5);
}

TEST_F(BasicFunction, ConstantFunction) {
    SCOPED_TRACE("Testing constant function");
    auto constant = funcFactory.Create("const", 10);
    EXPECT_EQ(constant->ToString(), "10");
    EXPECT_DOUBLE_EQ((*constant)(5), 10);
}

TEST_F(BasicFunction, PowerFunction) {
    SCOPED_TRACE("Testing power function");
    auto power = funcFactory.Create("power", 3);
    EXPECT_EQ(power->ToString(), "x^3");
    EXPECT_DOUBLE_EQ((*power)(2), 8);
}

TEST_F(BasicFunction, ExpFunction) {
    SCOPED_TRACE("Testing exp function");
    auto expFunc = funcFactory.Create("exp");
    EXPECT_EQ(expFunc->ToString(), "exp(x)");
    EXPECT_DOUBLE_EQ((*expFunc)(1), std::exp(1));
}

TEST_F(BasicFunction, PolynomialFunction) {
    SCOPED_TRACE("Testing polynomial function");
    auto poly = funcFactory.Create("polynomial", {3, -1, 0, 2});
    EXPECT_EQ(poly->ToString(), "3-1*x^1 + 2*x^3");
    EXPECT_DOUBLE_EQ((*poly)(2), 17);
}
// тест неправильного создания
TEST_F(BasicFunction, InvalidFunction) {
    EXPECT_THROW(funcFactory.Create("abc"), std::logic_error);
}
// --------------------------------------------------------
// тест вычисления производной простой функции
//// --------------------------------------------------------
class Dervative_Basic: public Test{};
TEST_F(Dervative_Basic, Ident) {
    auto ident = funcFactory.Create("ident");
    EXPECT_DOUBLE_EQ(ident->GetDerivative(1), 1);
}

TEST_F(Dervative_Basic, Const) {
    auto constant = funcFactory.Create("const", 10);
    EXPECT_DOUBLE_EQ(constant->GetDerivative(5), 0);
}

TEST_F(Dervative_Basic, Power) {
    auto power = funcFactory.Create("power", 3);
    EXPECT_DOUBLE_EQ(power->GetDerivative(2), 12);
}

TEST_F(Dervative_Basic, Exponential) {
    auto poly = funcFactory.Create("exp");
    EXPECT_DOUBLE_EQ(poly->GetDerivative(2), 7.38905609893065);
}


TEST_F(Dervative_Basic, Poly) {
    auto poly = funcFactory.Create("polynomial", {5, 0, 3});
    EXPECT_DOUBLE_EQ(poly->GetDerivative(2), 12);
}
// --------------------------------------------------------
// проверка операций
// --------------------------------------------------------
class Operation_Test: public Test{};
class Add_Test: public Operation_Test{};
class Minus_Test: public Operation_Test{};
class Multiply_Test: public Operation_Test{};
class Divide_Test: public Operation_Test{};
class Combined_Test: public Operation_Test{};
// --------------------------------------------------------
// проверка сложения
TEST_F(Add_Test, Ident_Const){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto comb = ident + con;
	EXPECT_EQ(comb->ToString(), "x + 10");
	EXPECT_DOUBLE_EQ(comb->Evaluate(20), 30);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), 1);
}

TEST_F(Add_Test, Power_Exp_Poly){
	auto power = funcFactory.Create("ident");
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = power + exp + poly;
	EXPECT_EQ(comb->ToString(), "x + exp(x) + 5 + 3*x^2"); // 3 + 5 + 27 + 20.085536923187668
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 55.085536923187668);
	EXPECT_DOUBLE_EQ(comb->Derivative(3), 39.085536923187668);

}

TEST_F(Add_Test, Add_ALL){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto power = funcFactory.Create("power", 4);
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = ident + con + power + exp + poly;
	EXPECT_EQ(comb->ToString(), "x + 10 + x^4 + exp(x) + 5 + 3*x^2"); // то что сверху но + 81 + 10
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 146.085536923187668);
	EXPECT_DOUBLE_EQ(comb->Derivative(3), 147.08553692318768);


}

// --------------------------------------------------------
// Вычитание

TEST_F(Minus_Test, Ident_Const){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto comb = ident - con;
	EXPECT_EQ(comb->ToString(), "x - (10)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(30), 20);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), 1);


}

TEST_F(Minus_Test, Power_Exp_Poly){
	auto power = funcFactory.Create("ident");
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = power - exp - poly;
	EXPECT_EQ(comb->ToString(), "x - (exp(x)) - (5 + 3*x^2)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), -49.085536923187668);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), -1.7392749415205012e+18);

}

TEST_F(Minus_Test, Minus_ALL){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto power = funcFactory.Create("power", 4);
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = ident - con - power - exp - poly;
	EXPECT_EQ(comb->ToString(), "x - (10) - (x^4) - (exp(x)) - (5 + 3*x^2)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), -140.08553692318768);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), -1.7392749415207977e+18);

}

// --------------------------------------------------------
// Умножение

TEST_F(Multiply_Test, Ident_Const){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto comb = ident * con;
	EXPECT_EQ(comb->ToString(), "(x) * (10)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 30);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), 10);

}

TEST_F(Multiply_Test, Power_Exp_Poly){
	auto power = funcFactory.Create("ident");
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = power * exp * poly;
	EXPECT_EQ(comb->ToString(), "((x) * (exp(x))) * (5 + 3*x^2)"); // 3 * 20.085536923187668 * (5 + 27)

	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 1928.2115446260161);
	EXPECT_DOUBLE_EQ(comb->Derivative(3), 3655.5677200201553);

}

TEST_F(Multiply_Test, Multiply_ALL){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto power = funcFactory.Create("power", 4);
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = ident * con * power * exp * poly;
	EXPECT_EQ(comb->ToString(), "((((x) * (10)) * (x^4)) * (exp(x))) * (5 + 3*x^2)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 1561851.3511470731);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), 1.4046713410322477e+31);

}

// --------------------------------------------------------
// Деление

TEST_F(Divide_Test, Ident_Const){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto comb = ident / con;
	EXPECT_EQ(comb->ToString(), "(x) / (10)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(20), 2);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), 0.1);

}

TEST_F(Divide_Test, Power_Exp_Poly){
	auto power = funcFactory.Create("ident");
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = power / exp / poly;
	EXPECT_EQ(comb->ToString(), "((x) / (exp(x))) / (5 + 3*x^2)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 0.0046675376594872446);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), -4.6671437240978767e-21);

}

TEST_F(Divide_Test, Divide_ALL){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto power = funcFactory.Create("power", 4);
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = ident / con / power / exp / poly;
	EXPECT_EQ(comb->ToString(), "((((x) / (10)) / (x^4)) / (exp(x))) / (5 + 3*x^2)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 5.7623921722064745e-06);
	EXPECT_DOUBLE_EQ(comb->Derivative(42), -1.6394004075078683e-28);

}

// --------------------------------------------------------
// Комбинированные

TEST_F(Combined_Test, Power_Exp_Poly){
	auto power = funcFactory.Create("ident");
	auto exp = funcFactory.Create("const", 10);
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = power * exp + poly;
	EXPECT_EQ(comb->ToString(), "(x) * (10) + 5 + 3*x^2"); 
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 62);
	EXPECT_DOUBLE_EQ(comb->Derivative(4), 34); // 6x + 10 

}

TEST_F(Combined_Test, Combined_ALL){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto power = funcFactory.Create("power", 4);
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = ident * con + power / exp - poly;
	EXPECT_EQ(comb->ToString(), "(x) * (10) + (x^4) / (exp(x)) - (5 + 3*x^2)");
	EXPECT_DOUBLE_EQ(comb->Evaluate(3), 2.0327525377969806);
	// -(x^4-4*x^3)*e^(-x)-6x+10
	EXPECT_DOUBLE_EQ(comb->Derivative(4), -14);

}
// --------------------------------------------------------
//	Проверка решения уравнений вида f(x) = 0 градиентным спуском
// --------------------------------------------------------
class Test_Grad: public Test{};

TEST_F(Test_Grad, Easy){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto comb = ident - con; // x - 10
	EXPECT_NEAR(FindRootByGradientDescent(comb, 5), 10, 1e-6);
	auto power = funcFactory.Create("power", 2);
	auto comb2 = power + comb;	// x^2 + x - 10
	EXPECT_NEAR(FindRootByGradientDescent(comb2, 3),   2.7015621187164243, 1e-6);
	EXPECT_NEAR(FindRootByGradientDescent(comb2, -3), -3.7015621187164243, 1e-6);


}


TEST_F(Test_Grad, Hard){
	auto ident = funcFactory.Create("ident");
	auto con = funcFactory.Create("const", 10);
	auto power = funcFactory.Create("power", 4);
	auto exp = funcFactory.Create("exp");
	auto poly = funcFactory.Create("polynomial", {5, 0, 3});
	auto comb = ident * con + power / exp - poly;
	//	:)WALFRAM ALPHA(: 
	//
	//	https://www.wolframalpha.com/input?i=%28x%29+*+%2810%29+%2B+%28x%5E4%29+%2F+%28exp%28x%29%29+-+%285+%2B+3*x%5E2%29%3D0
	//
	//	-1.5564882718739824718
	//	0.60130498112524060823
	//  3.2661651023049628374
	EXPECT_NEAR(FindRootByGradientDescent(comb, 0.5), 0.6013049811252406, 1e-6);
	EXPECT_NEAR(FindRootByGradientDescent(comb, -2), -1.5564882718739824, 1e-6);
	EXPECT_NEAR(FindRootByGradientDescent(comb, 4),   3.2661651023049628, 1e-6);

}





int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS(); 
}
